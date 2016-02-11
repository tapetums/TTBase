#pragma once

//---------------------------------------------------------------------------//
//
// TTBBridgePlugin.cpp
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <array>
#include <vector>

#include <windows.h>
#include <strsafe.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib") // PathRemoveFileSpec, PathRelativePathTo

#include "include/GenerateUUIDString.hpp"
#include "../Utility.hpp"
#include "BridgeData.hpp"

#include "TTBBridgePlugin.hpp"

//---------------------------------------------------------------------------//
// Global Variables
//---------------------------------------------------------------------------//

// WinMain.cpp で宣言
extern HINSTANCE g_hInst;

//---------------------------------------------------------------------------//
// ユーティリティ関数
//---------------------------------------------------------------------------//

// デシリアライズ ヘルパー関数
template<typename T>
void desirialize(T* t, const std::vector<uint8_t>& data, size_t* p)
{
    const auto size = sizeof(T);

    ::memcpy(t, data.data() + *p, size);
    *p += size;

    wchar_t buf[16];
    ::StringCchPrintfW(buf, 16, L"%x", *t);
    ::OutputDebugStringW(buf);
    ::OutputDebugStringW(L"\n");
}

// デシリアライズ ヘルパー関数 (文字列版)
template<>
void desirialize<LPWSTR>(LPWSTR* dst, const std::vector<uint8_t>& data, size_t* p)
{
    const auto src = LPWSTR(data.data() + *p);
    const auto size = (1 + lstrlenW(src)) * sizeof(wchar_t);

    *dst = new wchar_t[size];
    ::memcpy(*dst, src, size);
    *p += size;

    ::OutputDebugStringW(*dst);
    ::OutputDebugStringW(L"\n");
}

// プロセス境界を超えるため データをデシリアライズする
PLUGIN_INFO_W* DeserializePluginInfo(const std::vector<uint8_t>& data)
{
    auto info = new PLUGIN_INFO_W;

    size_t p = 0;
    desirialize(&info->NeedVersion,  data, &p);
    desirialize(&(info->Name),       data, &p);
    desirialize(&(info->Filename),   data, &p);
    desirialize(&info->PluginType,   data, &p);
    desirialize(&info->VersionMS,    data, &p);
    desirialize(&info->VersionLS,    data, &p);
    desirialize(&info->CommandCount, data, &p);

    const auto count = info->CommandCount;
    info->Commands = new PLUGIN_COMMAND_INFO_W[count];
    for ( DWORD idx = 0; idx < count; ++idx )
    {
        auto& cmd = info->Commands[idx];

        desirialize(&(cmd.Name),        data, &p);
        desirialize(&(cmd.Caption),     data, &p);
        desirialize(&cmd.CommandID,     data, &p);
        desirialize(&cmd.Attr,          data, &p);
        desirialize(&cmd.ResID,         data, &p);
        desirialize(&cmd.DispMenu,      data, &p);
        desirialize(&cmd.TimerInterval, data, &p);
        desirialize(&cmd.TimerCounter,  data, &p);
    }

    desirialize(&info->LoadTime, data, &p);

    return info;
}

//---------------------------------------------------------------------------//
// TTBBridgePlugin
//---------------------------------------------------------------------------//

TTBBridgePlugin::TTBBridgePlugin()
{
    using namespace tapetums;

    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("ブリッヂプラグインを生成"));

    // データの準備
    BridgeData data;
    GenerateUUIDStringW(data.filename, data.namelen);
    GenerateUUIDStringW(data.done,     data.namelen);
    evt_done = ::CreateEventW(nullptr, FALSE, FALSE, data.done);

    if ( ! shrmem.Map(sizeof(data), data.filename, File::ACCESS::WRITE) )
    {
        return;
    }
    shrmem.Write(data);

    // 子プロセスのパスを合成
    std::array<wchar_t, MAX_PATH> path;
    ::GetModuleFileNameW(g_hInst, path.data(), (DWORD)path.size());
    ::PathRemoveFileSpecW(path.data());
    ::StringCchCatW(path.data(), path.size(), LR"(\TTBBridge.exe)");

    // 子プロセスの生成
    std::array<wchar_t, MAX_PATH> args;
    ::StringCchCopyW(args.data(), args.size(), shrmem.name());

    STARTUPINFOW si;
    ::GetStartupInfoW(&si);

    PROCESS_INFORMATION pi { };
    const auto result = ::CreateProcessW
    (
        path.data(), args.data(),
        nullptr, nullptr, FALSE,
        NORMAL_PRIORITY_CLASS, nullptr, nullptr,
        &si, &pi
    );
    if ( ! result )
    {
        ::MessageBoxW(nullptr, L"子プロセスを起動できません\n",  L"hako", MB_OK);
        return;
    }
    threadId = pi.dwThreadId;

    // 接続テスト
    DWORD ret;
    ret = ::WaitForSingleObject(evt_done, 5'000);
    if ( ret != WAIT_OBJECT_0 )
    {
        ::MessageBoxW(nullptr, L"子プロセスを開始できません\n",  L"hako", MB_OK);
        ::TerminateProcess(pi.hProcess, 0);
        ::Sleep(100);
        return;
    }

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
    return;
}

//---------------------------------------------------------------------------//

TTBBridgePlugin::~TTBBridgePlugin()
{
    if ( ! m_loaded )
    {
        return; // ムーブデストラクタでは余計な処理をしない
    }

    Unload();
    FreeInfo();
    Free();

    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("ブリッヂプラグインを解放"));

    // 子プロセスの終了
    ::PostThreadMessage(threadId, WM_QUIT, 0, 0);

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

void TTBBridgePlugin::swap(TTBBridgePlugin&& rhs) noexcept
{
    std::swap(m_path,   rhs.m_path);
    std::swap(m_loaded, rhs.m_loaded);
    std::swap(m_info,   rhs.m_info);

    std::swap(threadId, rhs.threadId);
    std::swap(evt_done, rhs.evt_done);
    std::swap(shrmem,   rhs.shrmem);
}

//---------------------------------------------------------------------------//

void TTBBridgePlugin::info(PLUGIN_INFO* info) noexcept
{
    FreePluginInfo(m_info);
    m_info = CopyPluginInfo(info);
}

//---------------------------------------------------------------------------//

bool TTBBridgePlugin::Load
(
    LPCTSTR path
)
{
    ::OutputDebugStringW(L"Load\n");

    if ( m_loaded )
    {
        ::OutputDebugStringW(L"  読込済み\n");
        return false;
    }

    if ( ! shrmem.is_mapped() ) { return false; }

    using namespace tapetums;

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), sizeof(wchar_t) * uuid.size());

    File plugin_data;
    const auto result = plugin_data.Map
    (
        sizeof(PluginMsg) + MAX_PATH * sizeof(wchar_t),
        uuid.data(), File::ACCESS::WRITE
    );
    if ( result )
    {
        PluginMsg msg = PluginMsg::Load;
        plugin_data.Write(&msg, sizeof(msg));
    }

    // データの書き込み
    std::array<wchar_t, MAX_PATH> buf;
  #if defined(_UNICODE) && defined(UNICODE)
    ::StringCchCopyW(buf.data(), buf.size(), path);
  #else
    std::array<wchar_t, MAX_PATH> path_unicode;
    ::StringCchCopyW(buf.data(), buf.size(), path_unicode.data());
  #endif

    plugin_data.Write(buf.data(), buf.size() * sizeof(wchar_t));

    // データを送信
    BridgeData data;
    ::StringCchCopyW(data.filename, data.namelen, plugin_data.name());
    shrmem.Seek(0);
    shrmem.Write(data);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    const auto ret = ::WaitForSingleObject(evt_done, 3'000);
    if ( ret != WAIT_OBJECT_0 )
    {
        ::OutputDebugStringW(L"  残念\n");
        return false;
    }

    PluginMsg msg;
    plugin_data.Seek(0);
    plugin_data.Read(&msg);
    ::OutputDebugStringW(L"  ");
    ::OutputDebugStringW(PluginMsgTxt[(uint8_t)msg]);
    ::OutputDebugStringW(L"\n");
    if ( msg != PluginMsg::OK )
    {
        return false;
    }

    // DLLのフルパスを取得
    ::StringCchCopy(m_path, MAX_PATH, path);

    m_loaded = true;

    // 相対パスの生成
    std::array<TCHAR, MAX_PATH> exe_path;
    std::array<TCHAR, MAX_PATH> rel_path;
    ::GetModuleFileName
    (
        g_hInst, exe_path.data(), (DWORD)exe_path.size()
    );
    ::PathRelativePathTo
    (
        rel_path.data(),
        exe_path.data(), FILE_ATTRIBUTE_ARCHIVE,
        path,            FILE_ATTRIBUTE_ARCHIVE
    );

    return InitInfo(rel_path.data() + 2);
}

//---------------------------------------------------------------------------//

void TTBBridgePlugin::Free()
{
    ::OutputDebugStringW(L"Free\n");

    if ( ! m_loaded )
    {
        ::OutputDebugStringW(L"  解放済み\n");
        return;
    }

    if ( ! shrmem.is_mapped() ) { return; }

    using namespace tapetums;

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), sizeof(wchar_t) * uuid.size());

    File plugin_data;
    const auto result = plugin_data.Map
    (
        sizeof(PluginMsg) + 0,
        uuid.data(), File::ACCESS::WRITE
    );
    if ( result )
    {
        PluginMsg msg = PluginMsg::Free;
        plugin_data.Write(&msg, sizeof(msg));
    }

    // データを送信
    BridgeData data;
    ::StringCchCopyW(data.filename, data.namelen, plugin_data.name());
    shrmem.Seek(0);
    shrmem.Write(data);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    const auto ret = ::WaitForSingleObject(evt_done, 3'000);
    if ( ret != WAIT_OBJECT_0 )
    {
        ::OutputDebugStringW(L"  残念\n");
        return;
    }

    m_loaded = false;

    ::OutputDebugStringW(L"  OK\n");
    return;
}

//---------------------------------------------------------------------------//

bool TTBBridgePlugin::Reload()
{
    bool result;

    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグインの再読み込み"));

    // プラグインの読み込み
    result = Load(m_path);
    if ( ! result )
    {
        return false;
    }

    // 相対パスの生成
    std::array<TCHAR, MAX_PATH> exe_path;
    std::array<TCHAR, MAX_PATH> rel_path;
    ::GetModuleFileName
    (
        g_hInst, exe_path.data(), (DWORD)exe_path.size()
    );
    ::PathRelativePathTo
    (
        rel_path.data(),
        exe_path.data(), FILE_ATTRIBUTE_ARCHIVE,
        m_path,          FILE_ATTRIBUTE_ARCHIVE
    );

    // プラグイン情報の再取得
    //  relative_path の 先頭2文字 (".\") は要らないので ずらす
    InitInfo(rel_path.data() + 2);

    // プラグインの初期化
    result = Init(m_info->Filename, (DWORD_PTR)this);
    if ( ! result )
    {
        Unload();
        return false;
    }

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
    return true;
}

//---------------------------------------------------------------------------//

bool TTBBridgePlugin::InitInfo
(
    LPTSTR PluginFilename
)
{
    ::OutputDebugStringW(L"InitInfo\n");

    if ( ! m_loaded )
    {
        ::OutputDebugStringW(L"  未読込\n");
        return false;
    }

    if ( ! shrmem.is_mapped() ) { return false; }

    using namespace tapetums;

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), sizeof(wchar_t) * uuid.size());

    File plugin_data;
    const auto result = plugin_data.Map
    (
        sizeof(PluginMsg) + MAX_PATH * sizeof(wchar_t),
        uuid.data(), File::ACCESS::WRITE
    );
    if ( result )
    {
        PluginMsg msg = PluginMsg::InitInfo;
        plugin_data.Write(&msg, sizeof(msg));
    }

    // データの書き込み
    std::array<wchar_t, MAX_PATH> buf;
  #if defined(_UNICODE) && defined(UNICODE)
    ::StringCchCopyW(buf.data(), buf.size(), PluginFilename);
  #else
    std::array<wchar_t, MAX_PATH> path_unicode;
    ::StringCchCopyW(buf.data(), buf.size(), path_unicode.data());
  #endif

    plugin_data.Write(buf.data(), buf.size() * sizeof(wchar_t));

    // データを送信
    BridgeData data;
    ::StringCchCopyW(data.filename, data.namelen, plugin_data.name());
    shrmem.Seek(0);
    shrmem.Write(data);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    const auto ret = ::WaitForSingleObject(evt_done, 3'000);
    if ( ret != WAIT_OBJECT_0 )
    {
        ::OutputDebugStringW(L"  残念\n");
        return false;
    }

    // データの読み取り
    shrmem.Seek(0);
    shrmem.Read(&data);

    File info_data;
    if ( ! info_data.Open(data.filename, File::ACCESS::WRITE) )
    {
        ::OutputDebugStringW(L"  共有ファイルが開けません\n");
        return false;
    }

    uint32_t size; // 32-bit の size_t は 32-bit 幅!!!!
    info_data.Seek(0);
    info_data.Read(&size);

    std::vector<uint8_t> serialized;
    serialized.resize(size);
    info_data.Read(serialized.data(), serialized.size());

    FreePluginInfo(m_info);
  #if defined(_UNICODE) || defined(UNICODE)
    m_info = DeserializePluginInfo(serialized);
  #else
    m_info = &g_info; // dummy ... ホントは再マーシャリングが必要: TO DO LATER
  #endif
    ::OutputDebugStringW(L"  OK\n");

    if ( m_info->PluginType != ptAlwaysLoad )
    {
        Free(); return true;
    }

    return true;
}

//---------------------------------------------------------------------------//

void TTBBridgePlugin::FreeInfo()
{
    // プラグイン情報の解放
    if ( m_info )
    {
        FreePluginInfo(m_info);
        m_info = nullptr;
    }

    ::OutputDebugStringW(L"FreeInfo\n");

    if ( ! m_loaded )
    {
        ::OutputDebugStringW(L"  解放済み\n");
        return;
    }

    if ( ! shrmem.is_mapped() ) { return; }

    using namespace tapetums;

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), sizeof(wchar_t) * uuid.size());

    File plugin_data;
    const auto result = plugin_data.Map
    (
        sizeof(PluginMsg) + 0,
        uuid.data(), File::ACCESS::WRITE
    );
    if ( result )
    {
        PluginMsg msg = PluginMsg::FreeInfo;
        plugin_data.Write(&msg, sizeof(msg));
    }

    // データを送信
    BridgeData data;
    ::StringCchCopyW(data.filename, data.namelen, plugin_data.name());
    shrmem.Seek(0);
    shrmem.Write(data);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    const auto ret = ::WaitForSingleObject(evt_done, 3'000);
    if ( ret != WAIT_OBJECT_0 )
    {
        ::OutputDebugStringW(L"  残念\n");
        return;
    }

    ::OutputDebugStringW(L"  OK\n");
    return;
}

//---------------------------------------------------------------------------//

bool TTBBridgePlugin::Init
(
    LPTSTR PluginFilename, DWORD_PTR hPlugin
)
{
    ::OutputDebugStringW(L"Init\n");

    if ( ! m_loaded )
    {
        ::OutputDebugStringW(L"  未読込\n");
        return false;
    }

    if ( ! shrmem.is_mapped() ) { return false; }

    using namespace tapetums;

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), sizeof(wchar_t) * uuid.size());

    File plugin_data;
    const auto result = plugin_data.Map
    (
        sizeof(PluginMsg) + MAX_PATH * sizeof(wchar_t) + sizeof(DWORD_PTR),
        uuid.data(), File::ACCESS::WRITE
    );
    if ( result )
    {
        PluginMsg msg = PluginMsg::Init;
        plugin_data.Write(&msg, sizeof(msg));
    }

    // データの書き込み
    std::array<wchar_t, MAX_PATH> buf;
  #if defined(_UNICODE) && defined(UNICODE)
    ::StringCchCopyW(buf.data(), buf.size(), PluginFilename);
  #else
    std::array<wchar_t, MAX_PATH> path_unicode;
    ::StringCchCopyW(buf.data(), buf.size(), path_unicode.data());
  #endif

    plugin_data.Write(buf.data(), sizeof(wchar_t) * buf.size());
    plugin_data.Write(hPlugin);

    // データを送信
    BridgeData data;
    ::StringCchCopyW(data.filename, data.namelen, plugin_data.name());
    shrmem.Seek(0);
    shrmem.Write(data);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    const auto ret = ::WaitForSingleObject(evt_done, 3'000);
    if ( ret != WAIT_OBJECT_0 )
    {
        ::OutputDebugStringW(L"  残念\n");
        return false;
    }

    PluginMsg msg;
    plugin_data.Seek(0);
    plugin_data.Read(&msg);

    ::OutputDebugStringW(L"  ");
    ::OutputDebugStringW(PluginMsgTxt[(uint8_t)msg]);
    ::OutputDebugStringW(L"\n");

    return true;
}

//---------------------------------------------------------------------------//

void TTBBridgePlugin::Unload()
{
    ::OutputDebugStringW(L"Unload\n");

    if ( ! m_loaded )
    {
        ::OutputDebugStringW(L"  未読込\n");
        return;
    }

    if ( ! shrmem.is_mapped() ) { return; }

    using namespace tapetums;

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), sizeof(wchar_t) * uuid.size());

    File plugin_data;
    const auto result = plugin_data.Map
    (
        sizeof(PluginMsg) + 0,
        uuid.data(), File::ACCESS::WRITE
    );
    if ( result )
    {
        PluginMsg msg = PluginMsg::Unload;
        plugin_data.Write(&msg, sizeof(msg));
    }

    // データを送信
    BridgeData data;
    ::StringCchCopyW(data.filename, data.namelen, plugin_data.name());
    shrmem.Seek(0);
    shrmem.Write(data);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    const auto ret = ::WaitForSingleObject(evt_done, 3'000);
    if ( ret != WAIT_OBJECT_0 )
    {
        ::OutputDebugStringW(L"  残念\n");
        return;
    }

    ::OutputDebugStringW(L"  ");
    ::OutputDebugStringW(L"OK\n");

    return;
}

//---------------------------------------------------------------------------//

bool TTBBridgePlugin::Execute
(
    INT32 CmdID, HWND hwnd
)
{
    using namespace tapetums;
    ::OutputDebugStringW(L"Execute\n");

    if ( ! m_loaded )
    {
        ::OutputDebugStringW(L"  未読込\n");
        return false;
    }

    if ( ! shrmem.is_mapped() ) { return false; }

    using namespace tapetums;

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), sizeof(wchar_t) * uuid.size());

    File plugin_data;
    const auto result = plugin_data.Map
    (
        sizeof(PluginMsg) + sizeof(INT32) + sizeof(DWORD_PTR),
        uuid.data(), File::ACCESS::WRITE
    );
    if ( result )
    {
        PluginMsg msg = PluginMsg::Execute;
        plugin_data.Write(&msg, sizeof(msg));
    }

    // データの書き込み
    plugin_data.Write(CmdID);
    plugin_data.Write(hwnd);

    // データを送信
    BridgeData data;
    ::StringCchCopyW(data.filename, data.namelen, plugin_data.name());
    shrmem.Seek(0);
    shrmem.Write(data);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    const auto ret = ::WaitForSingleObject(evt_done, 3'000);
    if ( ret != WAIT_OBJECT_0 )
    {
        ::OutputDebugStringW(L"  残念\n");
        return false;
    }

    PluginMsg msg;
    plugin_data.Seek(0);
    plugin_data.Read(&msg);

    ::OutputDebugStringW(L"  ");
    ::OutputDebugStringW(PluginMsgTxt[(uint8_t)msg]);
    ::OutputDebugStringW(L"\n");

    return true;
}

//---------------------------------------------------------------------------//

void TTBBridgePlugin::Hook
(
    UINT Msg, WPARAM wParam, LPARAM lParam
)
{
    using namespace tapetums;
    ::OutputDebugStringW(L"Hook\n");

    if ( ! m_loaded )
    {
        ::OutputDebugStringW(L"  未読込\n");
        return;
    }

    if ( ! shrmem.is_mapped() ) { return; }

    using namespace tapetums;

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), sizeof(wchar_t) * uuid.size());

    File plugin_data;
    const auto result = plugin_data.Map
    (
        sizeof(PluginMsg) + sizeof(DWORD) + sizeof(WPARAM) + sizeof(LPARAM),
        uuid.data(), File::ACCESS::WRITE
    );
    if ( result )
    {
        PluginMsg msg = PluginMsg::Hook;
        plugin_data.Write(&msg, sizeof(msg));
    }

    // データの書き込み
    plugin_data.Write(Msg);
    plugin_data.Write(wParam);
    plugin_data.Write(lParam);

    // データを送信
    BridgeData data;
    ::StringCchCopyW(data.filename, data.namelen, plugin_data.name());
    shrmem.Seek(0);
    shrmem.Write(data);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    const auto ret = ::WaitForSingleObject(evt_done, 3'000);
    if ( ret != WAIT_OBJECT_0 )
    {
        ::OutputDebugStringW(L"  残念\n");
        return;
    }

    ::OutputDebugStringW(L"  ");
    ::OutputDebugStringW(L"OK\n");

    return;
}

//---------------------------------------------------------------------------//

// TTBBridgePlugin.cpp