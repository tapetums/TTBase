//---------------------------------------------------------------------------//
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
#include "../MessageDef.hpp"
#include "../Utility.hpp"
#include "PluginMgr.hpp"
#include "Settings.hpp"
#include "Command.hpp"

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
extern HWND      g_hwnd;

// MainWnd.cpp で宣言
extern UINT TTB_SHOW_SETTINGS;
extern UINT TTB_OPEN_FOLDER;
extern UINT TTB_SHOW_VER_INFO;
extern UINT TTB_RELOAD_PLUGINS;
extern UINT TTB_SET_PLUGIN_INFO;
extern UINT TTB_SET_MENU_PROPERTY;
extern UINT TTB_SET_TASK_TRAY_ICON;
extern UINT TTB_EXECUTE_COMMAND;

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
        (LPTSTR)TEXT("Exit"),                  // コマンド名（英名）
        (LPTSTR)TEXT("本体の終了"),            // コマンド説明（日本語）
        CMD_EXIT,                              // コマンドID
        0,                                     // Attr（未使用）
        -1,                                    // ResTd(未使用）
        DISPMENU(dmSystemMenu | dmHotKeyMenu), // DispMenu
        0,                                     // TimerInterval[msec] 0で使用しない
        0                                      // TimerCounter（未使用）
    },
    {
        (LPTSTR)TEXT("Settings"),              // コマンド名（英名）
        (LPTSTR)TEXT("本体の設定"),            // コマンド説明（日本語）
        CMD_SETTINGS,                          // コマンドID
        0,                                     // Attr（未使用）
        -1,                                    // ResTd(未使用）
        DISPMENU(dmSystemMenu | dmHotKeyMenu), // DispMenu
        0,                                     // TimerInterval[msec] 0で使用しない
        0                                      // TimerCounter（未使用）
    },
    {
        (LPTSTR)TEXT("Show Version Info"),     // コマンド名（英名）
        (LPTSTR)TEXT("本体のバージョン情報"),  // コマンド説明（日本語）
        CMD_SHOW_VER_INFO,                     // コマンドID
        0,                                     // Attr（未使用）
        -1,                                    // ResTd(未使用）
        DISPMENU(dmSystemMenu | dmHotKeyMenu), // DispMenu
        0,                                     // TimerInterval[msec] 0で使用しない
        0                                      // TimerCounter（未使用）
    },
    {
        (LPTSTR)TEXT("Reload all plugins"),    // コマンド名（英名）
        (LPTSTR)TEXT("プラグインを再ロード"),  // コマンド説明（日本語）
        CMD_RELOAD,                            // コマンドID
        0,                                     // Attr（未使用）
        -1,                                    // ResTd(未使用）
        DISPMENU(dmToolMenu | dmHotKeyMenu),   // DispMenu
        0,                                     // TimerInterval[msec] 0で使用しない
        0                                      // TimerCounter（未使用）
    },
    {
        (LPTSTR)TEXT("Open Installation Folder"),   // コマンド名（英名）
        (LPTSTR)TEXT("インストールフォルダを開く"), // コマンド説明（日本語）
        CMD_OPEN_FOLDER,                            // コマンドID
        0,                                          // Attr（未使用）
        -1,                                         // ResTd(未使用）
        DISPMENU(dmToolMenu | dmHotKeyMenu),        // DispMenu
        0,                                          // TimerInterval[msec] 0で使用しない
        0                                           // TimerCounter（未使用）
    },
    {
        (LPTSTR)TEXT("Show/Hide tray icon"),                 // コマンド名（英名）
        (LPTSTR)TEXT("トレイアイコンの表示を切替"),          // コマンド説明（日本語）
        CMD_TRAYICON,                                        // コマンドID
        0,                                                   // Attr（未使用）
        -1,                                                  // ResTd(未使用）
        DISPMENU(dmToolMenu | dmHotKeyMenu | dmMenuChecked), // DispMenu
        0,                                                   // TimerInterval[msec] 0で使用しない
        0                                                    // TimerCounter（未使用）
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

BOOL WINAPI Init()
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

void WINAPI Unload()
{
}

//---------------------------------------------------------------------------//

BOOL WINAPI Execute(INT32 CmdID, HWND hwnd)
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
            ::PostMessage(hwnd, TTB_SHOW_SETTINGS, 0, 0);
            return TRUE;
        }
        case CMD_SHOW_VER_INFO:
        {
            ::PostMessage(hwnd, TTB_SHOW_VER_INFO, 0, 0);
            return TRUE;
        }
        case CMD_RELOAD:
        {
            ::PostMessage(hwnd, TTB_RELOAD_PLUGINS, 0, 0);
            return TRUE;
        }
        case CMD_OPEN_FOLDER:
        {
            ::PostMessage(hwnd, TTB_OPEN_FOLDER, 0, 0);
            return TRUE;
        }
        case CMD_TRAYICON:
        {
            ::PostMessage(hwnd, TTB_SET_TASK_TRAY_ICON, 0, 0);
            return TRUE;
        }
        default:
        {
            return FALSE;
        }
    }
}

//---------------------------------------------------------------------------//

void WINAPI Hook(UINT, WPARAM, LPARAM)
{
}

//---------------------------------------------------------------------------//
// 本体側エクスポート関数
//---------------------------------------------------------------------------//

extern "C" PLUGIN_INFO* WINAPI TTBPlugin_GetPluginInfo
(
    DWORD_PTR hPlugin
)
{
    SystemLog(TEXT("%s"), TEXT("プラグインの情報を取得"));

    // リストに登録されているか調べる
    auto&& mgr = PluginMgr::GetInstance();
    for ( auto&& plugin: mgr )
    {
        if ( plugin.get() != (ITTBPlugin*)hPlugin ) { continue; }

        SystemLog(TEXT("  %s"), plugin->info()->Name);
        SystemLog(TEXT("  %s"), TEXT("OK"));

        // コピーしたものを返す
        return CopyPluginInfo(plugin->info());
    }

    // 知らないプラグインなのでどうしようもない
    SystemLog(TEXT("  %s"), TEXT("Not found"));
    return nullptr;
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_SetPluginInfo
(
    DWORD_PTR hPlugin, PLUGIN_INFO* PLUGIN_INFO
)
{
    SystemLog(TEXT("%s"), TEXT("プラグインの情報を設定"));

    // リストに登録されているか調べる
    auto&& mgr = PluginMgr::GetInstance();
    for ( auto&& plugin: mgr )
    {
        if ( plugin.get() != (ITTBPlugin*)hPlugin ) { continue; }

        SystemLog(TEXT("  %s"), plugin->info()->Name);
        SystemLog(TEXT("  %s"), TEXT("OK"));

        // プラグイン情報を差替
        plugin->info(PLUGIN_INFO); // 内部でコピーを保持 ... PluginMgr.hpp を参照

        // プラグイン一覧を更新
        ::PostMessage(g_hwnd, TTB_SET_PLUGIN_INFO, 0, 0);
        return;
    }

    // 知らないプラグインなのでどうしようもない
    SystemLog(TEXT("  %s"), TEXT("Not found"));
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_FreePluginInfo
(
    PLUGIN_INFO* PLUGIN_INFO
)
{
    SystemLog(TEXT("%s"), TEXT("プラグインの情報を解放"));

    if ( nullptr == PLUGIN_INFO )
    {
        SystemLog(TEXT("  %s"), TEXT("nullptr"));
        return;
    }

    const auto Filename = PLUGIN_INFO->Filename;

    // リストに登録されているプラグインのものか調べる
    auto&& mgr = PluginMgr::GetInstance();
    for ( auto&& plugin: mgr )
    {
        if ( 0 != lstrcmp(Filename, plugin->info()->Filename) ) { continue; }

        SystemLog(TEXT("  %s"), PLUGIN_INFO->Name);
        SystemLog(TEXT("  %s"), TEXT("OK"));

        FreePluginInfo(PLUGIN_INFO);
        return;
    }

    // 知らないプラグインなのでどうしようもない
    SystemLog(TEXT("  %s"), TEXT("Not found"));
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_SetMenuProperty
(
    DWORD_PTR hPlugin, INT32 CommandID, CHANGE_FLAG ChangeFlag, DISPMENU Flag
)
{
    SystemLog(TEXT("%s"), TEXT("メニューのプロパティを設定"));

    auto plugin = reinterpret_cast<ITTBPlugin*>(hPlugin);
    if ( nullptr == plugin )
    {
        SystemLog(TEXT("  %s"), TEXT("?"));
        return;
    }

    const auto& info = *plugin->info();
    SystemLog(TEXT("  名前:     %s"), info.Name);
    SystemLog(TEXT("  コマンド: %s"), info.Commands[CommandID].Caption);
    SystemLog(TEXT("  Flag:     %s"), Flag & dmMenuChecked ? TEXT("Checked")  : TEXT("Unchecked"));
    SystemLog(TEXT("  Flag:     %s"), Flag & dmDisabled    ? TEXT("Disabled") : TEXT("Enabled"));

    const auto CommandCount = info.CommandCount;
    if ( (DWORD)CommandID >= CommandCount )
    {
        SystemLog(TEXT("  %s"), TEXT("Bad index"));
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
    ::PostMessage(g_hwnd, TTB_SET_MENU_PROPERTY, (WPARAM)plugin, (LPARAM)CommandID);
    SystemLog(TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

extern "C" PLUGIN_INFO** WINAPI TTBPlugin_GetAllPluginInfo()
{
    SystemLog(TEXT("%s"), TEXT("すべてのプラグインの情報を取得"));
    auto&& mgr = PluginMgr::GetInstance();

    auto PluginInfoArray = (PLUGIN_INFO**) new DWORD_PTR[mgr.size() + 1];
    if ( nullptr == PluginInfoArray )
    {
        return nullptr;
    }

    size_t idx = 0;
    for ( auto&& plugin: mgr )
    {
        PluginInfoArray[idx] = const_cast<PLUGIN_INFO*>(plugin->info());
        ++idx;
    }

    // 配列はヌル終端にする
    PluginInfoArray[idx] = nullptr;

    SystemLog(TEXT("  %s"), TEXT("OK"));
    return PluginInfoArray;
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_FreePluginInfoArray
(
    PLUGIN_INFO** PluginInfoArray
)
{
    SystemLog(TEXT("%s"), TEXT("すべてのプラグインの情報を解放"));

    if ( nullptr == PluginInfoArray )
    {
        SystemLog(TEXT("  %s"), TEXT("nullptr"));
        return;
    }

    delete[] PluginInfoArray;

    SystemLog(TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_SetTaskTrayIcon
(
    HICON hIcon, LPCTSTR Tips
)
{
    SystemLog(TEXT("%s"), TEXT("タスクトレイのアイコンを変更"));

    // メインウィンドウ内のルーチンに処理を投げる
    ::PostMessage(g_hwnd, TTB_SET_TASK_TRAY_ICON, (WPARAM)hIcon, (LPARAM)Tips);

    SystemLog(TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

extern "C" void WINAPI TTBPlugin_WriteLog
(
    DWORD_PTR hPlugin, ERROR_LEVEL logLevel, LPCTSTR msg
)
{
    constexpr const TCHAR* const err_level[] =
    {
        TEXT(""),
        TEXT("[ERROR] "),
        TEXT("[WARN ] "),
        TEXT("[INFO ] "),
        TEXT("[DEBUG] "),
        TEXT("[SYS  ] "),
    };

    // 値の正規化
    if ( logLevel < 0 ) { logLevel = ERROR_LEVEL(0); }
    if ( logLevel > 5 ) { logLevel = ERROR_LEVEL(5); }

    // 設定以下のログレベル項目は出力しない
    if ( settings::get().logLevel == 0 )       { return; }
    if ( settings::get().logLevel < logLevel ) { return; }

    const auto processId = ::GetCurrentProcessId();

    SYSTEMTIME st;
    ::GetLocalTime(&st);

    static std::array<TCHAR, 4096> buf;
    ::StringCchPrintf
    (
        buf.data(), buf.size(),
        TEXT("%04u/%02u/%02u %02u:%02u:%02u.%03u> [%04X][%08x] %s%s\r\n"),
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        processId, hPlugin, err_level[logLevel], msg
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
    SystemLog(TEXT("%s"), TEXT("コマンドの実行"));
    SystemLog(TEXT("  %s|%i"), PluginFilename, CmdID);

    // 相対パスからプラグインのインスタンスを検索
    auto plugin = PluginMgr::GetInstance().Find(PluginFilename);
    if ( nullptr == plugin )
    {
        SystemLog(TEXT("  %s"), TEXT("Not found"));
        return FALSE;
    }

    // あとはメインウィンドウ内のルーチンに処理を任せる
    const auto result = ::PostMessage
    (
        g_hwnd, TTB_EXECUTE_COMMAND, (WPARAM)plugin, (LPARAM)CmdID
    );

    SystemLog(TEXT("  %s"), TEXT("OK"));
    return result;
}

//---------------------------------------------------------------------------//

// System.cpp