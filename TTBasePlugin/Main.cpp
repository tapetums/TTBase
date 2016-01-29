//---------------------------------------------------------------------------//
//
// Main.cpp
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "..\Plugin.hpp"
#include "..\MessageDef.hpp"
#include "..\Utility.hpp"

#include "Main.hpp"

//---------------------------------------------------------------------------//
//
// グローバル変数
//
//---------------------------------------------------------------------------//

HINSTANCE g_hInst { nullptr };

//---------------------------------------------------------------------------//

// プラグインの名前
LPCTSTR PLUGIN_NAME { TEXT("スケルトン") };

// コマンドの数
DWORD COMMAND_COUNT { 1 };

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
    TCHAR ininame[MAX_PATH];

    // iniファイル名取得
    const auto len = ::GetModuleFileName(g_hInst, ininame, MAX_PATH);
    ininame[len - 3] = 'i';
    ininame[len - 2] = 'n';
    ininame[len - 1] = 'i';

    // パラメータ取得の例
    auto param = ::GetPrivateProfileInt(TEXT("Setting"), TEXT("Param"), 0, ininame);
    UNREFERENCED_PARAMETER(param); // ERASE ME

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
    UNREFERENCED_PARAMETER(hWnd); // ERASE ME

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
}

//---------------------------------------------------------------------------//

// TTBEvent_WindowsHook() の内部実装
void Hook(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(Msg);    // ERASE ME
    UNREFERENCED_PARAMETER(wParam); // ERASE ME
    UNREFERENCED_PARAMETER(lParam); // ERASE ME
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