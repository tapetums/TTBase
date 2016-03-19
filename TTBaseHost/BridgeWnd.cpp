//---------------------------------------------------------------------------//
//
// BridgeWnd.cpp
//  プロセス間通信を使って別プロセスのプラグインからデータを受け取るウィンドウ
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <array>
#include <vector>

#include "include/File.hpp"
#include "include/GenerateUUIDString.hpp"
#include "../Utility.hpp"
#include "BridgeData.hpp"

#include "BridgeWnd.hpp"

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

// ブリッヂとの通信用ウィンドウメッセージ
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

//---------------------------------------------------------------------------//
// ユーティリティ関数 (TTBBridgePlugin.cpp で定義)
//---------------------------------------------------------------------------//

inline PLUGIN_INFO_W* DeserializePluginInfo(const std::vector<uint8_t>& data);

//---------------------------------------------------------------------------//
// ctor
//---------------------------------------------------------------------------//

BridgeWnd::BridgeWnd()
{
    // 本体との通信用ウィンドウメッセージを登録
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

    Register(TEXT("TTBBridgeWnd"));

    const auto style   = 0;
    const auto styleEx = 0;
    Create(TEXT("TTBBridgeWnd"), style, styleEx, nullptr, nullptr);
}

//---------------------------------------------------------------------------//
// ウィンドウプロシージャ
//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if ( msg == WM_CREATE )
    {
        return OnCreate();
    }
    if ( msg == WM_DESTROY )
    {
        return OnDestroy();
    }

    if ( msg == MSG_TTBPLUGIN_GETPLUGININFO )
    {
        return OnGetPluginInfo();
    }
    if ( msg == MSG_TTBPLUGIN_SETPLUGININFO )
    {
        return OnSetPluginInfo();
    }
    if ( msg == MSG_TTBPLUGIN_FREEPLUGININFO )
    {
        return OnFreePluginInfo();
    }
    if ( msg == MSG_TTBPLUGIN_SETMENUPROPERTY )
    {
        return OnSetMenuProperty();
    }
    if ( msg == MSG_TTBPLUGIN_GETALLPLUGININFO )
    {
        return OnGetAllPluginInfo();
    }
    if ( msg == MSG_TTBPLUGIN_FREEPLUGININFOARRAY )
    {
        return OnFreePluginInfoArray();
    }
    if ( msg == MSG_TTBPLUGIN_SETTASKTRAYICON )
    {
        return OnSetTaskTrayIcon();
    }
    if ( msg == MSG_TTBPLUGIN_WRITELOG )
    {
        return OnWriteLog();
    }
    if ( msg == MSG_TTBPLUGIN_EXECUTECOMMAND )
    {
        return OnExecuteCommand();
    }

    return super::WndProc(hwnd, msg, wp, lp);
}

//---------------------------------------------------------------------------//
// 内部メソッド
//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnCreate()
{
    BridgeData data;

    GenerateUUIDStringW(data.lock_downward,        data.namelen);
    GenerateUUIDStringW(data.lock_upward,          data.namelen);
    GenerateUUIDStringW(data.downward_input_done,  data.namelen);
    GenerateUUIDStringW(data.downward_output_done, data.namelen);
    GenerateUUIDStringW(data.upward_input_done,    data.namelen);
    GenerateUUIDStringW(data.upward_output_done,   data.namelen);
    lock_downward        = ::CreateMutexW(nullptr, FALSE, data.lock_downward);
    lock_upward          = ::CreateMutexW(nullptr, FALSE, data.lock_upward);
    downward_input_done  = ::CreateEventW(nullptr, TRUE, FALSE, data.downward_input_done);
    downward_output_done = ::CreateEventW(nullptr, TRUE, FALSE, data.downward_output_done);
    upward_input_done    = ::CreateEventW(nullptr, TRUE, FALSE, data.upward_input_done);
    upward_output_done   = ::CreateEventW(nullptr, TRUE, FALSE, data.upward_output_done);

    std::array<wchar_t, data.namelen> name;
    GenerateUUIDStringW(name.data(),  name.size());
    if ( shrmem.Map(sizeof(data), name.data(), File::ACCESS::WRITE) )
    {
        shrmem.Write(data);
    }

    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnDestroy()
{
    ::CloseHandle(lock_downward);
    ::CloseHandle(lock_upward);
    ::CloseHandle(downward_input_done);
    ::CloseHandle(downward_output_done);
    ::CloseHandle(upward_input_done);
    ::CloseHandle(upward_output_done);
    shrmem.Close();

    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnGetPluginInfo()
{
    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnSetPluginInfo()
{
    BridgeData data;
    shrmem.Seek(0);
    shrmem.Read(&data);
    SystemLog(TEXT("  downward_file: %s"), data.filename_downward);
    SystemLog(TEXT("  upward_file:   %s"), data.filename_upward);

    SystemLog(TEXT("OnSetPluginInfo: %s"), data.filename_upward);

    File info_data;
    if ( ! info_data.Open(data.filename_upward, File::ACCESS::READ) )
    {
        WriteLog(elError, TEXT("  %s"), TEXT("共有ファイルが開けません"));
        ::SetEvent(upward_input_done);
        return 0;
    }

    info_data.Seek(0);

    DWORD_PTR hPlugin;
    info_data.Read(&hPlugin);

    uint32_t size; // 32-bit の size_t は 32-bit 幅!!!!
    info_data.Read(&size);

    std::vector<uint8_t> serialized;
    serialized.resize(size);
    info_data.Read(serialized.data(), serialized.size());

    const auto info = DeserializePluginInfo(serialized);
    TTBPlugin_SetPluginInfo(hPlugin, info);
    FreePluginInfo(info);

    // 受信完了を通知
    ::SetEvent(upward_input_done);
  #endif

    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnFreePluginInfo()
{
    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnSetMenuProperty()
{
    BridgeData data;
    shrmem.Seek(0);
    shrmem.Read(&data);
    SystemLog(TEXT("  downward_file: %s"), data.filename_downward);
    SystemLog(TEXT("  upward_file:   %s"), data.filename_upward);

    SystemLog(TEXT("OnSetPluginInfo: %s"), data.filename_upward);

    File menu_data;
    if ( ! menu_data.Open(data.filename_upward, File::ACCESS::READ) )
    {
        WriteLog(elError, TEXT("  %s"), TEXT("共有ファイルが開けません"));
        ::SetEvent(upward_input_done);
        return 0;
    }

    menu_data.Seek(0);

    DWORD_PTR hPlugin;
    menu_data.Read(&hPlugin);

    INT32 CommandID;
    menu_data.Read(&CommandID);

    CHANGE_FLAG ChangeFlag;
    menu_data.Read(&ChangeFlag);

    DISPMENU Flag;
    menu_data.Read(&Flag);

    TTBPlugin_SetMenuProperty(hPlugin, CommandID, ChangeFlag, Flag);

    // 受信完了を通知
    ::SetEvent(upward_input_done);

    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnGetAllPluginInfo()
{
    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnFreePluginInfoArray()
{
    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnSetTaskTrayIcon()
{
    SystemLog(TEXT("%s"), TEXT("OnSetTaskTrayIcon:  "));
    SystemLog(TEXT("  %s"), TEXT("実装されていません"));

    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnWriteLog()
{
    BridgeData data;
    shrmem.Seek(0);
    shrmem.Read(&data);
    SystemLog(TEXT("  downward_file: %s"), data.filename_downward);
    SystemLog(TEXT("  upward_file:   %s"), data.filename_upward);

    SystemLog(TEXT("OnWriteLog: %s"), data.filename_upward);

    File log_data;
    if ( ! log_data.Open(data.filename_upward, File::ACCESS::READ) )
    {
        WriteLog(elError, TEXT("  %s"), TEXT("共有ファイルが開けません"));
        ::SetEvent(upward_input_done);
        return 0;
    }

    log_data.Seek(0);

    DWORD_PTR hPlugin;
    log_data.Read(&hPlugin);

    ERROR_LEVEL logLevel;
    log_data.Read(&logLevel);

    uint32_t cch;
    log_data.Read(&cch);

    std::vector<wchar_t> msg(cch);
    log_data.Read(msg.data(), sizeof(wchar_t) * cch);

    TTBPlugin_WriteLog(hPlugin, logLevel, msg.data());

    // 受信完了を通知
    ::SetEvent(upward_input_done);

    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnExecuteCommand()
{
    BridgeData data;
    shrmem.Seek(0);
    shrmem.Read(&data);
    SystemLog(TEXT("  downward_file: %s"), data.filename_downward);
    SystemLog(TEXT("  upward_file:   %s"), data.filename_upward);

    SystemLog(TEXT("OnExecuteCommand: %s"), data.filename_upward);

    File execute_data;
    if ( ! execute_data.Open(data.filename_upward, File::ACCESS::READ) )
    {
        WriteLog(elError, TEXT("  %s"), TEXT("共有ファイルが開けません"));
        ::SetEvent(upward_input_done);
        return 0;
    }

    execute_data.Seek(0);

    uint32_t cch;
    execute_data.Read(&cch);

    std::vector<wchar_t> PluginFilename(cch);
    execute_data.Read(PluginFilename.data(), sizeof(wchar_t) * cch);

    INT32 CmdID;
    execute_data.Read(&CmdID);

    TTBPlugin_ExecuteCommand(PluginFilename.data(), CmdID);

    // 受信完了を通知
    ::SetEvent(upward_input_done);

    return 0;
}

//---------------------------------------------------------------------------//

// BridgeWnd.cpp