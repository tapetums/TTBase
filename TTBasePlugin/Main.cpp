//---------------------------------------------------------------------------//
//
// Main.cpp
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "..\Plugin.h"
#include "..\MessageDef.h"
#include "Main.h"

//---------------------------------------------------------------------------//
//
// グローバル変数
//
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
enum CMD : INT32
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

    return TRUE;
}

//---------------------------------------------------------------------------//

// TTBEvent_Unload() の内部実装
void Unload(void)
{
}

//---------------------------------------------------------------------------//

// TTBEvent_Execute() の内部実装
BOOL Execute(INT32 CmdId, HWND hWnd)
{
    switch ( CmdId )
    {
    case CMD_DUMMY:
    default:
        return FALSE;
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

//---------------------------------------------------------------------------//

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