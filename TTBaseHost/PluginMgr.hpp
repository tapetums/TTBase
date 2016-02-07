#pragma once

//---------------------------------------------------------------------------//
//
// PluginMgr.hpp
//  TTBase プラグイン 管理クラス
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <list>

#include <windows.h>

#include "../Plugin.hpp"

//---------------------------------------------------------------------------//

class TTBasePlugin;
class PluginMgr;

//---------------------------------------------------------------------------//
// Classes
//---------------------------------------------------------------------------//

#define package private

class TTBasePlugin
{
    friend class PluginMgr;

package: // Members
    TCHAR        m_path[MAX_PATH];
    HMODULE      m_handle { nullptr };
    PLUGIN_INFO* m_info   { nullptr };

    TTBEVENT_INITPLUGININFO TTBEvent_InitPluginInfo { nullptr };
    TTBEVENT_FREEPLUGININFO TTBEvent_FreePluginInfo { nullptr };
    TTBEVENT_INIT           TTBEvent_Init           { nullptr };
    TTBEVENT_UNLOAD         TTBEvent_Unload         { nullptr };
    TTBEVENT_EXECUTE        TTBEvent_Execute        { nullptr };
    TTBEVENT_WINDOWSHOOK    TTBEvent_WindowsHook    { nullptr };

public: // ctor / dtor
    TTBasePlugin() = default;
    ~TTBasePlugin();

    TTBasePlugin(const TTBasePlugin&)             = delete;
    TTBasePlugin& operator =(const TTBasePlugin&) = delete;

    TTBasePlugin(TTBasePlugin&& rhs)             noexcept { swap(std::move(rhs)); }
    TTBasePlugin& operator =(TTBasePlugin&& rhs) noexcept { swap(std::move(rhs)); return *this; }

    explicit TTBasePlugin(LPCTSTR path) { Load(path ); }

public: // Move Constructor
    void swap(TTBasePlugin&&) noexcept;

public: // Properties
    bool is_loaded() const noexcept { return m_handle != nullptr; }

public: // Acessors
    const auto path()   const noexcept { return m_path; }
    const auto handle() const noexcept { return m_handle; }
    const auto info()   const noexcept { return m_info; }

    void info(PLUGIN_INFO* info) noexcept;

public: // Methods
    bool Load(LPCTSTR path);
    void Free();
    bool Reload();

    bool Init   (LPTSTR PluginFilename, DWORD_PTR hPlugin);
    void Unload ();
    bool Execute(INT32 CmdID, HWND hwnd);
    void Hook   (UINT Msg, WPARAM wParam, LPARAM lParam);

package: // Internal Methods
    bool InitInfo(LPTSTR PluginFilename);
    void FreeInfo();
};

#undef package

//---------------------------------------------------------------------------//

class PluginMgr
{
public: // Singleton
    static PluginMgr& GetInstance() { static PluginMgr mgr; return mgr; }

private: // Members
    std::list<TTBasePlugin> plugins;

public: // ctor / dtor
    PluginMgr();
    ~PluginMgr();

    PluginMgr(const PluginMgr&)             = delete;
    PluginMgr& operator =(const PluginMgr&) = delete;

    PluginMgr(PluginMgr&&)             = default;
    PluginMgr& operator =(PluginMgr&&) = default;

public: // Properties
    size_t size() const noexcept { return plugins.size(); }

public: // Acessors
    const auto& system() const noexcept { return plugins.front(); }
    auto&       system() noexcept       { return plugins.front(); }

public: // Iterators
    auto begin(){ return plugins.begin(); }
    auto end()  { return plugins.end(); }

    auto begin() const { return plugins.begin(); }
    auto end()   const { return plugins.end(); }

public: // Methods
    void LoadAll();
    void FreeAll();
    const TTBasePlugin* Find(LPCTSTR PluginFilename) const noexcept;

private: // Internal Methods
    void InitAll();
};

//---------------------------------------------------------------------------//

// PluginMgr.hpp