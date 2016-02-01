//---------------------------------------------------------------------------//
//
// Utility.cpp
//  TTB Plugin Template (C++11)
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <strsafe.h>

#include "Plugin.hpp"
#include "Utility.hpp"

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
    if ( Src == nullptr ) { return nullptr; }

    const auto info = new PLUGIN_INFO;
    if ( info == nullptr )
    {
        return nullptr;
    }

    // プラグイン情報のコピー
    *info = *Src;

    info->Name     = CopyString(Src->Name);
    info->Filename = CopyString(Src->Filename);
    info->Commands = (Src->CommandCount == 0) ?
        nullptr : new PLUGIN_COMMAND_INFO[Src->CommandCount];

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
    if ( PluginInfo == nullptr ) { return; }

    // コマンド情報構造体配列の破棄
    if ( PluginInfo->Commands != nullptr )
    {
        for ( size_t i = 0; i < PluginInfo->CommandCount; ++i )
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
    if ( VersionMS == nullptr || VersionLS == nullptr ) { return; }

    *VersionMS = 0;
    *VersionLS = 0;

    // API を取得
    const auto hModule = ::LoadLibraryEx
    (
        TEXT("version.dll"), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH
    );
    if ( hModule == nullptr )
    {
        return;
    }

    // 関数ポインタの取得
    DWORD (WINAPI* GetFileVersionInfoSize_)(LPCTSTR lptstrFilename, LPDWORD lpdwHandle);
    BOOL  (WINAPI* GetFileVersionInfo_)    (LPCTSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
    BOOL  (WINAPI* VerQueryValue_)         (const LPVOID pBlock, LPTSTR lpSubBlock, LPVOID * lplpBuffer, PUINT puLen);

  #if defined(_UNICODE) || defined(UNICODE)
    (FARPROC&)GetFileVersionInfoSize_ = GetProcAddress(hModule, "GetFileVersionInfoSizeW");
    (FARPROC&)GetFileVersionInfo_     = GetProcAddress(hModule, "GetFileVersionInfoW");
    (FARPROC&)VerQueryValue_          = GetProcAddress(hModule, "VerQueryValueW");
  #else
    (FARPROC&)GetFileVersionInfoSize_ = GetProcAddress(hModule, "GetFileVersionInfoSizeA");
    (FARPROC&)GetFileVersionInfo_     = GetProcAddress(hModule, "GetFileVersionInfoA");
    (FARPROC&)VerQueryValue_          = GetProcAddress(hModule, "VerQueryValueA");
  #endif

    if ( GetFileVersionInfoSize_ == nullptr || GetFileVersionInfo_ == nullptr || VerQueryValue_ == nullptr )
    {
        ::FreeLibrary(hModule); return;
    }

    // DLL ファイルに埋め込まれたバージョンリソースのサイズを取得
    DWORD VersionHandle;
    const auto VersionSize = GetFileVersionInfoSize_(Filename, &VersionHandle);
    if ( VersionSize == 0 )
    {
        ::FreeLibrary(hModule); return;
    }

    // バージョンリソースを読み込む
    const auto pVersionInfo = new BYTE[VersionSize];
    if ( pVersionInfo == nullptr )
    {
        ::FreeLibrary(hModule); return;
    }
    if ( GetFileVersionInfo_(Filename, VersionHandle, VersionSize, pVersionInfo) )
    {
        VS_FIXEDFILEINFO* FixedFileInfo;
        UINT itemLen;

        // バージョンリソースからファイルバージョンを取得
        if ( VerQueryValue_(pVersionInfo, (LPTSTR)TEXT("\\"), (void **)&FixedFileInfo, &itemLen) )
        {
            *VersionMS = FixedFileInfo->dwFileVersionMS;
            *VersionLS = FixedFileInfo->dwFileVersionLS;
        }
    }
    delete[] pVersionInfo;

    ::FreeLibrary(hModule);
}

//---------------------------------------------------------------------------//

// ログを出力する
void WriteLog(DWORD_PTR hPlugin, ERROR_LEVEL logLevel, LPCTSTR format, ...)
{
    // 本体が TTBPlugin_WriteLog をエクスポートしていない場合は何もしない
    if ( TTBPlugin_WriteLog == nullptr ) { return; }

    static constexpr size_t BUF_SIZE { 1024 + 1 };

    static TCHAR msg[BUF_SIZE];
    // TODO: 排他制御

    // 文字列の結合
    va_list al;
    va_start(al, format);
    {
      #if defined(_NODEFLIB)
        ::wvsprintf(msg, format, al);
        // NOTE: If the length of msg exceeds 1024 byte,
        //       wvsprintf() writes NULL at msg[1024].
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms647550(v=vs.85).aspx
      #else
        ::StringCchVPrintf(msg, BUF_SIZE, format, al);
      #endif
    }
    va_end(al);
    msg[BUF_SIZE - 1] = '\0';

    // ログの出力
    TTBPlugin_WriteLog(hPlugin, logLevel, msg);
}

//---------------------------------------------------------------------------//

// ほかのプラグインのコマンドを実行する
BOOL ExecutePluginCommand(LPCTSTR pluginName, INT32 CmdID)
{
    // 本体が TTBPlugin_ExecuteCommand をエクスポートしていない場合は何もしない
    if ( TTBPlugin_ExecuteCommand == nullptr ) { return FALSE; }

    return TTBPlugin_ExecuteCommand(pluginName, CmdID);
}

//---------------------------------------------------------------------------//

// Utility.cpp