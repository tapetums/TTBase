#pragma once

//---------------------------------------------------------------------------//
//
// TTBBridgePlugin.hpp
//  プロセス間通信を使って別プロセスのプラグインを操作する
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "include/File.hpp"
#include "../Plugin.hpp"

#include "PluginMgr.hpp"

//---------------------------------------------------------------------------//
// クラス
//---------------------------------------------------------------------------//

class TTBBridgePlugin : public ITTBPlugin
{
    using File = tapetums::File;

private: // Members
    TCHAR        m_path[MAX_PATH];
    bool         m_loaded { false };
    PLUGIN_INFO* m_info   { nullptr };

    DWORD  threadId { 0 };
    HANDLE evt_done { nullptr };

    File shrmem;

public: // ctor / dtor
    TTBBridgePlugin();
    ~TTBBridgePlugin();

    TTBBridgePlugin(const TTBBridgePlugin&)             = delete;
    TTBBridgePlugin& operator =(const TTBBridgePlugin&) = delete;

    TTBBridgePlugin(TTBBridgePlugin&& rhs)             noexcept { swap(std::move(rhs)); }
    TTBBridgePlugin& operator =(TTBBridgePlugin&& rhs) noexcept { swap(std::move(rhs)); return *this; }

    explicit TTBBridgePlugin(LPCTSTR path) : TTBBridgePlugin() { Load(path ); }

public: // mtor
    void swap(TTBBridgePlugin&&) noexcept;

public: // Properties
    bool    is_loaded() const noexcept { return m_loaded; }
    LPCTSTR type()      const noexcept { return TEXT("32"); }

public: // Acessors
    const TCHAR*       path()   const noexcept { return m_path; }
    const HMODULE      handle() const noexcept { return ::GetModuleHandle(nullptr); }
    const PLUGIN_INFO* info()   const noexcept { return m_info; }

    void info(PLUGIN_INFO* info) noexcept;

public:
    bool Load  (LPCTSTR path) override;
    void Free  ()             override;
    bool Reload()             override;

    bool InitInfo(LPTSTR PluginFilename)                    override;
    void FreeInfo()                                         override;
    bool Init    (LPTSTR PluginFilename, DWORD_PTR hPlugin) override;
    void Unload  ()                                         override;
    bool Execute (INT32 CmdID, HWND hwnd)                   override;
    void Hook    (UINT Msg, WPARAM wParam, LPARAM lParam)   override;
};

//---------------------------------------------------------------------------//

// TTBBridgePlugin.hpp