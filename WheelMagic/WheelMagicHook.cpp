/******************************************************************************
*                                                                             *
*    WheelMagicHook.cpp      Copyright(c) 2009-2011 itow,y., 2014 tapetums    *
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

#include "WheelMagicHook.h"

//---------------------------------------------------------------------------//

// フックハンドル
static HHOOK g_hHook = nullptr;

// インスタンスハンドル
extern HINSTANCE g_hInstance;

//---------------------------------------------------------------------------//

// 位置からウィンドウを探すための情報
struct FindWindowInfo
{
    POINT ptPos;
    HWND  hwnd;
};

//---------------------------------------------------------------------------//

// 位置からウィンドウを探すためのコールバック関数
static BOOL CALLBACK FindWindowProc(HWND hwnd, LPARAM lParam)
{
    const auto pInfo = (FindWindowInfo*)lParam;
    if ( pInfo == nullptr )
    {
        return FALSE;
    }

    RECT  rc;
    TCHAR szClass[64];

    if
    (
        hwnd != pInfo->hwnd
        && ::IsWindowVisible(hwnd)
        && ::GetWindowRect(hwnd, &rc)
        && ::PtInRect(&rc, pInfo->ptPos)
        && ::GetClassName(hwnd, szClass, sizeof(szClass)/sizeof(TCHAR)) > 0
        && ::lstrcmpi(szClass, TEXT("tooltips_class32")) != 0
        && ::lstrcmpi(szClass, TEXT("SysShadow")) != 0
    )
    {
        pInfo->hwnd = hwnd;
        return FALSE;
    }

    return TRUE;
}

//---------------------------------------------------------------------------//

// フックプロシージャ
static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
    if ( nCode != HC_ACTION || (wParam != WM_MOUSEWHEEL && wParam != WM_MOUSEHWHEEL) )
    {
        return ::CallNextHookEx(g_hHook, nCode, wParam, lParam);
    }

    // マウスカーソルの位置を取得
    const auto pmhs     = (MSLLHOOKSTRUCT*)lParam;
    const auto ptCursor = pmhs->pt;

    // マウスカーソル直下にあるウィンドウのハンドルを取得
    auto hwnd = ::WindowFromPoint(ptCursor);
    if ( hwnd == nullptr )
    {
        goto Skip;
    }

    // ウィンドウのクラス名を取得
    TCHAR szClass[64];
    if ( 0 == ::GetClassName(hwnd, szClass, sizeof(szClass)/sizeof(TCHAR)) )
    {
        // return 1;
        goto Skip;
    }

    HWND  hwndTarget;
    POINT ptClient;

    if ( 0 == lstrcmpi(szClass, TEXT("tooltips_class32")) )
    {
        // Tooltipに重なる場合、下のウィンドウを探す
        FindWindowInfo Info;
        Info.ptPos = ptCursor;
        Info.hwnd  = hwnd;

        ::EnumWindows(FindWindowProc, (LPARAM)&Info);
        if ( Info.hwnd == hwnd )
        {
            // Tooltip の下に何もなかった
            goto Skip;
        }

        hwndTarget = Info.hwnd;
        hwnd       = Info.hwnd;

        // 子ウィンドウを探す
        while ( true )
        {
            ptClient = ptCursor;
            ::ScreenToClient(hwnd, &ptClient);

            hwnd = ::RealChildWindowFromPoint(hwnd, ptClient);
            if ( hwnd == nullptr || hwnd == hwndTarget )
            {
                // 一番下の子孫ウィンドウだった
                break;
            }
            hwndTarget = hwnd;
        }
    }
    else
    {
        hwndTarget = hwnd;

        if ( (::GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD ) != 0 )
        {
            const auto hwndParent = ::GetParent(hwnd);
            if ( hwndParent != nullptr )
            {
                ptClient = ptCursor;
                ::ScreenToClient(hwndParent, &ptClient);

                hwndTarget = ::RealChildWindowFromPoint(hwndParent, ptClient);
                if ( hwndTarget == nullptr )
                {
                    hwndTarget = hwnd;
                }
            }
        }
    }

    WORD KeyState = 0;
    if ( ::GetAsyncKeyState(VK_LBUTTON) < 0 )
        KeyState |= MK_LBUTTON;
    if ( ::GetAsyncKeyState(VK_RBUTTON) < 0 )
        KeyState |= MK_RBUTTON;
    if ( ::GetAsyncKeyState(VK_MBUTTON) < 0 )
        KeyState |= MK_MBUTTON;
    if ( ::GetAsyncKeyState(VK_XBUTTON1) < 0 )
        KeyState |= MK_XBUTTON1;
    if ( ::GetAsyncKeyState(VK_XBUTTON2) < 0 )
        KeyState |= MK_XBUTTON2;
    if ( ::GetAsyncKeyState(VK_SHIFT)    < 0 )
        KeyState |= MK_SHIFT;
    if ( ::GetAsyncKeyState(VK_CONTROL)  < 0 )
        KeyState |= MK_CONTROL;

    ::PostMessage
    (
        hwndTarget, (UINT)wParam,
        MAKEWPARAM(KeyState, HIWORD(pmhs->mouseData)),
        MAKELPARAM((SHORT)ptCursor.x, (SHORT)ptCursor.y)
    );

    return 1;

Skip:
    return ::CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

//---------------------------------------------------------------------------//

// フックの開始
BOOL WMBeginHook(void)
{
    if ( g_hHook != nullptr )
    {
        return TRUE;
    }

    g_hHook = ::SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, g_hInstance, 0);

    return (g_hHook != nullptr) ? TRUE : FALSE;
}

//---------------------------------------------------------------------------//

// フックの終了
BOOL WMEndHook(void)
{
    if ( g_hHook == nullptr )
    {
        return FALSE;
    }

    ::UnhookWindowsHookEx(g_hHook);
    g_hHook = nullptr;

    return TRUE;
}

//---------------------------------------------------------------------------//

// 