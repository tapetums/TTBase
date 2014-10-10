//---------------------------------------------------------------------------//
//
// IniFile.h
//  INI 設定ファイル 操作関数
//   Copyright (C) 2014 tapetums
//
//---------------------------------------------------------------------------//

#define MAX_ACCOUNT 8
#define MAX_MESSAGE_LEN 1024

//---------------------------------------------------------------------------//
// グローバル変数
//---------------------------------------------------------------------------//

extern bool   g_ask_each_tweet;
extern size_t g_user_index;
extern TCHAR  g_username[MAX_ACCOUNT][MAX_PATH];
extern TCHAR  g_msgstub[MAX_MESSAGE_LEN];

//---------------------------------------------------------------------------//

// INI ファイルの読み書き
bool __stdcall LoadIniFile(LPCTSTR ininame);
bool __stdcall SaveIniFile(LPCTSTR ininame);

//---------------------------------------------------------------------------//

// IniFile.h