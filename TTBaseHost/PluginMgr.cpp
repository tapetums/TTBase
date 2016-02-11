//---------------------------------------------------------------------------//
//
// PluginMgr.cpp
//  TTBase プラグイン 管理クラス
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <array>

#include <strsafe.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib") // PathRemoveFileSpec, PathRelativePathTo

#include "../Utility.hpp"

#include "PluginMgr.hpp"
#include "TTBBridgePlugin.hpp"

//---------------------------------------------------------------------------//
// Global Variables
//---------------------------------------------------------------------------//

// WinMain.cpp で宣言
extern HINSTANCE g_hInst;

//---------------------------------------------------------------------------//
// TTBasePlugin
//---------------------------------------------------------------------------//

TTBasePlugin::~TTBasePlugin()
{
    if ( nullptr == m_handle )
    {
        return; // ムーブデストラクタでは余計な処理をしない
    }

    Free();
    FreeInfo();
}

//---------------------------------------------------------------------------//

void TTBasePlugin::swap(TTBasePlugin&& rhs) noexcept
{
    std::swap(m_path,   rhs.m_path);
    std::swap(m_handle, rhs.m_handle);
    std::swap(m_info,   rhs.m_info);

    std::swap(TTBEvent_InitPluginInfo, rhs.TTBEvent_InitPluginInfo);
    std::swap(TTBEvent_FreePluginInfo, rhs.TTBEvent_FreePluginInfo);
    std::swap(TTBEvent_Init,           rhs.TTBEvent_Init);
    std::swap(TTBEvent_Unload,         rhs.TTBEvent_Unload);
    std::swap(TTBEvent_Execute,        rhs.TTBEvent_Execute);
    std::swap(TTBEvent_WindowsHook,    rhs.TTBEvent_WindowsHook);
}

//---------------------------------------------------------------------------//

void TTBasePlugin::info(PLUGIN_INFO* info) noexcept
{
    FreePluginInfo(m_info);
    m_info = CopyPluginInfo(info);
}

//---------------------------------------------------------------------------//

bool TTBasePlugin::Load
(
    LPCTSTR plugin_path
)
{
    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグインの読み込み"));
    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), plugin_path);

    if ( m_handle )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("読み込み済み"));
        return true;
    }

    // DLLの読み込み
    m_handle = ::LoadLibraryEx(plugin_path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if ( nullptr == m_handle )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("NG"));
        return false;
    }

    // DLLのフルパスを取得
    ::GetModuleFileName(m_handle, m_path, MAX_PATH);

    // 関数ポインタの取得
    TTBEvent_InitPluginInfo = (TTBEVENT_INITPLUGININFO)::GetProcAddress(m_handle, "TTBEvent_InitPluginInfo");
    TTBEvent_FreePluginInfo = (TTBEVENT_FREEPLUGININFO)::GetProcAddress(m_handle, "TTBEvent_FreePluginInfo");
    TTBEvent_Init           = (TTBEVENT_INIT)          ::GetProcAddress(m_handle, "TTBEvent_Init");
    TTBEvent_Unload         = (TTBEVENT_UNLOAD)        ::GetProcAddress(m_handle, "TTBEvent_Unload");
    TTBEvent_Execute        = (TTBEVENT_EXECUTE)       ::GetProcAddress(m_handle, "TTBEvent_Execute");
    TTBEvent_WindowsHook    = (TTBEVENT_WINDOWSHOOK)   ::GetProcAddress(m_handle, "TTBEvent_WindowsHook");

    // 必須APIを実装しているか
    if ( nullptr == TTBEvent_InitPluginInfo || nullptr == TTBEvent_FreePluginInfo )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("有効なプラグインではありません"));
        Free();
        return false;
    }

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
    return true;
}

//---------------------------------------------------------------------------//

void TTBasePlugin::Free()
{
    Unload();

    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグインを解放"));
    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), m_path);

    if ( nullptr == m_handle )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("解放済み"));
        return;
    }

    TTBEvent_InitPluginInfo = nullptr;
    TTBEvent_FreePluginInfo = nullptr;
    TTBEvent_Init           = nullptr;
    TTBEvent_Unload         = nullptr;
    TTBEvent_Execute        = nullptr;
    TTBEvent_WindowsHook    = nullptr;

    ::FreeLibrary(m_handle);
    m_handle = nullptr;

    // Reload() する時のため m_path と m_info は記憶しておく

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

bool TTBasePlugin::Reload()
{
    bool result;

    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグインの再読み込み"));

    // プラグインの読み込み
    result = Load(m_path);
    if ( ! result )
    {
        return false;
    }

    // 相対パスの取得
    std::array<TCHAR, MAX_PATH> exe_path;
    std::array<TCHAR, MAX_PATH> relative_path;

    ::GetModuleFileName(g_hInst, exe_path.data(), (DWORD)exe_path.size());
    ::PathRelativePathTo
    (
        relative_path.data(),
        exe_path.data(), FILE_ATTRIBUTE_ARCHIVE,
        m_path,            FILE_ATTRIBUTE_ARCHIVE
    );

    // プラグイン情報の再取得
    //  relative_path の 先頭2文字 (".\") は要らないので ずらす
    InitInfo(relative_path.data() + 2);

    // プラグインの初期化
    result = Init(m_info->Filename, (DWORD_PTR)this);
    if ( ! result )
    {
        Unload();
        return false;
    }

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
    return true;
}

//---------------------------------------------------------------------------//

bool TTBasePlugin::InitInfo(LPTSTR PluginFilename)
{
    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグイン情報をコピー"));

    if ( nullptr == TTBEvent_InitPluginInfo )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("NG"));
        return false;
    }

    // プラグイン情報の取得
    const auto tmp = TTBEvent_InitPluginInfo(PluginFilename);
    if ( nullptr == tmp )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("情報取得失敗"));
        return false;
    }

    // プラグイン情報をコピー
    FreeInfo();
    m_info = CopyPluginInfo(tmp);

    // コピーし終わったので 元データを解放
    TTBEvent_FreePluginInfo(tmp);

    if ( nullptr == m_info )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("情報コピー失敗"));
        return false;
    }

    // ロードにかかった時間は一律 0 にする
    //  21世紀のコンピューターでは殆どの場合 1ms 未満になるため
    m_info->LoadTime = 0;

    WriteLog(ERROR_LEVEL(5), TEXT("  名前:       %s"), m_info->Name);
    WriteLog(ERROR_LEVEL(5), TEXT("  相対パス:   %s"), m_info->Filename);
    WriteLog(ERROR_LEVEL(5), TEXT("  タイプ:     %s"), m_info->PluginType == ptAlwaysLoad ? TEXT("常駐") : TEXT("都度"));
    WriteLog(ERROR_LEVEL(5), TEXT("  コマンド数: %u"), m_info->CommandCount);

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
    return true;
}

//---------------------------------------------------------------------------//

void TTBasePlugin::FreeInfo()
{
    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグイン情報を解放"));

    if ( m_info )
    {
        FreePluginInfo(m_info);
        m_info = nullptr;
    }

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

bool TTBasePlugin::Init(LPTSTR PluginFilename, DWORD_PTR hPlugin)
{
    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグインの初期化"));

    if ( nullptr == TTBEvent_Init )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("未実装"));
        return true;
    }

    const auto result = TTBEvent_Init(PluginFilename, hPlugin);
    if ( ! result )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("NG"));
        return false;
    }

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
    return true;
}

//---------------------------------------------------------------------------//

void TTBasePlugin::Unload()
{
    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグインの終了処理"));

    if ( nullptr == TTBEvent_Unload )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("未実装"));
        return;
    }

    TTBEvent_Unload();
    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

bool TTBasePlugin::Execute(INT32 CmdID, HWND hwnd)
{
    if ( nullptr == TTBEvent_Execute )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("未実装"));
        return true;
    }

    const auto result = TTBEvent_Execute(CmdID, hwnd);
    if ( ! result )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("NG"));
        return false;
    }

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
    return true;
}

//---------------------------------------------------------------------------//

void TTBasePlugin::Hook(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if ( nullptr == TTBEvent_WindowsHook )
    {
        WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("未実装"));
        return;
    }

    TTBEvent_WindowsHook(Msg, wParam, lParam);
    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//
// SystemPlugin
//---------------------------------------------------------------------------//

SystemPlugin::SystemPlugin()
{
    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("システムプラグインを生成"));

    m_handle = g_hInst;
    ::GetModuleFileName(m_handle, m_path, MAX_PATH);

    TTBEvent_InitPluginInfo = ::TTBEvent_InitPluginInfo;
    TTBEvent_FreePluginInfo = ::TTBEvent_FreePluginInfo;
    TTBEvent_Init           = ::TTBEvent_Init;
    TTBEvent_Unload         = ::TTBEvent_Unload;
    TTBEvent_Execute        = ::TTBEvent_Execute;
    TTBEvent_WindowsHook    = ::TTBEvent_WindowsHook;

    m_info = &g_info;
    TTBEvent_Init((LPTSTR)TEXT(":system"), (DWORD_PTR)this);

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

SystemPlugin::~SystemPlugin()
{
    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("システムプラグインを解放"));

    TTBEvent_Unload();
    m_info  = nullptr;

    TTBEvent_InitPluginInfo = nullptr;
    TTBEvent_FreePluginInfo = nullptr;
    TTBEvent_Init           = nullptr;
    TTBEvent_Unload         = nullptr;
    TTBEvent_Execute        = nullptr;
    TTBEvent_WindowsHook    = nullptr;

    m_handle = nullptr;

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//
// PluginMgr
//---------------------------------------------------------------------------//

PluginMgr::PluginMgr()
{
    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグインマネージャを生成"));

    // プラグインリストの一番最初にシステムプラグインを登録
    plugins.emplace_back(new SystemPlugin);

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

PluginMgr::~PluginMgr()
{
    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("プラグインマネージャを解放"));

    // ロードとは逆順に解放する
    FreeAll();

    // システムプラグインも解放
    plugins.clear();

    WriteLog(ERROR_LEVEL(5), TEXT("  %u plugin(s)"), plugins.size());
    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

void PluginMgr::LoadAll()
{
    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("すべてのプラグインを読み込み"));

    // インストールフォルダのパスを取得
    std::array<TCHAR, MAX_PATH> dir_path;
    ::GetModuleFileName(g_hInst, dir_path.data(), (DWORD)dir_path.size());
    ::PathRemoveFileSpec(dir_path.data());

    // インストールフォルダ以下の DLL ファイルを収集
    CollectFile(dir_path.data(), TEXT(".dll"));

    // 読み込んだ DLL を初期化
    InitAll();

    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

void PluginMgr::FreeAll()
{
    WriteLog(ERROR_LEVEL(5), TEXT("%s"), TEXT("すべてのプラグインを解放"));

    // ロードとは逆順に解放 ... システムプラグインは解放しない
    while ( plugins.size() > 1 )
    {
        plugins.pop_back();
    }

    WriteLog(ERROR_LEVEL(5), TEXT("  %u plugin(s)"), plugins.size());
    WriteLog(ERROR_LEVEL(5), TEXT("  %s"), TEXT("OK"));
}

//---------------------------------------------------------------------------//

const ITTBPlugin* PluginMgr::Find
(
    LPCTSTR PluginFilename
)
const noexcept
{
    for ( auto&& plugin: plugins )
    {
        if ( 0 == lstrcmp(PluginFilename, plugin->info()->Filename) )
        {
            return plugin.get();
        }
    }
    return nullptr;
}

//---------------------------------------------------------------------------//

void PluginMgr::CollectFile
(
    LPCTSTR dir_path, LPCTSTR ext
)
{
    TCHAR path[MAX_PATH];
    ::StringCchPrintf(path, MAX_PATH, TEXT(R"(%s\*)"), dir_path);

    WIN32_FIND_DATA fd { };
    const auto hFindFile = ::FindFirstFile(path, &fd);
    if ( INVALID_HANDLE_VALUE == hFindFile )
    {
        // 指定されたフォルダが見つからなかった
        return;
    }

    // フォルダ内にあるものを列挙する
    do
    {
        // 隠しファイルは飛ばす
        if ( fd.cFileName[0] == '.' )
        {
            continue;
        }
        if ( fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
        {
            continue;
        }

        // フルパスを合成
        ::StringCchPrintf
        (
            path, MAX_PATH, TEXT(R"(%s\%s)"), dir_path, fd.cFileName
        );

        if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            // サブフォルダを検索
            CollectFile(path, ext);
        }
        else
        {
            // 拡張子を取得
            TCHAR* s;
            for ( s = path + lstrlen(path) - 1; *s != '.'; --s );
            
            // 目的の拡張子だったら
            if ( 0 == lstrcmp(s, ext) )
            {
                // コンテナに収録
                ITTBPlugin* plugin;

                plugin = new TTBasePlugin(path);
                if ( plugin->is_loaded() )
                {
                    plugins.emplace_back(plugin);
                    continue;
                }
                else
                {
                    if ( plugin ) { delete plugin; }
                }

              #if INTPTR_MAX == INT64_MAX
                plugin = new TTBBridgePlugin(path);
                if ( plugin->is_loaded() )
                {
                    plugins.emplace_back(plugin);
                }
                else
                {
                    if ( plugin ) { delete plugin; }
                }
              #endif
            }
        }
    }
    while ( ::FindNextFile(hFindFile, &fd) );

    ::FindClose( hFindFile );

    return;
}

//---------------------------------------------------------------------------//

void PluginMgr::InitAll()
{
    std::array<TCHAR, MAX_PATH> exe_path;
    std::array<TCHAR, MAX_PATH> relative_path;

    // 本体の絶対パスを取得
    ::GetModuleFileName(g_hInst, exe_path.data(), (DWORD)exe_path.size());

    // プラグインの初期化
    // * システムプラグインは既に初期化済みのため、リストの2番目から開始
    auto it = ++plugins.begin();
    while ( it != plugins.end() )
    {
        auto&& plugin = *it;
        ::PathRelativePathTo
        (
            relative_path.data(),
            exe_path.data(), FILE_ATTRIBUTE_ARCHIVE,
            plugin->path(),  FILE_ATTRIBUTE_ARCHIVE
        );

        // プラグイン情報の取得
        //  relative_path の 先頭2文字 (".\") は要らないので ずらす
        const auto result = plugin->InitInfo(relative_path.data() + 2);
        if ( ! result )
        {
            it = plugins.erase(it);
            continue;
        }

        // プラグインの初期化
        if ( plugin->info()->PluginType != ptAlwaysLoad )
        {
            plugin->Free();
        }
        else
        {
            plugin->Init(plugin->info()->Filename, (DWORD_PTR)plugin.get());
        }

        ++it;
    }

    WriteLog(ERROR_LEVEL(5), TEXT("  %u plugin(s)"), plugins.size());
}

//---------------------------------------------------------------------------//

// PluginMgr.cpp