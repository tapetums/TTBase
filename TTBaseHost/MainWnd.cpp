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

#include "resource.h"

#include "../Plugin.hpp"
#include "../Utility.hpp"
#include "Settings.hpp"
#include "Command.hpp"
#include "PluginMgr.hpp"

#include "MainWnd.hpp"

//---------------------------------------------------------------------------//
// Global Variables
//---------------------------------------------------------------------------//

// WinMain.cpp で宣言
extern HINSTANCE g_hInst;

//---------------------------------------------------------------------------//

UINT WM_OPENFOLDER      { 0 };
UINT WM_RELOADPLUGINS   { 0 };
UINT WM_SETMENUPROPERTY { 0 };
UINT WM_SETTASKTRAYICON { 0 };
UINT WM_EXECUTECOMMAND  { 0 };

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
        TREEVIEW             = 1001,
        LABEL                = 1002,
        BTN_OPEN_INST_FOLDER = 1003,
        BTN_EXIT             = 1004,
        LIST_PLUGIN          = 1005,
        LIST_COMMAND         = 1006,
    };
};

struct LIST_PLG_ITEM
{
    enum : INT32
    {
        名前, 相対パス, バージョン, 常駐, コマンド数,
    };

    static constexpr LPCTSTR Name      = TEXT("名前");
    static constexpr LPCTSTR Filename  = TEXT("相対パス");
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

//---------------------------------------------------------------------------//
// Utility Functions
//---------------------------------------------------------------------------//

void SetPluginNames   (const PluginMgr& mgr, tapetums::ListWnd& list);
void SetPluginCommands(const PluginMgr& mgr, tapetums::ListWnd& list);
void ShowPluginInfo   (tapetums::ListWnd& list, tapetums::CtrlWnd& label);
void UpdateCheckState (tapetums::ListWnd& list, TTBasePlugin* plugin, INT32 CmdID);

//---------------------------------------------------------------------------//
// Methods
//---------------------------------------------------------------------------//

MainWnd::MainWnd()
{
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

LRESULT MainWnd::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_SHOWWINDOW && wParam == SW_SHOWNORMAL)
    {
        ToCenter(); Show(); ::SetForegroundWindow(hwnd); return 0;
    }
    if ( uMsg == WM_CTLCOLORSTATIC && label == (HWND)lParam )
    {
        return (LRESULT)GetStockBrush(WHITE_BRUSH); // 背景を白に
    }
    if ( uMsg == WM_NOTIFYICON )
    {
        OnNotifyIcon(hwnd, (UINT)lParam); return 0;
    }
    if ( uMsg == WM_OPENFOLDER )
    {
        OnCommand(hwnd, CTRL::BTN_OPEN_INST_FOLDER, nullptr, 0); return 0;
    }
    if ( uMsg == WM_RELOADPLUGINS )
    {
        OnReloadPlugins(); return 0;
    }
    if ( uMsg == WM_SETMENUPROPERTY )
    {
        UpdateCheckState(list_cmd, (TTBasePlugin*)wParam, (INT32)lParam); return 0;
    }
    if ( uMsg == WM_SETTASKTRAYICON )
    {
        OnSetTaskTrayIcon((HICON)wParam, (LPCTSTR)lParam); return 0;
    }
    if ( uMsg == WM_EXECUTECOMMAND )
    {
        OnExecuteCommand(hwnd, (TTBasePlugin*)wParam, (INT32)lParam); return 0;
    }

    switch ( uMsg )
    {
        HANDLE_MSG(hwnd, WM_CREATE,  OnCreate);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_SIZE,    OnSize);
        HANDLE_MSG(hwnd, WM_PAINT,   OnPaint);
        HANDLE_MSG(hwnd, WM_CLOSE,   OnClose);
        HANDLE_MSG(hwnd, WM_NOTIFY,  OnNotify);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        default: return super::WndProc(hwnd, uMsg, wParam, lParam);
    }
}

//---------------------------------------------------------------------------//

BOOL MainWnd::OnCreate(HWND hwnd, LPCREATESTRUCT)
{
    // 独自ウィンドウメッセージの登録
    WM_OPENFOLDER      = ::RegisterWindowMessage(TEXT("tapetums::WM_OPENFOLDER"));
    WM_RELOADPLUGINS   = ::RegisterWindowMessage(TEXT("tapetums::WM_RELOADPLUGINS"));
    WM_SETMENUPROPERTY = ::RegisterWindowMessage(TEXT("tapetums::WM_SETMENUPROPERTY"));
    WM_SETTASKTRAYICON = ::RegisterWindowMessage(TEXT("tapetums::WM_SETTASKTRAYICON"));
    WM_EXECUTECOMMAND  = ::RegisterWindowMessage(TEXT("tapetums::WM_EXECUTECOMMAND"));

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

    // コントロールの生成
    tree.Create(hwnd, CTRL::TREEVIEW);
    tree.SetFont(font);
    tree.SetImageList(ImageList_Create(1, 32, ILC_COLOR, 0, 0));
    tvi[0] = tree.InsertItem(TEXT("プラグイン"));
    tvi[1] = tree.InsertItem(TEXT("コマンド"));

    label.Create(ES_READONLY | ES_MULTILINE, hwnd, CTRL::LABEL);
    label.SetFont(font);
    label.Show();

    btn_open_inst_folder.Create(BS_PUSHBUTTON, hwnd, CTRL::BTN_OPEN_INST_FOLDER);
    btn_open_inst_folder.SetFont(font);
    btn_open_inst_folder.SetText(TEXT("インストールフォルダを開く"));

    btn_exit.Create(BS_PUSHBUTTON, hwnd, CTRL::BTN_EXIT);
    btn_exit.SetFont(font);
    btn_exit.SetText(TEXT("終了"));

    auto style = WS_VSCROLL | WS_HSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL;
    auto styleEx = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;
    list_plg.Create(style, styleEx, m_hwnd, CTRL::LIST_PLUGIN);
    list_plg.InsertColumn(LIST_PLG_ITEM::Name,     120, LIST_PLG_ITEM::名前);
    list_plg.InsertColumn(LIST_PLG_ITEM::Filename, 170, LIST_PLG_ITEM::相対パス);
    list_plg.InsertColumn(LIST_PLG_ITEM::Version,   80, LIST_PLG_ITEM::バージョン);
    list_plg.InsertColumn(LIST_PLG_ITEM::Residence, 40, LIST_PLG_ITEM::常駐);
    list_plg.InsertColumn(LIST_PLG_ITEM::CmdCount,  60, LIST_PLG_ITEM::コマンド数);

    styleEx |= LVS_EX_CHECKBOXES;
    list_cmd.Create(style, styleEx, hwnd, CTRL::LIST_COMMAND);
    list_cmd.InsertColumn(LIST_CMD_ITEM::Name,     120, LIST_CMD_ITEM::名前);
    list_cmd.InsertColumn(LIST_CMD_ITEM::CmdName,  120, LIST_CMD_ITEM::コマンド名);
    list_cmd.InsertColumn(LIST_CMD_ITEM::Filename, 170, LIST_CMD_ITEM::相対パス);
    list_cmd.InsertColumn(LIST_CMD_ITEM::CmdID,     60, LIST_CMD_ITEM::コマンドID);

    // リストビューに項目を登録
    const auto& mgr = PluginMgr::GetInstance();
    SetPluginNames   (mgr, list_plg);
    SetPluginCommands(mgr, list_cmd);

    // 「プラグイン」設定項目を選択
    tree.Select(tvi[0]);

    return TRUE;
}

//---------------------------------------------------------------------------//

void MainWnd::OnDestroy(HWND)
{
    // タスクトレイのアイコンを破棄
    DeleteNotifyIcon(ID_TRAYICON);
    ::DestroyIcon(icon);

    // アプリケーションを終了
    ::PostQuitMessage(0);
}

//---------------------------------------------------------------------------//

void MainWnd::OnSize(HWND, UINT , INT32 cx, INT32 cy)
{
    // コントロールを配置
    //  TODO: DPIスケーリングへの対応
    tree.Bounds(2, 2, 140 - 4, cy - 4);

    label.Bounds(140 + 12, 12, cx - 140 - 140 - 24, 100 - 24);
    btn_open_inst_folder.Bounds(cx - 128 - 12, 16, 128, 24);
    btn_exit.Bounds(cx - 128 - 12, 52, 128, 24);

    list_plg.Bounds(140 + 2, 90 + 2, cx - 140 - 4, cy - 90 - 4);
    list_cmd.Bounds(140 + 2, 90 + 2, cx - 140 - 4, cy - 90 - 4);
}

//---------------------------------------------------------------------------//

void MainWnd::OnPaint(HWND hwnd)
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

void MainWnd::OnClose(HWND)
{
    Hide();
}

//---------------------------------------------------------------------------//

LRESULT MainWnd::OnNotify(HWND hwnd, INT32, LPNMHDR pNMHdr)
{
    if ( pNMHdr->idFrom == CTRL::LIST_PLUGIN && pNMHdr->code == NM_CLICK )
    {
        // プラグインの詳細情報を表示
        ShowPluginInfo(list_plg, label);
    }
    else if ( pNMHdr->idFrom == CTRL::LIST_COMMAND && pNMHdr->code == NM_CLICK )
    {
        // ユーザーによるチェックボックスの操作を無効化
        POINT pt;
        ::GetCursorPos(&pt);
        ::ScreenToClient(pNMHdr->hwndFrom, &pt);

        if ( pt.x <= 16 ) { return TRUE; }
    }
    else if ( pNMHdr->idFrom == CTRL::LIST_COMMAND && pNMHdr->code == NM_DBLCLK )
    {
        // コマンドの実行
        const auto index = list_cmd.SelectedIndex();

        auto plugin = (TTBasePlugin*)list_cmd.GetItemLPARAM(index);
        const auto CmdID = list_cmd.GetItemToInt(index, LIST_CMD_ITEM::コマンドID);

        return OnExecuteCommand(hwnd, plugin, CmdID);
    }
    else if ( pNMHdr->idFrom == CTRL::TREEVIEW && pNMHdr->code == TVN_SELCHANGED )
    {
        // コントロール表示の切替
        const auto item = tree.GetSelection();
        if ( item == tvi[0] )
        {
            label.SetText(TEXT(""));
            list_plg.Show();
            list_cmd.Hide();

            // 一番上の項目を選択状態に
            list_plg.Select(0);
            ShowPluginInfo(list_plg, label);
        }
        else
        {
            label.SetText(TEXT("項目をダブルクリックして実行"));
            list_plg.Hide();
            list_cmd.Show();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------//

void MainWnd::OnCommand(HWND, UINT id, HWND, UINT)
{
    switch ( id )
    {
        case CTRL::BTN_EXIT:
        {
            // アプリケーションを終了
            Destroy();
            break;
        }
        case CTRL::BTN_OPEN_INST_FOLDER:
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
            break;
        }
    }
}

//---------------------------------------------------------------------------//

void MainWnd::OnNotifyIcon(HWND hwnd, UINT uMsg)
{
    if ( uMsg == WM_LBUTTONDOWN )
    {
        // ウィンドウを表示
        if ( ! m_is_fullscreen ) { ToCenter(); }

        Show();
        ::SetForegroundWindow(hwnd);
    }
    else if ( uMsg == WM_RBUTTONDOWN )
    {
        // アプリケーションを終了
        Destroy();
    }
}

//---------------------------------------------------------------------------//

void MainWnd::OnReloadPlugins()
{
    auto&& mgr = PluginMgr::GetInstance();

    // プラグインを再ロード
    mgr.FreeAll();
    mgr.LoadAll();

    // リストビューをクリア
    list_plg.DeleteAllItems();
    list_cmd.DeleteAllItems();

    // リストビューに項目を登録
    SetPluginNames   (mgr, list_plg);
    SetPluginCommands(mgr, list_cmd);

    // 一番上の項目を選択状態に
    const auto item = tree.GetSelection();
    if ( item  == tvi[0] )
    {
        list_plg.Select(0);
        ShowPluginInfo(list_plg, label);
    }
}

//---------------------------------------------------------------------------//

void MainWnd::OnSetTaskTrayIcon(HICON hIcon, LPCTSTR Tips)
{
    if ( 0 == lstrcmp(Tips, TEXT("TOGGLE")) )
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
        auto DispMenu = (DWORD)g_info.Commands[CMD_TRAYICON].DispMenu;
        DispMenu ^= dmMenuChecked;
        g_info.Commands[CMD_TRAYICON].DispMenu = (DISPMENU)DispMenu;

        UpdateCheckState(list_cmd, &PluginMgr::GetInstance().system(), CMD_TRAYICON);
    }
    else
    {
        // タスクトレイアイコンを差替
        if ( nullptr == hIcon )
        {
            ::MessageBox(nullptr, TEXT("無効なアイコンです"), Tips, MB_OK);
            return;
        }

        DeleteNotifyIcon(ID_TRAYICON);
        ::DestroyIcon(icon);
        icon = hIcon;

        AddNotifyIcon(ID_TRAYICON, icon);
        SetNotifyIconTip(ID_TRAYICON, Tips);
    }
}

//---------------------------------------------------------------------------//

bool MainWnd::OnExecuteCommand(HWND hwnd, TTBasePlugin* plugin, INT32 CmdID)
{
    //WriteLog(ERROR_LEVEL(5), TEXT("実行: %s|%d"), plugin->info->Name, CmdID);

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
            //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("ロード失敗"));
            return false;
        }

        result = plugin->Execute(CmdID, hwnd);
        plugin->Unload();
    }

    //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
    return result;
}

//---------------------------------------------------------------------------//
// Utility Functions
//---------------------------------------------------------------------------//

void SetPluginNames(const PluginMgr& mgr, tapetums::ListWnd& list)
{
    std::array<TCHAR, 64> buf;

    auto index = 0;
    for ( auto&& plugin: mgr )
    {
        const auto& info = *plugin.info();
        list.InsertItem(info.Name, index);
        list.SetItem(info.Filename, index, LIST_PLG_ITEM::相対パス);

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

void SetPluginCommands(const PluginMgr& mgr, tapetums::ListWnd& list)
{
    std::array<TCHAR, 64> buf;

    auto index = 0;
    for ( auto&& plugin: mgr )
    {
        const auto& info = *plugin.info();
        const auto count = info.CommandCount;

        for ( size_t cmd_idx = 0; cmd_idx < count; ++cmd_idx )
        {
            const auto& cmd = info.Commands[cmd_idx];

            list.InsertItem(info.Name, index, (LPARAM)&plugin);
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

void ShowPluginInfo(tapetums::ListWnd& list, tapetums::CtrlWnd& label)
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
    label.SetText(buf.data());
}

//---------------------------------------------------------------------------//

void UpdateCheckState(tapetums::ListWnd& list, TTBasePlugin* plugin, INT32 CmdID)
{
    //WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("チェックボックスの表示状態を更新"));

    const auto count = list.Count();
    for ( auto index = 0; index < count; ++index )
    {
        if ( plugin != (TTBasePlugin*)list.GetItemLPARAM(index) )
        {
            continue;
        }
        //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), plugin->info->Name);

      #if 1 // TTBase のバグを再現

        const auto CommandCount = plugin->info()->CommandCount;
        for ( size_t ID = 0; ID < CommandCount; ++ID, ++index )
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
                //WriteLog(ERROR_LEVEL(5), TEXT("  %i, %i, %s"), index, cmd_id, TEXT("Checked"));
            }
            break;;
        }
        else
        {
            list.Uncheck(index);
            if ( ! list.IsChecked(index) )
            {
                //WriteLog(ERROR_LEVEL(5), TEXT("  %i, %i, %s"), index, cmd_id, TEXT("Unhecked"));
            }
            break;;
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
                //WriteLog(ERROR_LEVEL(5), TEXT("  %i, %i, %s"), index, CmdID, TEXT("Checked"));
            }
            list.Check(index);
        }
        else
        {
            if ( list.IsChecked(index) )
            {
                //WriteLog(ERROR_LEVEL(5), TEXT("  %i, %i, %s"), index, CmdID, TEXT("Unhecked"));
            }
            list.Uncheck(index);
        }

      #endif
    }

    //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

// TestWnd.cpp