//---------------------------------------------------------------------------//
//
// UWnd.cpp
//  ウィンドウをカプセル化するクラス
//   Copyright (C) 2005-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <windowsx.h>
#include <strsafe.h>

#pragma comment(lib, "shell32.lib") // Shell_NotifyIcon

#include "UWnd.hpp"

//---------------------------------------------------------------------------//
// グローバル変数の実体宣言
//---------------------------------------------------------------------------//

UINT WM_NOTIFYICON     { 0 };
UINT WM_TASKBARCREATED { 0 };

//---------------------------------------------------------------------------//
// ユーティリティ関数
//---------------------------------------------------------------------------//

void tapetums::ShowLastError
(
    LPCTSTR window_title
)
{
    LPTSTR lpMsgBuf{ nullptr };
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
    ::MessageBox(nullptr, lpMsgBuf, window_title, MB_OK);
    ::LocalFree(lpMsgBuf);
}

//---------------------------------------------------------------------------//

void tapetums::AdjustRect
(
    HWND hwnd, INT32* w, INT32* h
)
{
    RECT rc{ 0, 0, *w, *h };
    const auto style   = (DWORD)::GetWindowLongPtr(hwnd, GWL_STYLE);
    const auto styleEx = (DWORD)::GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    const auto isChild = style & WS_CHILD;
    const BOOL hasMenu = (!isChild && ::GetMenu(hwnd)) ? TRUE : FALSE;

    ::AdjustWindowRectEx(&rc, style, hasMenu, styleEx);
    *w = rc.right  - rc.left;
    *h = rc.bottom - rc.top;
}

//---------------------------------------------------------------------------//

void tapetums::GetRectForMonitorUnderCursor
(
    RECT* rect
)
{
    POINT pt;
    ::GetCursorPos(&pt);
    const auto hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

    MONITORINFOEX miex{ };
    miex.cbSize = (DWORD)sizeof(MONITORINFOEX);
    ::GetMonitorInfo(hMonitor, &miex);

    *rect = miex.rcMonitor;
}

//---------------------------------------------------------------------------//

#include <ShellScalingApi.h>

void tapetums::GetDpiForMonitorUnderCursor
(
    POINT* dpi
)
{
    static const auto Shcore = ::LoadLibraryEx
    (
        TEXT("Shcore.dll"), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH
    );

    using F = HRESULT (__stdcall*)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);
    static const auto GetDpiForMonitor = (F)::GetProcAddress
    (
        Shcore, "GetDpiForMonitor"
    );

    if ( GetDpiForMonitor )
    {
        POINT pt;
        ::GetCursorPos(&pt);
        const auto hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

        GetDpiForMonitor
        (
            hMonitor, MONITOR_DPI_TYPE::MDT_EFFECTIVE_DPI,
            (UINT*)&dpi->x, (UINT*)&dpi->y
        );
    }
    else
    {
        const auto hDC = ::GetDC(nullptr);
        if ( hDC )
        {
            dpi->x = ::GetDeviceCaps(hDC, LOGPIXELSX);
            dpi->y = ::GetDeviceCaps(hDC, LOGPIXELSY);

            ::ReleaseDC(nullptr, hDC);
        }
    }
}

//---------------------------------------------------------------------------//
// UWnd 静的メソッド
//---------------------------------------------------------------------------//

LRESULT WINAPI tapetums::UWnd::WndMapProc
(
    HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp
)
{
    UWnd* wnd;

    // UWndオブジェクトのポインタを取得
    if ( uMsg == WM_NCCREATE )
    {
        wnd = (UWnd*)((CREATESTRUCT*)lp)->lpCreateParams;

        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);
    }
    else
    {
        wnd = (UWnd*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    // まだマップされていない場合はデフォルトプロシージャに投げる
    if ( nullptr == wnd )
    {
        return ::DefWindowProc(hwnd, uMsg, wp, lp);
    }

    // メンバ変数に情報を保存
    switch ( uMsg )
    {
        case WM_CREATE:
        {
            wnd->m_hwnd = hwnd; // ウィンドウハンドル
            break;
        }
        case WM_MOVE:
        {
            wnd->m_x = GET_X_LPARAM(lp); // ウィンドウX座標
            wnd->m_y = GET_Y_LPARAM(lp); // ウィンドウY座標
            break;
        }
        case WM_SIZE:
        {
            wnd->m_w = LOWORD(lp); // ウィンドウ幅
            wnd->m_h = HIWORD(lp); // ウィンドウ高
            break;
        }
        default:
        {
            break;
        }
    }

    // ウィンドウプロシージャの呼び出し
    return wnd->WndProc(hwnd, uMsg, wp, lp);
}

//---------------------------------------------------------------------------//
// UWnd コンストラクタ
//---------------------------------------------------------------------------//

tapetums::UWnd::UWnd()
{
    static ATOM atom { 0 };
    if ( atom == 0 ) { atom = Register(class_name); }
}

//---------------------------------------------------------------------------//
// UWnd メソッド
//---------------------------------------------------------------------------//

ATOM tapetums::UWnd::Register
(
    LPCTSTR lpszClassName
)
{
    m_class_name = lpszClassName;

    WNDCLASSEX wcex
    {
        sizeof(WNDCLASSEX),
        CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        WndMapProc,
        0, 0,
        ::GetModuleHandle(nullptr),
        nullptr,
        ::LoadCursor(nullptr, IDC_ARROW),
        nullptr,
        nullptr,
        lpszClassName,
        nullptr,
    };

    return ::RegisterClassEx(&wcex);
}

//---------------------------------------------------------------------------//

HWND tapetums::UWnd::Create
(
    LPCTSTR lpszWindowName,
    DWORD   style,
    DWORD   styleEx,
    HWND    hwndParent,
    HMENU   hMenu
)
{
    if ( m_hwnd ) { return m_hwnd; } // 二重生成防止!

    const auto hwnd = ::CreateWindowEx
    (
        styleEx, m_class_name, lpszWindowName, style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hwndParent, hMenu, ::GetModuleHandle(nullptr),
        reinterpret_cast<LPVOID>(this)
    );
    if ( nullptr == hwnd )
    {
        ShowLastError(class_name);
    }
    else
    {
        ::UpdateWindow(hwnd);
    }

    return hwnd;
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::Destroy()
{
    if ( nullptr == m_hwnd ) { return; }

    ::DestroyWindow(m_hwnd);
    m_hwnd = nullptr;
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::Close()
{
    ::SendMessage(m_hwnd, WM_CLOSE, 0, 0);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::Bounds
(
    INT32 x, INT32 y, INT32 w, INT32 h
)
{
    AdjustRect(m_hwnd, &w, &h);

    ::SetWindowPos
    (
        m_hwnd, nullptr,
        x, y, w, h,
        SWP_NOZORDER | SWP_FRAMECHANGED
    );
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::Hide()
{
    ::ShowWindowAsync(m_hwnd, SW_HIDE);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::Move
(
    INT32 x, INT32 y
)
{
    ::SetWindowPos
    (
        m_hwnd, nullptr,
        x, y, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
    );
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::Refresh()
{
    ::InvalidateRect(m_hwnd, nullptr, FALSE);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::Resize
(
    INT32 w, INT32 h
)
{
    AdjustRect(m_hwnd, &w, &h);

    ::SetWindowPos
    (
        m_hwnd, nullptr,
        0, 0, w, h,
        SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED
    );
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::Show()
{
    ::ShowWindowAsync(m_hwnd, SW_SHOWNORMAL);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::ToCenter()
{
    RECT rc;
    INT32 mx, my, mw, mh;

    if ( const auto parent = GetParent() )
    {
        ::GetWindowRect(parent, &rc);
        mx = rc.left;
        my = rc.top;
        mw = rc.right  - rc.left;
        mh = rc.bottom - rc.top;
    }
    else
    {
        GetRectForMonitorUnderCursor(&rc);
        mx = rc.left;
        my = rc.top;
        mw = rc.right  - rc.left;
        mh = rc.bottom - rc.top;
    }

    ::GetClientRect(m_hwnd, &rc);
    const auto w = rc.right  - rc.left;
    const auto h = rc.bottom - rc.top;

    const auto x = (mw - w) / 2 + mx;
    const auto y = (mh - h) / 2 + my;

    return Move(x, y);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::ToggleFullScreen()
{
    m_is_fullscreen = ! m_is_fullscreen;

    if ( m_is_fullscreen )
    {
        m_style_old = GetStyle();
        SetStyle(WS_VISIBLE | WS_POPUP | WS_MINIMIZEBOX);

        ::GetWindowRect(m_hwnd, &m_rc_old);

        RECT rc;
        GetRectForMonitorUnderCursor(&rc);
        const auto cx = rc.left;
        const auto cy = rc.top;
        const auto cw = rc.right  - rc.left;
        const auto ch = rc.bottom - rc.top;

        ::SetWindowPos
        (
            m_hwnd, nullptr,
            cx, cy, cw, ch,
            SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW
        );
    }
    else
    {
        SetStyle(m_style_old);

        const auto cx = m_rc_old.left;
        const auto cy = m_rc_old.top;
        const auto cw = m_rc_old.right  - m_rc_old.left;
        const auto ch = m_rc_old.bottom - m_rc_old.top;

        ::SetWindowPos
        (
            m_hwnd, nullptr,
            cx, cy, cw, ch,
            SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW
        );
    }
}

//---------------------------------------------------------------------------//

LRESULT tapetums::UWnd::Send
(
    UINT uMsg, WPARAM wp, LPARAM lp
)
{
    return ::SendMessage(m_hwnd, uMsg, wp, lp);
}

//---------------------------------------------------------------------------//

LRESULT tapetums::UWnd::Post
(
    UINT uMsg, WPARAM wp, LPARAM lp
)
{
    return ::PostMessage(m_hwnd, uMsg, wp, lp);
}

//---------------------------------------------------------------------------//

DWORD tapetums::UWnd::GetStyle() const noexcept
{
    return (DWORD)::GetWindowLongPtr(m_hwnd, GWL_STYLE);
}

//---------------------------------------------------------------------------//

DWORD tapetums::UWnd::GetStyleEx() const noexcept
{
    return (DWORD)::GetWindowLongPtr(m_hwnd, GWL_EXSTYLE);
}

//---------------------------------------------------------------------------//

HWND tapetums::UWnd::GetParent() const noexcept
{
    return (HWND)::GetWindowLongPtr(m_hwnd, GWLP_HWNDPARENT);
}

//---------------------------------------------------------------------------//

HFONT tapetums::UWnd::GetFont() const noexcept
{
    return (HFONT)::SendMessage(m_hwnd, WM_GETFONT, 0, 0);
}

//---------------------------------------------------------------------------//

HICON tapetums::UWnd::GetWindowIcon() const noexcept
{
    return (HICON)::GetClassLongPtr(m_hwnd, GCLP_HICON);
}

//---------------------------------------------------------------------------//

HICON tapetums::UWnd::GetWindowIconSm() const noexcept
{
    return (HICON)::GetClassLongPtr(m_hwnd, GCLP_HICONSM);
}

//---------------------------------------------------------------------------//

SIZE_T tapetums::UWnd::GetText
(
    TCHAR* buf, SIZE_T buf_size
)
const noexcept
{
    return (SIZE_T)::SendMessage
    (
        m_hwnd, WM_GETTEXT, (WPARAM)buf_size, (LPARAM)buf
    );
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::SetStyle
(
    DWORD style
)
noexcept
{
    ::SetWindowLongPtr(m_hwnd, GWL_STYLE, (LONG_PTR)style);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::SetStyleEx
(
    DWORD styleEx
)
noexcept
{
    ::SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, (LONG_PTR)styleEx);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::SetParent
(
    HWND parent
)
noexcept
{
    ::SetWindowLongPtr(m_hwnd, GWLP_HWNDPARENT, (LONG_PTR)parent);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::SetFont
(
    HFONT font
)
noexcept
{
    ::SendMessage(m_hwnd, WM_SETFONT, (WPARAM)font, (LPARAM)FALSE);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::SetWindowIcon
(
    HMODULE hInst, LPCTSTR lpszIconName
)
noexcept
{
    const auto hIcon = (HICON)::LoadImage
    (
        hInst, lpszIconName,
        IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED
    );
    
    const auto hIconSm = (HICON)::LoadImage
    (
        hInst, lpszIconName,
        IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED
    );

    return SetWindowIcon(hIcon, hIconSm);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::SetWindowIcon
(
    HICON hIcon, HICON hIconSm
)
noexcept
{
    ::SetClassLongPtr(m_hwnd, GCLP_HICON,   (LONG_PTR)hIcon);
    ::SetClassLongPtr(m_hwnd, GCLP_HICONSM, (LONG_PTR)hIconSm);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::SetWindowIcon
(
    HICON hIcon
)
noexcept
{
    ::SetClassLongPtr(m_hwnd, GCLP_HICON, (LONG_PTR)hIcon);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::SetWindowIconSm
(
    HICON hIconSm
)
noexcept
{
    ::SetClassLongPtr(m_hwnd, GCLP_HICONSM, (LONG_PTR)hIconSm);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::SetText
(
    LPCTSTR txt
)
noexcept
{
    ::SendMessage(m_hwnd, WM_SETTEXT, 0, (LPARAM)txt);
}

//---------------------------------------------------------------------------//

bool tapetums::UWnd::AddNotifyIcon
(
    UINT uID, HICON hIcon
)
{
    if ( WM_TASKBARCREATED == 0 )
    {
        WM_TASKBARCREATED = ::RegisterWindowMessage(TEXT("TaskbarCreated"));
    }
    if ( WM_NOTIFYICON == 0 )
    {
        WM_NOTIFYICON = ::RegisterWindowMessage(TEXT("NotifyIcon"));
    }

    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd   = m_hwnd;
    nid.uID    = uID;
    nid.uFlags = NIF_MESSAGE | NIF_ICON;
    nid.uCallbackMessage = WM_NOTIFYICON;
    nid.hIcon  = hIcon;

    for ( ; ; )
    {
        if ( ::Shell_NotifyIcon(NIM_ADD, &nid) )
        {
            return true;
        }

        // 以下、スタートアッププログラムとして起動した際
        // タイミングによってアイコンの登録が失敗する問題への対策
        // Microsoft: KB418138
        if ( ::GetLastError() != ERROR_TIMEOUT )
        {
            ::MessageBox
            (
                nullptr, nullptr, TEXT("Failed to add notify icon"),
                MB_ICONEXCLAMATION | MB_OK
            );
            return false;
        }

        if ( ::Shell_NotifyIcon(NIM_MODIFY, &nid) )
        {
            return true;
        }
        else
        {
            ::Sleep(1000);
        }
    }
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::DeleteNotifyIcon
(
    UINT uID
)
{
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd   = m_hwnd;
    nid.uID    = uID;
    nid.uFlags = 0;

    ::Shell_NotifyIcon(NIM_DELETE, &nid);
}

//---------------------------------------------------------------------------//

void tapetums::UWnd::SetNotifyIconTip
(
    UINT uID, LPCTSTR szTip
)
{
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd   = m_hwnd;
    nid.uID    = uID;
    nid.uFlags = NIF_TIP;
#if (NTDDI_VERSION < NTDDI_WIN2K)
    ::StringCchCopy(nid.szTip, 64, szTip);
#else
    ::StringCchCopy(nid.szTip, 128, szTip);
#endif

    ::Shell_NotifyIcon(NIM_MODIFY, &nid);
}

//---------------------------------------------------------------------------//

bool tapetums::UWnd::ShowNotifyIconInfo
(
    UINT uID, DWORD dwInfoFlags,
    LPCTSTR szInfoTitle, LPCTSTR szInfo, UINT uTimeout
)
{
#if NTDDI_VERSION < NTDDI_WIN2K
    return false;
#endif

    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd   = m_hwnd;
    nid.uID    = uID;
    nid.uFlags = NIF_INFO;
    ::StringCchCopy(nid.szInfo, 256, szInfo);
    nid.uTimeout = uTimeout;
    ::StringCchCopy(nid.szInfoTitle, 64, szInfoTitle);
    nid.dwInfoFlags = dwInfoFlags;

    ::Shell_NotifyIcon(NIM_MODIFY, &nid);

    return true;
}

//---------------------------------------------------------------------------//
// UWnd ウィンドウプロシージャ
//---------------------------------------------------------------------------//

LRESULT tapetums::UWnd::WndProc
(
    HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp
)
{
    if ( uMsg == WM_DESTROY )
    {
        ::PostQuitMessage(0);
    }
    else if ( uMsg == WM_LBUTTONDBLCLK )
    {
        this->ToggleFullScreen();
    }

    return ::DefWindowProc(hwnd, uMsg, wp, lp);
}

//---------------------------------------------------------------------------//

// UWnd.cpp