//---------------------------------------------------------------------------//
//
// Main.cpp
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "..\Plugin.h"
#include "..\Utility.h"
#include "Main.h"
#include "WheelMagicHook.h"

//---------------------------------------------------------------------------//
//
// グローバル変数
//
//---------------------------------------------------------------------------//

HINSTANCE g_hInstance  = nullptr;
HANDLE    g_hMutex     = nullptr;

//---------------------------------------------------------------------------//

// プラグインの名前
LPCTSTR PLUGIN_NAME = TEXT("WheelMagic for TTBase");

// コマンドの数
DWORD COMMAND_COUNT = 0;

//---------------------------------------------------------------------------//

// コマンドID
enum CMD : INT32
{
    CMD_DUMMY, // ダミー コマンド
};

//---------------------------------------------------------------------------//

// コマンドの情報
PLUGIN_COMMAND_INFO g_cmd_info[] =
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
    nullptr,             // コマンド
    0,                   // ロードにかかった時間（msec）
};

//---------------------------------------------------------------------------//
//
// CRT を使わないため new/delete を自前で実装
//
//---------------------------------------------------------------------------//

#ifndef _DEBUG

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

#endif

//---------------------------------------------------------------------------//

// TTBEvent_Init() の内部実装
BOOL Init(void)
{
    TCHAR ininame[MAX_PATH];

    // iniファイル名取得
    const auto len = ::GetModuleFileName(g_hInstance, ininame, MAX_PATH);
    ininame[len - 3] = 'i';
    ininame[len - 2] = 'n';
    ininame[len - 1] = 'i';
    WriteLog(elDebug, TEXT("%s: %s"), g_info.Name, ininame);

    auto param = ::GetPrivateProfileInt(TEXT("Setting"), TEXT("Param"), 0, ininame);

    g_hMutex = ::CreateMutex(nullptr, TRUE, g_info.Name);
    if ( g_hMutex == nullptr )
    {
        WriteLog(elError, TEXT("%s: Failed to create mutex"), g_info.Name);
        return FALSE;
    }
    if ( ::GetLastError() == ERROR_ALREADY_EXISTS )
    {
        WriteLog(elError, TEXT("%s: %s is already started"), g_info.Name, g_info.Name);
        return FALSE;
    }

    if ( ! WMBeginHook() )
    {
        WriteLog(elError, TEXT("%s: Failed to begin hook"), g_info.Name);
        return FALSE;
    }

    WriteLog(elInfo, TEXT("%s: successfully initialized"), g_info.Name);

    return TRUE;
}

//---------------------------------------------------------------------------//

// TTBEvent_Unload() の内部実装
void Unload(void)
{
    WMEndHook();

    if ( g_hMutex != nullptr )
    {
        ::ReleaseMutex(g_hMutex);
        ::CloseHandle(g_hMutex);
        g_hMutex = nullptr;
    }

    WriteLog(elInfo, TEXT("%s: successfully uninitialized"), g_info.Name);
}

//---------------------------------------------------------------------------//

// TTBEvent_Execute() の内部実装
BOOL Execute(INT32 CmdId, HWND hWnd)
{
    switch ( CmdId )
    {
        case CMD_DUMMY:
        {
            WriteLog(elDebug, TEXT("%s|%d"), g_info.Filename, CmdId);
            return TRUE;
        }
        default:
        {
            return FALSE;
        }
    }

    return TRUE;
}

//---------------------------------------------------------------------------//

// TTBEvent_WindowsHook() の内部実装
void Hook(UINT Msg, WPARAM wParam, LPARAM lParam)
{
}

//---------------------------------------------------------------------------//

#ifndef _DEBUG

// プログラムサイズを小さくするためにCRTを除外
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#pragma comment(linker, "/entry:DllMain")

#endif

// DLL エントリポイント
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
    if ( fdwReason == DLL_PROCESS_ATTACH )
    {
        g_hInstance = hInstance;
    }
    return TRUE;
}

//---------------------------------------------------------------------------//

// Main.cpp