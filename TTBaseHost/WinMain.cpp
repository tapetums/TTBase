//---------------------------------------------------------------------------//
//
// WinMain.cpp
//  アプリケーション エントリポイント
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <tchar.h>

#include "include/Application.hpp"
#include "include/File.hpp"

#include "PluginMgr.hpp"
#include "MainWnd.hpp"

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

    // プラグインマネージャの初期化
    auto&& mgr = PluginMgr::GetInstance();
    mgr.LoadAll();

    // ウィンドウの生成
    MainWnd wnd;
    g_hwnd = wnd.handle();

    // ウィンドウハンドルを共有メモリに保存
    if ( shared.Map(sizeof(HWND), SHARED_MEMORY_NAME, File::ACCESS::WRITE) )
    {
        shared.Write(&g_hwnd, sizeof(HWND));
    }

    // メッセージループ
    return Application::Run();
}

//---------------------------------------------------------------------------//

// WinMain.cpp