//---------------------------------------------------------------------------//
//
// Hook.cpp
//  フックプロシージャ
//   Copyright 2002(C) K2
//   Copyright 2002-2009(C) TTBase Project
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "..\MessageDef.hpp"

//---------------------------------------------------------------------------//
// 共有領域
//---------------------------------------------------------------------------//

#pragma data_seg(".shared")

// 共有領域のデータは初期化が必須
namespace
{
    HWND  g_hwnd       { nullptr };
    HHOOK g_hShellHook { nullptr };
    HHOOK g_hMouseHook { nullptr };
}

#pragma data_seg()

//---------------------------------------------------------------------------//
// グローバル変数
//---------------------------------------------------------------------------//

HINSTANCE g_hInst { nullptr };

//---------------------------------------------------------------------------//
// 前方宣言
//---------------------------------------------------------------------------//

LRESULT CALLBACK MouseProc(INT32 nCode, WPARAM wp, LPARAM lp);
LRESULT CALLBACK ShellProc(INT32 nCode, WPARAM wp, LPARAM lp);

//---------------------------------------------------------------------------//
// DLL エントリポイント
//---------------------------------------------------------------------------//

// プログラムサイズを小さくするためにCRTを除外
#if !defined(_DEBUG) && !defined(DEBUG)

#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#pragma comment(linker, "/entry:DllMain")

#endif

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID)
{
    if ( fdwReason == DLL_PROCESS_ATTACH )
    {
        g_hInst = hInstance;

        RegisterMessages();
    }

    return TRUE;
}

//---------------------------------------------------------------------------//
// DLL エクスポート関数
//---------------------------------------------------------------------------//

//フックを組み込む
extern "C" BOOL WINAPI InstallHook(HWND hWnd)
{
    g_hwnd = hWnd;

    g_hShellHook = ::SetWindowsHookEx(WH_SHELL, ShellProc, g_hInst, 0);
    if ( g_hShellHook == 0 )
    {
        return FALSE;
    }

    g_hMouseHook = ::SetWindowsHookEx(WH_MOUSE, MouseProc, g_hInst, 0);
    if ( g_hMouseHook == 0 )
    {
        return FALSE;
    }

    ::PostMessage(HWND_BROADCAST, WM_NULL, 0, 0);
    return TRUE;
}

//---------------------------------------------------------------------------//

//フックを解除する
extern "C" void WINAPI UninstallHook()
{
    ::UnhookWindowsHookEx(g_hShellHook);
    ::UnhookWindowsHookEx(g_hMouseHook);

    g_hwnd       = nullptr;
    g_hShellHook = nullptr;
    g_hMouseHook = nullptr;

    ::PostMessage(HWND_BROADCAST, WM_NULL, 0, 0);
}

//---------------------------------------------------------------------------//
// フックプロシージャ
//---------------------------------------------------------------------------//

LRESULT CALLBACK MouseProc(INT32 nCode, WPARAM wp, LPARAM lp)
{
    if ( nCode == HC_ACTION )
    {
        ::PostMessage
        (
            g_hwnd, TTB_HMOUSE_ACTION, wp, LPARAM((LPMOUSEHOOKSTRUCT(lp))->hwnd)
        );
    }

    return ::CallNextHookEx(g_hMouseHook, nCode, wp, lp);
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK ShellProc(INT32 nCode, WPARAM wp, LPARAM lp)
{
    switch ( nCode )
    {
        case HSHELL_ACTIVATESHELLWINDOW:
        {
            ::PostMessage(g_hwnd, TTB_HSHELL_ACTIVATESHELLWINDOW, wp, lp);
            break;
        }
        case HSHELL_GETMINRECT:
        {
            ::PostMessage(g_hwnd, TTB_HSHELL_GETMINRECT, wp, lp);
            break;
        }
        case HSHELL_LANGUAGE:
        {
            ::PostMessage(g_hwnd, TTB_HSHELL_LANGUAGE, wp, lp);
            break;
        }
        case HSHELL_REDRAW:
        {
            ::PostMessage(g_hwnd, TTB_HSHELL_REDRAW, wp, lp);
            break;
        }
        case HSHELL_TASKMAN:
        {
            ::PostMessage(g_hwnd, TTB_HSHELL_TASKMAN, wp, lp);
            break;
        }
        case HSHELL_WINDOWACTIVATED:
        {
            ::PostMessage(g_hwnd, TTB_HSHELL_WINDOWACTIVATED, wp, lp);
            break;
        }
        case HSHELL_WINDOWCREATED:
        {
            ::PostMessage(g_hwnd, TTB_HSHELL_WINDOWCREATED, wp, lp);
            break;
        }
        case HSHELL_WINDOWDESTROYED:
        {
            ::PostMessage(g_hwnd, TTB_HSHELL_WINDOWDESTROYED, wp, lp);
            break;
        }
    }

    return ::CallNextHookEx(g_hShellHook, nCode, wp, lp);
}

//---------------------------------------------------------------------------//

// Hook.cpp