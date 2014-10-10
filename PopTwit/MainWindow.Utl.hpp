//---------------------------------------------------------------------------//
//
// MainWindow.Utl.hpp
//  ユーティリティ関数
//   Copyright (C) 2014 tapetums
//
//---------------------------------------------------------------------------//

#pragma once

//---------------------------------------------------------------------------//

#ifdef _NODEFLIB
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#undef StringCchPrintf
#define StringCchPrintf wnsprintf
#endif

//---------------------------------------------------------------------------//
// リソース管理まわり
//---------------------------------------------------------------------------//

HFONT __stdcall MakeFont(INT32 font_size, LPCTSTR font_name)
{
    const auto hFont = ::CreateFont
    (
        font_size, 0, 0, 0,
        FW_REGULAR, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
        font_name
    );

    return hFont;
}

//---------------------------------------------------------------------------//

bool __stdcall DeleteFont(HFONT hFont)
{
    if ( hFont == nullptr )
    {
        return false;
    }

    ::DeleteObject(hFont);
    hFont = nullptr;

    return true;
}

//---------------------------------------------------------------------------//

void __stdcall ClearMessagestub(LPTSTR msgstub)
{
    memset(msgstub, 0, MAX_MESSAGE_LEN * sizeof(TCHAR));
}

//---------------------------------------------------------------------------//

bool __stdcall LoadMessageStub(LPTSTR msgstub)
{
    TCHAR path[MAX_PATH];
    ::SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path);

    TCHAR buf[MAX_PATH];
    ::StringCchPrintf(buf, MAX_PATH, TEXT("%s\\%s\\stub"), path, PLUGIN_NAME);

    const auto hFile = ::CreateFile
    (
        buf, GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if ( hFile == nullptr || hFile == INVALID_HANDLE_VALUE )
    {
        return false;
    }

    DWORD dw;
    ::ReadFile(hFile, msgstub, MAX_PATH, &dw, nullptr);

    ::CloseHandle(hFile);

    return (dw > 0) ? true : false;
}

//---------------------------------------------------------------------------//

bool __stdcall SaveMessageStub(LPCTSTR msgstub)
{
    TCHAR path[MAX_PATH];
    ::SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path);

    TCHAR buf[MAX_PATH];
    ::StringCchPrintf(buf, MAX_PATH, TEXT("%s\\%s\\stub"), path, PLUGIN_NAME);

    const auto hFile = ::CreateFile
    (
        buf, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if ( hFile == nullptr || hFile == INVALID_HANDLE_VALUE )
    {
        return false;
    }

    DWORD dw;
    ::WriteFile(hFile, msgstub, MAX_MESSAGE_LEN * sizeof(TCHAR), &dw, nullptr);

    ::CloseHandle(hFile);

    return (dw > 0) ? true : false;
}

//---------------------------------------------------------------------------//
// Windows コントロールまわり
//---------------------------------------------------------------------------//

void __stdcall SetUserNamesToComboBox(HWND cbx_user)
{
    ::SendMessage(cbx_user, CB_SETCURSEL, 0, 0);
    for ( size_t index = 0; index < MAX_ACCOUNT; ++index )
    {
        const auto username = g_username[index];
        ::SendMessage(cbx_user, CB_ADDSTRING, index, (LPARAM)username);

        if ( index == g_user_index )
        {
            ::SendMessage(cbx_user, CB_SETCURSEL, index, 0);
        }
    }
};

//---------------------------------------------------------------------------//

void __stdcall ClearComboBox(HWND cbx_user)
{
    while ( ::SendMessage(cbx_user, CB_GETCOUNT, 0, 0) > 0 )
    {
        ::SendMessage(cbx_user, CB_DELETESTRING, 0, 0);
    }
}

//---------------------------------------------------------------------------//
// サブクラス プロシージャ
//---------------------------------------------------------------------------//

LRESULT CALLBACK ComboBoxProc
(
    HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData
)
{
    if ( uMsg != WM_PAINT )
    {
        return ::DefSubclassProc(hwnd, uMsg, wp, lp);
    }

    PAINTSTRUCT ps;
    const auto hDC = ::BeginPaint(hwnd, &ps);
    const auto prc = &ps.rcPaint;

    HDC hMemDC = nullptr;
    const auto pb = ::BeginBufferedPaint
    (
        hDC, prc, BPBF_TOPDOWNDIB, 0, &hMemDC
    );
    if ( pb )
    {
        ::SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)hMemDC, PRF_CLIENT);
        ::BufferedPaintSetAlpha(pb, prc, 255);
        ::EndBufferedPaint(pb, TRUE);
    }

    ::EndPaint(hwnd, &ps);

    return 0;
};

//---------------------------------------------------------------------------//

LRESULT CALLBACK TextBoxProc
(
    HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData
)
{
    if ( uMsg == WM_KEYDOWN )
    {
        // TODO: Ctrl+Enter でも改行が入力されてしまうのを 修正する
        Wnd::Refresh(hwnd);
        OnKeyDown((HWND)dwRefData, (INT16)wp, (INT16)lp);
    }
    if ( uMsg == WM_DROPFILES )
    {
        ::MessageBox(nullptr, TEXT("未実装です"), TEXT(""), MB_OK);
        return 0; // TODO: UploadPicture() の実装
    }
    else if ( uMsg == WM_PAINT )
    {
        PAINTSTRUCT ps;
        const auto hDC = ::BeginPaint(hwnd, &ps);
        const auto prc = &ps.rcPaint;

        HDC hMemDC = nullptr;
        const auto pb = ::BeginBufferedPaint
        (
            hDC, prc, BPBF_TOPDOWNDIB, 0, &hMemDC
        );
        if ( pb )
        {
            ::FillRect(hMemDC, prc, (HBRUSH)::GetStockObject(WHITE_BRUSH));
            ::SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)hMemDC, PRF_CLIENT);
            ::BufferedPaintSetAlpha(pb, prc, 255);
            ::EndBufferedPaint(pb, TRUE);
        }

        ::EndPaint(hwnd, &ps);

        return 0;
    }

    return ::DefSubclassProc(hwnd, uMsg, wp, lp);
};

//---------------------------------------------------------------------------//

LRESULT CALLBACK ButtonProc
(
    HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData
)
{
    if ( uMsg == WM_LBUTTONUP )
    {
        OnCommand((HWND)dwRefData, (UINT16)uIdSubclass, 0);
    }
    else if ( uMsg == WM_PAINT )
    {
        PAINTSTRUCT ps;
        const auto hDC = ::BeginPaint(hwnd, &ps);
        const auto prc = &ps.rcPaint;

        HDC hMemDC = nullptr;
        const auto pb = ::BeginBufferedPaint
        (
            hDC, prc, BPBF_TOPDOWNDIB, 0, &hMemDC
        );
        if ( pb )
        {
            ::SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)hMemDC, PRF_CLIENT);
            ::BufferedPaintSetAlpha(pb, prc, 255);
            ::EndBufferedPaint(pb, TRUE);
        }

        ::EndPaint(hwnd, &ps);

        return 0;
    }

    return ::DefSubclassProc(hwnd, uMsg, wp, lp);
};

//---------------------------------------------------------------------------//
// ダイアログ プロシージャ
//---------------------------------------------------------------------------//

BOOL CALLBACK UserListDlgProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
    switch( uMsg )
    {
        case WM_INITDIALOG:
        {
            for ( int index = 0; index < MAX_ACCOUNT; ++index )
            {
                const auto edit = ::GetDlgItem(hwnd, 2001 + index);
                const auto username = g_username[index];
                ::SetWindowText(edit, username);
            }
            break;
        }
        case WM_COMMAND:
        {
            if ( LOWORD(wp) == IDOK )
            {
                ::SendMessage(hwnd, WM_CLOSE, 0, 0);
            }
            break;
        }
        case WM_CLOSE:
        {
            for ( int index = 0; index < MAX_ACCOUNT; ++index )
            {
                const auto edit = ::GetDlgItem(hwnd, 2001 + index);
                const auto username = g_username[index];
                ::GetWindowText(edit, username, MAX_PATH);
            }
            ::EndDialog(hwnd, IDOK);
            return TRUE;
        }
        default:
        {
            break;
        }
    }
    return  FALSE;
}

//---------------------------------------------------------------------------//

// MainWindow.Utl.hpp