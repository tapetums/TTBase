﻿//---------------------------------------------------------------------------//
//
// Main.cpp
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <strsafe.h>

#ifdef _NODEFLIB
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#undef StringCchPrintf
#define StringCchPrintf wnsprintf
#endif

#include "..\Plugin.h"
#include "..\MessageDef.h"
#include "..\Utility.h"

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
    ptLoadAtUse,         // プラグインのタイプ
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
    WriteLog(elInfo, TEXT("%s: Successfully initialized"), PLUGIN_NAME);

    return TRUE;
}

//---------------------------------------------------------------------------//

// TTBEvent_Unload() の内部実装
void Unload(void)
{
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
            TCHAR path[MAX_PATH];
            const auto length = ::GetModuleFileName(g_hInst, path, MAX_PATH);
            path[length - 4] = '\0';

            TCHAR filename[MAX_PATH];
            ::StringCchPrintf(filename, MAX_PATH, TEXT("%s\\%s.exe"), path, PLUGIN_NAME);
            ::ShellExecute(nullptr, TEXT("open"), filename, nullptr, nullptr, SW_SHOW);

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
//
// CRT を使わないため new/delete を自前で実装
//
//---------------------------------------------------------------------------//

#ifdef _NODEFLIB

void* __cdecl operator new(size_t size)
{
    return ::HeapAlloc(::GetProcessHeap(), 0, size);
}

void __cdecl operator delete(void* p)
{
    if ( p != nullptr ) ::HeapFree(::GetProcessHeap(), 0, p);
}

void* __cdecl operator new[](size_t size)
{
    return ::HeapAlloc(::GetProcessHeap(), 0, size);
}

void __cdecl operator delete[](void* p)
{
    if ( p != nullptr ) ::HeapFree(::GetProcessHeap(), 0, p);
}

//---------------------------------------------------------------------------//

// プログラムサイズを小さくするためにCRTを除外
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#pragma comment(linker, "/entry:DllMain")

#endif

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