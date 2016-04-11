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
    HICON      icon   { nullptr };
    HIMAGELIST il     { nullptr };
    HTREEITEM  tvi[3] { nullptr, nullptr, nullptr };

    tapetums::Font     font;
    tapetums::TreeWnd  tree;
    tapetums::EditWnd  edit;
    tapetums::ComboBox cbx_log;
    tapetums::BtnWnd   btn_compatible;
    tapetums::BtnWnd   btn_open_inst_folder;
    tapetums::BtnWnd   btn_exit;
    tapetums::BtnWnd   btn_log2wnd;
    tapetums::BtnWnd   btn_log2file;
    tapetums::ListWnd  list_plg;
    tapetums::ListWnd  list_cmd;
    tapetums::LabelWnd label;

public:
    MainWnd();

public:
    LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:

    BOOL    CALLBACK OnCreate         (HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void    CALLBACK OnDestroy        (HWND hwnd);
    void    CALLBACK OnSize           (HWND hwnd, UINT state, INT32 cx, INT32 cy);
    void    CALLBACK OnPaint          (HWND hwnd);
    void    CALLBACK OnClose          (HWND hwnd);
    void    CALLBACK OnEndSession     (HWND hwnd, BOOL fEnding);
    LRESULT CALLBACK OnNotify         (HWND hwnd, INT32 idCtrl, LPNMHDR pNMHdr);
    void    CALLBACK OnCommand        (HWND hwnd, UINT id, HWND hwndCtrl, UINT codeNotify);
    void    CALLBACK OnNotifyIcon     (HWND hwnd, UINT uMsg);
    void    CALLBACK OnRegisterIcon   ();
    void    CALLBACK OnShowSettings   (HWND hwnd);
    void    CALLBACK OmShowVerInfo    (HWND hwnd);
    void    CALLBACK OnSetTaskTrayIcon(HICON hIcon, LPCTSTR Tips);
    bool    CALLBACK OnExecuteCommand (HWND hwnd, ITTBPlugin* plugin, INT32 CmdID);
    void    CALLBACK OnReloadPlugins  ();
    void    CALLBACK OnWindowsHook    (UINT Msg, WPARAM wParam, LPARAM lParam);
};

//---------------------------------------------------------------------------//

// MainWnd.hpp