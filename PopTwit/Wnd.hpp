//---------------------------------------------------------------------------//
//
// Wnd.hpp
//  ウィンドウ操作 ヘルパ関数
//   Copyright (C) 2014 tapetums
//
//---------------------------------------------------------------------------//

#pragma once

//---------------------------------------------------------------------------//
// グローバル変数
//---------------------------------------------------------------------------//

extern HINSTANCE g_hInst;

extern UINT WM_NOTIFYICON;
extern UINT WM_TASKBARCREATED;

extern HICON g_hIcon;
extern HICON g_hIconSm;

//---------------------------------------------------------------------------//
// ウィンドウ操作ヘルパ関数の定義
//---------------------------------------------------------------------------//

namespace Wnd
{
    ATOM __stdcall Register
    (
        LPCTSTR lpszClassName = nullptr,
        WNDPROC lpfnWndProc   = nullptr,
        LPCTSTR lpszIconName  = nullptr
    );
    HWND __stdcall Create
    (
        LPCTSTR lpszClassName,
        LPCTSTR lpWindowName = nullptr,
        DWORD   style        = WS_OVERLAPPEDWINDOW,
        DWORD   styleEx      = 0,
        HWND    hwndParent   = nullptr,
        HMENU   hMenu        = nullptr
    );
    void __stdcall ShowLastError();
    void __stdcall AdjustRect(HWND hwnd, INT32& w, INT32& h);
    bool __stdcall Bounds(HWND hwnd, INT32 x, INT32 y, INT32 w, INT32 h);
    bool __stdcall Move(HWND hwnd, INT32 x, INT32 y);
    bool __stdcall Refresh(HWND hwnd, BOOL bErase = FALSE);
    bool __stdcall Resize(HWND hwnd, INT32 w, INT32 h);
    bool __stdcall Show(HWND hwnd, INT32 nCmdShow = SW_SHOWNORMAL);
    void __stdcall GetMonitorRectUnderCursor(RECT* rect);
    bool __stdcall ToCenter(HWND hwnd);
    bool __stdcall IsCompositionEnabled();
    void __stdcall EnableAero(HWND hwnd);
    void __stdcall DisableAero(HWND hwnd);
}

//---------------------------------------------------------------------------//

// Wnd.hpp