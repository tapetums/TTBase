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
    auto hwnd = ::WindowFromPoint(pt);
    if ( hwnd == nullptr )
    {
        return ::CallNextHookEx(g_hHook, nCode, wp, lp);
    }

    // マウスカーソルがウィンドウ上で設定された範囲にあるか調べる
    RECT rc;
    ::GetWindowRect(hwnd, &rc);

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

    /*const auto ret = ::SendMessage
    (
        hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)
    );
    if ( ret != HTCLOSE )
    {
        goto Skip;
    }*/

    // リソースからメニューを取得
    const auto hMenu    = ::LoadMenu(g_hInst, MAKEINTRESOURCE(100));
    const auto hSubMenu = ::GetSubMenu(hMenu, 0);

    const auto topmost = CheckTopMost(hwnd);
    {
        // チェックマークを付ける
        MENUITEMINFO mii;
        ::GetMenuItemInfo(hSubMenu, 0, TRUE, &mii);
        mii.cbSize = sizeof(mii);
        mii.fMask  = MIIM_STATE;
        mii.fState = topmost ? MFS_CHECKED : MFS_UNCHECKED;
        ::SetMenuItemInfo(hSubMenu, 0, TRUE, &mii);
    }

    // Article ID: Q135788
    // ポップアップメニューから処理を戻すために必要
    ::SetForegroundWindow(g_hwnd);

    // ポップアップメニューを表示
    const auto CmdID = ::TrackPopupMenu
    (
        hSubMenu, TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
        pt.x, pt.y, 0, g_hwnd, nullptr
    );
    WriteLog(elDebug, TEXT("%s: CmdId = %d"), PLUGIN_NAME, CmdID);

    // 表示したメニューを破棄
    ::DestroyMenu(hMenu);

    // Article ID: Q135788
    // ポップアップメニューから処理を戻すために必要
    ::PostMessage(hwnd, WM_NULL, 0, 0);
    ::PostMessage(g_hwnd, WM_NULL, 0, 0);

    // コマンドを実行
    if ( CmdID == 40000 )
    {
        ToggleTopMost(hwnd, topmost);
    }
    else if ( CmdID == 40001 )
    {
        OpenAppFolder(hwnd);
    }
    else if ( 40002 <= CmdID && CmdID <= 40005 )
    {
        ExecutePluginCommand(TEXT(":system"), CmdID - 40002);
    }
    else if ( CmdID == 40006 )
    {
        settings->load();
    }
    else if ( 41000 < CmdID && CmdID < 42000 )
    {
        SetOpaque(hwnd, BYTE(256 * (CmdID - 41000) / 100 - 1));
    }
    else if ( 42000 < CmdID && CmdID < 43000 )
    {
        SetPriority(hwnd, CmdID - 42001);
    }

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