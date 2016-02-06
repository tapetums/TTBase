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
#pragma comment(lib, "shlwapi.lib") // PathRemoveFileSpec

#include "include/File.hpp"

//---------------------------------------------------------------------------//
// Global Variables
//---------------------------------------------------------------------------//

// WinMain.cpp で宣言
extern HINSTANCE g_hInst;

//---------------------------------------------------------------------------//
// Class
//---------------------------------------------------------------------------//

class settings
{
public:
    static settings& get() { static settings s; return s; }

public:
    bool  TTBaseCompatible;
    bool  ShowTaskTrayIcon;
    DWORD logLevel;
    bool  logToWindow;
    bool  logToFile;

    tapetums::File file;

private:
    settings();
    ~settings();
};

//---------------------------------------------------------------------------//
// Methods
//---------------------------------------------------------------------------//

inline settings::settings()
{
    std::array<TCHAR, MAX_PATH> path;

    // iniファイル名取得
    ::GetModuleFileName(g_hInst, path.data(), (DWORD)path.size());
    ::PathRenameExtension(path.data(), TEXT(".ini"));

    // パラメータの取得
    TTBaseCompatible = ::GetPrivateProfileInt
    (
        TEXT("Setting"), TEXT("TTBaseCompatible"), 0, path.data()
    )
    ? true : false;

    ShowTaskTrayIcon = ::GetPrivateProfileInt
    (
        TEXT("Setting"), TEXT("ShowTaskTrayIcon"), 1, path.data()
    ) 
    ? true : false;

    logLevel = ::GetPrivateProfileInt
    (
        TEXT("Setting"), TEXT("logLevel"), ERROR_LEVEL::elDebug + 1, path.data()
    );

    logToWindow = ::GetPrivateProfileInt
    (
        TEXT("Setting"), TEXT("logToWindow"), 1, path.data()
    )
    ? true : false;

    logToFile = ::GetPrivateProfileInt
    (
        TEXT("Setting"), TEXT("logToFile"), 1, path.data()
    )
    ? true : false;

    // ログの出力先がファイルの場合
    if ( logToFile )
    {
        ::PathRenameExtension(path.data(), TEXT(".log"));

        using namespace tapetums;
        const auto result = file.Open
        (
            path.data(),
            File::ACCESS::WRITE, File::SHARE::WRITE, File::OPEN::OR_TRUNCATE
        );
        if ( !result )
        {
            ::MessageBox(nullptr, path.data(), TEXT("ログファイルを作成できません"), MB_OK);
            file.Close();
        }
        else
        {
          #if defined(_UNICODE) || defined(UNICODE)
            file.Write(UINT16(0xFEFF)); // UTF-16 BOM を出力
          #endif
        }
    }
}

//---------------------------------------------------------------------------//

inline settings::~settings()
{
    std::array<TCHAR, MAX_PATH> path;
    std::array<TCHAR, 16>       buf;

    // iniファイル名取得
    ::GetModuleFileName(g_hInst, path.data(), (DWORD)path.size());
    ::PathRenameExtension(path.data(), TEXT(".ini"));

    // パラメータの書き出し
    ::WritePrivateProfileString
    (
        TEXT("Setting"), TEXT("TTBaseCompatible"),
        TTBaseCompatible ? TEXT("1") : TEXT("0"), path.data()
    );

    ::WritePrivateProfileString
    (
        TEXT("Setting"), TEXT("ShowTaskTrayIcon"),
        ShowTaskTrayIcon ? TEXT("1") : TEXT("0"), path.data()
    );

    ::StringCchPrintf(buf.data(), buf.size(), TEXT("%u"), logLevel);
    ::WritePrivateProfileString
    (
        TEXT("Setting"), TEXT("logLevel"), buf.data(), path.data()
    );

    ::StringCchPrintf(buf.data(), buf.size(), TEXT("%u"), logToWindow);
    ::WritePrivateProfileString
    (
        TEXT("Setting"), TEXT("logToWindow"), buf.data(), path.data()
    );

    ::StringCchPrintf(buf.data(), buf.size(), TEXT("%u"), logToFile);
    ::WritePrivateProfileString
    (
        TEXT("Setting"), TEXT("logToFile"), buf.data(), path.data()
    );
}

//---------------------------------------------------------------------------//

// Settings.hpp