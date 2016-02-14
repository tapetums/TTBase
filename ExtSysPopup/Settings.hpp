#pragma once

//---------------------------------------------------------------------------//
//
// Settings.hpp
//  設定ファイル 管理クラス
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include <array>

#include <windows.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib") // PathRenameExtension

//---------------------------------------------------------------------------//
// Global Variables
//---------------------------------------------------------------------------//

// WinMain.cpp で宣言
extern HINSTANCE g_hInst;

//---------------------------------------------------------------------------//
// Class
//---------------------------------------------------------------------------//

class Settings
{
public:
    INT32 x;
    INT32 y;
    INT32 w;
    INT32 h;

public:
    Settings()  { load(); }
    ~Settings() { save(); }

public:
    void load();
    void save();
};

extern Settings* settings;

//---------------------------------------------------------------------------//
// Methods
//---------------------------------------------------------------------------//

inline void Settings::load()
{
    std::array<wchar_t, MAX_PATH> path;

    // iniファイル名取得
    ::GetModuleFileNameW(g_hInst, path.data(), (DWORD)path.size());
    ::PathRenameExtensionW(path.data(), L".ini");

    // パラメータの取得
    x = ::GetPrivateProfileIntW(L"Setting", L"x", -32, path.data());
    y = ::GetPrivateProfileIntW(L"Setting", L"y",   0, path.data());
    w = ::GetPrivateProfileIntW(L"Setting", L"w",  32, path.data());
    h = ::GetPrivateProfileIntW(L"Setting", L"h",  32, path.data());
}

//---------------------------------------------------------------------------//

inline void Settings::save()
{
    std::array<wchar_t, MAX_PATH> path;
    std::array<wchar_t, 16>       buf;

    // iniファイル名取得
    ::GetModuleFileNameW(g_hInst, path.data(), (DWORD)path.size());
    ::PathRenameExtensionW(path.data(), L".ini");

    // パラメータの書き出し
    ::wsprintfW(buf.data(), L"%i", x);
    ::WritePrivateProfileStringW(L"Setting", L"x", buf.data(), path.data());

    ::wsprintfW(buf.data(), L"%i", y);
    ::WritePrivateProfileStringW(L"Setting", L"y", buf.data(), path.data());

    ::wsprintfW(buf.data(), L"%i", w);
    ::WritePrivateProfileStringW(L"Setting", L"w", buf.data(), path.data());

    ::wsprintfW(buf.data(), L"%i", h);
    ::WritePrivateProfileStringW(L"Setting", L"h", buf.data(), path.data());
}

//---------------------------------------------------------------------------//

// Settings.hpp