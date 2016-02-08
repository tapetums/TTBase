#pragma once

//---------------------------------------------------------------------------//
//
// CollectFile.hpp
//  指定以下のフォルダの中から一致する拡張子のファイルを捜し、コンテナに収集する
//   Copyright (C) 2013-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <strsafe.h>

//---------------------------------------------------------------------------//

// 指定以下のフォルダの中から一致する拡張子のファイルを捜し、コンテナに収集する
//
// NOTE: 拡張子は .*** の形式 (ドット付き)
//       コンテナは emplace_back(LPCSTR) のインターフェイスを持っていること
template<typename Container>
inline void CollectFileByExt
(
    LPCTSTR dir_path, LPCTSTR ext, bool search_in_subfoler, Container& container
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
            // フォルダだったら
            if ( search_in_subfoler )
            {
                // サブフォルダを検索
                CollectFileByExt(path, ext, search_in_subfoler, container);
            }
        }
        else
        {
            // 拡張子を取得
            TCHAR* s;
            for ( s = path + lstrlen(path) - 1; *s != '.'; --s );
            
            // 目的の拡張子だったら
            if ( 0 == lstrcmp(s, ext) )
            {
                // コンテナに収集
                container.emplace_back(path);
            }
        }
    }
    while ( ::FindNextFile(hFindFile, &fd) );

    ::FindClose( hFindFile );

    return;
}

//---------------------------------------------------------------------------//

// CollectFile.hpp