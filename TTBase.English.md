#TTBase Plugin Sepcification

　If you execute many resident softwares just because of convenience, it causes lack of resources, taking an extremely long time to boot your PC, and many other annoying things.  
　Therefore, TTBase has been invented. It has only plugin interfaces. So you can get your own and only application as you want when you limit which plugin you use.

Official Web Site  
 [http://ttbase.sourceforge.jp/](http://ttbase.sourceforge.jp/)

The web site of K2, the author  
 [http://k2top.jpn.org/index.php?TTBase](http://k2top.jpn.org/index.php?TTBase)

 TTbase Uploader  
 [http://ttbase.coresv.com/uploader/](http://ttbase.coresv.com/uploader/)

 Software: 2ch BBS  
 【常駐】TTBaseってどうよ？その10【プラグイン】  
 [http://potato.2ch.net/test/read.cgi/software/1325925703/](http://potato.2ch.net/test/read.cgi/software/1325925703/)

---

##Introduction  
　TTBase is just an application which resident in task tray after starting.  
To enable this software, you need plugins (DLLs) to be loaded by TTBase.  
　Read this specification carefully and let's make plugins!

##The concept  
　When we want to make a slight resident tools, we usually write a whole application from scratch. But if we have such many tools, we are not happy with unexpected phenimenons such as having many processes, using a lot of resources, slowing down the boot time of the PC.  
　Therefore, if we make a one resident tool which calls small DLLs or lets them resident in its own process, we may have an ideal tool which is friendly to the environment. The origin of TTBase is from such an idea.  
　Using plugin SDK, you can write tiny tools immediately and it simplifies the process such as indicating task tray icon and/or constructing hotkeys and menus. So it is suitabe for making small applications.  
　Some of global hooks are also available for plugins. You can use almost all function of WH\_SHELL and some part of WH\_MOUSE without thinking about bothering DLL hooks and shared memory.

---

##Development Environment  
　The plugin interface works in any processing system beacuae the plugin is to be loaded dynamically  as a dynamic link library (DLL). Any plugin should be used in a processing system which supports DLL.  
　Now there is a template project for _**Visual Studio Community 2015 (C++11, 32/64-bit)**_.

---

##The sort of pluguins  
###Residental Type and At-use Type  
　Set _ptAlwaysLoad_ into _PluginType_ of the Plugin Information Structure, and it is the residental type. While TTBase is running, the plugin is always loaded in the process. If you want to hook mouse messages, choose this type.  
　When you set _ptLoadAtUse_, now it is the at-use type. It will be loaded at the time when users or the timer call commands, following _TTBPlugin\_Init_, _TTBPlugin\_Execute_, and then _TTBPlugin\_Unload_ is called and the plugin will be unloaded.  
　The at-use type saves memory because it is loaded on the memory only when the commands are executed. However, it takes more time to execute commands. If you want to make a plugin for such a use of reading a big data file, you should choose the residental type.  
　If you call _TTBPlugin\_WindowsHook_, you must choose the residental type. When making a residental plugin, you should use WindowsAPIs as you can and let the file size small. It is one of the option that a small residental type and a big at-use type collaborates with each other.

###The ways of how a command is called  
　TTBase calls commands in 5 ways below:  

- Tool Menu (Left click on the task tray or hoykeys)
- System Menu (Right click on the task tray or hoykeys)
- Hotkeys
- Timer
- WindowsHook（Now _WH\_SHELL_ and some part of _WH\_MOUSE_ are available)

　ツールメニュー・システムメニューに出すかどうかは、プラグインコマンド情報構造体の DispMenu に適切な値を設定することで決定されます。  
またホットキーの設定メニューの中にコマンドを出すかどうかも、DispMenu の設定で左右されます。  
　タイマーに関しては、PluginCommandInfo の IntervalTime に呼ばれる時間間隔を設定することで機能するようになります。使わない場合は 0 を設定します。  
　WindowsHook は、面倒な Hook.dll を作成しなくても、TTBase が Hook用 の手続きを踏んで、Plugin関数 を呼んでくれます。現在のところ、WH\_SHELL の全機能と、WH\_MOUSE の一部機能が使用できます。

###How plugins send a message to the host  
　TTBase は、起動時に実行フォルダ以下の DLL ファイルを検索し、いったんロードします。その後、DLL の _TTBPlugin\_InitPluginInfo_ イベントを呼んで、プラグイン情報構造体 (PLUGIN\_INFO) をプラグインから取得します。これによって、そのプラグインの名前・種類・コマンド情報をプラグインから得ます。その後、常駐型でない場合は、アンロードします。  
　常駐型の場合は、_Plugin\_Init_ が呼ばれ、そのままプロセスにロードされ続けます。  
　なお、TTBase の起動を速くするため、インストール後、２回目の起動以降は、TTBase.dat に保存されたプラグイン情報キャッシュを使ってプラグイン情報を得るようになります。DLL のファイルタイムが更新された場合以外は、そのままその情報が使用され続けます。 

---

#Constants and Structures  
　Pack the alignment of all structure.  

　Here is the definition in C++11.

```c
//---------------------------------------------------------------------------//
//
// Constants
//
//---------------------------------------------------------------------------//

// Plugin Load Type
enum PLUGINTYPE : WORD
{
   ptAlwaysLoad    = 0x0000, // Resident Plugin
   ptLoadAtUse     = 0x0001, // Load-at-use Plugin
   ptSpecViolation = 0xFFFF, // The DLLs but TTBase
};

// メニュー表示に関する定数
enum DISPMENU : DWORD
{
    dmNone        = 0,      // Show nothing
    dmSystemMenu  = 1,      // System Menu
    dmToolMenu    = 1 << 1, // Tool Menu
    dmHotKeyMenu  = 1 << 2, // Hotkey
    dmMenuChecked = 1 << 3, // with a Checkmark
    dmDisabled    = 1 << 4, // Disabled

    dmUnchecked   = 0,      // without Checkmarks
    dmEnabled     = 0,      // Enabled
};

// for ChangeFlag argument in TTBPlugin_SetMenuProperty
enum CHANGE_FLAG : DWORD
{
    DISPMENU_MENU    = dmSystemMenu | dmToolMenu,
    DISPMENU_ENABLED = dmDisabled,
    DISPMENU_CHECKED = dmMenuChecked,
};

// Constants for Log Output
enum ERROR_LEVEL : DWORD
{
    elNever   = 0, // No output
    elError   = 1, // for Erros
    elWarning = 2, // for Warnings
    elInfo    = 3, // for Informations
    elDebug   = 4, // for Debugging
};

//---------------------------------------------------------------------------//
//
// Structures
//
//---------------------------------------------------------------------------//

// Packing the alignment of structures
#pragma pack(push, 1)

// Command Structure (UNICODE)
struct PLUGIN_COMMAND_INFO_W
{
    LPWSTR   Name;          // Command Name (English)
    LPWSTR   Caption;       // Command's Description (in Any Languages)
    INT32    CommandID;     // Command Number
    INT32    Attr;          // Attributes (Not Used) must be 0
    INT32    ResID;         // Resoures Number (Not Used): must be -1
    DISPMENU DispMenu;      // Settings on displaying menus
    DWORD    TimerInterval; // Timer Interval for the command[msec] Set 0 if nou used
    DWORD    TimerCounter;  // System Internal Variant
};

// Command Structure (ANSI)
struct PLUGIN_COMMAND_INFO_A
{
    LPSTR    Name;          // Command Name (English)
    LPSTR    Caption;       // Command's Description (in Any Languages)
    INT32    CommandID;     // Command Number
    INT32    Attr;          // Attributes (Not Used) must be 0
    INT32    ResID;         // Resoures Number (Not Used): must be -1
    DISPMENU DispMenu;      // Settings on displaying menus
    DWORD    TimerInterval; // Timer Interval for the command[msec] Set 0 if nou used
    DWORD    TimerCounter;  // System Internal Variant
};

// Plugin Infornation Structure (UNICODE)
struct PLUGIN_INFO_W
{
    WORD                   NeedVersion;  // Plugin's I/F version
    LPWSTR                 Name;         // Plugin's Name (in Any Languages)
    LPWSTR                 Filename;     // Plugin's File Name (in relative path)
    PLUGINTYPE             PluginType;   // Load Type
    DWORD                  VersionMS;    // Version Number
    DWORD                  VersionLS;    // Version Number
    DWORD                  CommandCount; // The count of Commands
    PLUGIN_COMMAND_INFO_W* Commands;     // Commands
    DWORD                  LoadTime;     // The time that it took to load (msec)
};

// Plugin Infornation Structure (ANSI)
struct PLUGIN_INFO_A
{
    WORD                   NeedVersion;  // Plugin's I/F version
    LPSTR                  Name;         // Plugin's Name (in Any Languages)
    LPSTR                  Filename;     // Plugin's File Name (in relative path)
    PLUGINTYPE             PluginType;   // Load Type
    DWORD                  VersionMS;    // Version Number
    DWORD                  VersionLS;    // Version Number
    DWORD                  CommandCount; // The count of Commands
    PLUGIN_COMMAND_INFO_A* Commands;     // Commands
    DWORD                  LoadTime;     // The time that it took to load (msec)
};

#pragma pack(pop)

// Switch the structure with or without the definition of the UNICODE macro
#if defined(_UNICODE) || defined(UNICODE)
  using PLUGIN_COMMAND_INFO = PLUGIN_COMMAND_INFO_W;
  using PLUGIN_INFO         = PLUGIN_INFO_W;
#else
  using PLUGIN_COMMAND_INFO = PLUGIN_COMMAND_INFO_A;
  using PLUGIN_INFO         = PLUGIN_INFO_A;
#endif
```

---
###【Description】

There are two kinds of structures.

##[PLUGIN\_INFO Structure]
　The storage for plugin informatin. To tell the information to TTBase, call _Plugin\_SetPluginInfo_ and pass this structure. This includes command information (_PLUGIN\_COMMAND\_INFO_).

###WORD NeedVersion
　Plugin API version that it requires.  
Set **0** at this time.

###LPTSTR Name
　Name of the plugin. You can use any letter in addition to ascii.

###LPTSTR Filename
　Set the file name as **the relative path** from the installaion path.

###WORD PluginType
　Specify the plugin type whether residental or at-use.  

    ptAlwaysLoad:    Resident Plugin
    ptLoadAtUse:     Load-at-use Plugin
    ptSpecViolation: The DLLs but TTBase. DO NOT USE THIS VALUE.

　Choose _ ptLoadAtUse_ as possible if it works only at use.  
It saves the memory usage of TTBase.

###DWORD VersionMS, VersionLS
　Set the version of the plugin.  

    HIWORD(VersionMS): Major Version
    LOWORD(VersionMS): Minor Version
    HIWORD(VersionLS): Release Number
    LOWORD(VersionLS): Build Number

　I reccomend the patern above, but you don't have to obey it.

###DWORD CommandCount
　The number of the commands that the plugin has.  
The total number must be **eaqual or less than 256**.

###PLUGIN\_COMMAND\_INFO Commands
　Set the command information. 
Prepare a pointer for the array of the commands, then reserve the memory and set it into the pointer.

###DWORD LoadTime
　This member is used in the TTBase. So you don't have to care about it. The time that it takes to retrieve the information of the plugin will be set in milliseconds.  
　The value is less than msec in resolution since _QueryPerformanceTimer()_ is used.

※_peach_ and _hako_ always sets it **0**

---

##[PLUGIN\_COMMAND\_INFO Structure]
　The structure for _Commands_ member of _PLUGIN\_INFO_.  
Set the infomations of the plugin and pass it.

###LPTSTR Name
　The name of the command. Use **ratin alphabets, numbers**, and **\_** only.

###LPTSTR Caption
　The caption of the command. It is displayed on the menu and so on. You can use any letter.

###INT32 CommandID
　The number of the command. Set a unique number a command.

###INT32 Attr
　The attribute of the command. Now this value is not is used.

###INT32 ResID
　The resource ID of the command. Now this value is not is used.

###DWORD DispMenu
　Specify whether the command is displayed on the system/tool menu of TTBase. The principle is that settings are to be set as a system menu and the basic functions of the command are to be set as a tool menu.  
　In addition set whether the command is available as a hotkey. If you set 2 of them or more, please use the logical sum.  

    dmNone:        Shows nothing
    dmSystemMenu:  Displays on the system menu
    dmToolMenu:    Displays on the tool menu
    dmHotKeyMenu:  Displays on the hotkey list
    dmMenuChecked: Whether it has a checkmark or not
  
###DWORD TimerInterval
　The specification of sequential command execution with a timer.
Specify the interval time in [msec].  
If you don't use the timer function, set it 0.  
　Inside TTBase,  the timer event occurs in every about 100msec. Commads are executed in this time resolution, so it is meaningless if you set a too small value. Set it at about 100msec unit.

###DWORD TimerCounter
　Using inside TTBase. 

---

##Event Handlers
　必ず定義しなければならないイベントハンドラは、以下のものです。DLL で関数の名前をエクスポートしてください。  
エクスポートされていない場合は、その DLL をプラグインとは認識しません。

================================================================
###TTBEvent\_InitPluginInfo
================================================================
```c
PLUGIN_INFO* WINAPI TTBEvent_InitPluginInfo(LPTSTR PluginFilename);
```
　プラグイン情報構造体のメモリを確保し、情報をセットして返します。コマンドを持つ場合は、コマンド情報構造体のメモリも確保し、渡します。  
　PluginFilename には、そのプラグイン実行ファイル名が、 TTBase のインストールフォルダからの**相対パス**として格納されています。これは、プラグインの内部でも使用できますが、プラグイン情報構造体の Filename メンバにメモリを確保して、コピーして TTBase 本体に返してください。 

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
　TTBase のプラグイン情報キャッシュ機構のため、TTBEvent\_InitPluginInfo が**毎回呼ばれるわけではありません**。そこで、PluginFilename を使って、DLLは自分のファイル名を知ることができます。  
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
BOOL WINAPI TTBEvent_Execute(INT32 CommandID, HWND hWnd);
```
　コマンドが、何らかの形で呼ばれる場合、この関数が呼ばれます。  
　プラグイン情報構造体で、CommandCount を 1 以上に設定した場合は、このハンドラは必ず必要です。  
CommandID には、そのコマンドの ID が入っています。  
　hWnd には、TTBase が持っている NotifyWindow（表示はされませんが、メッセージ処理をするために必要です）のウィンドウハンドルが格納されています。  
　正常に処理を終了したら TRUE を返すようにしてください。  

　この関数が呼ばれるのは、以下の条件です。  

- TTBaseのツールメニューかシステムメニューが選択された  
- ユーザーが設定したホットキーからコマンドが呼ばれた  
- タイマー型のコマンド（PluginInfo の IntervalTime が 1 以上に設定されているコマンド）の場合、設定時間ごとに自動的に呼ばれる

================================================================
###TTBEvent\_WindowsHook
================================================================
```c
void WINAPI TTBEvent_WindowsHook(UINT Msg, WPARAM wParam, LPARAM lParam);
```
　TTBase本体が、Hook 用の DLL を持っています。これを使って、Hook で得られるイベントを、プラグインでも使うことができます。  
　現在サポートされているのは、ShellHook(WH\_SHELL) と、MouseHook(WH\_MOUSE) の２つがあります。

[WH\_SHELL]  
　コールバック関数で得られる nCode ごとに、"TTB\_HSHELL\_" で始まるユーザー定義メッセージが定義されています。これはテンプレートの MessageDef.cpp を参照してください。  
　このメッセージ番号が Msg に設定されます。またコールバック関数で得られる wParam と lParam も得ることができます。

[WH\_MOUSE]  
　コールバック関数で得られる nCode が HC\_ACTION の場合だけ通知されます。  
　Msgに TTB\_HMOUSE\_ACTION が設定されます（MessageDef.cpp 参照）。  
　wParam にマウスメッセージの種類、lParam にそのマウスイベントが起こったウィンドウのハンドルが設定されます。実際の WH\_MOUSE では、lParam に MOUSEHOOKSTRUCT へのポインタが設定されますが、これをすべてプラグインで受け取ることはできません。ウィンドウハンドルのみが提供されています。

---

##API Functions

================================================================
###TTBPlugin\_GetPluginInfo
================================================================
```c
extern PLUGIN_INFO* (WINAPI* TTBPlugin_GetPluginInfo)(DWORD_PTR hPlugin);
```
　hPlugin で指定したプラグインのプラグイン情報構造体を取得します。取得時点で、TTBase が管理している情報が引き出せます。メニューのチェック状態などを得るのに使います。  
　この API 関数で取得した (PLUGIN\_INFO*) 型のポインタは、TTBPlugin\_FreePluginInfo 関数で解放する必要があります。

================================================================
###TTBPlugin\_SetPluginInfo
================================================================
```c
extern void (WINAPI* TTBPlugin_SetPluginInfo)(DWORD_PTR hPlugin, PLUGIN_INFO* PluginInfo);
```
　hPlugin で指定したプラグインのプラグイン情報構造体を再設定します。プラグイン側から動的にプラグイン情報構造体の内容を変更したいときに使用します。  
　PluginInfo は、新たにメモリを確保して使用してください。**TTBPlugin\_GetPluginInfo で取得したものを使用してはいけません**。  
（プラグインテンプレートには、ユーティリティルーチンとして、CopyPluginInfo 関数が用意されているので、これで TTBPlugin\_GetPluginInfo 関数で得たプラグイン情報構造体をコピーして使用するようにします。もちろん TTBPlugin\_GetPluginInfo を使用しないでプラグイン情報構造体を作成しても構いません）  
　この関数が使用されると、TTBase は、渡されたプラグイン構造体をコピーして、内部で使用するようになります。そのため、プラグイン側で確保した PluginInfo は、**プラグイン側で明示的に解放**するようにしてください。

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
extern void (WINAPI* TTBPlugin_SetMenuChecked)(DWORD_PTR hPlugin, INT32 CommandID, DWORD ChagneFlag, DWORD Flag);
```
　hPlugin で指定したプラグインの、CommandID で示されるコマンドの、メニュー関係の属性を変更します。

_**※in TTBase 1.1.0, there is a bug that CommandID is not understood as the command ID but as the command index in the plugin.**_

ChangeFlag: 変更する属性の種類を指定します。複数のフラグを論理和で指定することもできます。

    DISPMENU_MENU   : システムメニュー・ツールメニューの種類変更します。  
                      このフラグをセットした時に dmToolMenu、dmSystemMenu の両方を指定しないと、メニューに表示されません。
    DISPMENU_ENABLED: メニューをグレーアウトするかどうか指定できます。
    DISPMENU_CHECKED: メニューにチェックを入れるかどうかを指定できます。

Flag: フラグには、以下の値の論理和として指定します。

    dmNone       =  0; // Show nothing
    dmSystemMenu =  1; // System Menu
    dmToolMenu   =  2; // Tool Menu
    dmUnChecked  =  0; // without a Checkmark
    dmChecked    =  8; // with a Checkmark
    dmEnabled    =  0; // the menu is enabled
    dmDisabled   = 16; // the menu is grayed out

　ChangeFlag との組み合わせで、効果を発揮するかどうかが決まります。たとえば、DISPMENU\_ENABLED だけを指定している時、dmToolMenu などを Flag にセットしても、効果を発揮しません。

================================================================
###TTBPlugin\_GetAllPluginInfo
================================================================
```c
PLUGIN_INFO** WINAPI TTBPlugin_GetAllPluginInfo(void);
```
　Returns the pointer of the pointer array that consists of all the information of the pluguin information structure of the pluguins in TTBase.  
The last value is set to be nullptr. You can detect the tail of the array with it.

================================================================
###TTBPlugin\_FreePluginInfoArray
================================================================
```c
void WINAPI TTBPlugin_FreePluginInfoArray(PLUGIN_INFO** PluginInfoArray);
```
　Releases PLUGIN\_INFO\_ARRAY that is retrieved by _TTBPlugin\_GetAllPluginInfo_.

***

####_TTBase v1.0.15 and later_


================================================================
###TTBPlugin\_SetTaskTrayIcon
================================================================
```c
void WINAPI TTBPlugin_SetTaskTrayIcon(HICON hIcon, LPCTSTR Tips);
```
　Change the system tray icon of TTBase  

※_peach_ does not plan to implement this feature

***

####_TTBase v1.0.17 alpha 2 and later_

================================================================
###TTBPlugin\_WriteLog
================================================================
```c
void WINAPI TTBPlugin_WriteLog(DWORD_PTR hPlugin, INT32 logLevel, LPCTSTR msg);
```
　Outputs logs from pluguins  

logLevel are below:

    elNever   = 0; // No output
    elError   = 1; // for Erros
    elWarning = 2; // for Warnings
    elInfo    = 3; // for Informations
    elDebug   = 4; // for Debugging

================================================================
###TTBPlugin\_ExecuteCommand
================================================================
```c
BOOL WINAPI TTBPlugin_ExecuteCommand(LPCTSTR PluginFilename, INT32 CmdID);
```
　Executes commands from other plugins.  
　Specify PluginFilename as the relative path from TTBase. You can get the path by _TTBPlugin\_GetPluginInfo_ or _TTBPlugin\_GetAllPluginInfo_.  
　Return value is to be returned from the plugin that executs the command. If system could not find the plugin you specified, it returns FALSE.

---

Thanks for reading.

**Original Introduction by K2:**  
[http://pc2.2ch.net/test/read.cgi/tech/1042029896/1-28](http://pc2.2ch.net/test/read.cgi/tech/1042029896/1-28)

######modified and translated by tapetums 2014-2016