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
void          (WINAPI* TTBPlugin_SetMenuProperty)    (DWORD_PTR hPlugin, INT32 CommandID, DWORD ChangeFlag, DWORD Flag);
PLUGIN_INFO** (WINAPI* TTBPlugin_GetAllPluginInfo)   (void);
void          (WINAPI* TTBPlugin_FreePluginInfoArray)(PLUGIN_INFO** PluginInfoArray);
void          (WINAPI* TTBPlugin_SetTaskTrayIcon)    (HICON hIcon, LPCTSTR Tips);
void          (WINAPI* TTBPlugin_WriteLog)           (DWORD_PTR hPlugin, INT32 logLevel, LPCTSTR msg);
BOOL          (WINAPI* TTBPlugin_ExecuteCommand)     (LPCTSTR PluginFilename, INT32 CmdID);
#ifdef __cplusplus
};
#endif

//---------------------------------------------------------------------------//
//
// ユーティリティルーチン
//
//---------------------------------------------------------------------------//

// 文字列の格納領域を確保し、文字列Srcをコピーして返す
LPTSTR MakeStringFrom(LPCTSTR Src)
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

// プラグイン情報構造体のSrcをコピーして返す
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

    pPinfoResult->Name     = MakeStringFrom(Src->Name);
    pPinfoResult->Filename = MakeStringFrom(Src->Filename);

    pPinfoResult->Commands = new PLUGIN_COMMAND_INFO[Src->CommandCount];
    for ( size_t i = 0; i < Src->CommandCount; ++i )
    {
        pPinfoResult->Commands[i] = Src->Commands[i];

        pPinfoResult->Commands[i].Name    = MakeStringFrom(Src->Commands[i].Name);
        pPinfoResult->Commands[i].Caption = MakeStringFrom(Src->Commands[i].Caption);
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

    const auto hModule = ::LoadLibrary(TEXT("version.dll"));
    if ( hModule == nullptr )
    {
        return;
    }

#ifdef _WIN64
    typedef DWORD (APIENTRY* GetFileVersionInfoSizeW)(__in LPCWSTR lptstrFilename, __out_opt LPDWORD lpdwHandle);
    typedef BOOL  (APIENTRY* GetFileVersionInfoW)    (__in LPCWSTR lptstrFilename, __in DWORD dwHandle, __in DWORD dwLen, __out_bcount(dwLen) LPVOID lpData);
    typedef BOOL  (APIENTRY* VerQueryValueW)         (const LPVOID pBlock, LPWSTR lpSubBlock, LPVOID * lplpBuffer, PUINT puLen);
    GetFileVersionInfoSizeW _GetFileVersionInfoSize;
    GetFileVersionInfoW     _GetFileVersionInfo;
    VerQueryValueW          _VerQueryValue;
    (FARPROC&)_GetFileVersionInfoSize = GetProcAddress(hModule, "GetFileVersionInfoSizeW");
    (FARPROC&)_GetFileVersionInfo     = GetProcAddress(hModule, "GetFileVersionInfoW");
    (FARPROC&)_VerQueryValue          = GetProcAddress(hModule, "VerQueryValueW");
    LPWSTR pSubBlock = L"\\";
#else
    typedef DWORD (APIENTRY* GetFileVersionInfoSizeA)(__in LPCSTR lptstrFilename, __out_opt LPDWORD lpdwHandle);
    typedef BOOL  (APIENTRY* GetFileVersionInfoA)    (__in LPCSTR lptstrFilename, __in DWORD dwHandle, __in DWORD dwLen, __out_bcount(dwLen) LPVOID lpData);
    typedef BOOL  (APIENTRY* VerQueryValueA)         (const LPVOID pBlock, LPSTR lpSubBlock, LPVOID * lplpBuffer, PUINT puLen);
    GetFileVersionInfoSizeA _GetFileVersionInfoSize;
    GetFileVersionInfoA     _GetFileVersionInfo;
    VerQueryValueA          _VerQueryValue;
    (FARPROC&)_GetFileVersionInfoSize = GetProcAddress(hModule, "GetFileVersionInfoSizeA");
    (FARPROC&)_GetFileVersionInfo     = GetProcAddress(hModule, "GetFileVersionInfoA");
    (FARPROC&)_VerQueryValue          = GetProcAddress(hModule, "VerQueryValueA");
    LPSTR pSubBlock = "\\";
#endif
    if ( _GetFileVersionInfoSize == nullptr || _GetFileVersionInfo == nullptr || _VerQueryValue == nullptr )
    {
        return;
    }

    // ------- ファイルにバージョン番号を埋め込んでいる場合
    // ------- このルーチンを使えば、そのバージョン番号を渡すことができる
    DWORD VersionHandle;
    const auto VersionSize = _GetFileVersionInfoSize(Filename, &VersionHandle);
    if ( VersionSize == 0 )
    {
        return;
    }

    const auto pVersionInfo = (LPVOID)new BYTE[VersionSize];
    if ( pVersionInfo == nullptr )
    {
        return;
    }
    if ( _GetFileVersionInfo(Filename, VersionHandle, VersionSize, pVersionInfo) )
    {
        VS_FIXEDFILEINFO* FixedFileInfo;
        UINT itemLen;

        if ( _VerQueryValue(pVersionInfo, pSubBlock, (void **)&FixedFileInfo, &itemLen) )
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
void WriteLog(INT32 logLevel, LPCTSTR msg)
{
    // 本体が TTBPlugin_WriteLog をエクスポートしていない場合は何もしない
    if ( TTBPlugin_WriteLog == nullptr )
    {
        return;
    }

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

    // 情報をコピー
    PLUGIN_FILENAME    = MakeStringFrom(PluginFilename);  // ファイル名（相対パス）
    result->Name       = MakeStringFrom(PLUGIN_NAME);     // プラグインの名前
    result->Filename   = MakeStringFrom(PLUGIN_FILENAME); // プラグインファイル名
    result->PluginType = PLUGIN_TYPE;                     // プラグインタイプ

    GetVersion(PLUGIN_FILENAME, &result->VersionMS, &result->VersionLS); // バージョン情報の取得
    result->CommandCount = COMMAND_COUNT; // コマンドの数

    // コマンド情報構造体配列の作成
    if ( COMMAND_COUNT < 1 )
    {
        result->Commands = nullptr;
    }
    else
    {
        result->Commands = new PLUGIN_COMMAND_INFO[COMMAND_COUNT];
        if ( result->Commands != nullptr )
        {
            // コマンド情報構造体の作成
            for ( size_t i = 0; i < COMMAND_COUNT; ++i )
            {
                const auto pCI = &result->Commands[i];
                *pCI = COMMAND_INFO[i];

                // コマンド名
                pCI->Name    = MakeStringFrom(COMMAND_INFO[i].Name);
                pCI->Caption = MakeStringFrom(COMMAND_INFO[i].Caption);
            }
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

    for ( size_t i = 0; i < PluginInfo->CommandCount; ++i )
    {
        const auto pCI = &PluginInfo->Commands[i];
        delete pCI->Name;
        delete pCI->Caption;
    }
    delete[] PluginInfo->Commands;

    delete PluginInfo->Filename;
    delete PluginInfo->Name;
    delete PluginInfo;
}

//---------------------------------------------------------------------------//

// プラグイン初期化
BOOL WINAPI TTBEvent_Init(LPTSTR PluginFilename, DWORD_PTR hPlugin)
{
    RegisterMessages();

    // キャッシュのために、TTBPlugin_InitPluginInfoは呼ばれない場合がある
    // そのため、InitでもPLUGIN_FILENAMEの初期化を行う
    if ( PLUGIN_FILENAME != nullptr )
    {
        delete PLUGIN_FILENAME;
    }
    PLUGIN_FILENAME = MakeStringFrom(PluginFilename);

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

    delete PLUGIN_FILENAME;
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