//---------------------------------------------------------------------------//
//
// WinMain.cpp
//  アプリケーション エントリポイント
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <tchar.h>

#include "include/File.hpp"
#include "../BridgeData.hpp"
#include "TTBasePluginAdapter.hpp"

//---------------------------------------------------------------------------//

// メモリリーク検出
#if defined(_DEBUG) || defined(DEBUG)
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
  #define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

//---------------------------------------------------------------------------//
// グローバル変数
//---------------------------------------------------------------------------//

using namespace tapetums;

// 本体との通信用オブジェクト
File   shrmem;
HANDLE lock_downward;
HANDLE lock_upward;
HANDLE downward_input_done;
HANDLE downward_output_done;
HANDLE upward_input_done;
HANDLE upward_output_done;

// 本体との通信用ウィンドウメッセージ
UINT MSG_TTBBRIDGE_COMMAND             { 0 };
UINT MSG_TTBPLUGIN_GETPLUGININFO       { 0 };
UINT MSG_TTBPLUGIN_SETPLUGININFO       { 0 };
UINT MSG_TTBPLUGIN_FREEPLUGININFO      { 0 };
UINT MSG_TTBPLUGIN_SETMENUPROPERTY     { 0 };
UINT MSG_TTBPLUGIN_GETALLPLUGININFO    { 0 };
UINT MSG_TTBPLUGIN_FREEPLUGININFOARRAY { 0 };
UINT MSG_TTBPLUGIN_SETTASKTRAYICON     { 0 };
UINT MSG_TTBPLUGIN_WRITELOG            { 0 };
UINT MSG_TTBPLUGIN_EXECUTECOMMAND      { 0 };

// プラグインのハンドル
uint64_t g_hPlugin64 { 0 };

//---------------------------------------------------------------------------//
// アプリケーション エントリポイント
//---------------------------------------------------------------------------//

INT32 APIENTRY _tWinMain
(
    HINSTANCE, HINSTANCE, LPTSTR, INT32
)
{
    // メモリリーク検出
  #if defined(_DEBUG) || (DEBUG)
    ::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
  #endif

    // コマンドラインオプションの取得
    INT32 argc;
    const auto argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

    // データをやりとりするための 共有ファイル名を受け取る
    const auto filename = argv[0];
    if ( ! shrmem.Open(filename, File::ACCESS::WRITE) )
    {
        return 0;
    }

    BridgeData data;
    shrmem.Seek(0);
    shrmem.Read(&data);

    // データをやり取りするための イベントオブジェクトを受け取る
    lock_downward        = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, data.lock_downward);
    lock_upward          = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, data.lock_upward);
    downward_input_done  = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, data.downward_input_done);
    downward_output_done = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, data.downward_output_done);
    upward_input_done    = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, data.upward_input_done);
    upward_output_done   = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, data.upward_output_done);

    if ( lock_downward == nullptr || lock_upward == nullptr )
    {
        return 0;
    }
    if ( downward_input_done == nullptr || downward_output_done == nullptr )
    {
        return 0;
    }
    if ( upward_input_done == nullptr || upward_output_done == nullptr )
    {
        return 0;
    }

    // 無事受け取ったことを本体に通知
    ::SetEvent(downward_input_done);

    // 本体との通信用ウィンドウメッセージを取得
    MSG_TTBBRIDGE_COMMAND             = ::RegisterWindowMessageW(L"MSG_TTBBRIDGE_COMMAND");
    MSG_TTBPLUGIN_GETPLUGININFO       = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_GETPLUGININFO");
    MSG_TTBPLUGIN_SETPLUGININFO       = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_SETPLUGININFO");
    MSG_TTBPLUGIN_FREEPLUGININFO      = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_FREEPLUGININFO");
    MSG_TTBPLUGIN_SETMENUPROPERTY     = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_SETMENUPROPERTY");
    MSG_TTBPLUGIN_GETALLPLUGININFO    = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_GETALLPLUGININFO");
    MSG_TTBPLUGIN_FREEPLUGININFOARRAY = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_FREEPLUGININFOARRAY");
    MSG_TTBPLUGIN_SETTASKTRAYICON     = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_SETTASKTRAYICON");
    MSG_TTBPLUGIN_WRITELOG            = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_WRITELOG");
    MSG_TTBPLUGIN_EXECUTECOMMAND      = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_EXECUTECOMMAND");

    // メッセージを受け取るアダプターを生成
    TTBasePluginAdapter adapter;

    // メッセージループ
    MSG msg { };
    while ( ::GetMessage(&msg, nullptr, 0, 0) > 0 )
    {
        // 親プロセスからの イベントを処理する
        if ( msg.hwnd == nullptr && msg.message == MSG_TTBBRIDGE_COMMAND )
        {
            //::OutputDebugStringW(L"TTBbridge:\n  ");
            //::OutputDebugStringW(PluginMsgTxt[msg.wParam]);
            //::OutputDebugStringW(L"\n");
            adapter();
        }

        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    // 通信用オブジェクトの解放
    ::CloseHandle(lock_downward);
    ::CloseHandle(lock_upward);
    ::CloseHandle(downward_input_done);
    ::CloseHandle(downward_output_done);
    ::CloseHandle(upward_input_done);
    ::CloseHandle(upward_output_done);
    shrmem.Close();

    return static_cast<INT32>(msg.wParam);
}

//---------------------------------------------------------------------------//

// WinMain.cpp