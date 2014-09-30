// ===========================================================================
//
//                         TTB Plugin Template(VC++)
//
//                               Plugin.h
//
// ===========================================================================

#pragma once

// プラグインのロードタイプ
#define ptAlwaysLoad    0x0000 // 常駐型プラグイン
#define ptLoadAtUse     0x0001 // 一発起動型プラグイン
#define ptSpecViolation 0xFFFF // TTBaseプラグイン以外のDLL

// メニュー表示に関する定数
#define dmNone            0 // 何も出さない
#define dmSystemMenu      1 // システムメニュー
#define dmToolMenu        2 // ツールメニュー
#define dmHotKeyMenu      4 // ホットキー
#define dmChecked         8 // メニューのチェックマーク
#define dmUnchecked       0 // メニューのチェックマークをつけない
#define dmEnabled         0 // メニューをEnableに
#define dmDisabled       16 // メニューをDisableする
#define DISPMENU_MENU    dmToolMenu | dmSystemMenu
#define DISPMENU_ENABLED dmDisabled
#define DISPMENU_CHECKED dmChecked

// ログ出力に関する定数
#define elNever   0 // 出力しない
#define elError   1 // エラー
#define elWarning 2 // 警告
#define elInfo    3 // 情報
#define elDebug   4 // デバッグ

// 構造体アライメント圧縮
#pragma pack(push,1)
// ---------------------------------------------------------//
//      構造体定義
// ---------------------------------------------------------//
// コマンド情報構造体
typedef struct
{
    LPWSTR Name;          // コマンドの名前（英名）
    LPWSTR Caption;       // コマンドの説明（日本語）
    int    CommandID;     // コマンド番号
    int    Attr;          // アトリビュート（未使用）
    int    ResID;         // リソース番号（未使用）
    int    DispMenu;      // システムメニューが1、ツールメニューが2、表示なしは0、ホットキーメニューは4
    DWORD  TimerInterval; // コマンド実行タイマー間隔[msec] 0で機能を使わない。
    DWORD  TimerCounter;  // システム内部で使用
} PLUGIN_COMMAND_INFO_W;

typedef struct
{
    LPSTR Name;          // コマンドの名前（英名）
    LPSTR Caption;       // コマンドの説明（日本語）
    int   CommandID;     // コマンド番号
    int   Attr;          // アトリビュート（未使用）
    int   ResID;         // リソース番号（未使用）
    int   DispMenu;      // システムメニューが1、ツールメニューが2、表示なしは0、ホットキーメニューは4
    DWORD TimerInterval; // コマンド実行タイマー間隔[msec] 0で機能を使わない。
    DWORD TimerCounter;  // システム内部で使用
} PLUGIN_COMMAND_INFO_A;

#ifdef _WIN64
typedef PLUGIN_COMMAND_INFO_W PLUGIN_COMMAND_INFO;
#else
typedef PLUGIN_COMMAND_INFO_A PLUGIN_COMMAND_INFO;
#endif

// プラグイン情報構造体
typedef struct
{
    WORD                   NeedVersion;  // プラグインI/F要求バージョン
    LPWSTR                 Name;         // プラグインの説明（日本語）
    LPWSTR                 Filename;     // プラグインのファイル名（相対パス）
    WORD                   PluginType;   // プラグインのロードタイプ
    DWORD                  VersionMS;    // バージョン
    DWORD                  VersionLS;    // バージョン
    DWORD                  CommandCount; // コマンド個数
    PLUGIN_COMMAND_INFO_W* Commands;     // コマンド
    DWORD                  LoadTime;     // ロードにかかった時間（msec）
} PLUGIN_INFO_W;

typedef struct
{
    WORD                   NeedVersion;  // プラグインI/F要求バージョン
    LPSTR                  Name;         // プラグインの説明（日本語）
    LPSTR                  Filename;     // プラグインのファイル名（相対パス）
    WORD                   PluginType;   // プラグインのロードタイプ
    DWORD                  VersionMS;    // バージョン
    DWORD                  VersionLS;    // バージョン
    DWORD                  CommandCount; // コマンド個数
    PLUGIN_COMMAND_INFO_A* Commands;     // コマンド
    DWORD                  LoadTime;     // ロードにかかった時間（msec）
} PLUGIN_INFO_A;

#ifdef _WIN64
typedef PLUGIN_INFO_W PLUGIN_INFO;
#else
typedef PLUGIN_INFO_A PLUGIN_INFO;
#endif
#pragma pack(pop)

// ---------------------------------------------------------//
//      プラグイン側エクスポート関数
// ---------------------------------------------------------//
#ifdef __cplusplus
extern "C" {
#endif
// 必須
PLUGIN_INFO* WINAPI TTBEvent_InitPluginInfo(LPTSTR PluginFilename);
void         WINAPI TTBEvent_FreePluginInfo(PLUGIN_INFO* PLUGIN_INFO);
// 任意
BOOL         WINAPI TTBEvent_Init          (LPTSTR PluginFilename, DWORD_PTR hPlugin);
void         WINAPI TTBEvent_Unload        (void);
BOOL         WINAPI TTBEvent_Execute       (int CommandID, HWND hWnd);
void         WINAPI TTBEvent_WindowsHook   (UINT Msg, WPARAM wParam, LPARAM lParam);
#ifdef __cplusplus
};
#endif

// ---------------------------------------------------------//
//      本体側エクスポート関数
// ---------------------------------------------------------//
extern PLUGIN_INFO*  (WINAPI* TTBPlugin_GetPluginInfo)      (DWORD_PTR hPlugin);
extern void          (WINAPI* TTBPlugin_SetPluginInfo)      (DWORD_PTR hPlugin, PLUGIN_INFO* PLUGIN_INFO);
extern void          (WINAPI* TTBPlugin_FreePluginInfo)     (PLUGIN_INFO* PLUGIN_INFO);
extern void          (WINAPI* TTBPlugin_SetMenuProperty)    (DWORD_PTR hPlugin, int CommandID, DWORD ChangeFlag, DWORD Flag);
extern PLUGIN_INFO** (WINAPI* TTBPlugin_GetAllPluginInfo)   (void);
extern void          (WINAPI* TTBPlugin_FreePluginInfoArray)(PLUGIN_INFO** PluginInfoArray);
extern void          (WINAPI* TTBPlugin_SetTaskTrayIcon)    (HICON hIcon, LPCTSTR Tips);
extern void          (WINAPI* TTBPlugin_WriteLog)           (DWORD_PTR hPlugin, int logLevel, LPCTSTR msg);
extern BOOL          (WINAPI* TTBPlugin_ExecuteCommand)     (LPCTSTR PluginFilename, int CmdID);

// ---------------------------------------------------------//
//      ユーティリティルーチン
// ---------------------------------------------------------//
LPTSTR       MakeStringFrom      (LPCTSTR Src);
PLUGIN_INFO* CopyPluginInfo      (PLUGIN_INFO* Src);
void         FreePluginInfo      (PLUGIN_INFO* PLUGIN_INFO);
void         GetVersion          (LPTSTR Filename, DWORD* VersionMS, DWORD* VersionLS);
void         WriteLog            (int logLevel, LPCTSTR msg);
BOOL         ExecutePluginCommand(LPTSTR pluginName, int CmdID);

// ---------------------------------------------------------//
//      グローバル変数
// ---------------------------------------------------------//
extern LPTSTR    PLUGIN_FILENAME; // プラグインのファイル名。TTBaseフォルダからの相対パス
extern DWORD_PTR PLUGIN_HANDLE;   // TTBaseがプラグインを識別するためのコード
