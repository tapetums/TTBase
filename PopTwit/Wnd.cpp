//---------------------------------------------------------------------------//
//
// Wnd.cpp
//  ウィンドウ操作 ヘルパ関数
//   Copyright (C) 2014 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include "Wnd.hpp"

//---------------------------------------------------------------------------//
// グローバル変数の実体宣言
//---------------------------------------------------------------------------//

UINT WM_NOTIFYICON     = 0;
UINT WM_TASKBARCREATED = 0;

HICON g_hIcon   = nullptr;
HICON g_hIconSm = nullptr;

//---------------------------------------------------------------------------//
// ユーティリティ関数
//---------------------------------------------------------------------------//

static void __stdcall ShowLastError()
{
    LPTSTR lpMsgBuf = nullptr;
    ::FormatMessage
    (
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        ::GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        nullptr
    );
    MessageBox(nullptr, lpMsgBuf, TEXT("GetLastError()"), MB_OK);
    ::LocalFree(lpMsgBuf);
}

//---------------------------------------------------------------------------//

namespace Wnd {

//---------------------------------------------------------------------------//
// ウィンドウ操作ヘルパ関数
//---------------------------------------------------------------------------//

ATOM __stdcall Register
(
    LPCTSTR lpszClassName, WNDPROC lpfnWndProc, LPCTSTR lpszIconName
)
{
    if ( lpszClassName == nullptr )
    {
        lpszClassName = TEXT("Wnd");
    }

    // アイコンのロード
    g_hIcon = (HICON)::LoadImage
    (
        g_hInst, lpszIconName,
        IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED
    );
    g_hIconSm = (HICON)::LoadImage
    (
        g_hInst, lpszIconName,
        IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED
    );

    // ウィンドウプロシージャ
    if ( lpfnWndProc == nullptr )
    {
        lpfnWndProc = [](HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) -> LRESULT
        {
            if ( uMsg == WM_DESTROY )
            {
                ::PostQuitMessage(0);
            }

            return ::DefWindowProc(hwnd, uMsg,wp, lp);
        };
    }

    // ウィンドウクラス情報
    WNDCLASSEX wcex =
    {
        sizeof(WNDCLASSEX),
        CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        lpfnWndProc,
        0, 0,
        g_hInst,
        g_hIcon,
        ::LoadCursor(nullptr, IDC_ARROW),
        nullptr,
        nullptr,
        lpszClassName,
        g_hIconSm,
    };

    return ::RegisterClassEx(&wcex);
}

//---------------------------------------------------------------------------//

HWND __stdcall Create
(
    LPCTSTR lpszClassName,
    LPCTSTR lpWindowName,
    DWORD   style,
    DWORD   styleEx,
    HWND    hwndParent,
    HMENU   hMenu
)
{
    // 親ウィンドウを持つ場合はウィンドウスタイルにWS_CHILDを追加
    style |= hwndParent ? WS_CHILD : 0;

    // ウィンドウを生成
    const auto hwnd = ::CreateWindowEx
    (
        styleEx, lpszClassName, lpWindowName, style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hwndParent, hMenu, g_hInst, nullptr
    );
    if ( hwnd )
    {
        ::UpdateWindow(hwnd);
    }

    return hwnd;
}

//---------------------------------------------------------------------------//

void __stdcall AdjustRect(HWND hwnd, INT32& w, INT32& h)
{
    RECT rc = { 0, 0, w, h };
    const BOOL hasMenu = ::GetMenu(hwnd) ? TRUE : FALSE;
    const auto style   = (DWORD)::GetWindowLongPtr(hwnd, GWL_STYLE);
    const auto styleEx = (DWORD)::GetWindowLongPtr(hwnd, GWL_EXSTYLE);

    ::AdjustWindowRectEx(&rc, style, hasMenu, styleEx);
    w = rc.right  - rc.left;
    h = rc.bottom - rc.top;
}

//---------------------------------------------------------------------------//

bool __stdcall Bounds(HWND hwnd, INT32 x, INT32 y, INT32 w, INT32 h)
{
    AdjustRect(hwnd, w, h);

    const auto ret = ::SetWindowPos
    (
        hwnd, nullptr,
        x, y, w, h,
        SWP_NOZORDER | SWP_FRAMECHANGED
    );

    return ret ? true : false;
}

//---------------------------------------------------------------------------//

bool __stdcall Move(HWND hwnd, INT32 x, INT32 y)
{
    const auto ret = ::SetWindowPos
    (
        hwnd, nullptr,
        x, y, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
    );

    return ret ? true : false;
}

//---------------------------------------------------------------------------//

bool __stdcall Refresh(HWND hwnd, BOOL bErase)
{
    return ::InvalidateRect(hwnd, nullptr, bErase) ? true : false;
}

//---------------------------------------------------------------------------//

bool __stdcall Resize(HWND hwnd, INT32 w, INT32 h)
{
    AdjustRect(hwnd, w, h);

    const auto ret = ::SetWindowPos
    (
        hwnd, nullptr,
        0, 0, w, h,
        SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED
    );

    return ret ? true : false;
}

//---------------------------------------------------------------------------//

bool __stdcall Show(HWND hwnd, INT32 nCmdShow)
{
    return ::ShowWindowAsync(hwnd, nCmdShow) ? true : false;
}

//---------------------------------------------------------------------------//

void __stdcall GetMonitorRectUnderCursor(RECT* rect)
{
    POINT pt;
    ::GetCursorPos(&pt);
    const auto hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

    MONITORINFOEX miex = { };
    miex.cbSize = sizeof(miex);
    ::GetMonitorInfo(hMonitor, &miex);

    *rect = miex.rcMonitor;
}

//---------------------------------------------------------------------------//

bool __stdcall ToCenter(HWND hwnd)
{
    const auto hwnd_parent = (DWORD)::GetWindowLongPtr(hwnd, GWLP_HWNDPARENT);
    if ( hwnd_parent )
    {
        return false;
    }

    RECT rc;
    GetMonitorRectUnderCursor(&rc);
    const auto mx = rc.left;
    const auto my = rc.top;
    const auto mw = rc.right  - rc.left;
    const auto mh = rc.bottom - rc.top;

    ::GetClientRect(hwnd, &rc);
    const auto w = rc.right  - rc.left;
    const auto h = rc.bottom - rc.top;

    const auto x = (mw  - w) / 2 + mx;
    const auto y = (mh - h) / 2 + my;

    const auto result = Move(hwnd, x, y);

    return result;
}

//---------------------------------------------------------------------------//

bool __stdcall IsCompositionEnabled()
{
    BOOL is_enabled;
    ::DwmIsCompositionEnabled(&is_enabled);

    return is_enabled ? true : false;
}

//---------------------------------------------------------------------------//

void __stdcall EnableAero(HWND hwnd)
{
    DWM_BLURBEHIND bb = { };
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = TRUE;
    bb.hRgnBlur = nullptr;
    ::DwmEnableBlurBehindWindow(hwnd, &bb);

    MARGINS margins = { -1 };
    ::DwmExtendFrameIntoClientArea(hwnd, &margins);

    ::InvalidateRect(hwnd, nullptr, TRUE);
}

//---------------------------------------------------------------------------//

void __stdcall DisableAero(HWND hwnd)
{
    DWM_BLURBEHIND bb = { };
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = FALSE;
    bb.hRgnBlur = nullptr;
    ::DwmEnableBlurBehindWindow(hwnd, &bb);

    MARGINS margins = { };
    ::DwmExtendFrameIntoClientArea(hwnd, &margins);

    ::InvalidateRect(hwnd, nullptr, TRUE);
}

//---------------------------------------------------------------------------//

} // namespace Wnd

//---------------------------------------------------------------------------//

// Wnd.cpp