#pragma once

//---------------------------------------------------------------------------//
//
// MainWnd.hpp
//  メインウィンドウ
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include "include/Font.hpp"
#include "include/UWnd.hpp"
#include "include/CtrlWnd.hpp"

//---------------------------------------------------------------------------//
// Forward Declaration
//---------------------------------------------------------------------------//

class TTBasePlugin;

//---------------------------------------------------------------------------//
// Class
//---------------------------------------------------------------------------//

class MainWnd : public tapetums::UWnd
{
    using super = UWnd;

private:
    HICON icon { nullptr };
    HTREEITEM tvi[2] { nullptr, nullptr };

    tapetums::Font     font;
    tapetums::TreeWnd  tree;
    tapetums::EditWnd  label;
    tapetums::BtnWnd   btn_open_inst_folder;
    tapetums::BtnWnd   btn_exit;
    tapetums::ListWnd  list_plg;
    tapetums::ListWnd  list_cmd;

public:
    MainWnd();

public:
    LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    BOOL    OnCreate         (HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void    OnDestroy        (HWND hwnd);
    void    OnSize           (HWND hwnd, UINT state, INT32 cx, INT32 cy);
    void    OnPaint          (HWND hwnd);
    void    OnClose          (HWND hwnd);
    LRESULT OnNotify         (HWND hwnd, INT32 idCtrl, LPNMHDR pNMHdr);
    void    OnCommand        (HWND hwnd, UINT id, HWND hwndCtrl, UINT codeNotify);
    void    OnNotifyIcon     (HWND hwnd, UINT uMsg);
    void    OnReloadPlugins  ();
    void    OnSetTaskTrayIcon(HICON hIcon, LPCTSTR Tips);
    bool    OnExecuteCommand (HWND hwnd, TTBasePlugin* plugin, INT32 CmdID);
};

//---------------------------------------------------------------------------//

// MainWnd.hpp