//---------------------------------------------------------------------------//
//
// Main.cpp
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "..\Plugin.h"
#include "..\MessageDef.h"
#include "..\Utility.h"

#include "Wnd.hpp"
#include "IniFile.h"
#include "MainWindow.h"
#include "Main.h"

//---------------------------------------------------------------------------//
//
// グローバル変数
//
//---------------------------------------------------------------------------//

HINSTANCE g_hInst = nullptr;
HWND      g_hwnd  = nullptr;

//---------------------------------------------------------------------------//

// プラグインの名前
LPCTSTR PLUGIN_NAME = TEXT("PopTwit");

// コマンドの数
DWORD COMMAND_COUNT = 1;

//---------------------------------------------------------------------------//

// コマンドID
enum CMD : INT32
{
    CMD_TWEET,
};

//---------------------------------------------------------------------------//

// コマンドの情報
PLUGIN_COMMAND_INFO g_cmd_info[] =
{
    {
        TEXT("Tweet"),    // コマンド名（英名）
        TEXT("つぶやく"), // コマンド説明（日本語）
        CMD_TWEET,        // コマンドID
        0,                // Attr（未使用）
        -1,               // ResTd(未使用）
        dmHotKeyMenu,     // DispMenu
        0,                // TimerInterval[msec] 0で使用しない
        0                 // TimerCounter（未使用）
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

// TTBEvent_Init() の内部実装
BOOL Init(void)
{
    TCHAR ininame[MAX_PATH];

    // INI ファイル名取得
    const auto len = ::GetModuleFileName(g_hInst, ininame, MAX_PATH);
    ininame[len - 3] = 'i';
    ininame[len - 2] = 'n';
    ininame[len - 1] = 'i';

    // INI ファイルの読み込み
    const auto enabled = LoadIniFile(ininame);
    if ( !enabled )
    {
        return FALSE;
    }

    // ウィンドウクラスの登録
    Wnd::Register(PLUGIN_NAME, MainWindowProc, MAKEINTRESOURCE(100));

    WriteLog(elInfo, TEXT("%s: Successfully initialized"), PLUGIN_NAME);

    return TRUE;
}

//---------------------------------------------------------------------------//

// TTBEvent_Unload() の内部実装
void Unload(void)
{
    TCHAR ininame[MAX_PATH];

    // INI ファイル名取得
    const auto len = ::GetModuleFileName(g_hInst, ininame, MAX_PATH);
    ininame[len - 3] = 'i';
    ininame[len - 2] = 'n';
    ininame[len - 1] = 'i';

    // INI ファイルの書き込み
    SaveIniFile(ininame);

    WriteLog(elInfo, TEXT("%s: Successfully uninitialized"), PLUGIN_NAME);
}

//---------------------------------------------------------------------------//

// TTBEvent_Execute() の内部実装
BOOL Execute(INT32 CmdId, HWND hWnd)
{
    switch ( CmdId )
    {
        case CMD_TWEET:
        {
            // 二重起動の防止
            if ( g_hwnd )
            {
                ::SetForegroundWindow(g_hwnd);
                WriteLog(elInfo, TEXT("%s: Window has been opened"), PLUGIN_NAME);
                return FALSE;
            }

            // ウィンドウの生成
            g_hwnd = Wnd::Create
            (
                PLUGIN_NAME, PLUGIN_NAME, 0, 0, nullptr
            );
            if ( g_hwnd == nullptr )
            {
                WriteLog(elInfo, TEXT("%s: Failed to open window"), PLUGIN_NAME);
                return FALSE;
            }
            Wnd::Resize(g_hwnd, 400, 128);
            Wnd::ToCenter(g_hwnd);
            Wnd::Show(g_hwnd);
            ::SetForegroundWindow(g_hwnd);

            WriteLog(elInfo, TEXT("%s: Successfully opened window"), PLUGIN_NAME);
            return TRUE;
        }
        default:
        {
            return FALSE;
        }
    }
}

//---------------------------------------------------------------------------//

// TTBEvent_WindowsHook() の内部実装
void Hook(UINT Msg, WPARAM wParam, LPARAM lParam)
{
}

//---------------------------------------------------------------------------//

// DLL エントリポイント
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
    if ( fdwReason == DLL_PROCESS_ATTACH )
    {
        g_hInst = hInstance;
    }
    return TRUE;
}

//---------------------------------------------------------------------------//

// Main.cpp