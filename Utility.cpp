//---------------------------------------------------------------------------//
//
// Utility.cpp
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <strsafe.h>

#include "Plugin.h"
#include "Utility.h"

//---------------------------------------------------------------------------//
//
// ユーティリティルーチン
//
//---------------------------------------------------------------------------//

// 文字列の格納領域を確保し、文字列をコピーして返す
LPTSTR CopyString(LPCTSTR Src)
{
    const auto len = 1 + ::lstrlen(Src);
    auto Dst = new TCHAR[len];
    if ( Dst != nullptr )
    {
        ::StringCchCopy(Dst, len, Src);
    }

    return Dst;
}

//---------------------------------------------------------------------------//

// 文字列を削除する
void DeleteString(LPCTSTR Str)
{
    if ( Str != nullptr )
    {
        delete Str;
    }
}

//---------------------------------------------------------------------------//

// プラグイン情報構造体をディープコピーして返す
PLUGIN_INFO* CopyPluginInfo(PLUGIN_INFO* Src)
{
    if ( Src == nullptr )
    {
        return nullptr;
    }

    const auto info = new PLUGIN_INFO;
    if ( info == nullptr )
    {
        return nullptr;
    }

    // プラグイン情報のコピー
    *info = *Src;

    info->Name     = CopyString(Src->Name);
    info->Filename = CopyString(Src->Filename);
    info->Commands = (Src->CommandCount < 1) ? nullptr : new PLUGIN_COMMAND_INFO[Src->CommandCount];

    // コマンド情報のコピー
    if ( info->Commands != nullptr && Src->Commands != nullptr )
    {
        for ( size_t i = 0; i < Src->CommandCount; ++i )
        {
            info->Commands[i] = Src->Commands[i];

            info->Commands[i].Name    = CopyString(Src->Commands[i].Name);
            info->Commands[i].Caption = CopyString(Src->Commands[i].Caption);
        }
    }

    return info;
}

//---------------------------------------------------------------------------//

// プラグイン側で作成されたプラグイン情報構造体を破棄する
void FreePluginInfo(PLUGIN_INFO* PluginInfo)
{
    if ( PluginInfo == nullptr )
    {
        return;
    }

    // コマンド情報構造体配列の破棄
    if ( PluginInfo->Commands != nullptr )
    {
        for ( DWORD i = 0; i < PluginInfo->CommandCount; ++i )
        {
            const auto pCI = &PluginInfo->Commands[i];

            DeleteString(pCI->Name);
            DeleteString(pCI->Caption);
        }
        delete[] PluginInfo->Commands;
    }

    DeleteString(PluginInfo->Filename);
    DeleteString(PluginInfo->Name);

    delete PluginInfo;
}

//---------------------------------------------------------------------------//

// バージョン情報を返す
void GetVersion(LPTSTR Filename, DWORD* VersionMS, DWORD* VersionLS)
{
    if ( VersionMS == nullptr || VersionLS == nullptr )
    {
        return;
    }
    else
    {
        *VersionMS = 0;
        *VersionLS = 0;
    }

    // API を取得
    const auto hModule = ::LoadLibrary(TEXT("version.dll"));
    if ( hModule == nullptr )
    {
        return;
    }

    DWORD (APIENTRY* _GetFileVersionInfoSize)(__in LPCTSTR lptstrFilename, __out_opt LPDWORD lpdwHandle);
    BOOL  (APIENTRY* _GetFileVersionInfo)    (__in LPCTSTR lptstrFilename, __in DWORD dwHandle, __in DWORD dwLen, __out_bcount(dwLen) LPVOID lpData);
    BOOL  (APIENTRY* _VerQueryValue)         (const LPVOID pBlock, LPTSTR lpSubBlock, LPVOID * lplpBuffer, PUINT puLen);

    // 関数ポインタの取得
#ifdef _WIN64
    (FARPROC&)_GetFileVersionInfoSize = GetProcAddress(hModule, "GetFileVersionInfoSizeW");
    (FARPROC&)_GetFileVersionInfo     = GetProcAddress(hModule, "GetFileVersionInfoW");
    (FARPROC&)_VerQueryValue          = GetProcAddress(hModule, "VerQueryValueW");
#else
    (FARPROC&)_GetFileVersionInfoSize = GetProcAddress(hModule, "GetFileVersionInfoSizeA");
    (FARPROC&)_GetFileVersionInfo     = GetProcAddress(hModule, "GetFileVersionInfoA");
    (FARPROC&)_VerQueryValue          = GetProcAddress(hModule, "VerQueryValueA");
#endif

    if ( _GetFileVersionInfoSize == nullptr || _GetFileVersionInfo == nullptr || _VerQueryValue == nullptr )
    {
        goto FREE;
    }

    // DLL ファイルに埋め込まれたバージョンリソースのサイズを取得
    DWORD VersionHandle;
    const auto VersionSize = _GetFileVersionInfoSize(Filename, &VersionHandle);
    if ( VersionSize == 0 )
    {
        goto FREE;
    }

    // バージョンリソースを読み込む
    const auto pVersionInfo = (LPVOID)new BYTE[VersionSize];
    if ( pVersionInfo == nullptr )
    {
        goto FREE;
    }
    if ( _GetFileVersionInfo(Filename, VersionHandle, VersionSize, pVersionInfo) )
    {
        VS_FIXEDFILEINFO* FixedFileInfo;
        UINT itemLen;

        // バージョンリソースからファイルバージョンを取得
        if ( _VerQueryValue(pVersionInfo, TEXT("\\"), (void **)&FixedFileInfo, &itemLen) )
        {
            *VersionMS = FixedFileInfo->dwFileVersionMS;
            *VersionLS = FixedFileInfo->dwFileVersionLS;
        }
    }
    delete[] pVersionInfo;

FREE:
    if ( hModule ) ::FreeLibrary(hModule);
}

//---------------------------------------------------------------------------//

#if defined(_MSC_VER) && _MSC_VER <= 1800
#define thread_local static
#endif

// ログを出力する
void WriteLog(ERROR_LEVEL logLevel, LPCTSTR format, ...)
{
    // 本体が TTBPlugin_WriteLog をエクスポートしていない場合は何もしない
    if ( TTBPlugin_WriteLog == nullptr )
    {
        return;
    }

    static const size_t BUF_SIZE = 1024;
    thread_local TCHAR msg[BUF_SIZE];

    va_list al;
    va_start(al, format);
    {
        // Warning: this function has a security problem
        ::wvsprintf(msg, format, al);
        msg[BUF_SIZE - 1] = '\0';
    }
    va_end(al);

    TTBPlugin_WriteLog(g_hPlugin, logLevel, msg);
}

//---------------------------------------------------------------------------//

// ほかのプラグインのコマンドを実行する
BOOL ExecutePluginCommand(LPCTSTR pluginName, INT32 CmdID)
{
    // 本体が TTBPlugin_ExecuteCommand をエクスポートしていない場合は何もしない
    if ( TTBPlugin_ExecuteCommand == nullptr )
    {
        return FALSE;
    }

    return TTBPlugin_ExecuteCommand(pluginName, CmdID);
}

//---------------------------------------------------------------------------//

// Utility.cpp