#pragma once

//---------------------------------------------------------------------------//
//
// MessageDef.hpp
//  TTB Plugin Template
//
//---------------------------------------------------------------------------//

#include <windows.h>

//---------------------------------------------------------------------------//
// Message定義
//---------------------------------------------------------------------------//

// TTBPlugin_WindowsHookで、Msgパラメータにこれらがやってきます。
// 内容は、MSDNで、WH_SHELLの該当ID, WH_MOUSEのHC_ACTIONを参照してください。
extern UINT TTB_HSHELL_ACTIVATESHELLWINDOW;
extern UINT TTB_HSHELL_GETMINRECT;
extern UINT TTB_HSHELL_LANGUAGE;
extern UINT TTB_HSHELL_REDRAW;
extern UINT TTB_HSHELL_TASKMAN;
extern UINT TTB_HSHELL_WINDOWACTIVATED;
extern UINT TTB_HSHELL_WINDOWCREATED;
extern UINT TTB_HSHELL_WINDOWDESTROYED;
extern UINT TTB_HMOUSE_ACTION;

//---------------------------------------------------------------------------//

// 内部使用。TaskTrayアイコン関係のメッセージです
extern UINT TTB_ICON_NOTIFY;

// TTBase.datをTTBaseにロードさせます
extern UINT TTB_LOAD_DATA_FILE;

// TTBase.datをTTBaseにセーブさせます
extern UINT TTB_SAVE_DATA_FILE;

//---------------------------------------------------------------------------//
// 関数
//---------------------------------------------------------------------------//

// 取得用関数
void RegisterMessages(void);

//---------------------------------------------------------------------------//

// MessageDef.hpp