#pragma once

//---------------------------------------------------------------------------//
//
// CtrlWnd.hpp
//  各種コントロールのラッパー
//   Copyright (C) 2015-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <windowsx.h>

#include <commctrl.h>
#pragma comment(lib, "comctl32.lib") // InitCommonControls

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib") // StrToInt

#include "UWnd.hpp"

//---------------------------------------------------------------------------//

namespace tapetums
{
    class CtrlWnd;
    class LabelWnd;
    class BtnWnd;
    class EditWnd;
    class ComboBox;
    class ListWnd;
    class TreeWnd;
    class TrackbarWnd;
}

//---------------------------------------------------------------------------//
// Classes
//---------------------------------------------------------------------------//

class tapetums::CtrlWnd : public tapetums::UWnd
{
    using super = UWnd;
    struct Init
    {
        Init()
        {
            INITCOMMONCONTROLSEX icex;

            icex.dwSize = sizeof(INITCOMMONCONTROLSEX );
            icex.dwICC  = ICC_WIN95_CLASSES | ICC_DATE_CLASSES | ICC_USEREX_CLASSES;
            ::InitCommonControlsEx(&icex);
        }
    };

protected:
    INT16 m_id { 0 };

public:
    CtrlWnd() { static Init init; }
    ~CtrlWnd() = default;

public:
    INT16 id() const noexcept { return m_id; }

    LONG_PTR id(INT16 id) noexcept
    {
        m_id = id;
        return ::SetWindowLongPtr(m_hwnd, GWLP_ID, (LONG_PTR)id);
    }

public:
    HWND Create(DWORD style, HWND hwndParent, INT16 id)
    {
        m_id = id;

        const auto hwnd = super::Create
        (
            nullptr,
            style,
            0,
            hwndParent,
            reinterpret_cast<HMENU>(id)
        );
        if ( nullptr == hwnd )
        {
            return nullptr;
        }

        const auto ret = ::SetWindowSubclass
        (
            hwnd, SubclassWndProc, id, reinterpret_cast<DWORD_PTR>(this)
        );
        if ( !ret )
        {
            return nullptr;
        }

        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

        CREATESTRUCT cs
        {
            this, ::GetModuleHandle(nullptr),
            reinterpret_cast<HMENU>(id), hwndParent,
            m_x, m_y, m_w, m_h,
            static_cast<LONG>(style), nullptr, m_class_name, 0
        };
        ::SendMessage(hwnd, WM_CREATE, 0, (LPARAM)&cs);

        return hwnd;
    }

public:
    static LRESULT WINAPI SubclassWndProc
    (
        HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp,
        UINT_PTR /*uIdSubclass*/, DWORD_PTR dwRefData
    )
    {
        auto wnd = reinterpret_cast<CtrlWnd*>(dwRefData);
        if ( nullptr == wnd )
        {
            return ::DefSubclassProc(hwnd, uMsg, wp, lp);
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

    LRESULT WndProc
    (
        HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp
    )
    {
        return ::DefSubclassProc(hwnd, uMsg, wp, lp);
    }
};

//---------------------------------------------------------------------------//

class tapetums::LabelWnd : public tapetums::CtrlWnd
{
    using super = CtrlWnd;

public:
    LabelWnd() { m_class_name = WC_STATIC; }
    ~LabelWnd() = default;

public:
    HWND Create(DWORD style, HWND hwndParent, INT16 id)
    {
        style |= WS_CHILD | WS_VISIBLE;

        return super::Create(style, hwndParent, id);
    }
};

//---------------------------------------------------------------------------//

class tapetums::BtnWnd : public tapetums::CtrlWnd
{
    using super = CtrlWnd;

public:
    BtnWnd() { m_class_name = WC_BUTTON; }
    ~BtnWnd() = default;

public:
    HWND Create(DWORD style, HWND hwndParent, INT16 id)
    {
        style |= WS_CHILD | WS_VISIBLE;

        return super::Create(style, hwndParent, id);
    }

    bool IsChecked()
    {
        Send(BM_GETCHECK, 0, 0) ? true : false;
    }

    void Check(bool checked)
    {
        Send(BM_SETCHECK, checked ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);
    }

    void Check()
    {
        Send(BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    }

    void Uncheck()
    {
        Send(BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    }
};

//---------------------------------------------------------------------------//

class tapetums::EditWnd : public tapetums::CtrlWnd
{
    using super = CtrlWnd;

public:
    EditWnd() { m_class_name = WC_EDIT; }
    ~EditWnd() = default;

public:
    HWND Create(DWORD style, HWND hwndParent, INT16 id)
    {
        style |= WS_CHILD | WS_VISIBLE;

        return super::Create(style, hwndParent, id);
    }
};

//---------------------------------------------------------------------------//

class tapetums::ComboBox : public tapetums::CtrlWnd
{
    using super = CtrlWnd;

public:
    ComboBox() { m_class_name = WC_COMBOBOX; }
    ~ComboBox() = default;

public:
    HWND Create(DWORD style, HWND hwndParent, INT16 id)
    {
        style |= WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST;

        return super::Create(style, hwndParent, id);
    }

    void AddString(LPCTSTR text)
    {
        Send(CB_ADDSTRING, 0, (LPARAM)text);
    }

    INT32 SelectedIndex()
    {
        return (INT32)Send(CB_GETCURSEL, 0, 0);
    }

    void Select(INT32 index)
    {
        Send(CB_SETCURSEL, index, 0);
    }

    INT32 Count()
    {
        return (INT32)Send(CB_GETCOUNT, 0, 0);
    }
};

//---------------------------------------------------------------------------//

class tapetums::ListWnd : public tapetums::CtrlWnd
{
    using super = CtrlWnd;

public:
    ListWnd() { m_class_name = WC_LISTVIEW; }
    ~ListWnd() = default;

public:
    HWND Create(DWORD style, DWORD styleEx, HWND hwndParent, INT16 id)
    {
        style |= WS_CHILD | WS_VISIBLE;

        const auto hwnd = super::Create(style, hwndParent, id);

        ListView_SetExtendedListViewStyle(hwnd, styleEx);

        return hwnd;
    }

    INT32 InsertColumn(LPCTSTR text, INT32 width, INT32 index = 0)
    {
        LVCOLUMN col;
        col.mask     = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.fmt      = LVCFMT_LEFT;
        col.cx       = width;
        col.iSubItem = index;
        col.pszText = (LPTSTR)text;
        return ListView_InsertColumn(m_hwnd, index , &col);
    }

    INT32 InsertItem(LPCTSTR text, INT32 index)
    {
        return InsertItem(text, index, 0, 0) ? true : false;
    }

    INT32 InsertItem(LPCTSTR text, INT32 index, LPARAM lp)
    {
        return InsertItem(text, index, 0, lp) ? true : false;
    }

    INT32 InsertItem(LPCTSTR text, INT32 index, INT32 image, LPARAM lp)
    {
        LVITEM item;
        item.mask     = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        item.pszText  = (LPTSTR)text;
        item.iItem    = index;
        item.iSubItem = 0;
        item.iImage   = image;
        item.lParam   = lp;
        return ListView_InsertItem(m_hwnd, &item) ? true : false;
    }

    INT32 SetItem(LPCTSTR text, INT32 index, INT32 sub_index)
    {
        LVITEM item;
        item.mask     = LVIF_TEXT;
        item.pszText  = (LPTSTR)text;
        item.iItem    = index;
        item.iSubItem = sub_index;
        return ListView_SetItem(m_hwnd, &item) ? true : false;
    }

    void DeleteAllItems()
    {
        ListView_DeleteAllItems(m_hwnd);
    }

    void SetImageList(HIMAGELIST list)
    {
        ListView_SetImageList(m_hwnd, list, LVSIL_STATE);
    }

    INT32 Count()
    {
        return ListView_GetItemCount(m_hwnd);
    }

    void Select(INT32 index)
    {
        ListView_SetItemState
        (
            m_hwnd, index,
            LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED
        );
    }

    INT32 SelectedIndex()
    {
        return ListView_GetNextItem(m_hwnd, -1, LVNI_ALL | LVNI_SELECTED);
    }

    void GetItemText(INT32 index, INT32 sub_index, LPTSTR pszText, INT32 cchTextMax)
    {
        LVITEM item;
        item.mask       = LVIF_TEXT;
        item.iItem      = index;
        item.iSubItem   = sub_index;
        item.pszText    = pszText;
        item.cchTextMax = cchTextMax;
        ListView_GetItem(m_hwnd, &item);
    }

    INT32 GetItemToInt(INT32 index, INT32 sub_index)
    {
        constexpr size_t cchTextMax { 16 };
        TCHAR buf[cchTextMax];

        LVITEM item;
        item.mask       = LVIF_TEXT;
        item.iItem      = index;
        item.iSubItem   = sub_index;
        item.pszText    = buf;
        item.cchTextMax = cchTextMax;
        ListView_GetItem(m_hwnd, &item);

        return ::StrToInt(buf);
    }

    UINT GetItemSatte(INT32 index, INT32 sub_index)
    {
        LVITEM item;
        item.mask       = LVIF_TEXT;
        item.iItem      = index;
        item.iSubItem   = sub_index;
        ListView_GetItem(m_hwnd, &item);

        return item.state;
    }

    LPARAM GetItemLPARAM(INT32 index)
    {
        LVITEM item;
        item.mask     = LVIF_PARAM;
        item.iItem    = index;
        item.iSubItem = 0;
        ListView_GetItem(m_hwnd, &item);

        return item.lParam;
    }

    bool IsChecked(INT32 index)
    {
        return ListView_GetCheckState(m_hwnd, index) ? true : false;
    }

    void Check(INT32 index)
    {
        ListView_SetCheckState(m_hwnd, index, true);
    }

    void Uncheck(INT32 index)
    {
        ListView_SetCheckState(m_hwnd, index, false);
    }
};

//---------------------------------------------------------------------------//

class tapetums::TreeWnd : public tapetums::CtrlWnd
{
    using super = CtrlWnd;

public:
    TreeWnd() { m_class_name = WC_TREEVIEW; }
    ~TreeWnd() = default;

public:
    HWND Create(HWND hwndParent, INT16 id)
    {
        const auto style = WS_CHILD | WS_VISIBLE |
            TVS_FULLROWSELECT | TVS_SHOWSELALWAYS;

        return super::Create(style, hwndParent, id);
    }

    HTREEITEM InsertItem(LPCTSTR text, HTREEITEM parent = TVI_ROOT)
    {
        TVINSERTSTRUCT tvis;
        tvis.hParent      = parent;
        tvis.hInsertAfter = TVI_LAST;
        tvis.item.mask    = TVIF_TEXT;
        tvis.item.pszText = (LPTSTR)text;
        return TreeView_InsertItem(m_hwnd, &tvis);
    }

    void SetImageList(HIMAGELIST list)
    {
        TreeView_SetImageList(m_hwnd, list, TVSIL_NORMAL);
    }

    void Select(HTREEITEM hitem)
    {
        TreeView_Select(m_hwnd, hitem, TVGN_CARET);
    }

    HTREEITEM GetSelection()
    {
        return TreeView_GetSelection(m_hwnd);
    }

public:
    LRESULT WndProc
    (
        HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp
    )
    {
        switch ( uMsg )
        {
            case WM_HSCROLL:
            case WM_VSCROLL:
            {
                return ::SendMessage(GetParent(), uMsg, wp, lp);
            }
            default:
            {
                return super::WndProc(hwnd, uMsg, wp, lp);
            }
        }
    }
};

//---------------------------------------------------------------------------//

#ifdef min
  #undef min
#endif
#ifdef max
  #undef max
#endif

class tapetums::TrackbarWnd : public tapetums::CtrlWnd
{
    using super = CtrlWnd;

public:
    struct RANGE { UINT min; UINT max; };

public:
    TrackbarWnd() { m_class_name = TRACKBAR_CLASS; }
    ~TrackbarWnd() = default;

public:
    UINT pos() const noexcept
    {
        return (UINT)::SendMessage(m_hwnd, TBM_GETPOS, 0, 0);
    }

    void pos(UINT pos) noexcept
    {
        this->Post(TBM_SETPOS, TRUE, (LPARAM)pos);
    }

    UINT page_size() const noexcept
    {
        return (UINT)::SendMessage(m_hwnd, TBM_GETPAGESIZE, 0, 0);
    }

    void page_size(UINT size) noexcept
    {
        this->Post(TBM_SETPAGESIZE, 0, (LPARAM)size);
    }

    RANGE range() const noexcept
    {
        const auto min = (UINT)::SendMessage(m_hwnd, TBM_GETRANGEMIN, 0, 0);
        const auto max = (UINT)::SendMessage(m_hwnd, TBM_GETRANGEMAX, 0, 0);
        return RANGE{ min, max };
    }

    void range(UINT min, UINT max) noexcept
    {
        ::PostMessage(m_hwnd, TBM_SETTICFREQ, 1, 0);
        ::PostMessage(m_hwnd, TBM_SETRANGE, TRUE, MAKELPARAM(min, max));
    }

public:
    HWND Create(HWND hwndParent, INT16 id)
    {
        const auto style = WS_CHILD | WS_VISIBLE |
            TBS_NOTICKS | TBS_TOOLTIPS | TBS_TRANSPARENTBKGND;

        return super::Create(style, hwndParent, id);
    }

public:
    LRESULT WndProc
    (
        HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp
    )
    {
        switch ( uMsg )
        {
            case WM_HSCROLL:
            case WM_VSCROLL:
            {
                return ::SendMessage(GetParent(), uMsg, wp, lp);
            }
            default:
            {
                return super::WndProc(hwnd, uMsg, wp, lp);
            }
        }
    }
};

//---------------------------------------------------------------------------//

// CtrlWnd.hpp