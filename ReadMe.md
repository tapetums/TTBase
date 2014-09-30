#TTBase プラグイン仕様書
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

##はじめに  
  　TTBase は、起動するとタスクトレイに常駐するだけのプログラムです。  
  このプログラムを使えるようにするためには、TTBase に読み込まれるプラグイン (DLL) が不可欠です。  
  　この仕様書をよく読んで、プラグインを開発してください。

##コンセプト  
　ちょっとした常駐ツールを作りたいとき、通常なら一本アプリを書きますが、そういうツールが多数に渡ると、起動しているプロセスが多い、リソース消費量が多くなる、OS の起動が遅くなるなどのあんまりうれしくない現象が増えてきます。  
　そこで、一本だけ常駐アプリを作って、そいつが小物 DLL を呼び出したり、自分のプロセスの中で常駐させたりするようにすれば、環境にやさしい常駐アプリが作れるのではないかというアイデアが TTBase の原点になっています。  
　プラグイン SDK を使えば、小物ツールならあっという間に書くことができますし、タスクトレイに常駐させる作業や、ホットキーやメニューを与える作業を簡略化できますので、自分で小物ツールを書くのにも適しています。  
　グローバルフックも数種類標準で、プラグインに対して提供しています。面倒な別 DLL や共有メモリ等を考慮しなくても、WH\_SHELL のほぼ全機能と WH\_MOUSE の一部機能を使用することができます。

---

##開発環境  
　プラグインインターフェイスは、作成されたダイナミックリンクライブラリ (DLL) に対しての動的ロードを使いますので、処理系を選びません。DLL を作成できる処理系なら、どんなものでも使用できるはずです。  
　現在、~~Delphi(5) 用と VC++(6.0),Borland C++ Free(5.5), C++Builder(5) のテンプレートが付属されています。~~  
VC++(2013) 32/64-bit のテンプレートがあります。

---

##プラグインの種類  
###常駐型と一発起動型  
　プラグイン情報構造体の PluginType に、ptAlwaysLoad を設定すると、常駐型になります。TTBase が起動している間は常にプラグインがプロセスにロードされます。マウスをフックしたりする場合は、こちらを使ってください。  
　ptLoadAtUse を設定すると、一発起動型になります。ユーザーがコマンドを呼ぶか、タイマーによって起動された場合、その都度プラグインは ロードされ、TTBPlugin\_Init が呼ばれ、その後 TTBPlugin\_Execute が呼ばれ、最後に TTBPlugin\_Unload が呼ばれてプロセスからアンロードされます。  
　一発起動型の方が、起動時だけメモリにロードされますから、メモリの節約になります。その代わり、コマンド実行が若干遅れます。大きなデータファイルを読まなければならないようなプラグインの場合は、常駐型にするべきでしょう。  
　TTBPlugin\_WindowsHook を使用する場合は、必ず常駐型にする必要があります。常駐型を作る場合は、できるだけ WindowsAPI だけを使ったコーディングを行い、ファイルサイズを小さくまとめてください。小さい常駐型と大きい一発起動型の２つのプラグインが連携するように作るのも一つの方法です。

###コマンドの呼ばれ方の種類  
　TTBaseのコマンドの呼び方には、以下の４つがあります。  

- ツールメニュー（タスクトレイアイコン左クリックまたはホットキー）
- システムメニュー（タスクトレイアイコン右クリックまたはホットキー）
- ホットキー
- タイマー
- WindowsHook（現状 WH\_SHELL と WH\_MOUSE の一部機能を提供）

　ツールメニュー・システムメニューに出すかどうかは、プラグインコマンド情報構造体の DispMenu に適切な値を設定することで決定されます。  
またホットキーの設定メニューの中にコマンドを出すかどうかも、DispMenu の設定で左右されます。  
　タイマーに関しては、PluginCommandInfo の IntervalTime に呼ばれる時間間隔を設定することで機能するようになります。使わない場合は 0 を設定します。  
　WindowsHook は、面倒な Hook.dll を作成しなくても、TTBase が Hook用 の手続きを踏んで、Plugin関数 を呼んでくれます。現在のところ、WH\_SHELL の全機能と、WH\_MOUSE の一部機能が使用できます。

###プラグインが本体に対してどうやって情報を伝えているのか  
　TTBase は、起動時に実行フォルダ以下の DLL ファイルを検索し、いったんロードします。その後、DLL の TTBPlugin\_InitPluginInfo イベントを呼んで、プラグイン情報構造体 (PLUGIN\_INFO) をプラグインから取得します。これによって、そのプラグインの名前・種類・コマンド情報をプラグインから得ます。その後、常駐型でない場合は、アンロードします。  
　常駐型の場合は、Plugin\_Init が呼ばれ、そのままプロセスにロードされ続けます。  
　なお、TTBase の起動を速くするため、インストール後、２回目の起動以降は、TTBase.dat に保存されたプラグイン情報キャッシュを使ってプラグイン情報を得るようになります。DLL のファイルタイムが更新された場合以外は、そのままその情報が使用され続けます。 

---

#定数と構造体定義  
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
```

---
###[解説]

使用する構造体は２つあります。

##PLUGIN\_INFO 構造体
　TTBase にプラグインのプロパティを教えるために、Plugin\_SetPluginInfo 関数を使って、この構造体を渡します。  
一緒にコマンド情報 (PLUGIN\_COMMAND\_INFO) も渡します。

###WORD NeedVersion
　必要とする TTBase プラグイン仕様のバージョンです。  
現時点では0 を指定してください。

###LPTSTR Name
　プラグインの名前です。任意の文字が使えます。

###LPTSTR Filename
　プラグインのファイル名を TTBase インストールフォルダからの相対パスで格納します。

###WORD PluginType
　常駐型か、一発起動型かを指定します。  

- ptAlwaysLoad:    常駐型
- ptLoadAtUse:     一発起動型
- ptSpecViolation: 内部使用。この値はセットしてはいけません。

　コマンド駆動の機能しか持たないプラグインは、できるだけ ptLoadAtUse を指定してください。  
これによって、TTBase の使用メモリ量を抑制することができます。

###DWORD VersionMS, VersionLS
　プラグインのバージョンを格納します。  

- HIWORD(VersionMS): Major Version
- LOWORD(VersionMS): Minor Version
- HIWORD(VersionLS): Release Number
- LOWORD(VersionLS): Build Number

　これを推奨しますが、意味付けは強制はしません。

###DWORD CommandCount
　プラグインが持つコマンドの数です。  
コマンドの数は合計で 256個以下 である必要があります。

---

##PLUGIN\_COMMAND\_INFO** Commands
　プラグインコマンド情報構造体へのポインタの配列へのポインタです。ここに、必要なメモリを確保してポインタを指定してください。 

###DWORD LoadTime
　このメンバーは、TTBase 内部で使用されるだけで、個別プラグイン には関係ありません。 そのプラグインの情報取得にかかった時間が msec で格納されます。  
　値は、QueryPerformanceTimer を使用して取得しますので、分解能は msec 以下です。

###PLUGIN\_COMMAND\_INFO
　PLUGIN\_INFO の Commands メンバに設定する構造体です。
コマンドの情報を格納して TTBase に渡します。

###LPTSTR Name
　コマンドの名前です。半角英数と \_ を使用して記述してください。

###LPTSTR Caption
　コマンドの説明です。メニューなどに表示されます。

###int CommandID
　コマンド番号です。コマンド一つに付き一つ、ユニークな値を定義します。

###int Attr
　コマンドアトリビュート。現在未使用です。

###int ResID
　リソース ID。現在未使用です。

###int DispMenu
　TTBase のツール・システムメニューに、このコマンドを表示するかどうかを指定します。システムメニューには設定系のメニューを、ツールメニューには、そのプラグインの基本機能を割り当てるのが原則です。  
　また、ホットキー設定ができるかどうかもここに設定します。 2 つ以上の設定を行うときは、論理和を使ってください。  

- dmNone:        表示しない
- dmSystemMenu:  システムメニューに表示
- dmTooMenu:     ツールメニューに表示
- dmHotKeyMenu:  ホットキー設定の選択可能コマンドに表示
- dmMenuChecked: メニューにチェックマークが入るかどうか
  
###DWORD TimerInterval
　タイマーによる連続起動コマンド指定です。  
時間を [msec] で設定します。  
タイマー機能を使用しないときは 0 を設定します。  
　TTBase 内部では、約 100msec ごとに Timer イベントが発生しています。この分解能でコマンドを実行しますので、あまり細かい時間を設定しても意味がありません。100msec 単位程度で指定しましょう。

###DWORD TimerCounter
　TTBase 内部で使用します。  

---

##イベントハンドラ
　絶対に定義しなければならないイベントハンドラは、以下のものです。DLL で関数の名前をエクスポートしてください。  
エクスポートされていない場合は、その DLL をプラグインとは認識しません。

================================================================
###TTBEvent\_InitPluginInfo
================================================================
```c
PLUGIN_INFO* WINAPI TTBEvent_InitPluginInfo(LPTSTR PluginFilename);
```
　プラグイン情報構造体のメモリを確保し、情報をセットして返します。コマンドを持つ場合は、コマンド情報構造体のメモリも確保し、渡します。  
　PluginFilename には、そのプラグイン実行ファイル名が、の TTBase インストールフォルダからの相対パスとして格納されています。これは、プラグインの内部でも使用できますが、プラグイン情報構造体の Filename メンバにメモリを確保して、コピーして TTBase 本体に返してください。 

================================================================
###TTBEvent\_FreePluginInfo
================================================================
```c
void WINAPI TTBEvent_FreePluginInfo(PLUGIN_INFO* PluginInfo);
```
　渡されたプラグイン情報構造体のメモリを解放します。  
　コマンド個数を見て、コマンド情報構造体のメモリも正確に解放するようにしてください。 このコマンドで解放するのは、TTBEvent\_InifPluginInfo 等プラグイン側で確保されたメモリ領域です。


***

　以下、必要に応じて定義するイベントハンドラです。

================================================================
###TTBEvent\_Init
================================================================
```c
BOOL WINAPI TTBEvent_Init(LPTSTR PluginFilename, DWORD_PTR hPlugin);
```
　プラグインの初期化関数です。ロードされた後、最初に呼ばれます。  
　TTBase のプラグイン情報キャッシュ機構のため、TTBEvent\_InitPluginInfo が毎回呼ばれるわけではありません。そこで、PluginFilename を使って、DLLは自分のファイル名を知ることができます。  
　hPlugin は、TTBase がプラグインを識別するための識別コードです。一部の API 関数で使用するので、グローバル変数等に保存するようにしてください。  
　初期化が成功したら、TRUE を返します。

================================================================
###TTBEvent\_Unload
================================================================
```c
void WINAPI TTBEvent_Unload(void);
```
　プラグインがアンロードされるときに呼ばれます。

================================================================
###TTBEvent\_Execute
================================================================
```c
BOOL WINAPI TTBEvent_Execute(int CommandID, HWND hWnd);
```
　コマンドが、何らかの形で呼ばれる場合、この関数が呼ばれます。  
　プラグイン情報構造体で、CommandCount を 1 以上に設定した場合は、このハンドラは必ず必要です。  
CommandID には、そのコマンドの ID が入っています。  
　hWnd には、TTBase が持っている NotifyWindow（表示はされませんが、メッセージ処理をするために必要です）のウィンドウハンドルが格納されています。  
　正常に処理を終了したら TRUE を返すようにしてください。  

　この関数が呼ばれるのは、以下の条件です。  

 + TTBaseのツールメニューかシステムメニューが選択された  
 + ユーザーが設定したホットキーからコマンドが呼ばれた  
 + タイマー型のコマンド（PluginInfo の IntervalTime が 1 以上に設定されているコマンド）の場合、設定時間ごとに自動的に呼ばれる。

================================================================
###TTBEvent\_WindowsHook
================================================================
```c
void WINAPI TTBEvent_WindowsHook(UINT Msg, WPARAM wParam, LPARAM lParam);
```
　TTBase本体が、Hook 用の DLL を持っています。これを使って、Hook で得られるイベントを、プラグインでも使うことができます。  
　現在サポートされているのは、ShellHook(WH\_SHELL) と、MouseHook(WH\_MOUSE) の２つがあります。

[WH\_SHELL]  
　コールバック関数で得られる nCode ごとに、TTB\_HSHELL\_ で始まるユーザー定義メッセージが定義されています。これはテンプレートの MessageDef.cpp を参照してください。  
　このメッセージ番号が Msg に設定されます。またコールバック関数で得られる wParam と lParam も得ることができます。

[WH\_MOUSE]  
　コールバック関数で得られる nCode が HC\_ACTION の場合だけ通知されます。  
　MsgにTTB\_HMOUSE\_ACTIONが設定されます（MessageDef.cpp参照）。  
　wParam にマウスメッセージの種類、lParam にそのマウスイベントが起こったウィンドウのハンドルが設定されます。実際の WH\_MOUSE では、lParam に MOUSEHOOKSTRUCT へのポインタが設定されますが、これをすべてプラグインで受け取ることはできません。ウィンドウハンドルのみが提供されています。

---

##API 関数

================================================================
###TTBPlugin\_GetPluginInfo
================================================================
```c
extern PLUGIN_INFO* (WINAPI* TTBPlugin_GetPluginInfo)(DWORD hPlugin);
```
　hPlugin で指定したプラグインのプラグイン情報構造体を取得します。取得時点で、TTBase が管理している情報が引き出せます。メニューのチェック状態などを得るのに使います。  
　この API 関数で取得した (PLUGIN\_INFO*) 型のポインタは、TTBPlugin\_FreePluginInfo 関数で解放する必要があります。

================================================================
###TTBPlugin\_SetPluginInfo
================================================================
```c
extern void (WINAPI* TTBPlugin_SetPluginInfo)(DWORD hPlugin, PLUGIN_INFO* PluginInfo);
```
　hPlugin で指定したプラグインのプラグイン情報構造体を再設定します。プラグイン側から動的にプラグイン情報構造体の内容を変更したいときに使用します。  
　PluginInfo は、新たにメモリを確保して使用してください。TTBPlugin\_GetPluginInfo で取得したものを使用してはいけません。  
（プラグインテンプレートには、ユーティリティルーチンとして、CopyPluginInfo 関数が用意されているので、これで TTBPlugin\_GetPluginInfo 関数で得たプラグイン情報構造体をコピーして使用するようにします。もちろん TTBPlugin\_GetPluginInfo を使用しないでプラグイン情報構造体を作成しても構いません）  
　この関数が使用されると、TTBase は、渡されたプラグイン構造体をコピーして、内部で使用するようになります。そのため、プラグイン側で確保した PluginInfo は、プラグイン側で明示的に解放するようにしてください。

================================================================
###TTBPlugin\_FreePluginInfo
================================================================
```c
extern void (WINAPI* TTBPlugin_FreePluginInfo)(PLUGIN_INFO* PluginInfo);
```
　TTBase 側で確保された (PLUGIN\_INFO*) 型のメモリを解放させます。  
　TTBPlugin\_GetPluginInfo で取得したメモリは、この API 関数を使って解放してください。

================================================================
###TTBPlugin_SetMenuProperty
================================================================
```c
extern void (WINAPI* TTBPlugin_SetMenuChecked)(DWORD hPlugin, int CommandID, DWORD ChagneFlag, DWORD Flag);
```
　hPlugin で指定したプラグインの、CommandID で示されるコマンドの、メニュー関係の属性を変更します。

+ ChangeFlag: 変更する属性の種類を指定します。複数のフラグを和で指定することもできます。
+ DISPMENU\_MENU:システムメニュー・ツールメニューの種類変更します。  
このフラグをセットした時に dmToolMenu、dmSystemMenu の両方を指定しないと、メニューに表示されません。
+ DISPMENU\_ENABLED:メニューをグレーアウトするかどうか指定できます。
+ DISPMENU\_CHECKED:メニューにチェックを入れるかどうかを指定できます。

Flag:フラグには、以下の値の和として指定します。

- dmNone       =  0; // 何も出さない
- dmToolMenu   =  2; // ツールメニュー
- dmSystemMenu =  1; // システムメニュー
- dmUnChecked  =  0; //
- dmChecked    =  8; // メニューがチェックされている
- dmEnabled    =  0; //
- dmDisabled   = 16; // メニューがグレイアウトされている

　ChangeFlag との組み合わせで、効果を発揮するかどうかが決まります。たとえば、DISPMENU\_ENABLED だけを指定している時、dmToolMenu などを Flag にセットしても、効果を発揮しません。

================================================================
###TTBPlugin\_GetAllPluginInfo
================================================================
```c
extern PLUGIN_INFO** (WINAPI* TTBPlugin_GetAllPluginInfo)(void);
```
　TTBase に読み込まれているすべてのプラグインのプラグイン情報構造体へのポインタの配列へのポインタを返します。  
最後のプラグイン情報構造体へのポインタの次の配列要素には、nullptr が格納されていますので、これで最後を判定してください。

================================================================
###TTBPlugin\_FreePluginInfoArray
================================================================
```c
extern void (WINAPI* TTBPlugin_FreePluginInfoArray)(PLUGIN_INFO** PluginInfoArray);
```
　TTBPlugin\_GetAllPluginInfo で取得した、PLUGIN\_INFO\_ARRAY を解放します。

---

以上です。

Original Introduction by K2:  
[http://pc2.2ch.net/test/read.cgi/tech/1042029896/1-28](http://pc2.2ch.net/test/read.cgi/tech/1042029896/1-28)

<div style="text-align: right;">Last modified 2014.09.30 by tapetums</div>