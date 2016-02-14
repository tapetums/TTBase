#pragma once

//---------------------------------------------------------------------------//
//
// Main.hpp
//  TTB Plugin Template (C++11)
//
//---------------------------------------------------------------------------//

#include <windows.h>

extern HINSTANCE g_hInst;
extern HANDLE    g_hMutex;
extern HWND      g_hwnd;

//---------------------------------------------------------------------------//

BOOL CheckTopMost (HWND hwnd);
BOOL ToggleTopMost(HWND hwnd, BOOL topmost);
BOOL OpenAppFolder(HWND hwnd);
BOOL SetOpaque    (HWND hwnd, BYTE alpha);
BOOL SetPriority  (HWND hwnd, INT32 priority);

//---------------------------------------------------------------------------//

// Main.hpp