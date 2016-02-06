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

class TTBasePlugin
{
    friend class PluginMgr;

private:
    TCHAR        m_path[MAX_PATH];
    HMODULE      m_handle { nullptr };
    PLUGIN_INFO* m_info   { nullptr };

private:
    TTBEVENT_INITPLUGININFO TTBEvent_InitPluginInfo { nullptr };
    TTBEVENT_FREEPLUGININFO TTBEvent_FreePluginInfo { nullptr };
    TTBEVENT_INIT           TTBEvent_Init           { nullptr };
    TTBEVENT_UNLOAD         TTBEvent_Unload         { nullptr };
    TTBEVENT_EXECUTE        TTBEvent_Execute        { nullptr };
    TTBEVENT_WINDOWSHOOK    TTBEvent_WindowsHook    { nullptr };

public:
    TTBasePlugin() = default;
    ~TTBasePlugin();

    TTBasePlugin(const TTBasePlugin&)             = delete;
    TTBasePlugin& operator =(const TTBasePlugin&) = delete;

    TTBasePlugin(TTBasePlugin&& rhs)             noexcept { swap(std::move(rhs)); }
    TTBasePlugin& operator =(TTBasePlugin&& rhs) noexcept { swap(std::move(rhs)); return *this; }

    explicit TTBasePlugin(LPCTSTR path) { Load(path ); }

public:
    void swap(TTBasePlugin&&) noexcept;

public:
    bool               is_loaded() const noexcept { return m_handle != nullptr; }
    const PLUGIN_INFO* info()      const noexcept { return m_info; }
    PLUGIN_INFO*       info()      noexcept       { return m_info; }

    void info(PLUGIN_INFO* info) noexcept { m_info = info; }

public:
    bool Load(LPCTSTR path);
    void Free();
    bool Reload();

    bool Init   (LPTSTR PluginFilename, DWORD_PTR hPlugin);
    void Unload ();
    bool Execute(INT32 CmdID, HWND hwnd);
    void Hook   (UINT Msg, WPARAM wParam, LPARAM lParam);

private:
    bool InitInfo(LPTSTR PluginFilename);
    void FreeInfo();
};

//---------------------------------------------------------------------------//

class PluginMgr
{
public:
    static PluginMgr& GetInstance() { static PluginMgr mgr; return mgr; }

private:
    std::list<TTBasePlugin> plugins;

public:
    PluginMgr();
    ~PluginMgr();

    PluginMgr(const PluginMgr&)             = delete;
    PluginMgr& operator =(const PluginMgr&) = delete;

    PluginMgr(PluginMgr&&)             = default;
    PluginMgr& operator =(PluginMgr&&) = default;

public:
    size_t size() const noexcept { return plugins.size(); }

    auto begin(){ return plugins.begin(); }
    auto end()  { return plugins.end(); }

    auto begin() const { return plugins.begin(); }
    auto end()   const { return plugins.end(); }

    auto& system() const noexcept { return plugins.front(); }
    auto& system() noexcept       { return plugins.front(); }

public:
    void LoadAll();
    void FreeAll();

private:
    void InitAll();

public:
    const TTBasePlugin* Find(LPCTSTR PluginFilename) const noexcept;
    TTBasePlugin*       Find(LPCTSTR PluginFilename) noexcept;
};

//---------------------------------------------------------------------------//

// PluginMgr.hpp