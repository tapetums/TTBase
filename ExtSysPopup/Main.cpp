//---------------------------------------------------------------------------//
//
// Main.cpp
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include "..\Plugin.hpp"
#include "..\MessageDef.hpp"
#include "..\Utility.hpp"
#include "MouseHook.hpp"

#include "Main.hpp"

//---------------------------------------------------------------------------//
//
// グローバル変数
//
//---------------------------------------------------------------------------//

HINSTANCE g_hInst  { nullptr };
HANDLE    g_hMutex { nullptr };
HWND      g_hwnd   { nullptr };

//---------------------------------------------------------------------------//

// プラグインの名前
LPCTSTR PLUGIN_NAME { TEXT("ExtSysPopup") };

// コマンドの数
DWORD COMMAND_COUNT { 2 };

//---------------------------------------------------------------------------//

// コマンドID
enum CMD : INT32
{
    CMD_ALWAYS_ON_TOP,
    CMD_OPEN_APP_FOLDER,
};

//---------------------------------------------------------------------------//

// コマンドの情報
PLUGIN_COMMAND_INFO g_cmd_info[] =
{
    {
        TEXT("AlwaysOnTop"), // コマンド名（英名）
        TEXT("常に手前"),    // コマンド説明（日本語）
        CMD_ALWAYS_ON_TOP,   // コマンドID
        0,                   // Attr（未使用）
        -1,                  // ResTd(未使用）
        dmNone,              // DispMenu
        0,                   // TimerInterval[msec] 0で使用しない
        0                    // TimerCounter（未使用）
    },
    {
        TEXT("OpenAppFolder"),  // コマンド名（英名）
        TEXT("フォルダを開く"), // コマンド説明（日本語）
        CMD_OPEN_APP_FOLDER,    // コマンドID
        0,                      // Attr（未使用）
        -1,                     // ResTd(未使用）
        dmNone,                 // DispMenu
        0,                      // TimerInterval[msec] 0で使用しない
        0                       // TimerCounter（未使用）
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

BOOL CheckTopMost(HWND hwnd)
{
    const auto styleEx = ::GetWindowLongPtr(hwnd, GWL_EXSTYLE);

    return (styleEx & WS_EX_TOPMOST) ? TRUE : FALSE;
}

//---------------------------------------------------------------------------//

BOOL ToggleTopMost(HWND hwnd, BOOL topmost)
{
    BOOL ret;

    if ( topmost )
    {
        ret = ::SetWindowPos
        (
            hwnd, HWND_NOTOPMOST,
            0, 0, 0, 0,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE
        );
    }
    else
    {
        ret = ::SetWindowPos
        (
            hwnd, HWND_TOPMOST,
            0, 0, 0, 0,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE
        );
    }

    return ret;
}

//---------------------------------------------------------------------------//

BOOL OpenAppFolder(HWND hwnd)
{
    WriteLog(g_hPlugin, elDebug, TEXT("%s: HWND = %p"), PLUGIN_NAME, hwnd);

    TCHAR path[MAX_PATH];

    DWORD pid;
    ::GetWindowThreadProcessId(hwnd, &pid);
    const auto hProcess = ::OpenProcess
    (
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid
    );
    if ( hProcess == nullptr )
    {
        WriteLog(g_hPlugin, elDebug, TEXT("%s: OpenProcess() failed"), PLUGIN_NAME);
        return FALSE;
    }

    HMODULE hModule;
    DWORD   cbRet;
    if ( ! ::EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbRet) )
    {
        WriteLog(g_hPlugin, elDebug, TEXT("%s: EnumProcessModules() failed"), PLUGIN_NAME);
        return FALSE;
    }

    const auto length = ::GetModuleFileNameEx(hProcess, hModule, path, MAX_PATH);
    WriteLog(g_hPlugin, elDebug, TEXT("%s: %s"), PLUGIN_NAME, path);

    for ( size_t index = length - 1; index > 0; --index )
    {
        if ( path[index] == '\\' )
        {
            path[index] = '\0';
            break;
        }
    }
    WriteLog(g_hPlugin, elDebug, TEXT("%s: %s"), PLUGIN_NAME, path);

    ::ShellExecute(nullptr, TEXT("open"), path, nullptr, nullptr, SW_SHOWNOACTIVATE);

    return TRUE;
}

//---------------------------------------------------------------------------//

// ウィンドウクラスを登録
ATOM Register(LPCTSTR lpszClassName)
{
    WNDCLASSEX wc;
    wc.cbSize        = sizeof(wc);
    wc.style         = 0;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = nullptr;
    wc.hCursor       = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName  = nullptr;
    wc.lpszClassName = lpszClassName;
    wc.hIconSm       = nullptr;
    wc.lpfnWndProc   = [](HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
    {
        return ::DefWindowProc(hwnd, uMsg, wp, lp);
    };

    return ::RegisterClassEx(&wc);
}

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
        WriteLog(g_hPlugin, elError, TEXT("%s: Failed to create mutex"), PLUGIN_NAME);
        return FALSE;
    }
    if ( ::GetLastError() == ERROR_ALREADY_EXISTS )
    {
        WriteLog(g_hPlugin, elError, TEXT("%s: Already started"), PLUGIN_NAME);
        return FALSE;
    }

    // ウィンドウクラスを登録
    const auto atom = Register(PLUGIN_NAME);
    if ( atom == 0 )
    {
        if ( GetLastError() == 0x582 )
        {
            // 「そのクラスは既にあります。」
        }
        else
        {
            WriteLog(g_hPlugin, elError, TEXT("%s: RegisterClassEx() failed"), PLUGIN_NAME);
            goto UNLOAD;
        }
    }

    // TrackPopupMenu() のためにウィンドウを生成
    if ( g_hwnd == nullptr )
    {
        g_hwnd = ::CreateWindowEx
        (
            0, PLUGIN_NAME, PLUGIN_NAME, 0,
            100, 100, 320, 240,
            nullptr, nullptr, g_hInst, nullptr
        );
    }
    if ( g_hwnd == nullptr )
    {
        WriteLog(g_hPlugin, elError, TEXT("%s: CreateWindowEx() failed"), PLUGIN_NAME);
        goto UNLOAD;
    }

    // マウスフックを登録
    if ( ! WMBeginHook() )
    {
        WriteLog(g_hPlugin, elError, TEXT("%s: Failed to begin hook"), PLUGIN_NAME);
        goto UNLOAD;
    }

    WriteLog(g_hPlugin, elInfo, TEXT("%s: Successfully initialized"), PLUGIN_NAME);

    return TRUE;

UNLOAD:
    WriteLog(g_hPlugin, elInfo, TEXT("%s: Initialization failed"), PLUGIN_NAME);

    Unload();

    return FALSE;
}

//---------------------------------------------------------------------------//

// TTBEvent_Unload() の内部実装
void Unload(void)
{
    // マウスフックを解除
    WMEndHook();

    // ウィンドウを破棄
    if ( g_hwnd != nullptr )
    {
        ::DestroyWindow(g_hwnd);
        g_hwnd = nullptr;
    }

    // ミューテックスを削除
    if ( g_hMutex != nullptr )
    {
        ::ReleaseMutex(g_hMutex);
        ::CloseHandle(g_hMutex);
        g_hMutex = nullptr;
    }

    WriteLog(g_hPlugin, elInfo, TEXT("%s: Successfully uninitialized"), PLUGIN_NAME);
}

//---------------------------------------------------------------------------//

// TTBEvent_Execute() の内部実装
BOOL Execute(INT32 CmdId, HWND hWnd)
{
    WriteLog(g_hPlugin, elDebug, TEXT("%s|%d"), g_info.Filename, CmdId);

    switch ( CmdId )
    {
        case CMD_ALWAYS_ON_TOP:
        {
            return ToggleTopMost(hWnd, CheckTopMost(hWnd));
        }
        case CMD_OPEN_APP_FOLDER:
        {
            return OpenAppFolder(hWnd);
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