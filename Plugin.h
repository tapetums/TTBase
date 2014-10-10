//---------------------------------------------------------------------------//
//
//                         TTB Plugin Template(VC++)
//
//                               Plugin.h
//
//---------------------------------------------------------------------------//

#pragma once

//---------------------------------------------------------------------------//
//
// 定数
//
//---------------------------------------------------------------------------//

// プラグインのロードタイプ
enum PLUGINTYPE : WORD
{
   ptAlwaysLoad    = 0x0000, // 常駐型プラグイン
   ptLoadAtUse     = 0x0001, // 一発起動型プラグイン
   ptSpecViolation = 0xFFFF, // TTBaseプラグイン以外のDLL
};

// メニュー表示に関する定数
enum DISPMENU : DWORD
{
    dmNone        = 0,      // 何も出さない
    dmSystemMenu  = 1,      // システムメニュー
    dmToolMenu    = 1 << 1, // ツールメニュー
    dmHotKeyMenu  = 1 << 2, // ホットキー
    dmMenuChecked = 1 << 3, // チェックマーク付き
    dmDisabled    = 1 << 4, // 無効

    dmUnchecked   = 0,      // チェックマークなし
    dmEnabled     = 0,      // 有効
};

// TTBPlugin_SetMenuProperty の ChangeFlag 定数
enum CHANGE_FLAG : DWORD
{
    DISPMENU_MENU    = dmSystemMenu | dmToolMenu,
    DISPMENU_ENABLED = dmDisabled,
    DISPMENU_CHECKED = dmMenuChecked,
};

// ログ出力に関する定数
enum ERROR_LEVEL : DWORD
{
    elNever   = 0, // 出力しない
    elError   = 1, // エラー
    elWarning = 2, // 警告
    elInfo    = 3, // 情報
    elDebug   = 4, // デバッグ
};

//---------------------------------------------------------------------------//
//
// 構造体定義
//
//---------------------------------------------------------------------------//

// 構造体アライメント圧縮
#pragma pack(push,1)

// コマンド情報構造体
typedef struct
{
    LPWSTR   Name;          // コマンドの名前（英名）
    LPWSTR   Caption;       // コマンドの説明（日本語）
    INT32    CommandID;     // コマンド番号
    INT32    Attr;          // アトリビュート（未使用）
    INT32    ResID;         // リソース番号（未使用）
    DISPMENU DispMenu;      // メニュー表示に関する設定
    DWORD    TimerInterval; // コマンド実行タイマー間隔[msec] 0で機能を使わない。
    DWORD    TimerCounter;  // システム内部で使用
} PLUGIN_COMMAND_INFO_W;

typedef struct
{
    LPSTR    Name;          // コマンドの名前（英名）
    LPSTR    Caption;       // コマンドの説明（日本語）
    INT32    CommandID;     // コマンド番号
    INT32    Attr;          // アトリビュート（未使用）
    INT32    ResID;         // リソース番号（未使用）
    DISPMENU DispMenu;      // メニュー表示に関する設定
    DWORD    TimerInterval; // コマンド実行タイマー間隔[msec] 0で機能を使わない。
    DWORD    TimerCounter;  // システム内部で使用
} PLUGIN_COMMAND_INFO_A;

// プラグイン情報構造体
typedef struct
{
    WORD                   NeedVersion;  // プラグインI/F要求バージョン
    LPWSTR                 Name;         // プラグインの説明（日本語）
    LPWSTR                 Filename;     // プラグインのファイル名（相対パス）
    PLUGINTYPE             PluginType;   // プラグインのロードタイプ
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
    PLUGINTYPE             PluginType;   // プラグインのロードタイプ
    DWORD                  VersionMS;    // バージョン
    DWORD                  VersionLS;    // バージョン
    DWORD                  CommandCount; // コマンド個数
    PLUGIN_COMMAND_INFO_A* Commands;     // コマンド
    DWORD                  LoadTime;     // ロードにかかった時間（msec）
} PLUGIN_INFO_A;

#pragma pack(pop)

// UNICODE マクロの定義の有無で使用する構造体を切り分ける
#if defined _UNICODE || defined UNICODE
typedef PLUGIN_COMMAND_INFO_W PLUGIN_COMMAND_INFO;
typedef PLUGIN_INFO_W PLUGIN_INFO;
#else
typedef PLUGIN_COMMAND_INFO_A PLUGIN_COMMAND_INFO;
typedef PLUGIN_INFO_A PLUGIN_INFO;
#endif

//---------------------------------------------------------------------------//
//
// 本体側エクスポート関数
//
//---------------------------------------------------------------------------//

#ifdef __cplusplus
extern "C" {
#endif
extern PLUGIN_INFO*  (WINAPI* TTBPlugin_GetPluginInfo)      (DWORD_PTR hPlugin);
extern void          (WINAPI* TTBPlugin_SetPluginInfo)      (DWORD_PTR hPlugin, PLUGIN_INFO* PLUGIN_INFO);
extern void          (WINAPI* TTBPlugin_FreePluginInfo)     (PLUGIN_INFO* PLUGIN_INFO);
extern void          (WINAPI* TTBPlugin_SetMenuProperty)    (DWORD_PTR hPlugin, INT32 CommandID, CHANGE_FLAG ChangeFlag, DISPMENU Flag);
extern PLUGIN_INFO** (WINAPI* TTBPlugin_GetAllPluginInfo)   (void);
extern void          (WINAPI* TTBPlugin_FreePluginInfoArray)(PLUGIN_INFO** PluginInfoArray);
extern void          (WINAPI* TTBPlugin_SetTaskTrayIcon)    (HICON hIcon, LPCTSTR Tips);
extern void          (WINAPI* TTBPlugin_WriteLog)           (DWORD_PTR hPlugin, ERROR_LEVEL logLevel, LPCTSTR msg);
extern BOOL          (WINAPI* TTBPlugin_ExecuteCommand)     (LPCTSTR PluginFilename, INT32 CmdID);
#ifdef __cplusplus
};
#endif

//---------------------------------------------------------------------------//
//
// プラグイン側エクスポート関数
//
//---------------------------------------------------------------------------//

#ifdef __cplusplus
extern "C" {
#endif
// 必須
PLUGIN_INFO* WINAPI TTBEvent_InitPluginInfo(LPTSTR PluginFilename);
void         WINAPI TTBEvent_FreePluginInfo(PLUGIN_INFO* PLUGIN_INFO);
// 任意
BOOL         WINAPI TTBEvent_Init          (LPTSTR PluginFilename, DWORD_PTR hPlugin);
void         WINAPI TTBEvent_Unload        (void);
BOOL         WINAPI TTBEvent_Execute       (INT32 CommandID, HWND hWnd);
void         WINAPI TTBEvent_WindowsHook   (UINT Msg, WPARAM wParam, LPARAM lParam);
#ifdef __cplusplus
};
#endif

//---------------------------------------------------------------------------//
//
// グローバル変数
//
//---------------------------------------------------------------------------//

// プラグインの名前
extern LPCTSTR PLUGIN_NAME;

// コマンドの数
extern DWORD COMMAND_COUNT;

// コマンドの情報
extern PLUGIN_COMMAND_INFO g_cmd_info[];

// プラグインの情報
extern PLUGIN_INFO g_info;

// 本体がプラグインを識別するためのコード
extern DWORD_PTR g_hPlugin;

//---------------------------------------------------------------------------//
//
// 関数定義 (Main.cpp)
//
//---------------------------------------------------------------------------//

BOOL Init   (void);
void Unload (void);
BOOL Execute(INT32 CmdId, HWND hWnd);
void Hook   (UINT Msg, WPARAM wParam, LPARAM lParam);

//---------------------------------------------------------------------------//

// Plugi.h