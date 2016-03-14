//---------------------------------------------------------------------------//
//
// BridgeWnd.cpp
//  プロセス間通信を使って別プロセスのプラグインからデータを受け取るウィンドウ
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <array>

#include "include/File.hpp"
#include "include/GenerateUUIDString.hpp"
#include "BridgeData.hpp"

#include "BridgeWnd.hpp"

//---------------------------------------------------------------------------//
// グローバル変数
//---------------------------------------------------------------------------//

using namespace tapetums;

// 本体との通信用オブジェクト
File   shrmem;
HANDLE input_done;
HANDLE output_done;
HANDLE shrlock;

// 本体との通信用ウィンドウメッセージ
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
// ctor
//---------------------------------------------------------------------------//

BridgeWnd::BridgeWnd()
{
    // 本体との通信用ウィンドウメッセージを登録
    MSG_TTBPLUGIN_GETPLUGININFO       = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_GETPLUGININFO");
    MSG_TTBPLUGIN_SETPLUGININFO       = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_SETPLUGININFO");
    MSG_TTBPLUGIN_FREEPLUGININFO      = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_FREEPLUGININFO");
    MSG_TTBPLUGIN_SETMENUPROPERTY     = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_SETMENUPROPERTY");
    MSG_TTBPLUGIN_GETALLPLUGININFO    = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_GETALLPLUGININFO");
    MSG_TTBPLUGIN_FREEPLUGININFOARRAY = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_FREEPLUGININFOARRAY");
    MSG_TTBPLUGIN_SETTASKTRAYICON     = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_SETTASKTRAYICON");
    MSG_TTBPLUGIN_WRITELOG            = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_WRITELOG");
    MSG_TTBPLUGIN_EXECUTECOMMAND      = ::RegisterWindowMessageW(L"MSG_TTBPLUGIN_EXECUTECOMMAND");

    Register(TEXT("BridgeWnd"));

    const auto style   = 0;
    const auto styleEx = 0;
    Create(TEXT("BridgeWnd"), style, styleEx, nullptr, nullptr);
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

    GenerateUUIDStringW(data.input_done,  data.namelen);
    GenerateUUIDStringW(data.output_done, data.namelen);
    GenerateUUIDStringW(data.lockname,    data.namelen);
    input_done  = ::CreateEventW(nullptr, TRUE, FALSE, data.input_done);
    output_done = ::CreateEventW(nullptr, TRUE, FALSE, data.output_done);
    shrlock     = ::CreateMutexW(nullptr, FALSE, data.lockname);

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
    ::CloseHandle(input_done);
    ::CloseHandle(output_done);
    ::CloseHandle(shrlock);
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
    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnWriteLog()
{
    return 0;
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK BridgeWnd::OnExecuteCommand()
{
    return 0;
}

//---------------------------------------------------------------------------//

// BridgeWnd.cpp