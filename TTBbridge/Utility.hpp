#pragma once

//---------------------------------------------------------------------------//
//
// Utility.hpp
//  ユーティリティルーチン
//   Copyright (C) 2014-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <vector>

using LPCWSTR = const wchar_t*;

struct PLUGIN_INFO_A;
struct PLUGIN_INFO_W;

//---------------------------------------------------------------------------//
// ユーティリティルーチン
//---------------------------------------------------------------------------//

// 文字列の格納領域を確保し、文字列をコピーして返す
LPWSTR CopyString(LPWSTR Src);

// 文字列を削除する
void DeleteString(LPCWSTR Str);

// プラグイン側で作成されたプラグイン情報構造体を破棄する
void FreePluginInfo(PLUGIN_INFO_W* PluginInfo);

// データを ANSI から UTF-16LE へマーシャリングする
PLUGIN_INFO_W* MarshallPluginInfo(PLUGIN_INFO_A* info_a);

// プロセス境界を超えるため データをシリアライズする
std::vector<uint8_t> SerializePluginInfo(const PLUGIN_INFO_W* const info);

//---------------------------------------------------------------------------//

// Utility.hpp