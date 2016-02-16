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
    ATOM Register(LPCTSTR lpszClassName);
    HWND Create(LPCTSTR lpszWindowName, DWORD style, DWORD styleEx, HWND hwndParent, HMENU hMenu);
    void Destroy();
    void Close();
    void Bounds(INT32 x, INT32 y, INT32 w, INT32 h);
    void Hide();
    void Move(INT32 x, INT32 y);
    void Refresh();
    void Resize(INT32 w, INT32 h);
    void Show();
    void ToCenter();
    void ToggleFullScreen();

    LRESULT Send(UINT uMsg, WPARAM wp, LPARAM lp);
    LRESULT Post(UINT uMsg, WPARAM wp, LPARAM lp);

    DWORD  GetStyle()        const noexcept;
    DWORD  GetStyleEx()      const noexcept;
    HWND   GetParent()       const noexcept;
    HFONT  GetFont()         const noexcept;
    HICON  GetWindowIcon()   const noexcept;
    HICON  GetWindowIconSm() const noexcept;
    SIZE_T GetText(TCHAR* buf, SIZE_T buf_size) const noexcept;

    void SetStyle(DWORD style)                              noexcept;
    void SetStyleEx(DWORD styleEx)                          noexcept;
    void SetParent(HWND parent)                             noexcept;
    void SetFont(HFONT font)                                noexcept;
    void SetWindowIcon(HMODULE hInst, LPCTSTR lpszIconName) noexcept;
    void SetWindowIcon(HICON hIcon, HICON hIconSm)          noexcept;
    void SetWindowIcon(HICON hIcon)                         noexcept;
    void SetWindowIconSm(HICON hIconSm)                     noexcept;
    void SetText(LPCTSTR txt)                               noexcept;

    bool AddNotifyIcon(UINT uID, HICON hIcon);
    void DeleteNotifyIcon(UINT uID);
    void SetNotifyIconTip(UINT uID, LPCTSTR szTip);
    bool ShowNotifyIconInfo(UINT uID, DWORD dwInfoFlags, LPCTSTR szInfoTitle, LPCTSTR szInfo, UINT uTimeout = -1);

public: // window procedures
    virtual LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);
};

//---------------------------------------------------------------------------//

// UWnd.hpp