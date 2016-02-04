#pragma once

//---------------------------------------------------------------------------//
//
// Utility.hpp
//  TTB Plugin Template (C++11)
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "Plugin.hpp"

//---------------------------------------------------------------------------//

LPTSTR       CopyString          (LPCTSTR Src);
void         DeleteString        (LPCTSTR Str);
PLUGIN_INFO* CopyPluginInfo      (PLUGIN_INFO* Src);
void         FreePluginInfo      (PLUGIN_INFO* PLUGIN_INFO);
void         GetVersion          (LPTSTR Filename, DWORD* VersionMS, DWORD* VersionLS);
void         WriteLog            (ERROR_LEVEL logLevel, LPCTSTR format, ...);
BOOL         ExecutePluginCommand(LPCTSTR pluginName, INT32 CmdID);

//---------------------------------------------------------------------------//

// Utility.hpp