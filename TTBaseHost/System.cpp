﻿//---------------------------------------------------------------------------//
//
// System.cpp
//  システムプラグイン および TTBase ホストプログラム 本体
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <array>

#include <windows.h>
#include <strsafe.h>

#include "../Plugin.hpp"
#include "../Utility.hpp"
#include "PluginMgr.hpp"
#include "Settings.hpp"
#include "Command.hpp"

//---------------------------------------------------------------------------//
// Global Variables
//---------------------------------------------------------------------------//

// WinMain.cpp で宣言
extern HINSTANCE g_hInst;
extern HWND      g_hwnd;

// MainWnd.cpp で宣言
extern UINT WM_OPENFOLDER;
extern UINT WM_RELOADPLUGINS;
extern UINT WM_SETTASKTRAYICON;
extern UINT WM_SETMENUPROPERTY;
extern UINT WM_SETTASKTRAYICON;
extern UINT WM_EXECUTECOMMAND;

//---------------------------------------------------------------------------//

// プラグインの名前
LPCTSTR PLUGIN_NAME { TEXT("hako") };

// コマンドの数
DWORD COMMAND_COUNT { CMD_COUNT };

//---------------------------------------------------------------------------//

// コマンドの情報
PLUGIN_COMMAND_INFO g_cmd_info[] =
{
    {
        (LPTSTR)TEXT("Exit"), // コマンド名（英名）
        (LPTSTR)TEXT("終了"), // コマンド説明（日本語）
        CMD_EXIT,             // コマンドID
        0,                    // Attr（未使用）
        -1,                   // ResTd(未使用）
        dmHotKeyMenu,         // DispMenu
        0,                    // TimerInterval[msec] 0で使用しない
        0                     // TimerCounter（未使用）
    },
    {
        (LPTSTR)TEXT("Settings"),   // コマンド名（英名）
        (LPTSTR)TEXT("本体の設定"), // コマンド説明（日本語）
        CMD_SETTINGS,               // コマンドID
        0,                          // Attr（未使用）
        -1,                         // ResTd(未使用）
        dmHotKeyMenu,               // DispMenu
        0,                          // TimerInterval[msec] 0で使用しない
        0                           // TimerCounter（未使用）
    },
    {
        (LPTSTR)TEXT("Open Folder"),                // コマンド名（英名）
        (LPTSTR)TEXT("インストールフォルダを開く"), // コマンド説明（日本語）
        CMD_OPEN_FOLDER,                            // コマンドID
        0,                                          // Attr（未使用）
        -1,                                         // ResTd(未使用）
        dmHotKeyMenu,                               // DispMenu
        0,                                          // TimerInterval[msec] 0で使用しない
        0                                           // TimerCounter（未使用）
    },
    {
        (LPTSTR)TEXT("Reload all plugins"),    // コマンド名（英名）
        (LPTSTR)TEXT("プラグインを再ロード"),  // コマンド説明（日本語）
        CMD_RELOAD,                            // コマンドID
        0,                                     // Attr（未使用）
        -1,                                    // ResTd(未使用）
        dmHotKeyMenu,                          // DispMenu
        0,                                     // TimerInterval[msec] 0で使用しない
        0                                      // TimerCounter（未使用）
    },
    {
        (LPTSTR)TEXT("Show/Hide tray icon"),        // コマンド名（英名）
        (LPTSTR)TEXT("トレイアイコンの表示を切替"), // コマンド説明（日本語）
        CMD_TRAYICON,                               // コマンドID
        0,                                          // Attr（未使用）
        -1,                                         // ResTd(未使用）
        (DISPMENU)(dmHotKeyMenu | dmMenuChecked),   // DispMenu
        0,                                          // TimerInterval[msec] 0で使用しない
        0                                           // TimerCounter（未使用）
    },
};

//---------------------------------------------------------------------------//

// プラグインの情報
PLUGIN_INFO g_info =
{
    0,                   // プラグインI/F要求バージョン
    (LPTSTR)PLUGIN_NAME, // プラグインの名前（任意の文字が使用可能）
    nullptr,             // プラグインのファイル名（相対パス）
    ptAlwaysLoad,        // プラグインのタイプ
    0,                   // バージョン
    0,                   // バージョン
    COMMAND_COUNT,       // コマンド個数
    &g_cmd_info[0],      // コマンド
    0,                   // ロードにかかった時間（msec）
};

//---------------------------------------------------------------------------//
// システムプラグインの内部実装
//---------------------------------------------------------------------------//

BOOL Init()
{
    // バージョン情報を設定
    std::array<TCHAR, MAX_PATH> path;
    ::GetModuleFileName(g_hInst, path.data(),(DWORD)path.size());
    GetVersion(path.data(), &g_info.VersionMS, &g_info.VersionLS);

    // 設定ファイルから読み込んだ項目を表示に反映
    const auto ShowTaskTrayIcon = settings::get().ShowTaskTrayIcon;
    TTBPlugin_SetMenuProperty
    (
        g_hPlugin, CMD_TRAYICON, DISPMENU_CHECKED,
        ShowTaskTrayIcon ? dmMenuChecked : dmUnchecked
    );

    return TRUE;
}

//---------------------------------------------------------------------------//

void Unload()
{
}

//---------------------------------------------------------------------------//

BOOL Execute(INT32 CmdID, HWND hwnd)
{
    // メインウィンドウ内のルーチンに処理を投げる
    switch ( CmdID )
    {
        case CMD_EXIT:
        {
            ::PostMessage(hwnd, WM_DESTROY, 0, 0);
            return TRUE;
        }
        case CMD_SETTINGS:
        {
            ::PostMessage(hwnd, WM_SHOWWINDOW, SW_SHOWNORMAL, 0);
            return TRUE;
        }
        case CMD_OPEN_FOLDER:
        {
            ::PostMessage(hwnd, WM_OPENFOLDER, 0, 0);
            return TRUE;
        }
        case CMD_RELOAD:
        {
            ::PostMessage(hwnd, WM_RELOADPLUGINS, 0, 0);
            return TRUE;
        }
        case CMD_TRAYICON:
        {
            ::PostMessage(hwnd, WM_SETTASKTRAYICON, 0, (LPARAM)TEXT("TOGGLE"));
            return TRUE;
        }
        default:
        {
            return FALSE;
        }
    }
}

//---------------------------------------------------------------------------//

void Hook(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    Msg; wParam; lParam;
    //WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("フック / 未実装"));
    //WriteLog(ERROR_LEVEL(5), TEXT("  %u"), Msg);
    //WriteLog(ERROR_LEVEL(5), TEXT("  %u"), wParam);
    //WriteLog(ERROR_LEVEL(5), TEXT("  %i"), lParam);
}

//---------------------------------------------------------------------------//
// 本体側エクスポート関数
//---------------------------------------------------------------------------//

extern "C" PLUGIN_INFO* WINAPI TTBPlugin_GetPluginInfo
(
    DWORD_PTR hPlugin
)
{
    //WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグインの情報を取得"));

    auto plugin = reinterpret_cast<TTBasePlugin*>(hPlugin);
    if ( plugin )
    {
        //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), plugin->info->Name);
        return CopyPluginInfo(plugin->info());
    }
    else
    {
        //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("?"));
        return nullptr;
    }
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_SetPluginInfo
(
    DWORD_PTR hPlugin, PLUGIN_INFO* PLUGIN_INFO
)
{
    ////WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグインの情報を設定"));

    auto plugin = reinterpret_cast<TTBasePlugin*>(hPlugin);
    if ( nullptr == plugin )
    {
        //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("?"));
        return;
    }

    FreePluginInfo(plugin->info());
    plugin->info(CopyPluginInfo(PLUGIN_INFO));

    //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_FreePluginInfo
(
    PLUGIN_INFO* PLUGIN_INFO
)
{
    //WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグインの情報を解放"));
    if ( nullptr == PLUGIN_INFO )
    {
        //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("?"));
        return;
    }

    //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), PLUGIN_INFO->Name);
    FreePluginInfo(PLUGIN_INFO);

    //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_SetMenuProperty
(
    DWORD_PTR hPlugin, INT32 CommandID, CHANGE_FLAG ChangeFlag, DISPMENU Flag
)
{
    //WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("メニューのプロパティを設定"));

    auto plugin = reinterpret_cast<TTBasePlugin*>(hPlugin);
    if ( nullptr == plugin )
    {
        //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("?"));
        return;
    }

    const auto& info = *plugin->info();
    //WriteLog(ERROR_LEVEL(5), TEXT("  名前: %s"), info.Name);
    //WriteLog(ERROR_LEVEL(5), TEXT("  ID:   %i"), CommandID);
    //WriteLog(ERROR_LEVEL(5), TEXT("  Flag: %u"), Flag);

    const auto CommandCount = info.CommandCount;
    if ( (DWORD)CommandID >= CommandCount )
    {
        //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("Bad index"));
        return;
    }

  #if 1 // TTBase のバグを再現

    auto DispMenu = (DWORD)info.Commands[CommandID].DispMenu;
    DispMenu = (DispMenu & ~ChangeFlag) | (ChangeFlag & Flag);
    info.Commands[CommandID].DispMenu = (DISPMENU)DispMenu;

  #else // 本来意図されていたであろう動作

    for ( auto idx = 0; idx < CommandCount; ++idx )
    {
        if ( CommandID == info.Commands[idx].CommandID )
        {
            auto DispMenu = (DWORD)info.Commands[idx].DispMenu;
            DispMenu = (DispMenu & ~ChangeFlag) | (ChangeFlag & Flag);
            info.Commands[idx].DispMenu = (DISPMENU)DispMenu;
        }
    }

  #endif

    // チェック状態を再描画する
    ::PostMessage(g_hwnd, WM_SETMENUPROPERTY, (WPARAM)plugin, (LPARAM)CommandID);
    //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

extern "C" PLUGIN_INFO** WINAPI TTBPlugin_GetAllPluginInfo()
{
    //WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("すべてのプラグインの情報を取得"));
    auto&& mgr = PluginMgr::GetInstance();

    auto PluginInfoArray = (PLUGIN_INFO**) new DWORD_PTR[mgr.size() + 1];
    if ( nullptr == PluginInfoArray )
    {
        return nullptr;
    }

    size_t idx = 0;
    for ( auto&& plugin: mgr )
    {
        PluginInfoArray[idx] = CopyPluginInfo(plugin.info());
        ++idx;
    }

    // 配列はヌル終端にする
    PluginInfoArray[idx] = nullptr;

    //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
    return PluginInfoArray;
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_FreePluginInfoArray
(
    PLUGIN_INFO** PluginInfoArray
)
{
    //WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("すべてのプラグインの情報を解放"));
    if ( nullptr == PluginInfoArray ) { return; }

    size_t idx = 0;
    while ( auto info = PluginInfoArray[idx] )
    {
        FreePluginInfo(info);
        ++idx;
    }

    delete[] PluginInfoArray;

    //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_SetTaskTrayIcon
(
    HICON hIcon, LPCTSTR Tips
)
{
    //WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("タスクトレイのアイコンを変更"));

    // メインウィンドウ内のルーチンに処理を投げる
    ::PostMessage(g_hwnd, WM_SETTASKTRAYICON, (WPARAM)hIcon, (LPARAM)Tips);

    //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_WriteLog
(
    DWORD_PTR /*hPlugin*/, ERROR_LEVEL logLevel, LPCTSTR msg
)
{
    constexpr const TCHAR* err_level[] =
    {
        TEXT(""),
        TEXT("[ERR]  "),
        TEXT("[WARN] "),
        TEXT("[INFO] "),
        TEXT("[DBG]  "),
        TEXT(""),
    };

    // 設定以下のログレベル項目は出力しない
    if ( logLevel > settings::get().logLevel ) { return; }

    SYSTEMTIME st;
    ::GetLocalTime(&st);

    //reinterpret_cast<TTBasePlugin*>(hPlugin);

    static std::array<TCHAR, 4096> buf;
    ::StringCchPrintf
    (
        buf.data(), buf.size(),
        TEXT("%04u/%02u/%02u %02u:%02u:%02u;%03u> %s%s\r\n"),
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        err_level[logLevel], msg
    );

    // 指定先にログを出力
    if ( settings::get().logToWindow )
    {
        ::OutputDebugString(buf.data());
    }
    if ( settings::get().logToFile )
    {
        auto&& file = settings::get().file;
        file.Write(buf.data(), lstrlen(buf.data()) * sizeof(TCHAR));
    }
}

//---------------------------------------------------------------------------//

extern "C" BOOL WINAPI TTBPlugin_ExecuteCommand
(
    LPCTSTR PluginFilename, INT32 CmdID
)
{
    //WriteLog(ERROR_LEVEL(5), TEXT("実行: %s|%d"), PluginFilename, CmdID);

    // 相対パスからプラグインのインスタンスを検索
    auto plugin = PluginMgr::GetInstance().Find(PluginFilename);
    if ( nullptr == plugin )
    {
        //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("Not found"));
        return FALSE;
    }

    // あとはメインウィンドウ内のルーチンに処理を任せる
    const auto result = ::PostMessage
    (
        g_hwnd, WM_EXECUTECOMMAND, (WPARAM)plugin, (LPARAM)CmdID
    );

    //WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
    return result;
}

//---------------------------------------------------------------------------//

// System.cpp