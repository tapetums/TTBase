#pragma once

//---------------------------------------------------------------------------//
//
// PluginMgr.hpp
//  TTBase プラグイン 管理クラス
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <list>
#include <memory>

#include <windows.h>

#include "../Plugin.hpp"

//---------------------------------------------------------------------------//

struct ITTBPlugin;
class  TTBasePlugin;
class  PluginMgr;

//---------------------------------------------------------------------------//
// Classes
//---------------------------------------------------------------------------//

struct ITTBPlugin
{
// Destructor
    virtual ~ITTBPlugin() = 0 { };

// Properties
    virtual bool is_loaded() const noexcept = 0;

// Acessors
    virtual const TCHAR*       path()   const noexcept = 0;
    virtual const HMODULE      handle() const noexcept = 0;
    virtual const PLUGIN_INFO* info()   const noexcept = 0;

    virtual void info(PLUGIN_INFO* info) noexcept = 0;

// Methods
    virtual bool Load  (LPCTSTR path) = 0;
    virtual void Free  () = 0;
    virtual bool Reload() = 0;

    virtual bool InitInfo(LPTSTR PluginFilename) = 0;
    virtual void FreeInfo() = 0;
    virtual bool Init    (LPTSTR PluginFilename, DWORD_PTR hPlugin) = 0;
    virtual void Unload  () = 0;
    virtual bool Execute (INT32 CmdID, HWND hwnd) = 0;
    virtual void Hook    (UINT Msg, WPARAM wParam, LPARAM lParam) = 0;
};

//---------------------------------------------------------------------------//

class TTBasePlugin : public ITTBPlugin
{
protected: // Members
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

public: // mtor
    void swap(TTBasePlugin&&) noexcept;

public: // Properties
    bool is_loaded() const noexcept { return m_handle != nullptr; }

public: // Acessors
    const TCHAR*       path()   const noexcept { return m_path; }
    const HMODULE      handle() const noexcept { return m_handle; }
    const PLUGIN_INFO* info()   const noexcept { return m_info; }

    void info(PLUGIN_INFO* info) noexcept;

public: // Methods
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

#undef package

//---------------------------------------------------------------------------//

class SystemPlugin : public TTBasePlugin
{
public: // ctor / dtor
    SystemPlugin();
    ~SystemPlugin();
};

//---------------------------------------------------------------------------//

class PluginMgr
{
public: // Singleton
    static PluginMgr& GetInstance() { static PluginMgr mgr; return mgr; }

private: // Members
    std::list<std::unique_ptr<ITTBPlugin>> plugins;

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
    const ITTBPlugin* Find(LPCTSTR PluginFilename) const noexcept;

private: // Internal Methods
    void CollectFile(LPCTSTR dir_path, LPCTSTR ext);
    void InitAll();
};

//---------------------------------------------------------------------------//

// PluginMgr.hpp