#pragma once

//---------------------------------------------------------------------------//
//
// BridgeWnd.hpp
//  プロセス間通信を使って別プロセスのプラグインからデータを受け取るウィンドウ
//   Copyright (C) 2016 tapetums
//
//---------------------------------------------------------------------------//

#include "include/UWnd.hpp"

//---------------------------------------------------------------------------//
// Class
//---------------------------------------------------------------------------//

class BridgeWnd : public tapetums::UWnd
{
    using super = UWnd;

public:
    BridgeWnd();

public:
    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) override;

private:
    LRESULT CALLBACK OnCreate();
    LRESULT CALLBACK OnDestroy();
    LRESULT CALLBACK OnGetPluginInfo();
    LRESULT CALLBACK OnSetPluginInfo();
    LRESULT CALLBACK OnFreePluginInfo();
    LRESULT CALLBACK OnSetMenuProperty();
    LRESULT CALLBACK OnGetAllPluginInfo();
    LRESULT CALLBACK OnFreePluginInfoArray();
    LRESULT CALLBACK OnSetTaskTrayIcon();
    LRESULT CALLBACK OnWriteLog();
    LRESULT CALLBACK OnExecuteCommand();
};

//---------------------------------------------------------------------------//

// BridgeWnd.hpp