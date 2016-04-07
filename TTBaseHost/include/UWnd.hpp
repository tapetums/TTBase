#pragma once

//---------------------------------------------------------------------------//
//
// UWnd.hpp
//  ウィンドウをカプセル化するクラス
//   Copyright (C) 2005-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <utility>

#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>

#pragma comment(lib, "shell32.lib") // Shell_NotifyIcon

#ifndef WM_DPICHANGED
  #define WM_DPICHANGED 0x02E0
#endif

//---------------------------------------------------------------------------//
// グローバル変数
//---------------------------------------------------------------------------//

extern UINT WM_NOTIFYICON;     // 使用の際は実体定義が必要 (WinMain.cpp 内を推奨)
extern UINT WM_TASKBARCREATED; // 使用の際は実体定義が必要 (WinMain.cpp 内を推奨)

//---------------------------------------------------------------------------//
// 前方宣言
//---------------------------------------------------------------------------//

namespace tapetums
{
    class UWnd;

    inline void ShowLastError    (LPCTSTR window_title);
    inline void AdjustRect       (HWND hwnd, INT32* w, INT32* h);
    inline void GetRectForMonitor(HWND hwnd, RECT* rect);
    inline void GetRectForMonitor(const POINT& pt, RECT* rect);
    inline void GetDpiForMonitor (HWND hwnd, POINT* dpi);
    inline void GetDpiForMonitor (const POINT& pt,POINT* dpi);
}

//---------------------------------------------------------------------------//
// クラス
//---------------------------------------------------------------------------//

// ウィンドウの基底クラス
class tapetums::UWnd
{
private: // static members
    static constexpr LPCTSTR class_name { TEXT("UWnd") };

protected: // members
    INT32   m_x             { CW_USEDEFAULT };
    INT32   m_y             { CW_USEDEFAULT };
    INT32   m_w             { CW_USEDEFAULT };
    INT32   m_h             { CW_USEDEFAULT };
    LPCTSTR m_class_name    { class_name };
    HWND    m_hwnd          { nullptr };
    bool    m_is_fullscreen { false };
    DWORD   m_style_old     { 0 };
    RECT    m_rc_old        { };

public: // accessors
    INT32 left()          const noexcept { return m_x; }
    INT32 top()           const noexcept { return m_y; }
    INT32 width()         const noexcept { return m_w; }
    INT32 height()        const noexcept { return m_h; }
    POINT position()      const noexcept { return { m_x, m_y }; }
    SIZE  size()          const noexcept { return { m_w, m_h }; }
    RECT  rect()          const noexcept { return { m_w, m_h, m_x + m_w, m_y + m_h }; }
    HWND  handle()        const noexcept { return m_hwnd; }
    bool  is_fullscreen() const noexcept { return m_is_fullscreen; }

public: // ctor / dtor
    UWnd();
    virtual ~UWnd() { Destroy(); }

    UWnd(const UWnd&)             = delete;
    UWnd& operator= (const UWnd&) = delete;

    UWnd(UWnd&& rhs)             noexcept { swap(std::move(rhs)); }
    UWnd& operator =(UWnd&& rhs) noexcept { swap(std::move(rhs)); return *this; }

public: // mover
    void swap(UWnd && rhs) noexcept;

public: // operator
    operator HWND() { return m_hwnd; }

public: // methods
    ATOM WINAPI Register        (LPCTSTR lpszClassName);
    HWND WINAPI Create          (LPCTSTR lpszWindowName, DWORD style, DWORD styleEx, HWND hwndParent, HMENU hMenu);
    void WINAPI Destroy         ();
    void WINAPI Close           ();
    void WINAPI Bounds          (INT32 x, INT32 y, INT32 w, INT32 h);
    void WINAPI Hide            ();
    void WINAPI Move            (INT32 x, INT32 y);
    void WINAPI Refresh         ();
    void WINAPI Resize          (INT32 w, INT32 h);
    void WINAPI Show            ();
    void WINAPI ToCenter        ();
    void WINAPI ToggleFullScreen();

    bool WINAPI AddNotifyIcon     (UINT uID, HICON hIcon);
    void WINAPI DeleteNotifyIcon  (UINT uID);
    void WINAPI SetNotifyIconTip  (UINT uID, LPCTSTR szTip);
    bool WINAPI ShowNotifyIconInfo(UINT uID, DWORD dwInfoFlags, LPCTSTR szInfoTitle, LPCTSTR szInfo, UINT uTimeout = -1);

    LRESULT WINAPI Send(UINT msg, WPARAM wp, LPARAM lp);
    LRESULT WINAPI Post(UINT msg, WPARAM wp, LPARAM lp);

public: // properties
    DWORD  WINAPI GetStyle       () const noexcept;
    DWORD  WINAPI GetStyleEx     () const noexcept;
    HWND   WINAPI GetParent      () const noexcept;
    HFONT  WINAPI GetFont        () const noexcept;
    HICON  WINAPI GetWindowIcon  () const noexcept;
    HICON  WINAPI GetWindowIconSm() const noexcept;
    SIZE_T WINAPI GetText(TCHAR* buf, SIZE_T buf_size) const noexcept;

    void WINAPI SetStyle       (DWORD style)                         noexcept;
    void WINAPI SetStyleEx     (DWORD styleEx)                       noexcept;
    void WINAPI SetParent      (HWND parent)                         noexcept;
    void WINAPI SetFont        (HFONT font)                          noexcept;
    void WINAPI SetWindowIcon  (HMODULE hInst, LPCTSTR lpszIconName) noexcept;
    void WINAPI SetWindowIcon  (HICON hIcon, HICON hIconSm)          noexcept;
    void WINAPI SetWindowIcon  (HICON hIcon)                         noexcept;
    void WINAPI SetWindowIconSm(HICON hIconSm)                       noexcept;
    void WINAPI SetText        (LPCTSTR txt)                         noexcept;

public: // window procedures
    static  LRESULT CALLBACK WndMapProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    virtual LRESULT CALLBACK WndProc   (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
};

//---------------------------------------------------------------------------//
// ユーティリティ関数
//---------------------------------------------------------------------------//

// エラーをメッセージボックスで表示する
inline void tapetums::ShowLastError
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

// クライアントサイズが指定の大きさになるよう計算する
inline void tapetums::AdjustRect
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

// ウィンドウのあるモニタのサイズを取得する
inline void tapetums::GetRectForMonitor
(
    HWND hwnd, RECT* rect
)
{
    const auto hMonitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    MONITORINFOEX miex{ };
    miex.cbSize = (DWORD)sizeof(MONITORINFOEX);
    ::GetMonitorInfo(hMonitor, &miex);

    *rect = miex.rcMonitor;
}

// 画面上のある点におけるモニタのサイズを取得する
inline void tapetums::GetRectForMonitor
(
    const POINT& pt, RECT* rect
)
{
    const auto hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

    MONITORINFOEX miex{ };
    miex.cbSize = (DWORD)sizeof(MONITORINFOEX);
    ::GetMonitorInfo(hMonitor, &miex);

    *rect = miex.rcMonitor;
}

//---------------------------------------------------------------------------//

#include <ShellScalingApi.h>

// ウィンドウのあるモニタの解像度を取得する
inline void tapetums::GetDpiForMonitor
(
    HWND hwnd, POINT* dpi
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
        const auto hMonitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

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

// 画面上のある点におけるモニタの解像度を取得する
inline void tapetums::GetDpiForMonitor
(
    const POINT& pt, POINT* dpi
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
// UWnd コンストラクタ
//---------------------------------------------------------------------------//

inline tapetums::UWnd::UWnd()
{
    // 自動的に "UWnd" ウィンドウクラスを登録
    static ATOM atom { 0 };
    if ( atom == 0 ) { atom = Register(class_name); }
}

//---------------------------------------------------------------------------//
// UWnd ムーバー
//---------------------------------------------------------------------------//

inline void tapetums::UWnd::swap(UWnd&& rhs) noexcept
{
    std::swap(m_x,             rhs.m_x);
    std::swap(m_y,             rhs.m_y);
    std::swap(m_w,             rhs.m_w);
    std::swap(m_h,             rhs.m_h);
    std::swap(m_class_name,    rhs.m_class_name);
    std::swap(m_hwnd,          rhs.m_hwnd);
    std::swap(m_is_fullscreen, rhs.m_is_fullscreen);
    std::swap(m_style_old,     rhs.m_style_old);
    std::swap(m_rc_old,        rhs.m_rc_old);
}

//---------------------------------------------------------------------------//
// UWnd メソッド
//---------------------------------------------------------------------------//

// ウィンドウクラスを登録する
inline ATOM WINAPI tapetums::UWnd::Register
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

// ウィンドウを生成する
inline HWND WINAPI tapetums::UWnd::Create
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

// ウィンドウを破棄する
inline void WINAPI tapetums::UWnd::Destroy()
{
    if ( nullptr == m_hwnd ) { return; }

    ::DestroyWindow(m_hwnd);
    m_hwnd = nullptr;
}

//---------------------------------------------------------------------------//

// ウィンドウを閉じる
inline void WINAPI tapetums::UWnd::Close()
{
    ::SendMessage(m_hwnd, WM_CLOSE, 0, 0);
}

//---------------------------------------------------------------------------//

// ウィンドウの位置とサイズを設定する
inline void WINAPI tapetums::UWnd::Bounds
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

// ウィンドウを隠す
inline void WINAPI tapetums::UWnd::Hide()
{
    ::ShowWindowAsync(m_hwnd, SW_HIDE);
}

//---------------------------------------------------------------------------//

// ウィンドウを移動する
inline void WINAPI tapetums::UWnd::Move
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

// ウィンドウを再描画する
inline void WINAPI tapetums::UWnd::Refresh()
{
    ::InvalidateRect(m_hwnd, nullptr, FALSE);
}

//---------------------------------------------------------------------------//

// ウィンドウのサイズを設定する
inline void WINAPI tapetums::UWnd::Resize
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

// ウィンドウを表示する
inline void WINAPI tapetums::UWnd::Show()
{
    ::ShowWindowAsync(m_hwnd, SW_SHOWNORMAL);
}

//---------------------------------------------------------------------------//

// ウィンドウを画面中央に移動する
inline void WINAPI tapetums::UWnd::ToCenter()
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
        POINT pt;
        ::GetCursorPos(&pt);

        GetRectForMonitor(pt, &rc);
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

// ウィンドウの全画面表示を切り替える
inline void WINAPI tapetums::UWnd::ToggleFullScreen()
{
    m_is_fullscreen = ! m_is_fullscreen;

    if ( m_is_fullscreen )
    {
        // ウィンドウスタイルを変更
        m_style_old = GetStyle();
        SetStyle(WS_VISIBLE | WS_POPUP | WS_MINIMIZEBOX);

        // ウィンドウサイズを記憶
        ::GetWindowRect(m_hwnd, &m_rc_old);

        // モニタのサイズを取得
        POINT pt;
        ::GetCursorPos(&pt);

        RECT rc;
        GetRectForMonitor(pt, &rc);
        const auto cx = rc.left;
        const auto cy = rc.top;
        const auto cw = rc.right  - rc.left;
        const auto ch = rc.bottom - rc.top;

        // 全画面にする
        ::SetWindowPos
        (
            m_hwnd, nullptr,
            cx, cy, cw, ch,
            SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW
        );
    }
    else
    {
        // ウィンドウスタイルを復帰
        SetStyle(m_style_old);

        // ウィンドウサイズを復帰
        const auto cx = m_rc_old.left;
        const auto cy = m_rc_old.top;
        const auto cw = m_rc_old.right  - m_rc_old.left;
        const auto ch = m_rc_old.bottom - m_rc_old.top;

        // ウィンドウの位置を復帰
        ::SetWindowPos
        (
            m_hwnd, nullptr,
            cx, cy, cw, ch,
            SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW
        );
    }
}

//---------------------------------------------------------------------------//

// タスクトレイアイコンを設定する
inline bool WINAPI tapetums::UWnd::AddNotifyIcon
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

// タスクトレイアイコンを削除する
inline void WINAPI tapetums::UWnd::DeleteNotifyIcon
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

// タスクトレイアイコンにツールチップを表示
inline void WINAPI tapetums::UWnd::SetNotifyIconTip
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

// タスクトレイアイコンにバルーンを表示
inline bool WINAPI tapetums::UWnd::ShowNotifyIconInfo
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

// ウィンドウにメッセージを送る
inline LRESULT WINAPI tapetums::UWnd::Send
(
    UINT msg, WPARAM wp, LPARAM lp
)
{
    return ::SendMessage(m_hwnd, msg, wp, lp);
}

//---------------------------------------------------------------------------//

// ウィンドウにメッセージを送る
inline LRESULT WINAPI tapetums::UWnd::Post
(
    UINT msg, WPARAM wp, LPARAM lp
)
{
    return ::PostMessage(m_hwnd, msg, wp, lp);
}

//---------------------------------------------------------------------------//
// UWnd プロパティ
//---------------------------------------------------------------------------//

inline DWORD WINAPI tapetums::UWnd::GetStyle() const noexcept
{
    return (DWORD)::GetWindowLongPtr(m_hwnd, GWL_STYLE);
}

//---------------------------------------------------------------------------//

inline DWORD WINAPI tapetums::UWnd::GetStyleEx() const noexcept
{
    return (DWORD)::GetWindowLongPtr(m_hwnd, GWL_EXSTYLE);
}

//---------------------------------------------------------------------------//

inline HWND WINAPI tapetums::UWnd::GetParent() const noexcept
{
    return (HWND)::GetWindowLongPtr(m_hwnd, GWLP_HWNDPARENT);
}

//---------------------------------------------------------------------------//

inline HFONT WINAPI tapetums::UWnd::GetFont() const noexcept
{
    return (HFONT)::SendMessage(m_hwnd, WM_GETFONT, 0, 0);
}

//---------------------------------------------------------------------------//

inline HICON WINAPI tapetums::UWnd::GetWindowIcon() const noexcept
{
    return (HICON)::GetClassLongPtr(m_hwnd, GCLP_HICON);
}

//---------------------------------------------------------------------------//

inline HICON WINAPI tapetums::UWnd::GetWindowIconSm() const noexcept
{
    return (HICON)::GetClassLongPtr(m_hwnd, GCLP_HICONSM);
}

//---------------------------------------------------------------------------//

inline SIZE_T WINAPI tapetums::UWnd::GetText
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

inline void WINAPI tapetums::UWnd::SetStyle
(
    DWORD style
)
noexcept
{
    ::SetWindowLongPtr(m_hwnd, GWL_STYLE, (LONG_PTR)style);
}

//---------------------------------------------------------------------------//

inline void WINAPI tapetums::UWnd::SetStyleEx
(
    DWORD styleEx
)
noexcept
{
    ::SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, (LONG_PTR)styleEx);
}

//---------------------------------------------------------------------------//

inline void WINAPI tapetums::UWnd::SetParent
(
    HWND parent
)
noexcept
{
    ::SetWindowLongPtr(m_hwnd, GWLP_HWNDPARENT, (LONG_PTR)parent);
}

//---------------------------------------------------------------------------//

inline void WINAPI tapetums::UWnd::SetFont
(
    HFONT font
)
noexcept
{
    ::SendMessage(m_hwnd, WM_SETFONT, (WPARAM)font, (LPARAM)FALSE);
}

//---------------------------------------------------------------------------//

inline void WINAPI tapetums::UWnd::SetWindowIcon
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

inline void WINAPI tapetums::UWnd::SetWindowIcon
(
    HICON hIcon, HICON hIconSm
)
noexcept
{
    ::SetClassLongPtr(m_hwnd, GCLP_HICON,   (LONG_PTR)hIcon);
    ::SetClassLongPtr(m_hwnd, GCLP_HICONSM, (LONG_PTR)hIconSm);
}

//---------------------------------------------------------------------------//

inline void WINAPI tapetums::UWnd::SetWindowIcon
(
    HICON hIcon
)
noexcept
{
    ::SetClassLongPtr(m_hwnd, GCLP_HICON, (LONG_PTR)hIcon);
}

//---------------------------------------------------------------------------//

inline void WINAPI tapetums::UWnd::SetWindowIconSm
(
    HICON hIconSm
)
noexcept
{
    ::SetClassLongPtr(m_hwnd, GCLP_HICONSM, (LONG_PTR)hIconSm);
}

//---------------------------------------------------------------------------//

inline void WINAPI tapetums::UWnd::SetText
(
    LPCTSTR txt
)
noexcept
{
    ::SendMessage(m_hwnd, WM_SETTEXT, 0, (LPARAM)txt);
}

//---------------------------------------------------------------------------//
// UWnd ウィンドウプロシージャ
//---------------------------------------------------------------------------//

// UWnd 静的ウィンドウプロシージャ
inline LRESULT CALLBACK tapetums::UWnd::WndMapProc
(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp
)
{
    UWnd* wnd;

    // UWndオブジェクトのポインタを取得
    if ( msg == WM_NCCREATE )
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
        return ::DefWindowProc(hwnd, msg, wp, lp);
    }

    // メンバ変数に情報を保存
    switch ( msg )
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
    return wnd->WndProc(hwnd, msg, wp, lp);
}

//---------------------------------------------------------------------------//

// UWnd ウィンドウプロシージャ
inline LRESULT CALLBACK tapetums::UWnd::WndProc
(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp
)
{
    return ::DefWindowProc(hwnd, msg, wp, lp);
}

//---------------------------------------------------------------------------//

// UWnd.hpp