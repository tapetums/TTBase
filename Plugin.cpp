//---------------------------------------------------------------------------//
//
//              TTBaseプラグインテンプレート(Plugin.cpp)
//
//              このファイルはできるだけ変更しない。
//              Main.cppに処理を書くことをお勧めします。
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <strsafe.h>

#include "Plugin.h"
#include "MessageDef.h"

//---------------------------------------------------------------------------//
//
// グローバル変数
//
//---------------------------------------------------------------------------//

// プラグインのファイル名。本体からの相対パス
LPTSTR PLUGIN_FILENAME = nullptr;

// プラグインを本体で認識するための識別コード
DWORD_PTR PLUGIN_HANDLE = 0;

// 本体側エクスポート関数への関数ポインタ
#ifdef __cplusplus
extern "C" {
#endif
PLUGIN_INFO*  (WINAPI* TTBPlugin_GetPluginInfo)      (DWORD_PTR hPlugin);
void          (WINAPI* TTBPlugin_SetPluginInfo)      (DWORD_PTR hPlugin, PLUGIN_INFO* PluginInfo);
void          (WINAPI* TTBPlugin_FreePluginInfo)     (PLUGIN_INFO* PluginInfo);
void          (WINAPI* TTBPlugin_SetMenuProperty)    (DWORD_PTR hPlugin, INT32 CommandID, CHANGE_FLAG ChangeFlag, DISPMENU Flag);
PLUGIN_INFO** (WINAPI* TTBPlugin_GetAllPluginInfo)   (void);
void          (WINAPI* TTBPlugin_FreePluginInfoArray)(PLUGIN_INFO** PluginInfoArray);
void          (WINAPI* TTBPlugin_SetTaskTrayIcon)    (HICON hIcon, LPCTSTR Tips);
void          (WINAPI* TTBPlugin_WriteLog)           (DWORD_PTR hPlugin, ERROR_LEVEL logLevel, LPCTSTR msg);
BOOL          (WINAPI* TTBPlugin_ExecuteCommand)     (LPCTSTR PluginFilename, INT32 CmdID);
#ifdef __cplusplus
};
#endif

//---------------------------------------------------------------------------//
//
// ユーティリティルーチン
//
//---------------------------------------------------------------------------//

// 文字列の格納領域を確保し、文字列をコピーして返す
static LPTSTR CopyString(LPCTSTR Src)
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
static void DeleteString(LPCTSTR Str)
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

    const auto pPinfoResult = new PLUGIN_INFO;
    if ( pPinfoResult == nullptr )
    {
        return nullptr;
    }
    *pPinfoResult = *Src;

    pPinfoResult->Name     = CopyString(Src->Name);
    pPinfoResult->Filename = CopyString(Src->Filename);
    pPinfoResult->Commands = (Src->CommandCount < 1) ? nullptr : new PLUGIN_COMMAND_INFO[Src->CommandCount];

    if ( pPinfoResult->Commands != nullptr && Src->Commands != nullptr )
    {
        for ( size_t i = 0; i < Src->CommandCount; ++i )
        {
            pPinfoResult->Commands[i] = Src->Commands[i];

            pPinfoResult->Commands[i].Name    = CopyString(Src->Commands[i].Name);
            pPinfoResult->Commands[i].Caption = CopyString(Src->Commands[i].Caption);
        }
    }

    return pPinfoResult;
}

//---------------------------------------------------------------------------//

// プラグイン側で作成されたプラグイン情報構造体を破棄する
void FreePluginInfo(PLUGIN_INFO* PluginInfo)
{
    TTBEvent_FreePluginInfo(PluginInfo);
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
    _GetFileVersionInfoSize = (decltype(_GetFileVersionInfoSize))GetProcAddress(hModule, "GetFileVersionInfoSizeW");
    _GetFileVersionInfo     = (decltype(_GetFileVersionInfo))    GetProcAddress(hModule, "GetFileVersionInfoW");
    _VerQueryValue          = (decltype(_VerQueryValue))         GetProcAddress(hModule, "VerQueryValueW");
#else
    _GetFileVersionInfoSize = (decltype(_GetFileVersionInfoSize))GetProcAddress(hModule, "GetFileVersionInfoSizeA");
    _GetFileVersionInfo     = (decltype(_GetFileVersionInfo))    GetProcAddress(hModule, "GetFileVersionInfoA");
    _VerQueryValue          = (decltype(_VerQueryValue))         GetProcAddress(hModule, "VerQueryValueA");
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

// ログを出力する
void WriteLog(ERROR_LEVEL logLevel, LPCTSTR format, ...)
{
    // 本体が TTBPlugin_WriteLog をエクスポートしていない場合は何もしない
    if ( TTBPlugin_WriteLog == nullptr )
    {
        return;
    }

    static const size_t BUF_SIZE = 1024;
    static TCHAR msg[BUF_SIZE];

    va_list al;
    va_start(al, format);
    {
        // Warning: this function has a security problem
        ::wvsprintf(msg, format, al);
        msg[BUF_SIZE - 1] = '\0';
    }
    va_end(al);

    TTBPlugin_WriteLog(PLUGIN_HANDLE, logLevel, msg);
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
//
// プラグイン イベント
//
//---------------------------------------------------------------------------//

// プラグイン情報構造体のセット
PLUGIN_INFO* WINAPI TTBEvent_InitPluginInfo(LPTSTR PluginFilename)
{
    // プラグイン情報構造体の生成
    const auto result = new PLUGIN_INFO;
    if ( result == nullptr )
    {
        return nullptr;
    }

    // ファイル名（相対パス）をコピー
    PLUGIN_FILENAME = CopyString(PluginFilename);

    // 構造体に情報をセット
    result->NeedVersion  = NEED_VERSION;
    result->Name         = CopyString(PLUGIN_NAME);
    result->Filename     = CopyString(PLUGIN_FILENAME);
    result->PluginType   = PLUGIN_TYPE;
    GetVersion(PLUGIN_FILENAME, &result->VersionMS, &result->VersionLS);
    result->CommandCount = COMMAND_COUNT;
    result->Commands     = (COMMAND_COUNT < 1) ? nullptr : new PLUGIN_COMMAND_INFO[COMMAND_COUNT];

    // コマンド情報構造体配列の作成
    if ( result->Commands != nullptr )
    {
        for ( DWORD i = 0; i < COMMAND_COUNT; ++i )
        {
            // コマンド情報構造体に情報をセット
            const auto pCI = &result->Commands[i];

            *pCI = COMMAND_INFO[i];

            pCI->Name    = CopyString(COMMAND_INFO[i].Name);
            pCI->Caption = CopyString(COMMAND_INFO[i].Caption);
        }
    }

    return result;
}

//---------------------------------------------------------------------------//

// プラグイン情報構造体の破棄
void WINAPI TTBEvent_FreePluginInfo(PLUGIN_INFO* PluginInfo)
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

// プラグイン初期化
BOOL WINAPI TTBEvent_Init(LPTSTR PluginFilename, DWORD_PTR hPlugin)
{
    RegisterMessages();

    // キャッシュのために、TTBPlugin_InitPluginInfoは呼ばれない場合がある
    // そのため、InitでもPLUGIN_FILENAMEの初期化を行う
    DeleteString(PLUGIN_FILENAME);
    PLUGIN_FILENAME = CopyString(PluginFilename);

    // 本体から、プラグインを認識するための識別コードを受け取る
    PLUGIN_HANDLE = hPlugin;

    // API関数の取得
    const auto hModule = ::GetModuleHandle(nullptr);
    (FARPROC&)TTBPlugin_GetAllPluginInfo    = GetProcAddress(hModule, "TTBPlugin_GetAllPluginInfo");
    (FARPROC&)TTBPlugin_FreePluginInfoArray = GetProcAddress(hModule, "TTBPlugin_FreePluginInfoArray");
    (FARPROC&)TTBPlugin_GetPluginInfo       = GetProcAddress(hModule, "TTBPlugin_GetPluginInfo");
    (FARPROC&)TTBPlugin_SetPluginInfo       = GetProcAddress(hModule, "TTBPlugin_SetPluginInfo");
    (FARPROC&)TTBPlugin_FreePluginInfo      = GetProcAddress(hModule, "TTBPlugin_FreePluginInfo");
    (FARPROC&)TTBPlugin_SetMenuProperty     = GetProcAddress(hModule, "TTBPlugin_SetMenuProperty");
    (FARPROC&)TTBPlugin_SetTaskTrayIcon     = GetProcAddress(hModule, "TTBPlugin_SetTaskTrayIcon");
    (FARPROC&)TTBPlugin_WriteLog            = GetProcAddress(hModule, "TTBPlugin_WriteLog");
    (FARPROC&)TTBPlugin_ExecuteCommand      = GetProcAddress(hModule, "TTBPlugin_ExecuteCommand");

    return Init();
}

//---------------------------------------------------------------------------//

// プラグインアンロード時の処理
void WINAPI TTBEvent_Unload(void)
{
    Unload();

    DeleteString(PLUGIN_FILENAME);
    PLUGIN_FILENAME = nullptr;
}

//---------------------------------------------------------------------------//

// コマンド実行
BOOL WINAPI TTBEvent_Execute(INT32 CommandID, HWND hWnd)
{
    return Execute(CommandID, hWnd);
}

//---------------------------------------------------------------------------//

// フック（ShellHook,MouseHook)
void WINAPI TTBEvent_WindowsHook(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    Hook(Msg, wParam, lParam);
}

//---------------------------------------------------------------------------//

// Plugin.cpp