//---------------------------------------------------------------------------//
//
// TTBasePluginAdapter.cpp
//  TTBase プラグインを プロセスを超えて使用するためのアダプタ
//   Copyright (C) 2014-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <array>

#include "include/GenerateUUIDString.hpp"
#include "include/Transcode.hpp"
#include "../BridgeData.hpp"
#include "Utility.hpp"

#include "TTBasePluginAdapter.hpp"

//---------------------------------------------------------------------------//
// グローバル変数
//---------------------------------------------------------------------------//

using namespace tapetums;

// 本体との通信用オブジェクト (WinMain.cpp で定義)
extern File   shrmem;
//extern HANDLE lock_downward;
extern HANDLE downward_input_done;
extern HANDLE downward_output_done;

// プラグインのハンドル (WinMain.cpp で定義)
extern uint64_t g_hPlugin64;

//---------------------------------------------------------------------------//
// メソッド
//---------------------------------------------------------------------------//

// イベントハンドラ
void TTBasePluginAdapter::operator ()()
{
    std::array<wchar_t, BridgeData::namelen> uuid;

    shrmem.Seek(0);
    shrmem.Read(uuid.data(), sizeof(wchar_t) * uuid.size());
    //::OutputDebugStringW(L"Received:\n  ");
    //::OutputDebugStringW(uuid.data());
    //::OutputDebugStringW(L"\n");

    File plugin_data;
    if ( ! plugin_data.Open(uuid.data(), File::ACCESS::WRITE) )
    {
        ::OutputDebugStringW(L"  データの受け取りに失敗\n");
        return;
    }

    PluginMsg msg;
    plugin_data.Read(&msg, sizeof(msg));
    //::OutputDebugStringW(L"Message:\n  ");
    //::OutputDebugStringW(PluginMsgTxt[(uint8_t)msg]);
    //::OutputDebugStringW(L"\n");

    bool result { true };
    switch ( msg )
    {
        case PluginMsg::Load:     result = load(plugin_data);    break;
        case PluginMsg::Free:     free();                        break;
        case PluginMsg::InitInfo: init_plugin_info(plugin_data); break;
        case PluginMsg::FreeInfo: free_plugin_info();            break;
        case PluginMsg::Init:     result = init(plugin_data);    break;
        case PluginMsg::Unload:   unload();                      break;
        case PluginMsg::Execute:  result = execute(plugin_data); break;
        case PluginMsg::Hook:     hook(plugin_data);             break;
        default: break;
    }

    msg = result ? PluginMsg::OK : PluginMsg::NG;
    plugin_data.Seek(0);
    plugin_data.Write(msg);

    //::OutputDebugStringW(result ? L"  OK\n" : L"  NG\n");
    ::SetEvent(downward_input_done);
}

//---------------------------------------------------------------------------//

// プラグインの読み込み
bool TTBasePluginAdapter::load(File& plugin_data)
{
    //::OutputDebugStringW(L"load()\n");

    if ( handle )
    {
        ::OutputDebugStringW(L"  読込済み\n");
        return true;
    }

    std::array<wchar_t, MAX_PATH> filename;
    plugin_data.Read(filename.data(), sizeof(wchar_t) * filename.size());
    //::OutputDebugStringW(L"  ");
    //::OutputDebugStringW(filename.data());
    //::OutputDebugStringW(L"\n");

    handle = ::LoadLibraryExW
    (
        filename.data(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH
    );
    if ( nullptr == handle )
    {
        ::OutputDebugStringW(L" 読込失敗 \n");
        return false;
    }

    // DLLのフルパスを取得
    ::GetModuleFileNameW(handle, path, MAX_PATH);

    // 関数ポインタの取得
    TTBEvent_InitPluginInfo = (TTBEVENT_INITPLUGININFO)::GetProcAddress(handle, "TTBEvent_InitPluginInfo");
    TTBEvent_FreePluginInfo = (TTBEVENT_FREEPLUGININFO)::GetProcAddress(handle, "TTBEvent_FreePluginInfo");
    TTBEvent_Init           = (TTBEVENT_INIT)          ::GetProcAddress(handle, "TTBEvent_Init");
    TTBEvent_Unload         = (TTBEVENT_UNLOAD)        ::GetProcAddress(handle, "TTBEvent_Unload");
    TTBEvent_Execute        = (TTBEVENT_EXECUTE)       ::GetProcAddress(handle, "TTBEvent_Execute");
    TTBEvent_WindowsHook    = (TTBEVENT_WINDOWSHOOK)   ::GetProcAddress(handle, "TTBEvent_WindowsHook");

    // 必須APIを実装しているか
    if ( nullptr == TTBEvent_InitPluginInfo || nullptr == TTBEvent_FreePluginInfo )
    {
        //::OutputDebugStringW(L"  有効なプラグインではありません\n");
        free();
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------//

// プラグインの解放
void TTBasePluginAdapter::free()
{
    unload();

    //::OutputDebugStringW(L"free()\n");
    //::OutputDebugStringW(L"  ");
    //::OutputDebugStringW(path);
    //::OutputDebugStringW(L"\n");

    TTBEvent_InitPluginInfo = nullptr;
    TTBEvent_FreePluginInfo = nullptr;
    TTBEvent_Init           = nullptr;
    TTBEvent_Unload         = nullptr;
    TTBEvent_Execute        = nullptr;
    TTBEvent_WindowsHook    = nullptr;

    if ( nullptr == handle )
    {
        //::OutputDebugStringW(L"  解放済み\n");
        return;
    }

    ::FreeLibrary(handle);
    handle = nullptr;
}

//---------------------------------------------------------------------------//

// プラグイン情報の初期化
void TTBasePluginAdapter::init_plugin_info(File& plugin_data)
{
    if ( nullptr == TTBEvent_InitPluginInfo ) { return; }
    if ( nullptr == TTBEvent_FreePluginInfo ) { return; }

    //::OutputDebugStringW(L"init_plugin_info()\n");

    std::array<wchar_t, MAX_PATH> filename;
    plugin_data.Read(filename.data(), sizeof(wchar_t) * filename.size());

    std::array<char, MAX_PATH> PluginFilename;
    toMBCS(filename.data(), PluginFilename.data(), PluginFilename.size());
    ::OutputDebugStringA("  ");
    ::OutputDebugStringA(PluginFilename.data());
    ::OutputDebugStringA("\n");

    // データの準備
    auto tmp = TTBEvent_InitPluginInfo(PluginFilename.data());
    const auto info = MarshallPluginInfo(tmp);
    TTBEvent_FreePluginInfo(tmp);

    const auto serialized = SerializePluginInfo(info);
    FreePluginInfo(info);

    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), uuid.size());

    File info_data;
    info_data.Map(sizeof(size_t) + serialized.size(), uuid.data(), File::ACCESS::WRITE);
    info_data.Write(serialized.size()); /// sizeof(size_t) == sizeof(uint32_t)
    info_data.Write(serialized.data(), serialized.size());

    // info_data の ファイル名を書き出し
    shrmem.Seek(0);
    shrmem.Write(uuid.data(), sizeof(wchar_t) * uuid.size());

    // 受信完了を通知
    ::ResetEvent(downward_output_done);
    ::SetEvent(downward_input_done);

    // 送信完了待ち
    ::WaitForSingleObject(downward_output_done, 100);
}

//---------------------------------------------------------------------------//

// プラグイン情報の解放
void TTBasePluginAdapter::free_plugin_info()
{
    //::OutputDebugStringW(L"free_plugin_info()\n");
}

//---------------------------------------------------------------------------//

// プラグインの初期化
bool TTBasePluginAdapter::init(File& plugin_data)
{
    //::OutputDebugStringW(L"init()\n");

    if ( nullptr == TTBEvent_Init ) { return true; }

    std::array<wchar_t, MAX_PATH> filename;
    plugin_data.Read(filename.data(), sizeof(wchar_t) * filename.size());

    plugin_data.Read(&g_hPlugin64);

    std::array<char, MAX_PATH> PluginFilename;
    toMBCS(filename.data(), PluginFilename.data(), PluginFilename.size());
    ::OutputDebugStringA("  ");
    ::OutputDebugStringA(PluginFilename.data());
    ::OutputDebugStringA("\n");

    const auto result = TTBEvent_Init(PluginFilename.data(), DWORD_PTR(g_hPlugin64))
                        ? true : false;

    return result;
}

//---------------------------------------------------------------------------//

// プラグインの解放
void TTBasePluginAdapter::unload()
{
    //::OutputDebugStringW(L"unload()\n");

    if ( nullptr == TTBEvent_Unload ) { return; }

    TTBEvent_Unload();
}

//---------------------------------------------------------------------------//

// コマンドの実行
bool TTBasePluginAdapter::execute(File& plugin_data)
{
    //::OutputDebugStringW(L"execute()\n");

    if ( nullptr == TTBEvent_Execute ) { return true; }

    INT32 CmdID;
    plugin_data.Read(&CmdID);

    uint64_t hwnd;
    plugin_data.Read(&hwnd);

    //wchar_t buf[16];
    //::StringCchPrintfW(buf, 16, L"%i", CmdID);
    //::OutputDebugStringW(L"  CmdID: ");
    //::OutputDebugStringW(buf);
    //::OutputDebugStringW(L"\n");

    const auto result = TTBEvent_Execute(CmdID, HWND(hwnd)) ? true : false;

    return result;
}

//---------------------------------------------------------------------------//

// フックの開始
void TTBasePluginAdapter::hook(File& plugin_data)
{
    //::OutputDebugStringW(L"hook()\n");

    if ( nullptr == TTBEvent_WindowsHook ) { return; }

    std::array<wchar_t, MAX_PATH> filename;
    plugin_data.Read(filename.data(), sizeof(wchar_t) * filename.size());

    UINT Msg;
    plugin_data.Read(&Msg);

    uint64_t wp;
    plugin_data.Read(&wp);

    int64_t lp;
    plugin_data.Read(&lp);

    TTBEvent_WindowsHook(Msg, WPARAM(wp), LPARAM(lp));
}

//---------------------------------------------------------------------------//

// TTBasePluginAdapter.cpp