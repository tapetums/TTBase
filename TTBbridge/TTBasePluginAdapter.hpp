#pragma once

//---------------------------------------------------------------------------//
//
// TTBasePluginAdapter.hpp
//  TTBase プラグインを プロセスを超えて使用するためのアダプタ
//   Copyright (C) 2014-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "include/File.hpp"
#include "../Plugin.hpp"

//---------------------------------------------------------------------------//
// クラス
//---------------------------------------------------------------------------//

class TTBasePluginAdapter
{
    using File = tapetums::File;

private:
    wchar_t path [MAX_PATH];
    HMODULE handle { nullptr };

    TTBEVENT_INITPLUGININFO TTBEvent_InitPluginInfo { nullptr };
    TTBEVENT_FREEPLUGININFO TTBEvent_FreePluginInfo { nullptr };
    TTBEVENT_INIT           TTBEvent_Init           { nullptr };
    TTBEVENT_UNLOAD         TTBEvent_Unload         { nullptr };
    TTBEVENT_EXECUTE        TTBEvent_Execute        { nullptr };
    TTBEVENT_WINDOWSHOOK    TTBEvent_WindowsHook    { nullptr };

public:
    TTBasePluginAdapter() = default;
    ~TTBasePluginAdapter() { free(); }

public:
    // イベントハンドラ
    void operator ()();

    // プラグインの読み込み
    bool load(File& plugin_data);

    // プラグインの解放
    void free();

    // プラグイン情報の初期化
    void init_plugin_info(File& plugin_data);

    // プラグイン情報の解放
    void free_plugin_info();

    // プラグインの初期化
    bool init(File& plugin_data);

    // プラグインの解放
    void unload();

    // コマンドの実行
    bool execute(File& plugin_data);

    // フックの開始
    void hook(File& plugin_data);
};

//---------------------------------------------------------------------------//

// TTBasePluginAdapter.hpp