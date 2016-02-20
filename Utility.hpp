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
PLUGIN_INFO* CopyPluginInfo      (const PLUGIN_INFO* Src);
void         FreePluginInfo      (PLUGIN_INFO* PLUGIN_INFO);
void         GetVersion          (LPTSTR Filename, DWORD* VersionMS, DWORD* VersionLS);
BOOL         ExecutePluginCommand(LPCTSTR pluginName, INT32 CmdID);

//---------------------------------------------------------------------------//

#if NO_WRITELOG
  #define WriteLog(logLevel, format, ...)
#else
  void WriteLog (ERROR_LEVEL logLevel, LPCTSTR format, ...);
#endif

//---------------------------------------------------------------------------//

// Utility.hpp