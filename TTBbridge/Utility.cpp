#pragma once

//---------------------------------------------------------------------------//
//
// Utility.cpp
//  ユーティリティルーチン
//   Copyright (C) 2014-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <array>
#include <vector>

#include <windows.h>
#include <strsafe.h>

#include "include/Transcode.hpp"
#include "../Plugin.hpp"

#include "Utility.hpp"

//---------------------------------------------------------------------------//
// ユーティリティルーチン
//---------------------------------------------------------------------------//

// 文字列の格納領域を確保し、文字列をコピーして返す
LPWSTR CopyString(LPWSTR Src)
{
    const auto len = 1 + ::lstrlenW(Src);

    auto Dst = new WCHAR[len];
    if ( Dst != nullptr )
    {
        ::StringCchCopyW(Dst, len, Src);
    }

    return Dst;
}

//---------------------------------------------------------------------------//

// 文字列を削除する
void DeleteString(LPCWSTR Str)
{
    if ( Str != nullptr )
    {
        delete[] Str;
    }
}

//---------------------------------------------------------------------------//

// プラグイン側で作成されたプラグイン情報構造体を破棄する
void FreePluginInfo(PLUGIN_INFO_W* PluginInfo)
{
    if ( PluginInfo == nullptr ) { return; }

    // コマンド情報構造体配列の破棄
    if ( PluginInfo->Commands != nullptr )
    {
        for ( size_t i = 0; i < PluginInfo->CommandCount; ++i )
        {
            const auto pCI = &PluginInfo->Commands[i];

            DeleteString(pCI->Name);
            DeleteString(pCI->Caption);
        }
        delete[] PluginInfo->Commands;
    }

    DeleteString(PluginInfo->Filename);
    DeleteString(PluginInfo->Name);

    delete PluginInfo;
}

//---------------------------------------------------------------------------//

// データを ANSI から UTF-16LE へマーシャリングする
PLUGIN_INFO_W* MarshallPluginInfo(PLUGIN_INFO_A* info_a)
{
    using namespace tapetums;

    std::array<wchar_t, 1024> buf; // 1kiB 以上の文字列は打ち切られる

    auto info = new PLUGIN_INFO_W;

    info->NeedVersion  = info_a->NeedVersion;
    info->PluginType   = info_a->PluginType;
    info->VersionMS    = info_a->VersionMS;
    info->VersionLS    = info_a->VersionLS;
    info->CommandCount = info_a->CommandCount;
    info->LoadTime     = info_a->LoadTime;

    toUTF16(info_a->Name, buf.data(), buf.size());
    info->Name = CopyString(buf.data());

    toUTF16(info_a->Filename, buf.data(), buf.size());
    info->Filename = CopyString(buf.data());

    const auto count = info_a->CommandCount;
    info->Commands = new PLUGIN_COMMAND_INFO_W[count];

    for ( DWORD idx = 0; idx < count; ++idx )
    {
        auto& cmd = info->Commands[idx];
        const auto& cmd_a = info_a->Commands[idx];

        toUTF16(cmd_a.Name, buf.data(), buf.size());
        cmd.Name = CopyString(buf.data());

        toUTF16(cmd_a.Caption, buf.data(), buf.size());
        cmd.Caption = CopyString(buf.data());

        cmd.CommandID     = cmd_a.CommandID;
        cmd.Attr          = cmd_a.Attr;
        cmd.ResID         = cmd_a.ResID;
        cmd.DispMenu      = cmd_a.DispMenu;
        cmd.TimerInterval = cmd_a.TimerInterval;
        cmd.TimerCounter  = cmd_a.TimerCounter;
    }

    return info;
}

//---------------------------------------------------------------------------//

// シリアライズ ヘルパー関数
template<typename T>
inline void serialize(T t, std::vector<uint8_t>& data, size_t* p)
{
    const auto size = sizeof(T);
    if ( data.size() < *p + size ) { data.resize(*p + size); }

    ::memcpy(data.data() + *p, &t, size);
    *p += size;
}

// シリアライズ ヘルパー関数 <文字列版>
template<>
inline void serialize<LPWSTR>(LPWSTR str, std::vector<uint8_t>& data, size_t* p)
{
    const auto size = (1 + lstrlenW(str)) * sizeof(wchar_t);
    if ( data.size() < *p + size ) { data.resize(*p + size); }

    ::memcpy(data.data() + *p, str, size);
    *p += size;
}

// プロセス境界を超えるため データをシリアライズする
std::vector<uint8_t> SerializePluginInfo(const PLUGIN_INFO_W* const info)
{
    std::vector<uint8_t> data;
    data.reserve(1024);

    size_t p = 0;
    serialize(info->NeedVersion,  data, &p);
    serialize(info->Name,         data, &p);
    serialize(info->Filename,     data, &p);
    serialize(info->PluginType,   data, &p);
    serialize(info->VersionMS,    data, &p);
    serialize(info->VersionLS,    data, &p);
    serialize(info->CommandCount, data, &p);

    const auto count = info->CommandCount;
    for ( DWORD idx = 0; idx < count; ++idx )
    {
        auto& cmd = info->Commands[idx];

        serialize(cmd.Name,          data, &p);
        serialize(cmd.Caption,       data, &p);
        serialize(cmd.CommandID,     data, &p);
        serialize(cmd.Attr,          data, &p);
        serialize(cmd.ResID,         data, &p);
        serialize(cmd.DispMenu,      data, &p);
        serialize(cmd.TimerInterval, data, &p);
        serialize(cmd.TimerCounter,  data, &p);
    }

    serialize(info->LoadTime, data, &p);

    return data;
}

//---------------------------------------------------------------------------//

// Utility.cpp