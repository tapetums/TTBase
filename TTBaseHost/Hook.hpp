#pragma once

//---------------------------------------------------------------------------//
//
// Hook.hpp
//  フック操作
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <strsafe.h>

#include "../Utility.hpp"

#if SYS_DEBUG

template<typename C, typename... Args>
void SystemLog(const C* const format, Args... args)
{
    WriteLog(ERROR_LEVEL(5), format, args...);
}

#else

#define SystemLog(format, ...)

#endif

//---------------------------------------------------------------------------//
// Global Variables
//---------------------------------------------------------------------------//

// WinMain.cpp で宣言
extern HINSTANCE g_hInst;

//---------------------------------------------------------------------------//
// 関数型宣言
//---------------------------------------------------------------------------//

extern "C"
{
    using INSTALLHOOK   = BOOL (WINAPI*)(HWND hwnd);
    using UNINSTALLHOOK = void (WINAPI*)();
}

//---------------------------------------------------------------------------//
// クラス
//---------------------------------------------------------------------------//

// Hook.DLL を保持する RAII クラス
class HookDll
{
private:
    HMODULE       m_handle        { nullptr };
    INSTALLHOOK   m_InstallHook   { nullptr };
    UNINSTALLHOOK m_UninstallHook { nullptr };

public:
    HookDll()  { Load(); }
    ~HookDll() { Unload(); }

public:
    void InstallHook(HWND hwnd)
    {
        if ( m_InstallHook )
        {
            m_InstallHook(hwnd);
        }
    }

    void UninstallHook()
    {
        if ( m_UninstallHook )
        {
            m_UninstallHook();
        }
    }

private:
    void Load()
    {
        SystemLog(TEXT("%s"), TEXT("Hook.dll の ロードを開始"));

        // Hook.dll の フルパスを求める
        TCHAR path [MAX_PATH];
        const auto len = ::GetModuleFileName(g_hInst, path, MAX_PATH);

        size_t i = len - 1;
        while ( path[i] != '\\' ) { --i; }
        path[i + 1] = '\0';

        ::StringCchCat(path, MAX_PATH, TEXT("Hook.dll"));
        SystemLog(TEXT("  %s"), path);

        // Hook.dll が 隠しファイルかどうかを調べる
        const auto attr = ::GetFileAttributes(path);
        if ( attr == DWORD(-1) )
        {
            WriteLog(elError, TEXT("%s"), TEXT("Hook.dll の属性を取得できませんでした"));
            return;
        }
        if ( attr & FILE_ATTRIBUTE_HIDDEN )
        {
            WriteLog(elInfo, TEXT("%s"), TEXT("Hook.dll は隠しファイルです。ロードを中断します"));
            return;
        }

        // Hook.dll の 読み込み
        m_handle = ::LoadLibraryEx(path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        if ( nullptr == m_handle )
        {
            WriteLog(elWarning, TEXT("%s"), TEXT("Hook.dll の ロードに失敗しました"));
            return;
        }

        // 関数ポインタの取得
        m_InstallHook   = (INSTALLHOOK)  ::GetProcAddress(m_handle, "InstallHook");
        m_UninstallHook = (UNINSTALLHOOK)::GetProcAddress(m_handle, "UninstallHook");

        SystemLog(TEXT("  %s"), TEXT("OK"));
    }

    void Unload()
    {
        SystemLog(TEXT("%s"), TEXT("Hook.dll の アンロードを開始"));

        if ( nullptr == m_handle )
        {
            WriteLog(elInfo, TEXT("%s"), TEXT("Hook.dll はロードされていませんでした"));
            return;
        }

        UninstallHook();

        ::FreeLibrary(m_handle);
        m_handle = nullptr;

        SystemLog(TEXT("  %s"), TEXT("OK"));
    }
};

//---------------------------------------------------------------------------//

// Hook.hpp