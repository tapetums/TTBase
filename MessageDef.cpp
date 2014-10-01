//---------------------------------------------------------------------------//
//
//                         TTB Plugin Template(VC++)
//
//                              MessageDef.cpp
//
//---------------------------------------------------------------------------//

#include <windows.h>

// Message—p•¶Žš—ñ
#ifdef _WIN64
#define TTB_HSHELL_ACTIVATESHELLWINDOW_MESSAGE L"TTBase HShell Activate ShellWindow"
#define TTB_HSHELL_GETMINRECT_MESSAGE          L"TTBase HShell GetMinRect"
#define TTB_HSHELL_LANGUAGE_MESSAGE            L"TTBase HShell Language"
#define TTB_HSHELL_REDRAW_MESSAGE              L"TTBase HShell Redraw"
#define TTB_HSHELL_TASKMAN_MESSAGE             L"TTBase HShell TaskMan"
#define TTB_HSHELL_WINDOWACTIVATED_MESSAGE     L"TTBase HShell WindowActivated"
#define TTB_HSHELL_WINDOWCREATED_MESSAGE       L"TTBase HShell WindowCreated"
#define TTB_HSHELL_WINDOWDESTROYED_MESSAGE     L"TTBase HShell WindowDestroyed"
#define TTB_HMOUSE_ACTION_MESSAGE              L"TTBase HMouse Action"
#define TTB_ICON_NOTIFY_MESSAGE                L"TTBase ICON NOTIFY"
#define TTB_LOAD_DATA_FILE_MESSAGE             L"TTBase LOAD DATA FILE"
#define TTB_SAVE_DATA_FILE_MESSAGE             L"TTBase SAVE DATA FILE"
#else
#define TTB_HSHELL_ACTIVATESHELLWINDOW_MESSAGE "TTBase HShell Activate ShellWindow"
#define TTB_HSHELL_GETMINRECT_MESSAGE          "TTBase HShell GetMinRect"
#define TTB_HSHELL_LANGUAGE_MESSAGE            "TTBase HShell Language"
#define TTB_HSHELL_REDRAW_MESSAGE              "TTBase HShell Redraw"
#define TTB_HSHELL_TASKMAN_MESSAGE             "TTBase HShell TaskMan"
#define TTB_HSHELL_WINDOWACTIVATED_MESSAGE     "TTBase HShell WindowActivated"
#define TTB_HSHELL_WINDOWCREATED_MESSAGE       "TTBase HShell WindowCreated"
#define TTB_HSHELL_WINDOWDESTROYED_MESSAGE     "TTBase HShell WindowDestroyed"
#define TTB_HMOUSE_ACTION_MESSAGE              "TTBase HMouse Action"
#define TTB_ICON_NOTIFY_MESSAGE                "TTBase ICON NOTIFY"
#define TTB_LOAD_DATA_FILE_MESSAGE             "TTBase LOAD DATA FILE"
#define TTB_SAVE_DATA_FILE_MESSAGE             "TTBase SAVE DATA FILE"
#endif

// Message’è‹`
UINT TTB_HSHELL_ACTIVATESHELLWINDOW;
UINT TTB_HSHELL_GETMINRECT;
UINT TTB_HSHELL_LANGUAGE;
UINT TTB_HSHELL_REDRAW;
UINT TTB_HSHELL_TASKMAN;
UINT TTB_HSHELL_WINDOWACTIVATED;
UINT TTB_HSHELL_WINDOWCREATED;
UINT TTB_HSHELL_WINDOWDESTROYED;
UINT TTB_HMOUSE_ACTION;
UINT TTB_ICON_NOTIFY;
UINT TTB_LOAD_DATA_FILE;
UINT TTB_SAVE_DATA_FILE;

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
