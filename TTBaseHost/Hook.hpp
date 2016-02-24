#pragma once

//---------------------------------------------------------------------------//
//
// Hook.hpp
//  フック操作
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>

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
    HookDll()
    {
        SystemLog(TEXT("%s"), TEXT("Hook.dll の ロードを開始"));

        m_handle = ::LoadLibraryEx(TEXT("Hook.dll"), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        if ( nullptr == m_handle )
        {
            WriteLog(elError, TEXT("%s"), TEXT("Hook.dll の ロードに失敗しました"));
            return;
        }

        // 関数ポインタの取得
        m_InstallHook   = (INSTALLHOOK)  ::GetProcAddress(m_handle, "InstallHook");
        m_UninstallHook = (UNINSTALLHOOK)::GetProcAddress(m_handle, "UninstallHook");

        SystemLog(TEXT("  %s"), TEXT("OK"));
    }

    ~HookDll()
    {
        SystemLog(TEXT("%s"), TEXT("Hook.dll の アンロードを開始"));

        if ( nullptr == m_handle )
        {
            WriteLog(elError, TEXT("%s"), TEXT("Hook.dll の アンロードに失敗しました"));
            return;
        }

        UninstallHook();

        ::FreeLibrary(m_handle);
        m_handle = nullptr;

        SystemLog(TEXT("  %s"), TEXT("OK"));
    }

public:
    bool InstallHook(HWND hwnd)
    {
        return m_InstallHook ? m_InstallHook(hwnd) ? true : false : false;
    }

    void UninstallHook()
    {
        if ( m_UninstallHook )
        {
            m_UninstallHook();
        }
    }
};

//---------------------------------------------------------------------------//

// Hook.hpp