﻿//---------------------------------------------------------------------------//
//
//              TTBaseプラグインテンプレート(Plugin.cpp)
//
//              このファイルはできるだけ変更しない。
//              Main.cppに処理を書くことをお勧めします。
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "Plugin.h"
#include "MessageDef.h"
#include "Utility.h"

//---------------------------------------------------------------------------//
//
// グローバル変数
//
//---------------------------------------------------------------------------//

// プラグインを本体で認識するための識別コード
DWORD_PTR g_hPlugin = 0;

//---------------------------------------------------------------------------//

// 本体側エクスポート関数
#ifdef __cplusplus
extern "C" {
#endif
PLUGIN_INFO*  (WINAPI* TTBPlugin_GetPluginInfo)      (DWORD_PTR hPlugin) = nullptr;
void          (WINAPI* TTBPlugin_SetPluginInfo)      (DWORD_PTR hPlugin, PLUGIN_INFO* PluginInfo) = nullptr;
void          (WINAPI* TTBPlugin_FreePluginInfo)     (PLUGIN_INFO* PluginInfo) = nullptr;
void          (WINAPI* TTBPlugin_SetMenuProperty)    (DWORD_PTR hPlugin, INT32 CommandID, CHANGE_FLAG ChangeFlag, DISPMENU Flag) = nullptr;
PLUGIN_INFO** (WINAPI* TTBPlugin_GetAllPluginInfo)   (void) = nullptr;
void          (WINAPI* TTBPlugin_FreePluginInfoArray)(PLUGIN_INFO** PluginInfoArray) = nullptr;
void          (WINAPI* TTBPlugin_SetTaskTrayIcon)    (HICON hIcon, LPCTSTR Tips) = nullptr;
void          (WINAPI* TTBPlugin_WriteLog)           (DWORD_PTR hPlugin, ERROR_LEVEL logLevel, LPCTSTR msg) = nullptr;
BOOL          (WINAPI* TTBPlugin_ExecuteCommand)     (LPCTSTR PluginFilename, INT32 CmdID) = nullptr;
#ifdef __cplusplus
};
#endif

//---------------------------------------------------------------------------//
//
// プラグイン イベント
//
//---------------------------------------------------------------------------//

// プラグイン情報構造体のセット
PLUGIN_INFO* WINAPI TTBEvent_InitPluginInfo(LPTSTR PluginFilename)
{
    // プラグイン情報を初期化
    g_info.Filename = CopyString(PluginFilename);
    GetVersion(PluginFilename, &g_info.VersionMS, &g_info.VersionLS);

    return &g_info;
}

//---------------------------------------------------------------------------//

// プラグイン情報構造体の破棄
void WINAPI TTBEvent_FreePluginInfo(PLUGIN_INFO* PluginInfo)
{
}

//---------------------------------------------------------------------------//

// プラグイン初期化
BOOL WINAPI TTBEvent_Init(LPTSTR PluginFilename, DWORD_PTR hPlugin)
{
    RegisterMessages();

    // キャッシュのために、TTBPlugin_InitPluginInfoは呼ばれない場合がある
    // そのため、Initでもプラグイン情報を初期化する
    DeleteString(g_info.Filename);

    g_info.Filename = CopyString(PluginFilename);
    GetVersion(PluginFilename, &g_info.VersionMS, &g_info.VersionLS);

    // 本体から、プラグインを認識するための識別コードを受け取る
    g_hPlugin = hPlugin;

    // API関数の取得
    const auto hModule = ::GetModuleHandle(nullptr);
    (FARPROC&)TTBPlugin_GetAllPluginInfo    = GetProcAddress(hModule, "TTBPlugin_GetAllPluginInfo");
    (FARPROC&)TTBPlugin_FreePluginInfoArray = GetProcAddress(hModule, "TTBPlugin_FreePluginInfoArray");
    (FARPROC&)TTBPlugin_GetPluginInfo       = GetProcAddress(hModule, "TTBPlugin_GetPluginInfo");
    (FARPROC&)TTBPlugin_SetPluginInfo       = GetProcAddress(hModule, "TTBPlugin_SetPluginInfo");
    (FARPROC&)TTBPlugin_FreePluginInfo      = GetProcAddress(hModule, "TTBPlugin_FreePluginInfo");
    (FARPROC&)TTBPlugin_SetMenuProperty     = GetProcAddress(hModule, "TTBPlugin_SetMenuProperty");
    (FARPROC&)TTBPlugin_SetTaskTrayIcon     = GetProcAddress(hModule, "TTBPlugin_SetTaskTrayIcon");
    (FARPROC&)TTBPlugin_WriteLog            = GetProcAddress(hModule, "TTBPlugin_WriteLog");
    (FARPROC&)TTBPlugin_ExecuteCommand      = GetProcAddress(hModule, "TTBPlugin_ExecuteCommand");

    return Init();
}

//---------------------------------------------------------------------------//

// プラグインアンロード時の処理
void WINAPI TTBEvent_Unload(void)
{
    Unload();

    DeleteString(g_info.Filename);
    g_info.Filename = nullptr;
}

//---------------------------------------------------------------------------//

// コマンド実行
BOOL WINAPI TTBEvent_Execute(INT32 CommandID, HWND hWnd)
{
    return Execute(CommandID, hWnd);
}

//---------------------------------------------------------------------------//

// フック（ShellHook,MouseHook)
void WINAPI TTBEvent_WindowsHook(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    Hook(Msg, wParam, lParam);
}

//---------------------------------------------------------------------------//

// Plugin.cpp