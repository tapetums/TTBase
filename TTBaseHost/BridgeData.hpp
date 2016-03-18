﻿#pragma once

//---------------------------------------------------------------------------//
//
// BridgeData.hpp
//  プロセス間通信を使って別プロセスのプラグインを操作するためのデータ
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <cstdint>

struct BridgeData
{
    static constexpr size_t namelen = 40;

    wchar_t downward_file [namelen];
    wchar_t upward_file   [namelen];
    wchar_t downward_lock [namelen];
    wchar_t upward_lock   [namelen];
    wchar_t input_done    [namelen];
    wchar_t output_done   [namelen];
    wchar_t upward_done   [namelen];
};

enum class PluginMsg : uint8_t
{
    OK, NG,
    Load, Free,
    InitInfo, FreeInfo,
    Init, Unload, Execute, Hook,
};

const wchar_t* const PluginMsgTxt[] =
{
    L"OK", L"NG",
    L"Load", L"Free",
    L"InitInfo", L"FreeInfo",
    L"Init", L"Unload", L"Execute", L"Hook",
};

//---------------------------------------------------------------------------//

// BridgeData.hpp