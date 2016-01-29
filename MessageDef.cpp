//---------------------------------------------------------------------------//
//
// MessageDef.cpp
//  TTB Plugin Template (C++11)
//
//---------------------------------------------------------------------------//

#include <windows.h>

//---------------------------------------------------------------------------//
// Message用文字列
//---------------------------------------------------------------------------//

constexpr auto TTB_HSHELL_ACTIVATESHELLWINDOW_MESSAGE = TEXT("TTBase HShell Activate ShellWindow");
constexpr auto TTB_HSHELL_GETMINRECT_MESSAGE          = TEXT("TTBase HShell GetMinRect");
constexpr auto TTB_HSHELL_LANGUAGE_MESSAGE            = TEXT("TTBase HShell Language");
constexpr auto TTB_HSHELL_REDRAW_MESSAGE              = TEXT("TTBase HShell Redraw");
constexpr auto TTB_HSHELL_TASKMAN_MESSAGE             = TEXT("TTBase HShell TaskMan");
constexpr auto TTB_HSHELL_WINDOWACTIVATED_MESSAGE     = TEXT("TTBase HShell WindowActivated");
constexpr auto TTB_HSHELL_WINDOWCREATED_MESSAGE       = TEXT("TTBase HShell WindowCreated");
constexpr auto TTB_HSHELL_WINDOWDESTROYED_MESSAGE     = TEXT("TTBase HShell WindowDestroyed");
constexpr auto TTB_HMOUSE_ACTION_MESSAGE              = TEXT("TTBase HMouse Action");
constexpr auto TTB_ICON_NOTIFY_MESSAGE                = TEXT("TTBase ICON NOTIFY");
constexpr auto TTB_LOAD_DATA_FILE_MESSAGE             = TEXT("TTBase LOAD DATA FILE");
constexpr auto TTB_SAVE_DATA_FILE_MESSAGE             = TEXT("TTBase SAVE DATA FILE");

//---------------------------------------------------------------------------//
// Message定義
//---------------------------------------------------------------------------//

UINT TTB_HSHELL_ACTIVATESHELLWINDOW { 0 };
UINT TTB_HSHELL_GETMINRECT          { 0 };
UINT TTB_HSHELL_LANGUAGE            { 0 };
UINT TTB_HSHELL_REDRAW              { 0 };
UINT TTB_HSHELL_TASKMAN             { 0 };
UINT TTB_HSHELL_WINDOWACTIVATED     { 0 };
UINT TTB_HSHELL_WINDOWCREATED       { 0 };
UINT TTB_HSHELL_WINDOWDESTROYED     { 0 };
UINT TTB_HMOUSE_ACTION              { 0 };
UINT TTB_ICON_NOTIFY                { 0 };
UINT TTB_LOAD_DATA_FILE             { 0 };
UINT TTB_SAVE_DATA_FILE             { 0 };

//---------------------------------------------------------------------------//
// 取得用関数
//---------------------------------------------------------------------------//

void RegisterMessages(void)
{
    TTB_HSHELL_ACTIVATESHELLWINDOW = ::RegisterWindowMessage(TTB_HSHELL_ACTIVATESHELLWINDOW_MESSAGE);
    TTB_HSHELL_GETMINRECT          = ::RegisterWindowMessage(TTB_HSHELL_GETMINRECT_MESSAGE);
    TTB_HSHELL_LANGUAGE            = ::RegisterWindowMessage(TTB_HSHELL_LANGUAGE_MESSAGE);
    TTB_HSHELL_REDRAW              = ::RegisterWindowMessage(TTB_HSHELL_REDRAW_MESSAGE);
    TTB_HSHELL_TASKMAN             = ::RegisterWindowMessage(TTB_HSHELL_TASKMAN_MESSAGE);
    TTB_HSHELL_WINDOWACTIVATED     = ::RegisterWindowMessage(TTB_HSHELL_WINDOWACTIVATED_MESSAGE);
    TTB_HSHELL_WINDOWCREATED       = ::RegisterWindowMessage(TTB_HSHELL_WINDOWCREATED_MESSAGE);
    TTB_HSHELL_WINDOWDESTROYED     = ::RegisterWindowMessage(TTB_HSHELL_WINDOWDESTROYED_MESSAGE);
    TTB_HMOUSE_ACTION              = ::RegisterWindowMessage(TTB_HMOUSE_ACTION_MESSAGE);
    TTB_ICON_NOTIFY                = ::RegisterWindowMessage(TTB_ICON_NOTIFY_MESSAGE);
    TTB_LOAD_DATA_FILE             = ::RegisterWindowMessage(TTB_LOAD_DATA_FILE_MESSAGE);
    TTB_SAVE_DATA_FILE             = ::RegisterWindowMessage(TTB_SAVE_DATA_FILE_MESSAGE);
}

//---------------------------------------------------------------------------//

// MessageDef.cpp