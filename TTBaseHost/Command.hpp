#pragma once

//---------------------------------------------------------------------------//
//
// Command.hpp
//  システムプラグインで使用するコマンドの定義
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

// コマンドID
enum CMD : INT32
{
    CMD_EXIT,
    CMD_SETTINGS,
    CMD_SHOW_VER_INFO,
    CMD_RELOAD,
    CMD_OPEN_FOLDER,
    CMD_TRAYICON,
    CMD_COUNT
};

//---------------------------------------------------------------------------//

// Command.hpp