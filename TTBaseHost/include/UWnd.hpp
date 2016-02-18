#pragma once

//---------------------------------------------------------------------------//
//
// UWnd.hpp
//  ウィンドウをカプセル化するクラス
//   Copyright (C) 2005-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>

//---------------------------------------------------------------------------//
// グローバル変数
//---------------------------------------------------------------------------//

extern UINT WM_NOTIFYICON;
extern UINT WM_TASKBARCREATED;

//---------------------------------------------------------------------------//
// 前方宣言
//---------------------------------------------------------------------------//

namespace tapetums
{
    class UWnd;

    void ShowLastError(LPCTSTR window_title);
    void AdjustRect(HWND hwnd, INT32* w, INT32* h);
    void GetRectForMonitorUnderCursor(RECT* rect);
    void GetDpiForMonitorUnderCursor(POINT* dpi);
}

//---------------------------------------------------------------------------//
// ウィンドウの基底クラス
//---------------------------------------------------------------------------//

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

    UWnd(UWnd&& rhs)             noexcept = default;
    UWnd& operator =(UWnd&& rhs) noexcept = default;

public: // operator
    operator HWND() { return m_hwnd; }

public: // static methods
    static LRESULT CALLBACK WndMapProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);

public: // methods
    ATOM WINAPI Register(LPCTSTR lpszClassName);
    HWND WINAPI Create(LPCTSTR lpszWindowName, DWORD style, DWORD styleEx, HWND hwndParent, HMENU hMenu);
    void WINAPI Destroy();
    void WINAPI Close();
    void WINAPI Bounds(INT32 x, INT32 y, INT32 w, INT32 h);
    void WINAPI Hide();
    void WINAPI Move(INT32 x, INT32 y);
    void WINAPI Refresh();
    void WINAPI Resize(INT32 w, INT32 h);
    void WINAPI Show();
    void WINAPI ToCenter();
    void WINAPI ToggleFullScreen();

    LRESULT WINAPI Send(UINT uMsg, WPARAM wp, LPARAM lp);
    LRESULT WINAPI Post(UINT uMsg, WPARAM wp, LPARAM lp);

    DWORD  WINAPI GetStyle()        const noexcept;
    DWORD  WINAPI GetStyleEx()      const noexcept;
    HWND   WINAPI GetParent()       const noexcept;
    HFONT  WINAPI GetFont()         const noexcept;
    HICON  WINAPI GetWindowIcon()   const noexcept;
    HICON  WINAPI GetWindowIconSm() const noexcept;
    SIZE_T WINAPI GetText(TCHAR* buf, SIZE_T buf_size) const noexcept;

    void WINAPI SetStyle(DWORD style)                              noexcept;
    void WINAPI SetStyleEx(DWORD styleEx)                          noexcept;
    void WINAPI SetParent(HWND parent)                             noexcept;
    void WINAPI SetFont(HFONT font)                                noexcept;
    void WINAPI SetWindowIcon(HMODULE hInst, LPCTSTR lpszIconName) noexcept;
    void WINAPI SetWindowIcon(HICON hIcon, HICON hIconSm)          noexcept;
    void WINAPI SetWindowIcon(HICON hIcon)                         noexcept;
    void WINAPI SetWindowIconSm(HICON hIconSm)                     noexcept;
    void WINAPI SetText(LPCTSTR txt)                               noexcept;

    bool WINAPI AddNotifyIcon(UINT uID, HICON hIcon);
    void WINAPI DeleteNotifyIcon(UINT uID);
    void WINAPI SetNotifyIconTip(UINT uID, LPCTSTR szTip);
    bool WINAPI ShowNotifyIconInfo(UINT uID, DWORD dwInfoFlags, LPCTSTR szInfoTitle, LPCTSTR szInfo, UINT uTimeout = -1);

public: // window procedures
    virtual LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);
};

//---------------------------------------------------------------------------//

// UWnd.hpp