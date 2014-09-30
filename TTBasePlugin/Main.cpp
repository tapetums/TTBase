// Main.cpp

//---------------------------------------------------------------------------//

#include <windows.h>

#include "..\Plugin.h"
#include "..\MessageDef.h"
#include "Main.h"

//---------------------------------------------------------------------------//

HINSTANCE g_hInstance  = nullptr;

//---------------------------------------------------------------------------//

// プラグインの名前（任意の文字が使用可能）
LPCTSTR PLUGIN_NAME = TEXT("プラグイン スケルトン");

// プラグインのタイプ
WORD PLUGIN_TYPE = ptLoadAtUse;

//---------------------------------------------------------------------------//

// コマンドの数
DWORD COMMAND_COUNT = 1;

// コマンドID
enum CMD : int
{
    CMD_DUMMY,
};

// コマンドの情報
PLUGIN_COMMAND_INFO COMMAND_INFO[] =
{
    {
        TEXT("Dummy"),  // コマンド名（英名）
        TEXT("ダミー"), // コマンド説明（日本語）
        CMD_DUMMY,      // コマンドID
        0,              // Attr（未使用）
        -1,             // ResTd(未使用）
        dmHotKeyMenu,   // DispMenu
        0,              // TimerInterval[msec] 0で使用しない
        0               // TimerCounter（未使用）
    },
};

//---------------------------------------------------------------------------//

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
    if ( fdwReason == DLL_PROCESS_ATTACH )
    {
        g_hInstance = hInstance;
    }
    return TRUE;
}

//---------------------------------------------------------------------------//

BOOL Init(void)
{
    TCHAR ininame[MAX_PATH];

    // iniファイル名取得
    const auto len = ::GetModuleFileName(g_hInstance, ininame, MAX_PATH);
    ininame[len - 3] = 'i';
    ininame[len - 2] = 'n';
    ininame[len - 1] = 'i';

    return TRUE;
}

//---------------------------------------------------------------------------//

void Unload(void)
{
}

//---------------------------------------------------------------------------//

BOOL Execute(int CmdId, HWND hWnd)
{
    switch ( CmdId )
    {
    default:
        return FALSE;
    }

    return TRUE;
}

//---------------------------------------------------------------------------//

void Hook(UINT Msg, WPARAM wParam, LPARAM lParam)
{
}

//---------------------------------------------------------------------------//

// Main.cpp