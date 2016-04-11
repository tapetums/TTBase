//---------------------------------------------------------------------------//
//
// WinMain.cpp
//  アプリケーション エントリポイント
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <tchar.h>

#include "include/File.hpp"

#include "../Utility.hpp"
#include "PluginMgr.hpp"
#include "BridgeWnd.hpp"
#include "MainWnd.hpp"
#include "Hook.hpp"

using namespace tapetums;

//---------------------------------------------------------------------------//

// メモリリーク検出
#if defined(_DEBUG) || defined(DEBUG)
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
  #define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

//---------------------------------------------------------------------------//
// Global Variables
//---------------------------------------------------------------------------//

HINSTANCE g_hInst { nullptr };
HWND      g_hwnd  { nullptr };

UINT WM_NOTIFYICON     { 0 };
UINT WM_TASKBARCREATED { 0 };

//---------------------------------------------------------------------------//

constexpr auto SHARED_MEMORY_NAME = L"tapetums::hako::SharedMemory";

//---------------------------------------------------------------------------//
// アプリケーション エントリポイント
//---------------------------------------------------------------------------//

INT32 APIENTRY _tWinMain
(
    HINSTANCE hInstance, HINSTANCE, LPTSTR, INT32
)
{
    // メモリリーク検出
  #if defined(_DEBUG) || defined(DEBUG)
    ::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
  #endif

    // グローバル変数に情報を記憶
    g_hInst = hInstance;

    // 二重起動を禁止
    File shared;
    if ( shared.Open(SHARED_MEMORY_NAME, File::ACCESS::READ) )
    {
        // 既にあるウィンドウをアクティブにし、自分自身は終了
        HWND hwnd;
        shared.Read(&hwnd, sizeof(HWND));
        ::PostMessage(hwnd, WM_SHOWWINDOW, SW_SHOWNORMAL, 0);
        return 0;
    }

    TTBPlugin_WriteLog(0, elInfo, TEXT("------------------------------------------------------------"));
    TTBPlugin_WriteLog(0, elInfo, TEXT("hako を 起動しました"));

    if ( ! shared.Map(sizeof(HWND), SHARED_MEMORY_NAME, File::ACCESS::WRITE) )
    {
        TTBPlugin_WriteLog(0, elError, TEXT("二重起動防止用ロックの生成に失敗"));
        return 0;
    }

    // COM の初期化
    const auto hr = ::CoInitialize(nullptr);
    if ( FAILED(hr) )
    {
        return 0;
    }

    MSG msg { };
    try
    {
        // セッション終了時の優先順位を上げる
        ::SetProcessShutdownParameters(0x4FF, 0);

      #if INTPTR_MAX == INT64_MAX
        // プロセス間通信用ウィンドウの生成
        BridgeWnd bdgwnd;
      #endif

        // プラグインマネージャの初期化
        auto&& mgr = PluginMgr::GetInstance();
        mgr.LoadAll();

        // メインウィンドウの生成
        MainWnd wnd;
        g_hwnd = wnd.handle();

        // ウィンドウハンドルを共有メモリに保存
        shared.Write(&g_hwnd, sizeof(HWND));

        // フックの開始
        HookDll hook;
        hook.InstallHook(g_hwnd);

        // メッセージループ
        while ( ::GetMessage(&msg, nullptr, 0, 0) > 0 )
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        // プラグインマネージャの終了処理
        mgr.FreeAll();
    }
    catch (...)
    {
        ::MessageBox
        (
            nullptr,
            TEXT("例外が発生したため、プログラムを終了します。\n")
            TEXT("ご不便をお掛けして申し訳ありません。"),
            TEXT("hako"), MB_OK
        );
    }

    // COM の終了処理
    ::CoUninitialize();

    TTBPlugin_WriteLog(0, elInfo, TEXT("hako を 終了しました"));
    TTBPlugin_WriteLog(0, elInfo, TEXT("------------------------------------------------------------"));

    return static_cast<INT32>(msg.wParam);
}

//---------------------------------------------------------------------------//

// WinMain.cpp