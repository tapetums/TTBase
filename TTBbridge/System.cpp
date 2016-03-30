//---------------------------------------------------------------------------//
//
// System.cpp
//  システムプラグイン および TTBase ホストプログラム 本体
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <array>
#include <vector>

#include <windows.h>
#include <strsafe.h>

#include "include/File.hpp"
#include "include/GenerateUUIDString.hpp"
#include "include/Transcode.hpp"
#include "../Plugin.hpp"
#include "../BridgeData.hpp"
#include "Utility.hpp"

//---------------------------------------------------------------------------//
// グローバル変数
//---------------------------------------------------------------------------//

using namespace tapetums;

// 本体との通信用オブジェクト (WinMain.cpp で定義)
extern File   shrmem;
extern HANDLE lock_upward;
extern HANDLE upward_input_done;
extern HANDLE upward_output_done;

// 本体との通信用ウィンドウメッセージ (WinMain.cpp で定義)
extern UINT MSG_TTBPLUGIN_GETPLUGININFO;
extern UINT MSG_TTBPLUGIN_GETPLUGININFO;
extern UINT MSG_TTBPLUGIN_SETPLUGININFO;
extern UINT MSG_TTBPLUGIN_FREEPLUGININFO;
extern UINT MSG_TTBPLUGIN_SETMENUPROPERTY;
extern UINT MSG_TTBPLUGIN_GETALLPLUGININFO;
extern UINT MSG_TTBPLUGIN_FREEPLUGININFOARRAY;
extern UINT MSG_TTBPLUGIN_SETTASKTRAYICON;
extern UINT MSG_TTBPLUGIN_WRITELOG;
extern UINT MSG_TTBPLUGIN_EXECUTECOMMAND;

// プラグインのハンドル (WinMain.cpp で定義)
extern uint64_t g_hPlugin64;

// 通信用ウィンドウ
HWND g_hwnd { nullptr };

//---------------------------------------------------------------------------//
// ユーティリティ関数
//---------------------------------------------------------------------------//

void FindBridgeWnd()
{
    if ( nullptr == g_hwnd )
    {
        g_hwnd = ::FindWindow(TEXT("TTBBridgeWnd"), TEXT("TTBBridgeWnd"));
    }
}

//---------------------------------------------------------------------------//
// 本体側エクスポート関数
//---------------------------------------------------------------------------//

extern "C" PLUGIN_INFO* WINAPI TTBPlugin_GetPluginInfo
(
    DWORD_PTR
)
{
    FindBridgeWnd();
    if ( nullptr == g_hwnd ) { return nullptr; }

    ::PostMessage(g_hwnd, MSG_TTBPLUGIN_GETPLUGININFO, 0, 0);

    return nullptr;
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_SetPluginInfo
(
    DWORD_PTR, PLUGIN_INFO* PLUGIN_INFO
)
{
    FindBridgeWnd();
    if ( nullptr == g_hwnd ) { return; }

    const auto info = MarshallPluginInfo(PLUGIN_INFO);
    const auto serialized = SerializePluginInfo(info);
    FreePluginInfo(info);

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), uuid.size());
    //::OutputDebugStringW(L"info_data:\n  ");
    //::OutputDebugStringW(uuid.data());
    //::OutputDebugStringW(L"\n");

    File info_data;
    info_data.Map(sizeof(size_t) + serialized.size(), uuid.data(), File::ACCESS::WRITE);
    info_data.Write(g_hPlugin64);
    info_data.Write(serialized.size()); /// sizeof(size_t) == sizeof(uint32_t)
    info_data.Write(serialized.data(), serialized.size());

    // データを送信
    ::WaitForSingleObject(lock_upward, INFINITE);

    BridgeData data;
    shrmem.Seek(0);
    shrmem.Read(&data);
    ::StringCchCopyW(data.filename_upward, data.namelen, uuid.data());
    shrmem.Seek(0);
    shrmem.Write(data);

    ::ResetEvent(upward_input_done);
    ::PostMessage(g_hwnd, MSG_TTBPLUGIN_SETPLUGININFO, 0, 0);

    // 受信完了待ち
    ::WaitForSingleObject(upward_input_done, INFINITE);

    ::ReleaseMutex(lock_upward);
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_FreePluginInfo
(
    PLUGIN_INFO* PLUGIN_INFO
)
{
    if ( nullptr == PLUGIN_INFO ) { return; }

    FindBridgeWnd();
    if ( nullptr == g_hwnd ) { return; }

    ::PostMessage(g_hwnd, MSG_TTBPLUGIN_FREEPLUGININFO, 0, 0);
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_SetMenuProperty
(
    DWORD_PTR, INT32 CommandID, CHANGE_FLAG ChangeFlag, DISPMENU Flag
)
{
    FindBridgeWnd();
    if ( nullptr == g_hwnd ) { return; }

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), uuid.size());
    //::OutputDebugStringW(L"menu_data:\n  ");
    //::OutputDebugStringW(uuid.data());
    //::OutputDebugStringW(L"\n");

    const auto size = sizeof(INT32) + sizeof(CHANGE_FLAG) + sizeof(DISPMENU);

    File info_data;
    info_data.Map(size, uuid.data(), File::ACCESS::WRITE);
    info_data.Write(g_hPlugin64);
    info_data.Write(CommandID);
    info_data.Write(ChangeFlag);
    info_data.Write(Flag);

    // データを送信
    ::WaitForSingleObject(lock_upward, INFINITE);

    BridgeData data;
    shrmem.Seek(0);
    shrmem.Read(&data);
    ::StringCchCopyW(data.filename_upward, data.namelen, uuid.data());
    shrmem.Seek(0);
    shrmem.Write(data);

    ::ResetEvent(upward_input_done);
    ::PostMessage(g_hwnd, MSG_TTBPLUGIN_SETMENUPROPERTY, 0, 0);

    // 受信完了待ち
    ::WaitForSingleObject(upward_input_done, INFINITE);

    ::ReleaseMutex(lock_upward);
}

//---------------------------------------------------------------------------//

extern "C" PLUGIN_INFO** WINAPI TTBPlugin_GetAllPluginInfo()
{
    FindBridgeWnd();
    if ( nullptr == g_hwnd ) { return nullptr; }

    ::PostMessage(g_hwnd, MSG_TTBPLUGIN_GETALLPLUGININFO, 0, 0);

    return nullptr;
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_FreePluginInfoArray
(
    PLUGIN_INFO** PluginInfoArray
)
{
    if ( nullptr == PluginInfoArray ) { return; }

    FindBridgeWnd();
    if ( nullptr == g_hwnd ) { return; }

    ::PostMessage(g_hwnd, MSG_TTBPLUGIN_FREEPLUGININFOARRAY, 0, 0);
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_SetTaskTrayIcon
(
    HICON, LPCTSTR
)
{
    FindBridgeWnd();
    if ( nullptr == g_hwnd ) { return; }

    ::PostMessage(g_hwnd, MSG_TTBPLUGIN_SETTASKTRAYICON, 0, 0);
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_WriteLog
(
    DWORD_PTR, ERROR_LEVEL logLevel, LPCTSTR msg
)
{
    FindBridgeWnd();
    if ( nullptr == g_hwnd ) { return; }

    std::vector<wchar_t> buf;
    buf.resize(UTF16Len(msg) + 1);
    toUTF16(msg, buf.data(), buf.size());

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), uuid.size());
    //::OutputDebugStringW(L"log_data:\n  ");
    //::OutputDebugStringW(uuid.data());
    //::OutputDebugStringW(L"\n");
    //::OutputDebugStringW(buf.data());
    //::OutputDebugStringW(L"\n");

    const int64_t size = sizeof(UINT64) + sizeof(ERROR_LEVEL) + sizeof(size_t) + sizeof(wchar_t) * buf.size();

    File log_data;
    log_data.Map(size, uuid.data(), File::ACCESS::WRITE);

    log_data.Write(g_hPlugin64);
    log_data.Write(logLevel);
    log_data.Write(buf.size());
    log_data.Write(buf.data(), sizeof(wchar_t) * buf.size());

    // データを送信
    ::WaitForSingleObject(lock_upward, INFINITE);

    BridgeData data;
    shrmem.Seek(0);
    shrmem.Read(&data);
    ::StringCchCopyW(data.filename_upward, data.namelen, uuid.data());
    shrmem.Seek(0);
    shrmem.Write(data);

    ::ResetEvent(upward_input_done);
    ::PostMessage(g_hwnd, MSG_TTBPLUGIN_WRITELOG, 0, 0);

    // 受信完了待ち
    ::WaitForSingleObject(upward_input_done, INFINITE);

    ::ReleaseMutex(lock_upward);
}

//---------------------------------------------------------------------------//

extern "C" BOOL WINAPI TTBPlugin_ExecuteCommand
(
    LPCTSTR PluginFilename, INT32 CmdID
)
{
    FindBridgeWnd();
    if ( nullptr == g_hwnd ) { return FALSE; }

    std::vector<wchar_t> buf;
    buf.resize(UTF16Len(PluginFilename) + 1);
    toUTF16(PluginFilename, buf.data(), buf.size());

    // データの準備
    std::array<wchar_t, BridgeData::namelen> uuid;
    GenerateUUIDStringW(uuid.data(), uuid.size());
    //::OutputDebugStringW(L"execute_data:\n  ");
    //::OutputDebugStringW(uuid.data());
    //::OutputDebugStringW(L"\n  ");
    //::OutputDebugStringW(buf.data());
    //::OutputDebugStringW(L"\n");

    const int64_t size = sizeof(size_t) + sizeof(wchar_t) * buf.size() + sizeof(INT32);

    File execute_data;
    execute_data.Map(size, uuid.data(), File::ACCESS::WRITE);

    execute_data.Write(buf.size());
    execute_data.Write(buf.data(), sizeof(wchar_t) * buf.size());
    execute_data.Write(CmdID);

    // データを送信
    ::WaitForSingleObject(lock_upward, INFINITE);

    BridgeData data;
    shrmem.Seek(0);
    shrmem.Read(&data);
    ::StringCchCopyW(data.filename_upward, data.namelen, uuid.data());
    shrmem.Seek(0);
    shrmem.Write(data);

    ::ResetEvent(upward_input_done);
    ::PostMessage(g_hwnd, MSG_TTBPLUGIN_EXECUTECOMMAND, 0, 0);

    // 送信完了待ち
    ::WaitForSingleObject(upward_input_done, INFINITE);

    ::ReleaseMutex(lock_upward);
    return TRUE;
}

//---------------------------------------------------------------------------//

// System.cpp