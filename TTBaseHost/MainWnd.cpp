﻿//---------------------------------------------------------------------------//
//
// MainWnd.cpp
//  メインウィンドウ
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <array>

#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>

#pragma comment(lib, "shell32.lib") // ShellExecute

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib") // PathRemoveFileSpec, StrToInt

#pragma warning(disable: 4005)
#include "resource.h"

#include "../Plugin.hpp"
#include "../MessageDef.hpp"
#include "../Utility.hpp"
#include "Settings.hpp"
#include "Command.hpp"
#include "PluginMgr.hpp"

#include "MainWnd.hpp"

#if SYS_DEBUG

template<typename C, typename... Args>
void SystemLog(const C* const format, Args... args)
{
    WriteLog(ERROR_LEVEL(5), format, args...);
}

#else

#define SystemLog(format, ...)

#endif

//---------------------------------------------------------------------------//
// Global Variables
//---------------------------------------------------------------------------//

// WinMain.cpp で宣言
extern HINSTANCE g_hInst;

//---------------------------------------------------------------------------//

UINT TTB_SHOW_SETTINGS      { 0 };
UINT TTB_OPEN_FOLDER        { 0 };
UINT TTB_SHOW_VER_INFO      { 0 };
UINT TTB_RELOAD_PLUGINS     { 0 };
UINT TTB_SET_PLUGIN_INFO    { 0 };
UINT TTB_SET_MENU_PROPERTY  { 0 };
UINT TTB_SET_TASK_TRAY_ICON { 0 };
UINT TTB_EXECUTE_COMMAND    { 0 };

constexpr UINT ID_TRAYICON { 1 };

constexpr INT32   FONT_SIZE { 16 };
constexpr LPCTSTR FONT_FACE { TEXT("Meiryo UI") };

//---------------------------------------------------------------------------//
// Window Controls
//---------------------------------------------------------------------------//

struct CTRL
{
    enum : INT16
    {
        TREEVIEW              = 1001,
        LABEL_INFO            = 1002,
        CBX_LOGLEVEL          = 1003,
        BTN_TTBASE_COMPATIBLE = 1004,
        BTN_LOG2WND           = 1005,
        BTN_LOG2FILE          = 1006,
        BTN_OPEN_INST_FOLDER  = 1007,
        BTN_EXIT              = 1008,
        LIST_PLUGIN           = 1009,
        LIST_COMMAND          = 1010,
        LABEL_RESTART         = 1011,
    };
};

struct CTRL_TEXT
{
    static constexpr LPCTSTR Settings = TEXT("設定");
    static constexpr LPCTSTR Plugins  = TEXT("プラグイン");
    static constexpr LPCTSTR Commands = TEXT("コマンド");
    static constexpr LPCTSTR Note     = TEXT("*次回起動時より有効");

    static constexpr LPCTSTR Exit        = TEXT("終了");
    static constexpr LPCTSTR OpenFolder  = TEXT("インストールフォルダを開く");
    static constexpr LPCTSTR Compatible  = TEXT("ウィンドウクラス名とウィンドウ名をTTBaseと同じにする*");
    static constexpr LPCTSTR logToWindow = TEXT("ログをデバッガに送る");
    static constexpr LPCTSTR logToFile   = TEXT("ログをファイルに出力する*");
};

struct LIST_PLG_ITEM
{
    enum : INT32
    {
        名前, 相対パス, bits, バージョン, 常駐, コマンド数,
    };

    static constexpr LPCTSTR Name      = TEXT("名前");
    static constexpr LPCTSTR Filename  = TEXT("相対パス");
    static constexpr LPCTSTR BitCount  = TEXT("bits");
    static constexpr LPCTSTR Version   = TEXT("バージョン");
    static constexpr LPCTSTR Residence = TEXT("常駐");
    static constexpr LPCTSTR CmdCount  = TEXT("コマンド数");
};

struct LIST_CMD_ITEM
{
    enum: INT32
    {
        名前, コマンド名, 相対パス, コマンドID, 
    };

    static constexpr LPCTSTR Name     = TEXT("名前");
    static constexpr LPCTSTR CmdName  = TEXT("コマンド名");
    static constexpr LPCTSTR Filename = TEXT("相対パス");
    static constexpr LPCTSTR CmdID    = TEXT("コマンドID");
};

LPCTSTR CBX_LOGLEVEL_TEXT[6] =
{
    TEXT("0 : 出力しない"),
    TEXT("1 : エラー"),
    TEXT("2 : エラー/警告"),
    TEXT("3 : エラー/警告/情報"),
    TEXT("4 : エラー/警告/情報/デバッグ"),
    TEXT("5 : エラー/警告/情報/デバッグ/デバッグ(本体)"),
};

//---------------------------------------------------------------------------//
// Utility Functions
//---------------------------------------------------------------------------//

void SetPluginNames      (const PluginMgr& mgr, tapetums::ListWnd& list);
void SetPluginCommands   (const PluginMgr& mgr, tapetums::ListWnd& list);
void UpdataPluginCommands(const PluginMgr& mgr, tapetums::ListWnd& list);
void ShowPluginInfo      (tapetums::ListWnd& list, tapetums::CtrlWnd& edit);
void UpdateCheckState    (tapetums::ListWnd& list, const ITTBPlugin* plugin, INT32 CmdID);
void PopupSysMenu        (HWND hwnd);
void PopupToolMenu       (HWND hwnd);

//---------------------------------------------------------------------------//
// Methods
//---------------------------------------------------------------------------//

MainWnd::MainWnd()
{
    // 独自ウィンドウメッセージの登録
    TTB_SHOW_SETTINGS      = ::RegisterWindowMessage(TEXT("TTB_SHOW_SETTINGS"));
    TTB_OPEN_FOLDER        = ::RegisterWindowMessage(TEXT("TTB_OPEN_FOLDER"));
    TTB_SHOW_VER_INFO      = ::RegisterWindowMessage(TEXT("TTB_SHOW_VER_INFO"));
    TTB_RELOAD_PLUGINS     = ::RegisterWindowMessage(TEXT("TTB_RELOAD_PLUGINS"));
    TTB_SET_PLUGIN_INFO    = ::RegisterWindowMessage(TEXT("TTB_SET_PLUGIN_INFO"));
    TTB_SET_MENU_PROPERTY  = ::RegisterWindowMessage(TEXT("TTB_SET_MENU_PROPERTY"));
    TTB_SET_TASK_TRAY_ICON = ::RegisterWindowMessage(TEXT("TTB_SET_TASK_TRAY_ICON"));
    TTB_EXECUTE_COMMAND    = ::RegisterWindowMessage(TEXT("TTB_EXECUTE_COMMAND"));

    // TTBase 互換設定の読み込み
    const auto lpszClassName  = settings::get().TTBaseCompatible ?
                                TEXT("TTBaseMain") : TEXT("UWnd");
    const auto lpszWindowName = settings::get().TTBaseCompatible ?
                                TEXT("TTBase") : PLUGIN_NAME;

    // フォントの作成
    //  TODO: DPIスケーリングへの対応
    font.create(FONT_SIZE, FONT_FACE, FW_REGULAR);
    SetFont(font);

    // ウィンドウの生成
    //  TODO: DPIスケーリングへの対応
    Register(lpszClassName);

    const auto style   = WS_THICKFRAME | WS_CAPTION | WS_SYSMENU;
    const auto styleEx = 0;//WS_EX_TOOLWINDOW;
    Create(lpszWindowName, style, styleEx, nullptr, nullptr);
    Resize(640, 480);
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK MainWnd::WndProc
(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam
)
{
    if ( uMsg == WM_SHOWWINDOW && wParam == SW_SHOWNORMAL )
    {
        ToCenter(); Show(); ::SetForegroundWindow(hwnd); return 0;
    }
    if ( uMsg == WM_CTLCOLORSTATIC )
    {
        return (LRESULT)GetStockBrush(WHITE_BRUSH); // 背景を白に
    }
    if ( uMsg == WM_QUERYENDSESSION )
    {
        return TRUE;
    }
    if ( uMsg == WM_TASKBARCREATED )
    {
        OnRegisterIcon(); return 0;
    }
    if ( uMsg == WM_NOTIFYICON )
    {
        OnNotifyIcon(hwnd, (UINT)lParam); return 0;
    }
    if ( uMsg == TTB_SHOW_SETTINGS )
    {
        OnShowSettings(hwnd); return 0;
    }
    if ( uMsg == TTB_OPEN_FOLDER )
    {
        OnCommand(hwnd, CTRL::BTN_OPEN_INST_FOLDER, nullptr, 0); return 0;
    }
    if ( uMsg == TTB_SHOW_VER_INFO )
    {
        OmShowVerInfo(hwnd); return 0;
    }
    if ( uMsg == TTB_SET_PLUGIN_INFO )
    {
        UpdataPluginCommands(PluginMgr::GetInstance(), list_cmd); return 0;
    }
    if ( uMsg == TTB_SET_MENU_PROPERTY )
    {
        UpdateCheckState(list_cmd, (ITTBPlugin*)wParam, (INT32)lParam); return 0;
    }
    if ( uMsg == TTB_SET_TASK_TRAY_ICON )
    {
        OnSetTaskTrayIcon((HICON)wParam, (LPCTSTR)lParam); return 0;
    }
    if ( uMsg == TTB_EXECUTE_COMMAND )
    {
        OnExecuteCommand(hwnd, (ITTBPlugin*)wParam, (INT32)lParam); return 0;
    }
    if ( uMsg == TTB_RELOAD_PLUGINS || uMsg == TTB_LOAD_DATA_FILE )
    {
        OnReloadPlugins(); return 0;
    }
    if ( uMsg == TTB_SAVE_DATA_FILE )
    {
        WriteLog(elError, TEXT("%s"), TEXT("TTB_SAVE_DATA_FILE は実装していません"));
        return 0;
    }
    if
    (
        uMsg == TTB_HSHELL_ACTIVATESHELLWINDOW ||
        uMsg == TTB_HSHELL_GETMINRECT          ||
        uMsg == TTB_HSHELL_LANGUAGE            ||
        uMsg == TTB_HSHELL_REDRAW              ||
        uMsg == TTB_HSHELL_TASKMAN             ||
        uMsg == TTB_HSHELL_WINDOWACTIVATED     ||
        uMsg == TTB_HSHELL_WINDOWCREATED       ||
        uMsg == TTB_HSHELL_WINDOWDESTROYED     ||
        uMsg == TTB_HMOUSE_ACTION
    )
    {
        OnWindowsHook(uMsg, wParam, lParam); return 0;
    }

    switch ( uMsg )
    {
        HANDLE_MSG(hwnd, WM_CREATE,     OnCreate);
        HANDLE_MSG(hwnd, WM_DESTROY,    OnDestroy);
        HANDLE_MSG(hwnd, WM_SIZE,       OnSize);
        HANDLE_MSG(hwnd, WM_PAINT,      OnPaint);
        HANDLE_MSG(hwnd, WM_CLOSE,      OnClose);
        HANDLE_MSG(hwnd, WM_ENDSESSION, OnEndSession);
        HANDLE_MSG(hwnd, WM_NOTIFY,     OnNotify);
        HANDLE_MSG(hwnd, WM_COMMAND,    OnCommand);
        default: return super::WndProc(hwnd, uMsg, wParam, lParam);
    }
}

//---------------------------------------------------------------------------//

BOOL CALLBACK MainWnd::OnCreate
(
    HWND hwnd, LPCREATESTRUCT
)
{
    OnRegisterIcon();

    // コントロールの生成
    tree.Create(hwnd, CTRL::TREEVIEW);
    tree.SetFont(font);
    tree.SetImageList(ImageList_Create(1, 32, ILC_COLOR, 0, 0));
    tvi[0] = tree.InsertItem(CTRL_TEXT::Settings);
    tvi[1] = tree.InsertItem(CTRL_TEXT::Plugins);
    tvi[2] = tree.InsertItem(CTRL_TEXT::Commands);

    edit.Create(ES_READONLY | ES_MULTILINE, hwnd, CTRL::LABEL_INFO);
    edit.SetFont(font);
    edit.Show();

    cbx_log.Create(CBS_DROPDOWNLIST, hwnd, CTRL::CBX_LOGLEVEL);
    cbx_log.SetFont(font);
    cbx_log.Resize(328, 800);
    for ( auto idx = 0; idx < 6; ++idx)
    {
        cbx_log.AddString(CBX_LOGLEVEL_TEXT[idx]);
    }
    cbx_log.Select(settings::get().logLevel);

    btn_compatible.Create(BS_CHECKBOX | BS_AUTOCHECKBOX , hwnd, CTRL::BTN_TTBASE_COMPATIBLE);
    btn_compatible.SetFont(font);
    btn_compatible.SetText(CTRL_TEXT::Compatible);
    btn_compatible.Check(settings::get().TTBaseCompatible);

    btn_open_inst_folder.Create(BS_PUSHBUTTON, hwnd, CTRL::BTN_OPEN_INST_FOLDER);
    btn_open_inst_folder.SetFont(font);
    btn_open_inst_folder.SetText(CTRL_TEXT::OpenFolder);

    btn_exit.Create(BS_PUSHBUTTON, hwnd, CTRL::BTN_EXIT);
    btn_exit.SetFont(font);
    btn_exit.SetText(CTRL_TEXT::Exit);

    btn_log2wnd.Create(BS_CHECKBOX | BS_AUTOCHECKBOX , hwnd, CTRL::BTN_LOG2WND);
    btn_log2wnd.SetFont(font);
    btn_log2wnd.SetText(CTRL_TEXT::logToWindow);
    btn_log2wnd.Check(settings::get().logToWindow);

    btn_log2file.Create(BS_CHECKBOX | BS_AUTOCHECKBOX , hwnd, CTRL::BTN_LOG2FILE);
    btn_log2file.SetFont(font);
    btn_log2file.SetText(CTRL_TEXT::logToFile);
    btn_log2file.Check(settings::get().logToFile);

    auto style = WS_VSCROLL | WS_HSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL;
    auto styleEx = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP;
    list_plg.Create(style, styleEx, m_hwnd, CTRL::LIST_PLUGIN);
    list_plg.InsertColumn(LIST_PLG_ITEM::Name,     130, LIST_PLG_ITEM::名前);
    list_plg.InsertColumn(LIST_PLG_ITEM::Filename, 150, LIST_PLG_ITEM::相対パス);
    list_plg.InsertColumn(LIST_PLG_ITEM::BitCount,  33, LIST_PLG_ITEM::bits);
    list_plg.InsertColumn(LIST_PLG_ITEM::Version,   60, LIST_PLG_ITEM::バージョン);
    list_plg.InsertColumn(LIST_PLG_ITEM::Residence, 40, LIST_PLG_ITEM::常駐);
    list_plg.InsertColumn(LIST_PLG_ITEM::CmdCount,  60, LIST_PLG_ITEM::コマンド数);

    label.Create(ES_LEFT, hwnd, CTRL::LABEL_RESTART);
    label.SetFont(font);
    label.SetText(CTRL_TEXT::Note);

    list_cmd.Create(style, styleEx, hwnd, CTRL::LIST_COMMAND);
    list_cmd.InsertColumn(LIST_CMD_ITEM::Name,     130, LIST_CMD_ITEM::名前);
    list_cmd.InsertColumn(LIST_CMD_ITEM::CmdName,  133, LIST_CMD_ITEM::コマンド名);
    list_cmd.InsertColumn(LIST_CMD_ITEM::Filename, 150, LIST_CMD_ITEM::相対パス);
    list_cmd.InsertColumn(LIST_CMD_ITEM::CmdID,     60, LIST_CMD_ITEM::コマンドID);

    il = ::ImageList_Create(16, 16, ILC_COLOR24, 3, 0);

    HICON tmp;
    tmp = ::LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_NOCHEK));
    ::ImageList_AddIcon(il, tmp);
    ::DestroyIcon(tmp);

    tmp = ::LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON_CHECKED));
    ::ImageList_AddIcon(il, tmp);
    ::DestroyIcon(tmp);

    list_cmd.SetImageList(il);

    // リストビューに項目を登録
    const auto& mgr = PluginMgr::GetInstance();
    SetPluginNames   (mgr, list_plg);
    SetPluginCommands(mgr, list_cmd);

    // 「設定」設定項目を選択
    tree.Select(tvi[0]);

    return TRUE;
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnDestroy
(
    HWND
)
{
    // 設定ファイルを保存
    settings::get().save();

    // イメージリストを破棄
    ::ImageList_Destroy(il);

    // タスクトレイのアイコンを破棄
    DeleteNotifyIcon(ID_TRAYICON);
    ::DestroyIcon(icon);

    // アプリケーションを終了
    ::PostQuitMessage(0);
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnSize
(
    HWND, UINT , INT32 cx, INT32 cy
)
{
    // コントロールを配置
    //  TODO: DPIスケーリングへの対応
    tree.Bounds(2, 2, 140 - 4, cy - 4);

    btn_compatible.Bounds(140 + 16, 16, 328, 24);
    cbx_log.Bounds(140 + 16, 52 + 1, 328, 240);
    btn_log2wnd.Bounds(140 + 48, 102, 328, 24);
    btn_log2file.Bounds(140 + 48, 136, 328, 24);

    edit.Bounds(140 + 12, 12, cx - 140 - 140 - 24, 100 - 24);
    btn_open_inst_folder.Bounds(cx - 128 - 12, 16, 128, 24);
    btn_exit.Bounds(cx - 128 - 12, 52, 128, 24);

    list_plg.Bounds(140 + 2, 90 + 2, cx - 140 - 4, cy - 90 - 4);
    list_cmd.Bounds(140 + 2, 90 + 2, cx - 140 - 4, cy - 90 - 4);

    label.Bounds(140 + 16, cy - 36, 328, 24);
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnPaint
(
    HWND hwnd
)
{
    PAINTSTRUCT ps;
    const auto hdc = ::BeginPaint(hwnd, &ps);

    // 枠線の描画
    //  TODO: DPIスケーリングへの対応
    {
        RECT rc { 0, 0, m_w, m_h };
        ::FillRect(hdc, &rc, GetStockBrush(WHITE_BRUSH));
    }
    {
        RECT rc { 0, 0, 140, m_h };
        ::DrawEdge(hdc, &rc, BDR_SUNKENINNER | BDR_RAISEDOUTER, BF_RECT);
    }
    {
        RECT rc { 140, 90, m_w, m_h };
        ::DrawEdge(hdc, &rc, BDR_SUNKENINNER | BDR_RAISEDOUTER, BF_RECT);
    }

    ::EndPaint(hwnd, &ps);
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnClose
(
    HWND
)
{
    // 設定ファイルを保存
    settings::get().save();

    Hide();
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnEndSession
(
    HWND hwnd, BOOL fEnding
)
{
    if ( fEnding )
    {
        SystemLog(TEXT("%s"), TEXT("セッションを終了しています..."));

        ::ShutdownBlockReasonCreate(hwnd, L"プラグインを解放しています...");

        // プラグインマネージャの終了処理
        auto&& mgr = PluginMgr::GetInstance();
        mgr.FreeAll();

        // メインウィンドウの破棄
        OnDestroy(hwnd);

        ::ShutdownBlockReasonDestroy(hwnd);

        SystemLog(TEXT("%s"), TEXT("OK"));
    }
}

//---------------------------------------------------------------------------//

LRESULT CALLBACK MainWnd::OnNotify
(
    HWND hwnd, INT32, LPNMHDR pNMHdr
)
{
    if ( pNMHdr->idFrom == CTRL::LIST_PLUGIN )
    {
        if ( pNMHdr->code == NM_CLICK )
        {
            // プラグインの詳細情報を表示
            ShowPluginInfo(list_plg, edit);
        }
        else if ( pNMHdr->code == LVN_GETINFOTIP )
        {
            TCHAR buf [MAX_PATH];
            auto pNMinfo = LPNMLVGETINFOTIP(pNMHdr);

            LVITEM item { };
            item.mask       = TVIF_TEXT;
            item.iItem      = pNMinfo->iItem;
            item.pszText    = buf;
            item.cchTextMax = MAX_PATH;
            ListView_GetItem(list_plg, &item);
            pNMinfo->pszText = item.pszText;
        }
    }
    else if ( pNMHdr->idFrom == CTRL::LIST_COMMAND )
    {
        if ( pNMHdr->code == NM_DBLCLK )
        {
            POINT pt;
            ::GetCursorPos(&pt);
            ::ScreenToClient(pNMHdr->hwndFrom, &pt);

            // ユーザーによるチェックボックスの操作を無効化
            if ( pt.x <= 16 ) { return TRUE; }

            // コマンドの実行
            const auto index = list_cmd.SelectedIndex();

            auto plugin = (ITTBPlugin*)list_cmd.GetItemLPARAM(index);
            const auto CmdID = list_cmd.GetItemToInt(index, LIST_CMD_ITEM::コマンドID);

            return OnExecuteCommand(hwnd, plugin, CmdID);
        }
        else if ( pNMHdr->code == LVN_GETINFOTIP )
        {
            TCHAR buf [MAX_PATH];
            auto pNMinfo = LPNMLVGETINFOTIP(pNMHdr);

            LVITEM item { };
            item.mask       = TVIF_TEXT;
            item.iItem      = pNMinfo->iItem;
            item.pszText    = buf;
            item.cchTextMax = MAX_PATH;
            ListView_GetItem(list_cmd, &item);

            pNMinfo->pszText = item.pszText;
        }
    }
    else if ( pNMHdr->idFrom == CTRL::TREEVIEW && pNMHdr->code == TVN_SELCHANGED )
    {
        // コントロール表示の切替
        const auto item = tree.GetSelection();
        if ( item == tvi[1] )
        {
            edit.SetText(TEXT(""));
            edit.Show();
            cbx_log.Hide();
            btn_compatible.Hide();
            btn_log2wnd.Hide();
            btn_log2file.Hide();
            list_plg.Show();
            list_cmd.Hide();
            label.Hide();

            // 一番上の項目を選択状態に
            list_plg.Select(0);
            ShowPluginInfo(list_plg, edit);
        }
        else if ( item == tvi[2] )
        {
            edit.SetText(TEXT("項目をダブルクリックして実行"));
            edit.Show();
            cbx_log.Hide();
            btn_compatible.Hide();
            btn_log2wnd.Hide();
            btn_log2file.Hide();
            list_plg.Hide();
            list_cmd.Show();
            label.Hide();
        }
        else
        {
            edit.Hide();
            cbx_log.Show();
            btn_compatible.Show();
            btn_log2wnd.Show();
            btn_log2file.Show();
            list_plg.Hide();
            list_cmd.Hide();
            label.Show();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnCommand
(
    HWND hwnd, UINT id, HWND hwndCtrl, UINT codeNotify
)
{
    hwnd; hwndCtrl; codeNotify;

    if ( id == CTRL::BTN_EXIT )
    {
        // アプリケーションを終了
        Destroy();
    }
    else if ( id == CTRL::BTN_OPEN_INST_FOLDER )
    {
        // 本体のインストールフォルダを開く
        std::array<TCHAR, MAX_PATH> path;
        ::GetModuleFileName(g_hInst, path.data(), (DWORD)path.size());
        ::PathRemoveFileSpec(path.data());

        ::ShellExecute
        (
            nullptr, TEXT("open"), path.data(),
            nullptr, nullptr, SW_SHOWNOACTIVATE
        );
    }
    else if ( id == CTRL::BTN_TTBASE_COMPATIBLE )
    {
        settings::get().TTBaseCompatible ^= true;
    }
    else if ( id == CTRL::BTN_LOG2WND )
    {
        settings::get().logToWindow ^= true;
    }
    else if ( id == CTRL::BTN_LOG2FILE )
    {
        settings::get().logToFile ^= true;
    }
    else if ( id == CTRL::CBX_LOGLEVEL )
    {
        if ( codeNotify != CBN_SELCHANGE ) { return; }

        const auto index = cbx_log.SelectedIndex();
        settings::get().logLevel = index;
    }
    else if ( id >= 10000 )
    {
        const INT32 index = id > 20000 ? id - 20000 : id - 10000;
        if ( index >= list_cmd.Count() )
        {
            return;
        }

        if ( id < 20000 )
        {
            SystemLog(TEXT("%s"), TEXT("システムメニューの表示"));
        }
        else
        {
            SystemLog(TEXT("%s"), TEXT("ツールメニューの表示"));
        }

        // リストビューコントロールから必要なデータを取得
        const auto plugin = (ITTBPlugin*)list_cmd.GetItemLPARAM(index);
        const auto CmdID = list_cmd.GetItemToInt(index, LIST_CMD_ITEM::コマンドID);

        // コマンドを実行
        if ( plugin )
        {
            SystemLog(TEXT("  %s|%i"), plugin->info()->Filename, CmdID);
            OnExecuteCommand(hwnd, plugin, CmdID);
        }
    }
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnNotifyIcon
(
    HWND hwnd, UINT uMsg
)
{
    if ( uMsg == WM_RBUTTONUP )
    {
        PopupSysMenu(hwnd);
    }
    else if ( uMsg == WM_LBUTTONUP )
    {
        PopupToolMenu(hwnd);
    }
    else if ( uMsg == WM_MBUTTONUP )
    {
        ::PostMessage(hwnd, WM_SHOWWINDOW, SW_SHOWNORMAL, 0);
    }
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnRegisterIcon()
{
    // ウィンドウにアイコンを登録
    icon = (HICON)::LoadImage
    (
        g_hInst, MAKEINTRESOURCE(IDI_ICON),
        IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_DEFAULTCOLOR | LR_SHARED
    );
    SetWindowIcon(icon);

    // タスクトレイにアイコンを登録
    if ( settings::get().ShowTaskTrayIcon )
    {
        AddNotifyIcon(ID_TRAYICON, icon);
        SetNotifyIconTip(ID_TRAYICON, PLUGIN_NAME);
    }
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnShowSettings(HWND hwnd)
{
    // 「設定」設定項目を選択
    tree.Select(tvi[0]);

    ::PostMessage(hwnd, WM_SHOWWINDOW, SW_SHOWNORMAL, 0);
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OmShowVerInfo(HWND hwnd)
{
    // 「プラグイン」設定項目を選択
    tree.Select(tvi[1]);
    list_plg.Select(0);
    ShowPluginInfo(list_plg, edit);

    ::PostMessage(hwnd, WM_SHOWWINDOW, SW_SHOWNORMAL, 0);
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnSetTaskTrayIcon
(
    HICON hIcon, LPCTSTR Tips
)
{
    if ( hIcon )
    {
        // タスクトレイアイコンを差替
        DeleteNotifyIcon(ID_TRAYICON);
        ::DestroyIcon(icon);
        icon = hIcon;

        AddNotifyIcon(ID_TRAYICON, icon);
        SetNotifyIconTip(ID_TRAYICON, Tips);
    }
    else
    {
        // タスクトレイアイコンの表示/非表示を切替
        DeleteNotifyIcon(ID_TRAYICON);

        auto&& visible = settings::get().ShowTaskTrayIcon;
        if ( visible )
        {
            visible = false;
        }
        else
        {
            visible = true;
            AddNotifyIcon(ID_TRAYICON, icon);
            SetNotifyIconTip(ID_TRAYICON, PLUGIN_NAME);
        }

        // プラグイン一覧でのチェックボックスの状態も更新
        auto&& system = PluginMgr::GetInstance().system();
        auto&& cmd = system->info()->Commands[CMD_TRAYICON];

        auto DispMenu = (DWORD)cmd.DispMenu;
        DispMenu ^= dmMenuChecked;
        cmd.DispMenu = (DISPMENU)DispMenu;

        UpdateCheckState(list_cmd, system.get(), CMD_TRAYICON);
    }
}

//---------------------------------------------------------------------------//

bool CALLBACK MainWnd::OnExecuteCommand
(
    HWND hwnd, ITTBPlugin* plugin, INT32 CmdID
)
{
    SystemLog(TEXT("%s"), TEXT("コマンドの実行"));
    SystemLog(TEXT("  %s|%i"), plugin->info()->Filename, CmdID);

    bool result;

    // コマンドの実行
    if ( plugin->is_loaded() )
    {
        // 常駐プラグインの場合
        result = plugin->Execute(CmdID, hwnd);
    }
    else
    {
        // 非常駐プラグインの場合
        result = plugin->Reload();
        if ( ! result )
        {
            SystemLog(TEXT("  %s"), TEXT("ロード失敗"));
            return false;
        }

        result = plugin->Execute(CmdID, hwnd);
        plugin->Free();
    }

    SystemLog(TEXT("  %s"), TEXT("OK"));
    return result;
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnReloadPlugins()
{
    auto&& mgr = PluginMgr::GetInstance();

    // プラグインを再ロード
    mgr.FreeAll();
    ::Sleep(100);
    mgr.LoadAll();

    // リストビューをクリア
    list_plg.DeleteAllItems();
    list_cmd.DeleteAllItems();

    // リストビューに項目を登録
    SetPluginNames   (mgr, list_plg);
    SetPluginCommands(mgr, list_cmd);

    // 一番上の項目を選択状態に
    const auto item = tree.GetSelection();
    if ( item  == tvi[1] )
    {
        list_plg.Select(0);
        ShowPluginInfo(list_plg, edit);
    }
}

//---------------------------------------------------------------------------//

void CALLBACK MainWnd::OnWindowsHook
(
    UINT Msg, WPARAM wParam, LPARAM lParam
)
{
    auto&& mgr = PluginMgr::GetInstance();
    for ( auto&& plugin: mgr )
    {
        plugin->Hook(Msg, wParam, lParam);
    }
}

//---------------------------------------------------------------------------//
// Utility Functions
//---------------------------------------------------------------------------//

void SetPluginNames
(
    const PluginMgr& mgr, tapetums::ListWnd& list
)
{
    std::array<TCHAR, 64> buf;

    auto index = 0;
    for ( auto&& plugin: mgr )
    {
        const auto& info = *plugin->info();
        list.InsertItem(info.Name, index);
        list.SetItem(info.Filename, index, LIST_PLG_ITEM::相対パス);

        list.SetItem(plugin->type(), index, LIST_PLG_ITEM::bits);
        ::StringCchPrintf
        (
            buf.data(), buf.size(), TEXT("%u.%u.%u.%u"),
            HIWORD(info.VersionMS), LOWORD(info.VersionMS),
            HIWORD(info.VersionLS), LOWORD(info.VersionLS)
        );
        list.SetItem(buf.data(), index, LIST_PLG_ITEM::バージョン);

        const auto type = (info.PluginType == ptAlwaysLoad) ?
                          TEXT("YES") : TEXT("NO");
        list.SetItem(type, index, LIST_PLG_ITEM::常駐);

        ::StringCchPrintf
        (
            buf.data(), buf.size(), TEXT("%u"), info.CommandCount
        );
        list.SetItem(buf.data(), index, LIST_PLG_ITEM::コマンド数);

        ++index;
    }
}

//---------------------------------------------------------------------------//

void SetPluginCommands
(
    const PluginMgr& mgr, tapetums::ListWnd& list
)
{
    std::array<TCHAR, 64> buf;

    auto index = 0;
    for ( auto&& plugin: mgr )
    {
        const auto& info = *plugin->info();
        const auto count = info.CommandCount;

        for ( size_t cmd_idx = 0; cmd_idx < count; ++cmd_idx )
        {
            const auto& cmd = info.Commands[cmd_idx];

            list.InsertItem(info.Name, index, (LPARAM)plugin.get());
            list.SetItem(cmd.Caption, index, LIST_CMD_ITEM::コマンド名);
            list.SetItem(info.Filename, index, LIST_CMD_ITEM::相対パス);

            ::StringCchPrintf(buf.data(), buf.size(), TEXT("%u"), cmd.CommandID);
            list.SetItem(buf.data(), index, LIST_CMD_ITEM::コマンドID);

            if ( cmd.DispMenu & DISPMENU::dmMenuChecked )
            {
                list.Check(index);
            }

            ++index;
        }
    }
}

//---------------------------------------------------------------------------//

void UpdataPluginCommands
(
    const PluginMgr& mgr, tapetums::ListWnd& list
)
{
    list.DeleteAllItems();

    SetPluginCommands(mgr, list);
}

//---------------------------------------------------------------------------//

void ShowPluginInfo
(
    tapetums::ListWnd& list, tapetums::CtrlWnd& edit
)
{
    // 選択項目のインデックスを取得
    const auto index = list.SelectedIndex();

    // 選択項目から相対パスを取得
    std::array<TCHAR, MAX_PATH> relative_path;
    list.GetItemText
    (
        index, LIST_PLG_ITEM::相対パス,
        relative_path.data(), (INT32)relative_path.size()
    );

    // 相対パスからプラグインのインスタンスを取得
    const auto plugin = PluginMgr::GetInstance().Find(relative_path.data());
    if ( nullptr == plugin )
    {
        return;
    }

    // プラグインの詳細情報を取得
    const auto& info = *plugin->info();
    std::array<TCHAR, 1024> buf;
    ::StringCchPrintf
    (
        buf.data(), buf.size(), 
        TEXT
        (
            "%s  version %u.%u.%u.%u\r\n\r\n  %s\r\n\r\n"
        ),
        info.Name,
        HIWORD(info.VersionMS), LOWORD(info.VersionMS),
        HIWORD(info.VersionLS), LOWORD(info.VersionLS),
        info.Filename
    );

    // 情報を表示
    edit.SetText(buf.data());
}

//---------------------------------------------------------------------------//

void UpdateCheckState
(
    tapetums::ListWnd& list, const ITTBPlugin* plugin, INT32 CmdID
)
{
    SystemLog(TEXT("%s"), TEXT("チェックボックスの表示状態を更新"));

    const auto count = list.Count();
    for ( auto index = 0; index < count; ++index )
    {
        if ( plugin != (ITTBPlugin*)list.GetItemLPARAM(index) )
        {
            continue;
        }
        SystemLog(TEXT("  %s"), plugin->info()->Name);

      #if 1 // TTBase のバグを再現

        const INT32 CommandCount = plugin->info()->CommandCount;
        for ( auto ID = 0; ID < CommandCount; ++ID, ++index )
        {
            if ( ID == CmdID ) { break; }
        }

        const auto& cmd = plugin->info()->Commands[CmdID];
        const auto cmd_id = list.GetItemToInt(index, LIST_CMD_ITEM::コマンドID);
        if ( cmd.DispMenu & DISPMENU_CHECKED )
        {
            list.Check(index);
            if ( list.IsChecked(index) )
            {
                SystemLog(TEXT("  %i, %i, %s"), index, cmd_id, TEXT("Checked"));
            }
            break;
        }
        else
        {
            list.Uncheck(index);
            if ( ! list.IsChecked(index) )
            {
                SystemLog(TEXT("  %i, %i, %s"), index, cmd_id, TEXT("Unhecked"));
            }
            break;
        }

      #else // 本来意図されていたであろう動作

        if ( CmdID != list.GetItemToInt(index, LIST_CMD_ITEM::ID) )
        {
            continue;
        }

        const auto&& cmd = plugin->info->Commands[CmdID];
        if ( cmd.DispMenu & DISPMENU_CHECKED )
        {
            if ( ! list.IsChecked(index) )
            {
                SystemLog(TEXT("  %i, %i, %s"), index, CmdID, TEXT("Checked"));
            }
            list.Check(index);
        }
        else
        {
            if ( list.IsChecked(index) )
            {
                SystemLog(TEXT("  %i, %i, %s"), index, CmdID, TEXT("Unhecked"));
            }
            list.Uncheck(index);
        }

      #endif
    }

    SystemLog(TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

void PopupSysMenu(HWND hwnd)
{
    auto&& mgr = PluginMgr::GetInstance();

    // マウスカーソルの位置を取得
    POINT pt;
    ::GetCursorPos(&pt);

    // リソースからメニューを取得
    const auto hMenu    = ::LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_POPUP_MENU));
    const auto hSubMenu = ::GetSubMenu(hMenu, 0);

    // メニュー項目を生成
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask  = MIIM_STRING | MIIM_STATE | MIIM_ID | MIIM_DATA;

    UINT wID = 0; // +10'000 しておいて、WM_COMMAND で取り出す
    for ( auto it = mgr.begin(); it != mgr.end(); ++it )
    {
        const auto& plugin = *it;
        const auto CommandCount = plugin->info()->CommandCount;
        for ( size_t idx = 0; idx < CommandCount; ++idx, ++wID )
        {
            const auto& cmd = plugin->info()->Commands[idx];
            const auto& DispMenu = cmd.DispMenu;
            if ( 0 == (DispMenu & dmSystemMenu) ) { continue; }

            UINT state { 0 };
            state |= DispMenu & dmMenuChecked ? MFS_CHECKED  : MFS_UNCHECKED;
            state |= DispMenu & dmDisabled    ? MFS_DISABLED : MFS_ENABLED;

            mii.dwTypeData = cmd.Caption;
            mii.fState     = state;
            mii.wID        = wID + 10'000;
            mii.dwItemData = (ULONG_PTR)&plugin;
            InsertMenuItem(hSubMenu, wID, FALSE, &mii);
        }
    }

    // Article ID: Q135788
    // ポップアップメニューから処理を戻すために必要
    ::SetForegroundWindow(hwnd);

    // セパレータの位置を移動
    //  メニューリソースでは項目が予め最低一つないと
    //  メニューが表示されないバグがあるため、
    //  最初にダミーとして一つセパレータを入れてある
    DeleteMenu(hSubMenu, 1, MF_BYPOSITION);
    mii.fMask = MIIM_FTYPE;
    mii.fType = MFT_SEPARATOR;
    InsertMenuItem(hSubMenu, 3, TRUE, &mii);

    // ポップアップメニューを表示
    ::TrackPopupMenu
    (
        hSubMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr
    );

    // 表示したメニューを破棄
    ::DestroyMenu(hMenu);

    // Article ID: Q135788
    // ポップアップメニューから処理を戻すために必要
    ::PostMessage(hwnd, WM_NULL, 0, 0);
}

//---------------------------------------------------------------------------//

void PopupToolMenu(HWND hwnd)
{
    auto&& mgr = PluginMgr::GetInstance();

    // マウスカーソルの位置を取得
    POINT pt;
    ::GetCursorPos(&pt);

    // リソースからメニューを取得
    const auto hMenu    = ::LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_POPUP_MENU));
    const auto hSubMenu = ::GetSubMenu(hMenu, 1);

    // メニュー項目を生成
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask  = MIIM_STRING | MIIM_STATE | MIIM_ID | MIIM_DATA;

    UINT wID = 0; // +20'000 しておいて、WM_COMMAND で取り出す
    for ( auto it = mgr.begin(); it != mgr.end(); ++it )
    {
        const auto& plugin = *it;
        const auto CommandCount = plugin->info()->CommandCount;
        for ( size_t idx = 0; idx < CommandCount; ++idx, ++wID )
        {
            const auto& cmd = plugin->info()->Commands[idx];
            const auto& DispMenu = cmd.DispMenu;
            if ( 0 == (DispMenu & dmToolMenu) ) { continue; }

            UINT state { 0 };
            state |= DispMenu & dmMenuChecked ? MFS_CHECKED  : MFS_UNCHECKED;
            state |= DispMenu & dmDisabled    ? MFS_DISABLED : MFS_ENABLED;

            mii.dwTypeData = cmd.Caption;
            mii.fState     = state;
            mii.wID        = wID + 20'000;
            mii.dwItemData = (ULONG_PTR)&plugin;
            InsertMenuItem(hSubMenu, wID, FALSE, &mii);
        }
    }

    // セパレータの位置を移動
    //  メニューリソースでは項目が予め最低一つないと
    //  メニューが表示されないバグがあるため、
    //  最初にダミーとして一つセパレータを入れてある
    DeleteMenu(hSubMenu, 0, MF_BYPOSITION);
    mii.fMask = MIIM_FTYPE;
    mii.fType = MFT_SEPARATOR;
    InsertMenuItem(hSubMenu, 3, TRUE, &mii);

    // Article ID: Q135788
    // ポップアップメニューから処理を戻すために必要
    ::SetForegroundWindow(hwnd);

    // ポップアップメニューを表示
    ::TrackPopupMenu
    (
        hSubMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr
    );

    // 表示したメニューを破棄
    ::DestroyMenu(hMenu);

    // Article ID: Q135788
    // ポップアップメニューから処理を戻すために必要
    ::PostMessage(hwnd, WM_NULL, 0, 0);
}

//---------------------------------------------------------------------------//

// TestWnd.cpp