#pragma once

//---------------------------------------------------------------------------//
//
// Settings.hpp
//  設定ファイル 管理クラス
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

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
    wchar_t path [MAX_PATH];

    // iniファイル名取得
    ::GetModuleFileNameW(g_hInst, path, MAX_PATH);
    ::PathRenameExtensionW(path, L".ini");

    // パラメータの取得
    x = ::GetPrivateProfileIntW(L"Setting", L"x", -32, path);
    y = ::GetPrivateProfileIntW(L"Setting", L"y",   0, path);
    w = ::GetPrivateProfileIntW(L"Setting", L"w",  32, path);
    h = ::GetPrivateProfileIntW(L"Setting", L"h",  32, path);
}

//---------------------------------------------------------------------------//

inline void Settings::save()
{
    wchar_t path [MAX_PATH];
    wchar_t buf  [16];

    // iniファイル名取得
    ::GetModuleFileNameW(g_hInst, path, MAX_PATH);
    ::PathRenameExtensionW(path, L".ini");

    // パラメータの書き出し
    ::wsprintfW(buf, L"%i", x);
    ::WritePrivateProfileStringW(L"Setting", L"x", buf, path);

    ::wsprintfW(buf, L"%i", y);
    ::WritePrivateProfileStringW(L"Setting", L"y", buf, path);

    ::wsprintfW(buf, L"%i", w);
    ::WritePrivateProfileStringW(L"Setting", L"w", buf, path);

    ::wsprintfW(buf, L"%i", h);
    ::WritePrivateProfileStringW(L"Setting", L"h", buf, path);
}

//---------------------------------------------------------------------------//

// Settings.hpp