//---------------------------------------------------------------------------//
//
// Plugin.hpp
//  TTB Plugin Template (C++11)
//
//  このファイルはできるだけ変更しない。
//  Main.cppに処理を書くことをお勧めします。
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "Plugin.hpp"
#include "MessageDef.hpp"
#include "Utility.hpp"

//---------------------------------------------------------------------------//
//
// グローバル変数
//
//---------------------------------------------------------------------------//

// プラグインを本体で認識するための識別コード
DWORD_PTR g_hPlugin { 0 };

//---------------------------------------------------------------------------//

#if defined(_USRDLL) // プラグイン側で使用

// 本体側エクスポート関数
extern "C"
{
    TTBPLUGIN_GETPLUGININFO       TTBPlugin_GetPluginInfo       = nullptr;
    TTBPLUGIN_SETPLUGININFO       TTBPlugin_SetPluginInfo       = nullptr;
    TTBPLUGIN_FREEPLUGININFO      TTBPlugin_FreePluginInfo      = nullptr;
    TTBPLUGIN_SETMENUPROPERTY     TTBPlugin_SetMenuProperty     = nullptr;
    TTBPLUGIN_GETALLPLUGININFO    TTBPlugin_GetAllPluginInfo    = nullptr;
    TTBPLUGIN_FREEPLUGININFOARRAY TTBPlugin_FreePluginInfoArray = nullptr;
    TTBPLUGIN_SETTASKTRAYICON     TTBPlugin_SetTaskTrayIcon     = nullptr;
    TTBPLUGIN_WRITELOG            TTBPlugin_WriteLog            = nullptr;
    TTBPLUGIN_EXECUTECOMMAND      TTBPlugin_ExecuteCommand      = nullptr;
}

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
    if ( PluginInfo != &g_info ) { return; }

    DeleteString(g_info.Filename);
    g_info.Filename = nullptr;
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

    // 本体から、プラグインを認識するための識別コードを受け取る
    g_hPlugin = hPlugin;

  #if defined(_USRDLL) // プラグイン側で使用
    // リソースからバージョン情報を取得
    GetVersion(PluginFilename, &g_info.VersionMS, &g_info.VersionLS);

    // API関数の取得
    const auto hModule = ::GetModuleHandle(nullptr);
    (FARPROC&)TTBPlugin_GetPluginInfo       = ::GetProcAddress(hModule, "TTBPlugin_GetPluginInfo");
    (FARPROC&)TTBPlugin_SetPluginInfo       = ::GetProcAddress(hModule, "TTBPlugin_SetPluginInfo");
    (FARPROC&)TTBPlugin_FreePluginInfo      = ::GetProcAddress(hModule, "TTBPlugin_FreePluginInfo");
    (FARPROC&)TTBPlugin_SetMenuProperty     = ::GetProcAddress(hModule, "TTBPlugin_SetMenuProperty");
    (FARPROC&)TTBPlugin_GetAllPluginInfo    = ::GetProcAddress(hModule, "TTBPlugin_GetAllPluginInfo");
    (FARPROC&)TTBPlugin_FreePluginInfoArray = ::GetProcAddress(hModule, "TTBPlugin_FreePluginInfoArray");
    (FARPROC&)TTBPlugin_SetTaskTrayIcon     = ::GetProcAddress(hModule, "TTBPlugin_SetTaskTrayIcon");
    (FARPROC&)TTBPlugin_WriteLog            = ::GetProcAddress(hModule, "TTBPlugin_WriteLog");
    (FARPROC&)TTBPlugin_ExecuteCommand      = ::GetProcAddress(hModule, "TTBPlugin_ExecuteCommand");
  #endif

    return Init();
}

//---------------------------------------------------------------------------//

// プラグインアンロード時の処理
void WINAPI TTBEvent_Unload()
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