#pragma once

//---------------------------------------------------------------------------//
//
// BridgeWnd.hpp
//  プロセス間通信を使って別プロセスのプラグインからデータを受け取るウィンドウ
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include "include/UWnd.hpp"

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
// Class
//---------------------------------------------------------------------------//

class BridgeWnd : public tapetums::UWnd
{
    using super = UWnd;

public:
    BridgeWnd()
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

public:
    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) override
    {
        return super::WndProc(hwnd, msg, wp, lp);
    }
};

//---------------------------------------------------------------------------//

// BridgeWnd.hpp