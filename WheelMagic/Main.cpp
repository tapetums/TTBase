//---------------------------------------------------------------------------//
//
// Main.cpp
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "..\Plugin.hpp"
#include "..\Utility.hpp"
#include "WheelMagicHook.hpp"

#include "Main.hpp"

//---------------------------------------------------------------------------//
//
// グローバル変数
//
//---------------------------------------------------------------------------//

HINSTANCE g_hInst  { nullptr };
HANDLE    g_hMutex { nullptr };

//---------------------------------------------------------------------------//

// プラグインの名前
LPCTSTR PLUGIN_NAME { TEXT("WheelMagic for TTBase") };

// コマンドの数
DWORD COMMAND_COUNT { 0 };

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

// TTBEvent_Init() の内部実装
BOOL Init(void)
{
    // フックのために二重起動を禁止
    if ( g_hMutex == nullptr )
    {
        g_hMutex = ::CreateMutex(nullptr, TRUE, PLUGIN_NAME);
    }
    if ( g_hMutex == nullptr )
    {
        WriteLog(g_hPlugin, elError, TEXT("%s: Failed to create mutex"), g_info.Name);
        return FALSE;
    }
    if ( ::GetLastError() == ERROR_ALREADY_EXISTS )
    {
        WriteLog(g_hPlugin, elError, TEXT("%s: %s is already started"), g_info.Name, g_info.Name);
        return FALSE;
    }

    // マウスフックを登録
    if ( ! WMBeginHook() )
    {
        WriteLog(g_hPlugin, elError, TEXT("%s: Failed to begin hook"), g_info.Name);
        return FALSE;
    }

    WriteLog(g_hPlugin, elInfo, TEXT("%s: successfully initialized"), g_info.Name);

    return TRUE;
}

//---------------------------------------------------------------------------//

// TTBEvent_Unload() の内部実装
void Unload(void)
{
    // マウスフックを解除
    WMEndHook();

    // ミューテックスを削除
    if ( g_hMutex != nullptr )
    {
        ::ReleaseMutex(g_hMutex);
        ::CloseHandle(g_hMutex);
        g_hMutex = nullptr;
    }

    WriteLog(g_hPlugin, elInfo, TEXT("%s: successfully uninitialized"), g_info.Name);
}

//---------------------------------------------------------------------------//

// TTBEvent_Execute() の内部実装
BOOL Execute(INT32 CmdId, HWND hWnd)
{
    UNREFERENCED_PARAMETER(CmdId);
    UNREFERENCED_PARAMETER(hWnd);
    return TRUE;
}

//---------------------------------------------------------------------------//

// TTBEvent_WindowsHook() の内部実装
void Hook(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(Msg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
}

//---------------------------------------------------------------------------//
//
// CRT を使わないため new/delete を自前で実装
//
//---------------------------------------------------------------------------//

#if defined(_NODEFLIB)

void* __cdecl operator new(size_t size)
{
    return ::HeapAlloc(::GetProcessHeap(), 0, size);
}

void __cdecl operator delete(void* p)
{
    if ( p != nullptr ) ::HeapFree(::GetProcessHeap(), 0, p);
}

void __cdecl operator delete(void* p, size_t) // C++14
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

void __cdecl operator delete[](void* p, size_t) // C++14
{
    if ( p != nullptr ) ::HeapFree(::GetProcessHeap(), 0, p);
}

// プログラムサイズを小さくするためにCRTを除外
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#pragma comment(linker, "/entry:DllMain")

#endif

//---------------------------------------------------------------------------//

// DLL エントリポイント
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID)
{
    if ( fdwReason == DLL_PROCESS_ATTACH )
    {
        g_hInst = hInstance;
    }
    return TRUE;
}

//---------------------------------------------------------------------------//

// Main.cpp