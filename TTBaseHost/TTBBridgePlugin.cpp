#pragma once

//---------------------------------------------------------------------------//
//
// TTBBridgePlugin.cpp
//  プロセス間通信を使って別プロセスのプラグインを操作する
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

#pragma warning(disable: 4366)

#if SYS_DEBUG

template<typename C, typename... Args>
void SystemLog(const C* const format, Args... args)
{
    WriteLog(ERROR_LEVEL(5), format, args...);
}

#else

#define SystemLog(format, ...)

#endif

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

    SystemLog(TEXT("  deserializing... %i"), *t);
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

    SystemLog(TEXT("  deserializing... %s"), *dst);
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

    SystemLog(TEXT("%s"), TEXT("ブリッヂプラグインを生成"));

    // データの準備
    BridgeData data;
    GenerateUUIDStringW(data.filename, data.namelen);
    GenerateUUIDStringW(data.done,     data.namelen);
    evt_done = ::CreateEventW(nullptr, TRUE, FALSE, data.done);

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
        WriteLog(elError, TEXT("  %s"), TEXT("TTBBridge.exe を起動できません"));
        return;
    }
    threadId = pi.dwThreadId;

    // 接続テスト
    const auto ret = ::WaitForSingleObject(evt_done, 5'000);
    if ( ret != WAIT_OBJECT_0 )
    {
        WriteLog(elError, TEXT("  %s"), TEXT("TTBBridge.exe を開始できません"));
        ::TerminateProcess(pi.hProcess, 0);
        return;
    }

    SystemLog(TEXT("  %s"), TEXT("OK"));
    return;
}

//---------------------------------------------------------------------------//

TTBBridgePlugin::~TTBBridgePlugin()
{
    if ( is_loaded() )
    {
        Unload();
        FreeInfo();
        Free();
    }

    SystemLog(TEXT("%s"), TEXT("ブリッヂプラグインを解放"));

    // 子プロセスの終了
    ::PostThreadMessage(threadId, WM_QUIT, 0, 0);

    SystemLog(TEXT("  %s"), TEXT("OK"));
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
    SystemLog(TEXT("%s"), TEXT("プラグインの読み込み"));
    SystemLog(TEXT("  %s"), path);

    if ( m_loaded )
    {
        SystemLog(TEXT("  %s"), TEXT("読込済み"));
        return false;
    }

    if ( ! shrmem.is_mapped() ) { return false; }

    // DLLのフルパスを取得
    ::StringCchCopy(m_path, MAX_PATH, path);

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
    shrmem.Flush();

    ::ResetEvent(evt_done);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    ::WaitForSingleObject(evt_done, 100);

    PluginMsg msg;
    plugin_data.Seek(0);
    plugin_data.Read(&msg);

    if ( msg == PluginMsg::OK )
    {
        // 読み込み済みのフラグをオン
        m_loaded = true;
    }

    SystemLog(TEXT("  %s"), PluginMsgTxt[(uint8_t)msg]);
    return true;
}

//---------------------------------------------------------------------------//

void TTBBridgePlugin::Free()
{
    SystemLog(TEXT("%s"), TEXT("プラグインを解放"));
    SystemLog(TEXT("  %s"), m_path);

    if ( ! m_loaded )
    {
        SystemLog(TEXT("  %s"), TEXT("解放済み"));
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
    shrmem.Flush();

    ::ResetEvent(evt_done);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    ::WaitForSingleObject(evt_done, 100);

    // 読み込み済みのフラグをオフ
    m_loaded = false;

    SystemLog(TEXT("  %s"), TEXT("OK"));
    return;
}

//---------------------------------------------------------------------------//

bool TTBBridgePlugin::Reload()
{
    bool result;

    SystemLog(TEXT("%s"), TEXT("プラグインの再読み込み"));

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

    // relative_path の 先頭2文字 (".\") は要らないので ずらす
    const auto PluginFilename = rel_path.data() + 2;

    // プラグイン情報の再取得
    //  relative_path の 先頭2文字 (".\") は要らないので ずらす
    //InitInfo(PluginFilename);

    // プラグインの初期化
    result = Init(PluginFilename, (DWORD_PTR)this);
    if ( ! result )
    {
        Unload();
        return false;
    }

    SystemLog(TEXT("  %s"), TEXT("OK"));
    return true;
}

//---------------------------------------------------------------------------//

bool TTBBridgePlugin::InitInfo
(
    LPTSTR PluginFilename
)
{
    SystemLog(TEXT("%s"), TEXT("プラグイン情報をコピー"));

    if ( ! m_loaded )
    {
        SystemLog(TEXT("  %s"), TEXT("未読込"));
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
    PluginFilename;
    ::StringCchCopyW(buf.data(), buf.size(), path_unicode.data());
  #endif

    plugin_data.Write(buf.data(), buf.size() * sizeof(wchar_t));

    // データを送信
    BridgeData data;
    ::StringCchCopyW(data.filename, data.namelen, plugin_data.name());
    shrmem.Seek(0);
    shrmem.Write(data);
    shrmem.Flush();

    ::ResetEvent(evt_done);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    ::WaitForSingleObject(evt_done, 100);

    // データの読み取り
    shrmem.Seek(0);
    shrmem.Read(&data);

    File info_data;
    if ( ! info_data.Open(data.filename, File::ACCESS::WRITE) )
    {
        WriteLog(elError, TEXT("  %s"), TEXT("共有ファイルが開けません"));
        return false;
    }

    // プラグイン情報の取得
    uint32_t size; // 32-bit の size_t は 32-bit 幅!!!!
    info_data.Seek(0);
    info_data.Read(&size);

    std::vector<uint8_t> serialized;
    serialized.resize(size);
    info_data.Read(serialized.data(), serialized.size());

    // プラグイン情報のマーシャリング
    FreePluginInfo(m_info);
  #if defined(_UNICODE) || defined(UNICODE)
    m_info = DeserializePluginInfo(serialized);
  #else
    m_info = &g_info; // dummy ... ホントは再マーシャリングが必要: TO DO LATER
  #endif

    SystemLog(TEXT("  %s"), TEXT("OK"));
    return true;
}

//---------------------------------------------------------------------------//

void TTBBridgePlugin::FreeInfo()
{
    SystemLog(TEXT("%s"), TEXT("プラグイン情報を解放"));

    // プラグイン情報の解放
    if ( m_info )
    {
        FreePluginInfo(m_info);
        m_info = nullptr;
    }

    if ( ! m_loaded )
    {
        SystemLog(TEXT("  %s"), TEXT("解放済み"));
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
    shrmem.Flush();

    ::ResetEvent(evt_done);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    ::WaitForSingleObject(evt_done, 100);

    SystemLog(TEXT("  %s"), TEXT("OK"));
    return;
}

//---------------------------------------------------------------------------//

bool TTBBridgePlugin::Init
(
    LPTSTR PluginFilename, DWORD_PTR hPlugin
)
{
    SystemLog(TEXT("%s"), TEXT("プラグインの初期化"));

    if ( ! m_loaded )
    {
        SystemLog(TEXT("  %s"), TEXT("未実装"));
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
    PluginFilename;
    ::StringCchCopyW(buf.data(), buf.size(), path_unicode.data());
  #endif

    plugin_data.Write(buf.data(), sizeof(wchar_t) * buf.size());
    plugin_data.Write(hPlugin);

    // データを送信
    BridgeData data;
    ::StringCchCopyW(data.filename, data.namelen, plugin_data.name());
    shrmem.Seek(0);
    shrmem.Write(data);
    shrmem.Flush();

    ::ResetEvent(evt_done);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    ::WaitForSingleObject(evt_done, 100);

    PluginMsg msg;
    plugin_data.Seek(0);
    plugin_data.Read(&msg);

    SystemLog(TEXT("  %s"), PluginMsgTxt[(uint8_t)msg]);
    return true;
}

//---------------------------------------------------------------------------//

void TTBBridgePlugin::Unload()
{
    SystemLog(TEXT("%s"), TEXT("プラグインの終了処理"));

    if ( ! m_loaded )
    {
        SystemLog(TEXT("  %s"), TEXT("未実装"));
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
    shrmem.Flush();

    ::ResetEvent(evt_done);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    ::WaitForSingleObject(evt_done, 100);

    SystemLog(TEXT("  %s"), TEXT("OK"));
    return;
}

//---------------------------------------------------------------------------//

bool TTBBridgePlugin::Execute
(
    INT32 CmdID, HWND hwnd
)
{
    SystemLog(TEXT("%s"), TEXT("コマンドの実行"));
    SystemLog(TEXT("  %s|%i"), info()->Filename, CmdID);

    if ( ! m_loaded )
    {
        SystemLog(TEXT("  %s"), TEXT("未実装"));
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
    shrmem.Flush();

    ::ResetEvent(evt_done);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    ::WaitForSingleObject(evt_done, 100);

    PluginMsg msg;
    plugin_data.Seek(0);
    plugin_data.Read(&msg);

    SystemLog(TEXT("  %s"), PluginMsgTxt[uint8_t(msg)]);
    return true;
}

//---------------------------------------------------------------------------//

void TTBBridgePlugin::Hook
(
    UINT Msg, WPARAM wParam, LPARAM lParam
)
{
    SystemLog(TEXT("%s"), TEXT("プラグインをフック"));

    if ( ! m_loaded )
    {
        SystemLog(TEXT("  %s"), TEXT("未実装"));
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
    shrmem.Flush();

    ::ResetEvent(evt_done);
    ::PostThreadMessage(threadId, WM_COMMAND, 0, 0);

    // 受信完了待ち
    ::WaitForSingleObject(evt_done, 100);

    SystemLog(TEXT("  %s"), TEXT("OK"));
    return;
}

//---------------------------------------------------------------------------//

// TTBBridgePlugin.cpp