#TTBase プラグイン仕様書
---

  便利だからと常駐ソフトを沢山起動していると、リソース不足やPCの起動時間が大幅にかかったりと、いいことがありません。  
 　そこで、考案されたのがTTBaseです。 プラグインインターフェイスを持っていて、使うプラグインを制限することにより、自分の欲しい機能だけを備えた自分だけのオールインワンソフトができるのです。

 作者K2のサイト  
 [http://web.kyoto-inet.or.jp/people/koyabu/](http://web.kyoto-inet.or.jp/people/koyabu/)

 TTBase本体・プラグイン置き場  
 [http://web.kyoto-inet.or.jp/people/koyabu/TTBase.html](http://web.kyoto-inet.or.jp/people/koyabu/TTBase.html)

 ソフトウェア板スレ  
 【常駐】TTBaseってどうよ【プラグイン】  
 [http://pc3.2ch.net/test/read.cgi/software/1041375122/](http://pc3.2ch.net/test/read.cgi/software/1041375122/)

---

 以下にプラグイン仕様書の全文を載せます。

----

 ##はじめに
  　TTBase は、起動するとタスクトレイに常駐するだけのプログラムです。  
  このプログラムを使えるようにするためには、TTBase に読み込まれるプラグ イン（DLL）が不可欠です。  
  　この仕様書をよく読んで、プラグインを開発してください。

 ##コンセプト
  　ちょっとした常駐ツールを作りたいとき、通常なら一本アプリを書きますが、そういうツールが多数に渡ると、起動しているプロセスが多い、リソース消費量が多くなる、OS の起動が遅くなるなどのあんまりうれしくない現象が増えてきます。  
 　そこで、一本だけ常駐アプリを作って、そいつが小物 DLL を呼び出したり、自分のプロセスの中で常駐させたりするようにすれば、環境にやさしい常駐アプリが作れるのではないかというアイデアが TTBase の原点になっています。  
  　プラグイン SDK を使えば、小物ツールならあっという間に書くことができますし、タスクトレイに常駐させる作業や、ホットキーやメニューを与える作業を簡略化できますので、自分で小物ツールを書くのにも適しています。  
  　グローバルフックも数種類標準で、プラグインに対して提供しています。面倒な別 DLL や共有メモリ等を考慮しなくても、WH_SHELL のほぼ全機能とWH_MOUSE の一部機能を使用することができます。

---

 ##開発環境
  　プラグインインターフェイスは、作成されたダイナミックリンクライブラリ（DLL）に対しての動的ロードを使いますので、処理系を選びません。DLL を作成できる処理系なら、どんなものでも使用できるはずです。  
  　現在、Delphi(5) 用と VC++(6.0),Borland C++ Free(5.5), C++Builder(5) のテンプレートが付属されています。

---

 ##プラグインの種類
  ###常駐型と一発起動型  
   　プラグイン情報構造体の PluginType に、ptAlwaysLoad を設定すると、常駐型になります。TTBase が起動している間は常にプラグインがプロセスにロードされます。マウスをフックしたりする場合は、こちらを使ってください。  
   　ptLoadAtUse を設定すると、一発起動型になります。ユーザーがコマンドを呼ぶか、タイマーによって起動された場合、その都度プラグインは ロードされ、TTBPlugin_Init が呼ばれ、その後 TTBPlugin_Execute が呼ばれ、最後に TTBPlugin_Unload が呼ばれてプロセスからアンロードされます。  
   　一発起動型の方が、起動時だけメモリにロードされますから、メモリの節約になります。その代わり、コマンド実行が若干遅れます。大きなデータファイルを読まなければならないようなプラグインの場合は、常駐型にするべきでしょう。  
   　TTBPlugin_WindowsHook を使用する場合は、必ず常駐型にする必要があります。  
   常駐型を作る場合は、できるだけ WindowsAPI だけを使ったコーディングを行い、ファイルサイズを小さくまとめてください。小さい常駐型と大きい一発起動型の２つのプラグインが連携するように作るのも一つの方法です。

  ###コマンドの呼ばれ方の種類  
   　TTBaseのコマンドの呼び方には、以下の４つがあります。  

    - ツールメニュー（タスクトレイアイコン左クリックまたはホットキー）  
    - システムメニュー（タスクトレイアイコン右クリックまたはホットキー）  
    - ホットキー  
    - タイマー  
    - WindowsHook（現状WH_SHELLとWH_MOUSEの一部機能を提供）  

   　ツールメニュー・システムメニューに出すかどうかは、プラグインコマンド情報構造体の DispMenu に適切な値を設定することで決定されます。  
   またホットキーの設定メニューの中にコマンドを出すかどうかも、DispMenuの設定で左右されます。  
   　タイマーに関しては、PluginCommandInfo の IntervalTime に呼ばれる時間間隔を設定することで機能するようになります。使わない場合は 0 を設定します。  
   　WindowsHook は、面倒な Hook.dll を作成しなくても、TTBase が Hook用の手続きを踏んで、Plugin 関数を呼んでくれます。現在のところ、WH_SHELL の全機能と、WH_MOUSE の一部機能が使用できます。

  ###プラグインが本体に対してどうやって情報を伝えているのか  
   　TTBase は、起動時に実行フォルダ以下の DLL ファイルを検索し、いったんロードします。その後、DLL の TTBPlugin_InitPluginInfo イベントを呼んで、プラグイン情報構造体（TPluginInfo or PLUGIN_INFO）をプラグインから取得します。これによって、そのプラグインの名前・種類・コマンド情報をプラグインから得ます。その後、常駐型でない場合は、アンロードします。  
   　常駐型の場合は、Plugin_Init が呼ばれ、そのままプロセスにロードされ続けます。  
   　なお、TTBase の起動を速くするため、インストール後、２回目の起動以降は、TTBase.dat に保存されたプラグイン情報キャッシュを使ってプラグイン情報を得るようになります。DLL のファイルタイムが更新された場合以外は、そのままその情報が使用され続けます。 

  ###定数と構造体定義  
   　構造体のアライメントは、圧縮してください。
```c
// プラグインのロードタイプ
#define ptAlwaysLoad    0
#define ptLoadAtUse     1
#define ptSpecViolation 0xFFFF

// メニュー表示に関する定数
#define dmNone        0 // 何も出さない
#define dmSystemMenu  1 // システムメニュー
#define dmToolMenu    2 // ツールメニュー
#define dmHotKeyMenu  4 // ホットキー
#define dmMenuChecked 8 // メニューのチェックマーク

//--------------------------------------------------------//
// 構造体定義
//--------------------------------------------------------//

// コマンド情報構造体
typedef struct
{
    char* Name;          // コマンドの名前（英名）
    char* Caption;       // コマンドの説明（日本語）
    int   CommandID;     // コマンド番号
    int   Attr;          // アトリビュート（未使用）
    int   ResID;         // リソース番号（未使用）
    int   DispMenu;      // メニューに出すかどうか。 SysMenu:1 ToolMenu:2 None: 0
    DWORD TimerInterval; // タイマー実行間隔[msec]　0: 使用しない
    DWORD TimerCounter;  // システム内部で使用
} PLUGIN_COMMAND_INFO;

// プラグイン情報構造体
typedef struct
{
    WORD   NeedVersion;            // プラグインI/F要求バージョン
    char*  Name;                   // プラグインの説明（日本語）
    char*  Filename;               // プラグインのファイル名（相対パス）
    WORD   PluginType;             // プラグインのロードタイプ
    DWORD  VersionMS;              // バージョン
    DWORD  VersionLS;              // バージョン
    DWORD  CommandCount;           // コマンド個数
    PLUGIN_COMMAND_INFO* Commands; // コマンド
    // 以下システムで、TTBase本体で使用する
    DWORD  LoadTime;               // ロードにかかった時間（msec）
} PLUGIN_INFO;
```
```Delphi
// プラグインのロードタイプ
// TPluginInfo.PluginType
ptAlwaysLoad    = 0;
ptLoadAtUse     = 1;
ptSpecViolation = $FFFF;

// メニュー表示に関する定数
dmNone        =  0; // 何も出さない
dmToolMenu    =  2; // ツールメニュー
dmSystemMenu  =  1; // システムメニュー
dmHotKeyMenu  =  4; // ホットキー
dmUnChecked   =  0; //
dmChecked     =  8; // メニューがチェックされている
dmEnabled     =  0; //
dmDisabled    = 16; // メニューがグレイアウトされている
DISPMENU_MENU = dmToolMenu or dmSystemMenu;
DISPMENU_ENABLED = dmDisabled;
DISPMENU_CHECKED = dmChecked;

// --------------------------------------------------------
// 構造体定義
// --------------------------------------------------------
// コマンド情報構造体
PPluginCommandInfo = ^TPluginCommandInfo;
TPluginCommandInfo = packed record
    Name:          PChar;   // コマンドの名前（英名）
    Caption:       PChar;   // コマンドの説明（日本語）
    CommandID:     integer; // コマンド番号
    Attr:          integer; // アトリビュート（未使用）
    ResID:         integer; // リソース番号（未使用）
    DispMenu:      integer; // メニューに出すかどうか
    TimerInterval: DWORD;   // コマンド実行タイマー間隔[msec] 0で機能を使わない
    TimerCounter:  DWORD;   // システム内部で使用
end;
PPluginCommandInfoArray = ^TPluginCommandInfoArray;
TPluginCommandInfoArray = array[0..65535] of TPluginCommandInfo;

// プラグイン情報構造体
PPluginInfo = ^TPluginInfo;
TPluginInfo = packed record
    NeedVersion:  WORD;                    // プラグインI/F要求バージョン
    Name:         PChar;                   // プラグインの説明（日本語）
    Filename:     PChar;                   // プラグインのファイル名（相対パス）
    PluginType:   WORD;                    // プラグインのロードタイプ
    VersionMS:    DWORD;                   // バージョン
    VersionLS:    DWORD;                   // バージョン
    CommandCount: DWORD;                   // コマンド個数
    Commands:     PPluginCommandInfoArray; // コマンド
    // 以下システムで、TTBase本体で使用する
    LoadTime: DWORD;                       // ロードにかかった時間（msec）
end;
PPluginInfoArray = ^TPluginInfoArray;
TPluginInfoArray = array[0..65535] of PPluginInfo;
```

---
#[解説]

==============================================================

  使用する構造体は２つあります。  

---

  ##PLUGIN_INFO（TPluginInfo）  
   　TTBase にプラグインのプロパティを教えるために、Plugin_SetPluginInfo関数を使って、この構造体を渡します。  
   一緒にコマンド情報（PLUGIN_COMMAND_INFO）も渡します。

   ###WORD NeedVersion (WORD)
   　必要とする TTBase プラグイン仕様のバージョンです。  
   現時点では0 を指定してください。

   ###char* Name (PChar)  
   　プラグインの名前です。２バイト文字が使えます。

   ###char* Filename (PChar)  
   　プラグインのファイル名を TTBase インストールフォルダからの相対パスで格納します。

   ###WORD PluginType  
   　常駐型か、一発起動型かを指定します。  

    - ptAlwaysLoad:    常駐型  
    - ptLoadAtUse:     一発起動型  
    - ptSpecViolation: 内部使用。この値はセットしてはいけません。  

   　コマンド駆動の機能しか持たないプラグインは、できるだけ ptLoadAtUse を指定してください。  
   これによって、TTBase の使用メモリ量を抑制することができます。

   ###DWORD VersionMS, VersionLS (DWORD)  
   　プラグインのバージョンを格納します。  

    - HIWORD(VersionMS): Major Version  
    - LOWORD(VersionMS): Minor Version  
    - HIWORD(VersionLS): Release Number  
    - LOWORD(VersionLS): Build Number  

   　これを推奨しますが、意味付けは強制はしません。

   ###DWORD CommandCount (DWORD)  
   　プラグインが持つコマンドの数です。  
   コマンドの数は合計で256個以下である必要があります。

---

  ##PLUGIN_COMMAND_INFO** Commands (Commands PPluginCommandInfoArray)  
   プラグインコマンド情報構造体へのポインタの配列へのポインタです。ここに、必要なメモリを確保してポインタを指定してください。 

   ###DWORD LoadTime (DWORD)  
   　このメンバーは、TTBase 内部で使用されるだけで、個別プラグイン には関係ありません。 そのプラグインの情報取得にかかった時間が msec で格納されます。  
   　値は、QueryPerformanceTimer を使用して取得しますので、分解能は msec 以下です。

   ###PLUGIN_COMMAND_INFO(TPluginCommandInfo)  
   　PLUGIN_INFO の Commands メンバに設定する構造体です。
   コマンドの情報を格納して TTBase に渡します。

   ###char* Name (PChar)  
   　コマンドの名前です。半角英数と _ を使用して記述してください。

   ###char* Caption (PChar)  
   　コマンドの説明です。メニューなどに表示されます。２バイト文字も使用できます。

   ###int CommandID (Integer)  
   　コマンド番号。コマンド一つに付き一つ、ユニークな値を定義します。

   ###int Attr (Integer)  
   　コマンドアトリビュート。現在未使用です。

   ###int ResID(Integer)  
   　リソース ID。現在未使用です。

   ###int DispMenu (Integer)  
   　TTBase のツール・システムメニューに、このコマンドを表示するかどうかを指定します。システムメニューには設定系のメニューを、ツールメニューには、そのプラグインの基本機能を割り当てるのが原則です。  
   　また、ホットキー設定ができるかどうかもここに設定します。２つ以上の設定を行うときは、和を使ってください。  

    - dmNone:        表示しない
    - dmSystemMenu:  システムメニューに表示
    - dmTooMenu:     ツールメニューに表示
    - dmHotKeyMenu:  ホットキー設定の選択可能コマンドに表示
    - dmMenuChecked: メニューにチェックマークが入るかどうか
  
   ###DWORD TimerInterval (DWORD)  
   　タイマーによる連続起動コマンド指定です。  
   時間を [msec] で設定します。  
   タイマー機能を使用しないときは 0 を設定します。  
   　TTBase 内部では、約 100msec ごとに Timer イベントが発生しています。この分解能でコマンドを実行しますので、あまり細かい時間を設定しても意味がありません。100msec 単位程度で指定しましょう。

   ###DWORD TimerCounter (DWORD)  
   　TTBase 内部で使用します。  

---

#イベントハンドラ
　　絶対に定義しなければならないイベントハンドラは、以下のものです。DLL で関数の名前をエクスポートしてください。  
　エクスポートされていない場合は、その DLL をプラグインとは認識しません。

================================================================
TTBEvent_InitPluginInfo
================================================================
```c
PLUGIN_INFO* WINAPI TTBEvent_InitPluginInfo(char* PluginFilename);
```
```Delphi
function TTBEvent_InitPluginInfo(PluginFilename: PChar): PPluginInfo; stdcall; export;
```
　プラグイン情報構造体のメモリを確保し、情報をセットして返します。コマンドを持つ場合は、コマンド情報構造体のメモリも確保し、渡します。  
　PluginFilename には、そのプラグイン実行ファイル名が、の TTBase インストールフォルダからの相対パスとして格納されています。これは、プラグインの内部でも使用できますが、プラグイン情報構造体の Filename メンバにメモリを確保して、コピーして TTBase本体に返してください。 

================================================================
TTBEvent_FreePluginInfo
================================================================
```c
void WINAPI TTBEvent_FreePluginInfo(PLUGIN_INFO* PluginInfo);
```
```Delphi
procedure TTBEvent_FreePluginInfo(PluginInfo: PPluginInfo); stdcall; export;
```
　渡されたプラグイン情報構造体のメモリを解放します。  
　コマンド個数を見て、コマンド情報構造体のメモリも正確に解放するようにしてください。 このコマンドで解放するのは、TTBEvent_InifPluginInfo 等プラグイン側で確保されたメモリ領域です。


***

　以下、必要に応じて定義するイベントハンドラです。

================================================================
TTBEvent_Init
================================================================
```c
BOOL WINAPI TTBEvent_Init(char* PluginFilename, DWORD hPlugin);
```
```Delphi
function TTBEvent_Init(PluginFilename: PChar; hPlugin: DWORD): BOOL; stdcall; export;
```
　プラグインの初期化関数です。ロードされた後、最初に呼ばれます。  
　TTBase のプラグイン情報キャッシュ機構のため、TTBEvent_InitPluginInfoが毎回呼ばれるわけではありません。そこで、PluginFilename を使って、DLLは自分のファイル名を知ることができます。  
　hPlugin は、TTBase がプラグインを識別するための識別コードです。一部のAPI 関数で使用するので、グローバル変数等に保存するようにしてください。  
　初期化が成功したら、TRUE を返します。

================================================================
TTBEvent_Unload
================================================================
```c
void WINAPI TTBEvent_Unload(void);
```
```Delphi
procedure TTBEvent_Unload; stdcall; export;
```
　プラグインがアンロードされるときに呼ばれます。

================================================================
TTBEvent_Execute
================================================================
```c
BOOL WINAPI TTBEvent_Execute(int CommandID, HWND hWnd);
```
```Delphi
function TTBEvent_Execute(CommandID: Integer; hWnd: THandle): BOOL; stdcall; export;
```
　コマンドが、何らかの形で呼ばれる場合、この関数が呼ばれます。  
　プラグイン情報構造体で、CommandCount を 1 以上に設定した場合は、このハンドラは必ず必要です。  
　CommandID には、そのコマンドの ID が入っています。  
　hWnd には、TTBase が持っている NotifyWindow （表示はされませんが、メッセージ処理をするために必要です）のウィンドウハンドルが格納されています。  
　正常に処理を終了したら TRUE を返すようにしてください。  

　この関数が呼ばれるのは、以下の条件です。  

 + TTBaseのツールメニューかシステムメニューが選択された  
 + ユーザーが設定したホットキーからコマンドが呼ばれた  
 + タイマー型のコマンド（PluginInfo の IntervalTime が 1 以上に設定されているコマンド）の場合、設定時間ごとに自動的に呼ばれる。

================================================================
TTBEvent_WindowsHook
================================================================
```c
void WINAPI TTBEvent_WindowsHook(UINT Msg, DWORD wParam, DWORD lParam);
```
```Delphi
procedure TTBEvent_WindowsHook(Msg: Word; wParam: DWORD; lParam: DWORD); stdcall; export;
```
　TTBase本体が、Hook 用の DLL を持っています。これを使って、Hook で得られるイベントを、プラグインでも使うことができます。  
　現在サポートされているのは、ShellHook(WH_SHELL) と、MouseHook(WH_MOUSE)の２つがあります。

[WH_SHELL]  
　コールバック関数で得られる nCode ごとに、TTB_HSHELL_ で始まるユーザー定義メッセージが定義されています。これはテンプレートの MessageDef.pas を参照してください。  
　このメッセージ番号が Msg に設定されます。またコールバック関数で得られる wParam と lParam も得ることができます。

[WH_MOUSE]  
　コールバック関数で得られる nCode が HC_ACTION の場合だけ通知されます。  
　MsgにTTB_HMOUSE_ACTIONが設定されます（MessageDef.pas参照）。  
　wParam にマウスメッセージの種類、lParam にそのマウスイベントが起こったウィンドウのハンドルが設定されます。実際の WH_MOUSE では、lParam にMOUSEHOOKSTRUCT へのポインタが設定されますが、これをすべてプラグインで受け取ることはできません。ウィンドウハンドルのみが提供されています。

---

#API 関数

================================================================
TTBPlugin_GetPluginInfo
================================================================
```c
extern PLUGIN_INFO* (WINAPI *TTBPlugin_GetPluginInfo)(DWORD hPlugin);
```
```Delphi
TTTBPlugin_GetPluginInfo = function (hPlugin: DWORD): PPluginInfo; stdcall;
```
　hPlugin で指定したプラグインのプラグイン情報構造体を取得します。取得時点で、TTBase が管理している情報が引き出せます。メニューのチェック状態などを得るのに使います。  
　この API 関数で取得した (PLUGIN_INFO *) 型のポインタは、TTBPlugin_FreePluginInfo 関数で解放する必要があります。

================================================================
TTBPlugin_SetPluginInfo
================================================================
```c
extern void (WINAPI *TTBPlugin_SetPluginInfo)(DWORD hPlugin, PLUGIN_INFO *PluginInfo);
```
```Delphi
TTTBPlugin_SetPluginInfo = procedure (hPlugin: DWORD; PluginInfo: PPluginInfo); stdcall;
```
　hPlugin で指定したプラグインのプラグイン情報構造体を再設定します。プラグイン側から動的にプラグイン情報構造体の内容を変更したいときに使用します。  
　PluginInfo は、新たにメモリを確保して使用してください。  TTBPlugin_GetPluginInfo で取得したものを使用してはいけません。  
（プラグインテンプレートには、ユーティリティルーチンとして、CopyPluginInfo 関数が用意されているので、これで TTBPlugin_GetPluginInfo関数で得たプラグイン情報構造体をコピーして使用するようにします。もちろん TTBPlugin_GetPluginInfo を使用しないでプラグイン情報構造体を作成しても構いません）  
　この関数が使用されると、TTBase は、渡されたプラグイン構造体をコピーして、内部で使用するようになります。そのため、プラグイン側で確保したPluginInfo は、プラグイン側で明示的に解放するようにしてください。

================================================================
TTBPlugin_FreePluginInfo
================================================================
```c
extern void (WINAPI *TTBPlugin_FreePluginInfo)(PLUGIN_INFO *PluginInfo);
```
```Delphi
TTTBPlugin_FreePluginInfo = procedure (PluginInfo: PPluginInfo); stdcall;
```
　TTBase 側で確保された (PLUGIN_INFO *) 型のメモリを解放させます。  
　TTBPlugin_GetPluginInfo で取得したメモリは、この API 関数を使って解放してください。

================================================================
TTBPlugin_SetMenuProperty
================================================================
```c
extern void (WINAPI *TTBPlugin_SetMenuChecked)(DWORD hPlugin, int CommandID, DWORD ChagneFlag, DWORD Flag);
```
```Delphi
TTTBPlugin_SetMenuChecked = procedure (hPlugin: DWORD; CommandID: Integer; ChangeFlag, Flag: DWORD); stdcall;
```
　hPlugin で指定したプラグインの、CommandID で示されるコマンドの、メニュー関係の属性を変更します。

+ ChangeFlag: 変更する属性の種類を指定します。複数のフラグを和で指定することもできます。
+ DISPMENU_MENU:システムメニュー・ツールメニューの種類変更します。 このフラグをセットした時に dmToolMenu、dmSystemMenu の両方を指定しないと、メニューに表示されません。
+ DISPMENU_ENABLED:メニューをグレーアウトするかどうか指定できます。
+ DISPMENU_CHECKED:メニューにチェックを入れるかどうかを指定できます。

---
Flag:フラグには、以下の値の和として指定します。

+ dmNone       =  0; // 何も出さない
+ dmToolMenu   =  2; // ツールメニュー
+ dmSystemMenu =  1; // システムメニュー
+ dmUnChecked  =  0; //
+ dmChecked    =  8; // メニューがチェックされている
+ dmEnabled    =  0; //
+ dmDisabled   = 16; // メニューがグレイアウトされている

---
　ChangeFlag との組み合わせで、効果を発揮するかどうかが決まります。たとえば、DISPMENU_ENABLED だけを指定している時、dmToolMenu などを Flag にセットしても、効果を発揮しません。

================================================================
TTBPlugin_GetAllPluginInfo
================================================================
```c
extern PLUGIN_INFO** (WINAPI* TTBPlugin_GetAllPluginInfo)(void);
```
```Delphi
TPlugin_GetAllPluginInfo = function : PPluginInfoArray; stdcall;
```
　TTBase に読み込まれているすべてのプラグインのプラグイン情報構造体へのポインタの配列へのポインタを返します。  
最後のプラグイン情報構造体へのポインタの次の配列要素には、NULL が格納されていますので、これで最後を判定してください。

================================================================
TTBPlugin_FreePluginInfoArray
================================================================
```c
extern void (WINAPI* TTBPlugin_FreePluginInfoArray)(PLUGIN_INFO** PluginInfoArray);
```
```Delphi
TPlugin_FreePluginInfoArray = procedure (PluginInfoArray: PPluginInfoArray); stdcall;
```
　TTBPlugin_GetAllPluginInfo で取得した、PLUGIN_INFO_ARRAY (PPluginInfoArray) を解放します。

---

以上全文です。
