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
#pragma comment(lib, "shlwapi.lib") // PathRenameExtension, PathRemoveFileSpec

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

public:
    void load();
    void save();

private:
    settings() { load(); }
    ~settings() = default;
};

//---------------------------------------------------------------------------//
// Methods
//---------------------------------------------------------------------------//

inline void settings::load()
{
    std::array<wchar_t, MAX_PATH> path;

    // iniファイル名取得
    ::GetModuleFileNameW(g_hInst, path.data(), (DWORD)path.size());
    ::PathRenameExtensionW(path.data(), L".ini");

    // パラメータの取得
    TTBaseCompatible = ::GetPrivateProfileIntW
    (
        L"Setting", L"TTBaseCompatible", 0, path.data()
    )
    ? true : false;

    ShowTaskTrayIcon = ::GetPrivateProfileIntW
    (
        L"Setting", L"ShowTaskTrayIcon", 1, path.data()
    ) 
    ? true : false;

    logLevel = ::GetPrivateProfileIntW
    (
        L"Setting", L"logLevel", ERROR_LEVEL(5), path.data()
    );

    logToWindow = ::GetPrivateProfileIntW
    (
        L"Setting", L"logToWindow", 1, path.data()
    )
    ? true : false;

    logToFile = ::GetPrivateProfileIntW
    (
        L"Setting", L"logToFile", 1, path.data()
    )
    ? true : false;

    // ログの出力先がファイルの場合
    if ( logLevel && logToFile )
    {
        // ログフォルダのパスを合成
        ::PathRemoveFileSpecW(path.data());
        ::StringCchCatW(path.data(), path.size(), LR"(\log)");
        if ( ! ::PathIsDirectoryW(path.data()) )
        {
            // フォルダを作成
            ::CreateDirectoryW(path.data(), nullptr);
        }

        // ログファイルのパスを合成
        SYSTEMTIME st;
        ::GetLocalTime(&st);

        std::array<wchar_t, MAX_PATH> log;
        ::StringCchPrintfW
        (
            log.data(), log.size(), LR"(%s\%04u%02u%02u.log)",
            path.data(), st.wYear, st.wMonth, st.wDay
        );

        // ログファイルを開く
        using namespace tapetums;
        const auto result = file.Open
        (
            log.data(),
            File::ACCESS::WRITE, File::SHARE::WRITE, File::OPEN::OR_CREATE
        );
        if ( ! result )
        {
            ::MessageBoxW(nullptr, log.data(), L"ログファイルを作成できません", MB_OK);
            file.Close();
        }
        else
        {
            // 末尾に追記していく
            file.Seek(0, File::ORIGIN::END);

          #if defined(_UNICODE) || defined(UNICODE)
            if ( file.position() == 0 )
            {
                file.Write(UINT16(0xFEFF)); // UTF-16 BOM を出力
            }
          #endif
        }
    }
}

//---------------------------------------------------------------------------//

inline void settings::save()
{
    std::array<wchar_t, MAX_PATH> path;
    std::array<wchar_t, 16>       buf;

    // iniファイル名取得
    ::GetModuleFileNameW(g_hInst, path.data(), (DWORD)path.size());
    ::PathRenameExtensionW(path.data(), L".ini");

    // パラメータの書き出し
    ::WritePrivateProfileStringW
    (
        L"Setting", L"TTBaseCompatible",
        TTBaseCompatible ? L"1" : L"0", path.data()
    );

    ::WritePrivateProfileStringW
    (
        L"Setting", L"ShowTaskTrayIcon",
        ShowTaskTrayIcon ? L"1" : L"0", path.data()
    );

    ::StringCchPrintfW(buf.data(), buf.size(), L"%u", logLevel);
    ::WritePrivateProfileStringW
    (
        L"Setting", L"logLevel", buf.data(), path.data()
    );

    ::StringCchPrintfW(buf.data(), buf.size(), L"%u", logToWindow);
    ::WritePrivateProfileStringW
    (
        L"Setting", L"logToWindow", buf.data(), path.data()
    );

    ::StringCchPrintfW(buf.data(), buf.size(), L"%u", logToFile);
    ::WritePrivateProfileStringW
    (
        L"Setting", L"logToFile", buf.data(), path.data()
    );
}

//---------------------------------------------------------------------------//

// Settings.hpp