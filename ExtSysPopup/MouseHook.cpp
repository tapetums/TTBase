/******************************************************************************
*                                                                             *
*    MouseHook.cpp      Copyright(c) 2009-2011 itow,y., 2014-2016 tapetums    *
*                                                                             *
******************************************************************************/

/******************************************************************************

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

******************************************************************************/

#include <windows.h>

#include "..\Plugin.hpp"
#include "..\Utility.hpp"
#include "Main.hpp"

#include "Settings.hpp"

#include "MouseHook.hpp"

//---------------------------------------------------------------------------//

// フックハンドル
namespace{ HHOOK g_hHook { nullptr }; }

//---------------------------------------------------------------------------//

// フックプロシージャ
static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wp, LPARAM lp)
{
    //if ( nCode != HC_ACTION || wp != WM_NCRBUTTONUP )
    if ( wp != WM_NCRBUTTONUP && wp != WM_RBUTTONUP )
    {
        return ::CallNextHookEx(g_hHook, nCode, wp, lp);
    }

    // マウスカーソルの位置を取得
    const auto pmhs = (MSLLHOOKSTRUCT*)lp;
    const auto pt   = pmhs->pt;

    // マウスカーソル直下にあるウィンドウのハンドルを取得
    auto hwnd_target = ::WindowFromPoint(pt);
    if ( nullptr == hwnd_target )
    {
        return ::CallNextHookEx(g_hHook, nCode, wp, lp);
    }

    // トップレベルのオーナーウィンドウをさがす
    HWND parent = hwnd_target;
    do
    {
        parent = ::GetParent(parent);
        if ( parent )
        {
            hwnd_target = parent;
        }
    }
    while ( parent );

    // マウスカーソルがウィンドウ上で設定された範囲にあるか調べる
    RECT rc;
    ::GetWindowRect(hwnd_target, &rc);

    const auto w = settings->w;
    const auto h = settings->h;

    INT32 x = settings->x;
    INT32 y = settings->y;

    x = ( x >= 0 ) ? rc.left + x : rc.right  + x;
    y = ( y >= 0 ) ? rc.top  + y : rc.bottom + y;

    WriteLog(elDebug, TEXT("%s: %i < %i < %i"), PLUGIN_NAME, x, pt.x, x + w);
    if ( pt.x < x || x + w < pt.x )
    {
        return ::CallNextHookEx(g_hHook, nCode, wp, lp);
    }
    WriteLog(elDebug, TEXT("%s: %i < %i < %i"), PLUGIN_NAME, y, pt.y, y + h);
    if ( pt.y < y || y + h < pt.y )
    {
        return ::CallNextHookEx(g_hHook, nCode, wp, lp);
    }

    // ポップアップメニューを表示
    PostMessage(g_hwnd, WM_COMMAND, 0, LPARAM(hwnd_target));

    return 1L;
}

//---------------------------------------------------------------------------//

// フックの開始
BOOL WMBeginHook(void)
{
    if ( g_hHook != nullptr ) { return TRUE; }

    g_hHook = ::SetWindowsHookEx(WH_MOUSE, MouseHookProc, g_hInst, 0);

    return (g_hHook != nullptr) ? TRUE : FALSE;
}

//---------------------------------------------------------------------------//

// フックの終了
BOOL WMEndHook(void)
{
    if ( g_hHook == nullptr ) { return FALSE; }

    ::UnhookWindowsHookEx(g_hHook);
    g_hHook = nullptr;

    return TRUE;
}

//---------------------------------------------------------------------------//

// MouseHook.cpp