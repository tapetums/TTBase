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

#include "MouseHook.hpp"

//---------------------------------------------------------------------------//

// フックハンドル
namespace{ HHOOK g_hHook { nullptr }; }

//---------------------------------------------------------------------------//

// フックプロシージャ
static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wp, LPARAM lp)
{
    if ( nCode != HC_ACTION || wp != WM_NCRBUTTONUP )
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
        goto Skip;
    }

    // マウスカーソルが閉じるボタンの上にあるか調べる
    const auto ret = ::SendMessage
    (
        hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)
    );
    if ( ret != HTCLOSE )
    {
        goto Skip;
    }

    // リソースからメニューを取得
    const auto hMenu    = ::LoadMenu(g_hInst, MAKEINTRESOURCE(100));
    const auto hSubMenu = ::GetSubMenu(hMenu, 0);

    const auto topmost = CheckTopMost(hwnd);
    if ( topmost )
    {
        // チェックマークを付ける
        MENUITEMINFO mii;
        ::GetMenuItemInfo(hSubMenu, 0, TRUE, &mii);
        mii.cbSize = sizeof(mii);
        mii.fMask  = MIIM_STATE;
        mii.fState = topmost ? MFS_CHECKED : MFS_UNCHECKED;
        ::SetMenuItemInfo(hSubMenu, 0, TRUE, &mii);
    }

    // ポップアップメニューを表示
    const auto CmdId = ::TrackPopupMenu
    (
        hSubMenu, TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
        pt.x, pt.y, 0, g_hwnd, nullptr
    );
    WriteLog(elDebug, TEXT("%s: CmdId = %d"), PLUGIN_NAME, CmdId);

    // 表示したメニューを破棄
    ::DestroyMenu(hMenu);

    // Article ID: Q135788
    // ポップアップメニューから処理を戻すために必要
    ::PostMessage(hwnd, WM_NULL, 0, 0);

    // コマンドを実行
    switch ( CmdId )
    {
        case 40000:
        {
            ToggleTopMost(hwnd, topmost); break;
        }
        case 40001:
        {
            OpenAppFolder(hwnd); break;
        }
        case 40002: case 40004: case 40003: case 40005:
        {
            ExecutePluginCommand(TEXT(":system"), CmdId - 40002 ); break;
        }
        default:
        {
            break;
        }
    }

    return 1L;

Skip:
    return ::CallNextHookEx(g_hHook, nCode, wp, lp);
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