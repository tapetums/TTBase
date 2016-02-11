#pragma once

//---------------------------------------------------------------------------//
// BridgeData.hpp
//---------------------------------------------------------------------------//

#include <cstdint>

struct BridgeData
{
    static constexpr size_t namelen = 40;

    wchar_t filename [namelen];
    wchar_t done     [namelen];
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