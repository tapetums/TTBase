#TTBase Plugin Sepcification

　If you execute many resident softwares just because of convenience, it causes lack of resources, takes an extremely long time to boot your PC, and brings many other annoying things.  
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
　TTBase is just an application which is resident in task tray after starting.  
To enable this software, you need plugins (DLLs) to be loaded by TTBase.  
　Read this specification carefully and let's make plugins!

##The concept  
　When we want to make a slight resident tools, we usually write a whole application from scratch. But if we have such many tools, we are not happy with unexpected phenomenons such as having many processes, using a lot of resources, slowing down the boot time of the PC.  
　Therefore, if we make a one resident tool which calls small DLLs or lets them resident in its own process, we may have an ideal tool which is friendly to the environment. The origin of TTBase is from such an idea.  
　Using plugin SDK, you can write tiny tools immediately and it simplifies the process such as indicating task tray icon and/or constructing hotkeys and menus. So it is suitabe for making small applications.  
　Some of global hooks are also available for plugins. You can use almost all function of _WH\_SHELL_ and some part of _WH\_MOUSE_ without thinking about bothering DLL hooks and shared memory. 

※There are some implementations that do not provide hooks such as _peach_,
_hako_, etc.

---

##Development Environment  
　The plugin interface works in any processing system beacuae the plugin is to be loaded dynamically  as a dynamic link library (DLL). Any plugin should be used in a processing system which supports DLL.  
　Now there is a project template for _**Visual Studio Community 2015 (C++11, 32/64-bit)**_.

---

##The sort of pluguins  
###Resident Type and At-use Type  
　Set _ptAlwaysLoad_ into _PluginType_ of the plugin information structure, and it is the resident type. While TTBase is running, the plugin is always loaded in the process. If you want to hook mouse messages, choose this type.  
　When you set _ptLoadAtUse_, now it is the at-use type. It will be loaded at the time when users or the timer call commands, following _TTBPlugin\_Init_, _TTBPlugin\_Execute_, and then _TTBPlugin\_Unload_ is called and the plugin will be unloaded.  
　The at-use type saves memory because it is loaded on the memory only when the commands are executed. However, it takes more time to execute commands. If you want to make a plugin for such a use of reading a big data file, you should choose the resident type.  
　If you call _TTBPlugin\_WindowsHook_, you must choose the resident type. When making a residental plugin, you should use WindowsAPIs as you can and let the file size small. It is one of the option that a small resident type and a big at-use type collaborates with each other.

###The ways of how a command is called  
　TTBase calls commands in 5 ways below:  

- Tool Menu (Left click on the task tray or hoykeys)
- System Menu (Right click on the task tray or hoykeys)
- Hotkeys
- Timer
- WindowsHook（At now _WH\_SHELL_ and some part of _WH\_MOUSE_ are available)

　Displaying tool menu / system menu or not is determined by setting the value properly into DispMenu of the plugin information structure. Moreover, displaying commands in the hotkey setting menu is also determined by the value of DispMenu.  
　About the timer, it gets function by setting the interval time into IntervalTime of PluginCommandInfo. Set 0 if not use.  
　On WindowsHook, TTBase goes through a procedure for Hook and calls for plugin functions without paniful works for Hook.dll. At now, you can use almost all function of _WH\_SHELL_ and some part of _WH\_MOUSE_.  

※ _peach_ and _hako_ do not provide timer nor hook by itself

###How plugins send a message to the host  
　TTBase searches DLL files in the installation folder and the subfolders, and once load them when it is started. Then it calls _TTBPlugin\_InitPluginInfo_ event of the plugin and gets the plugin information structure (PLUGIN\_INFO) from the plugin. By this, it obtains the name, sort, and command informations of the plugin. After that, it unloads plugins if they are not residental.  
　If the plugin is a resident type, TTBase calls _Plugin\_Init_ and it will stay loaded in the process.  
　By the way, to accelerate the boot time, TTBase uses the cache of the plugin information which is stored in TTBase.dat after the first installation of the plugin. Unless the file time of the plugin is updated, it keeps using the cache.

※There are some implementations that do not have cache mechanism such as _pbox_, _peach_,
_hako_, etc.

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

// The constants about displaying menus
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

There are two kinds of structure.

##[PLUGIN\_INFO Structure]
　The storage for plugin informatin. To tell the information to TTBase, call _Plugin\_SetPluginInfo_ and pass this structure. This includes command information (_PLUGIN\_COMMAND\_INFO_).

###WORD NeedVersion
　Plugin API version that the plugin requires.  
Set **0** at this time.

###LPTSTR Name
　Name of the plugin. You can use any letter including to ascii.

###LPTSTR Filename
　Set the file name as **a relative path** from the installaion path.

###WORD PluginType
　Specify the plugin type whether it is residental or at-use type.  

    ptAlwaysLoad:    Resident Plugin
    ptLoadAtUse:     Load-at-use Plugin
    ptSpecViolation: The DLLs but TTBase. DO NOT USE THIS VALUE.

　Choose _ptLoadAtUse_ as possible if it works only at use.  
That saves the memory usage of TTBase.

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
Prepare a pointer for the array of the commands, and then allocate the memory and set them into the pointer.

###DWORD LoadTime
　This member is used in the TTBase. So you don't have to care about it. The time that it takes to get the information of the plugin will be set in the unit of millisecond.  
　The value is less than msec in resolution since _QueryPerformanceTimer()_ is used.

※_peach_ and _hako_ always sets it **0**

---

##[PLUGIN\_COMMAND\_INFO Structure]
　The structure for _Commands_ member of _PLUGIN\_INFO_.  
Set the infomation of the commands and pass it.

###LPTSTR Name
　The name of the command. Use **ratin alphabets, numbers**, and **\_** only.

###LPTSTR Caption
　The caption of the command. It is displayed on the menu and so on. You can use any letter.

###INT32 CommandID
　The identity of the command. Set a unique number a command.

###INT32 Attr
　The attribute of the command. At now this value is not is used.

###INT32 ResID
　The resource ID of the command. At now this value is not is used.

###DWORD DispMenu
　Specify whether the command is displayed on the system/tool menu of TTBase. The principle is that settings are to be set as a system menu and the basic functions of the command are to be set as a tool menu.  
　In addition set whether the command is available as a hotkey. If you are to set 2 of them or more, use the logical sum.  

    dmNone:        Shows nothing
    dmSystemMenu:  Displays on the system menu
    dmToolMenu:    Displays on the tool menu
    dmHotKeyMenu:  Displays on the hotkey list
    dmMenuChecked: Whether it has a checkmark or not
  
###DWORD TimerInterval
　The specification for sequential command execution with the timer.
Specify the interval time in [msec].  
If you don't use the timer function, set it 0.  
　Inside TTBase,  the timer event occurs in every about 100msec. Commads are executed in this time resolution, so it is meaningless if you set a too small value. Set it at about 100msec unit.

###DWORD TimerCounter
　This is used inside TTBase. 

---

##Event Handlers
　The event handlers that you must define are below. Export the names of the function in your DLL.  
If not exported the functions, TTBase does not recognize the DLL as a plugin.

================================================================
###TTBEvent\_InitPluginInfo
================================================================
```c
PLUGIN_INFO* WINAPI TTBEvent_InitPluginInfo(LPTSTR PluginFilename);
```
　Allocate the memory for the plugin information structure, set the information and return it. If the plugin has commands, allocate the memory for the command information too.  
　In _PluginFilename_, the execution path of the plugin is set as a **relative path** from TTBase. You can use it in your plugin, but copy it with allocating the memory for the _Filename_ member of the plugin information structure, and return it to TTBase. 

================================================================
###TTBEvent\_FreePluginInfo
================================================================
```c
void WINAPI TTBEvent_FreePluginInfo(PLUGIN_INFO* PluginInfo);
```
　Free the memory of the plugin information structure which is passed.  
　Ensure the command count and free all the memory of the command information correctly.  
The memory that is freed by this event is the memory allocated by _TTBEvent\_InitPluginInfo_.

***

　The followings are the event handers to define as needed.

================================================================
###TTBEvent\_Init
================================================================
```c
BOOL WINAPI TTBEvent_Init(LPTSTR PluginFilename, DWORD_PTR hPlugin);
```
　Initialize the plugin. This event is called at the first time when the plugin is loaded.  
　Because of the cache mechanism of TTBase, _TTBEvent\_InitPluginInfo_ **would not be called in every time**. So, you can get your DLL's file name by _PluginFilename_.  
　_hPlugin_ is the identity code that TTBase distinguish plugins. You need this in some functions, so store it in a gloabal variable.  
　Return TRUE when the initialization has been done successfully.

※There are some implementations that do not have cache mechanism such as _pbox_, _peach_,
_hako_, etc.

================================================================
###TTBEvent\_Unload
================================================================
```c
void WINAPI TTBEvent_Unload(void);
```
　This event is called when the plugin is to unload.

================================================================
###TTBEvent\_Execute
================================================================
```c
BOOL WINAPI TTBEvent_Execute(INT32 CommandID, HWND hWnd);
```
　This event is called when a command is about to execute in some way.  
　If you set _CommandCount_ of the plugin information structure 1 or greater, this handler is necessary.  
In _CommandID_, the command ID is set.  
　In _hWnd_, the window handle of the NotifyWindow which TTBase has is stored. (Though the window is not shown, it is necessary to handle window messages.)  
　If you finish the process successfully, return TRUE.  

　The situations that this event handler is called are below:  

- The tool menu or the system menu in TTBase is selected  
- The command is called from the hotkey that user had set  
- The timer calls the timer-type command automatically every setting time, which _IntervalTime_ of _PluginInfo_ is set 1 or greater.

================================================================
###TTBEvent\_WindowsHook
================================================================
```c
void WINAPI TTBEvent_WindowsHook(UINT Msg, WPARAM wParam, LPARAM lParam);
```
　TTBase itself has DLL for Hook. Using this, the plugin can handle the events which can be retrieved by Hook.  
　At now th supported is two messages, ShellHook(WH\_SHELL) and MouseHook(WH\_MOUSE).

[WH\_SHELL]  
　Every _nCode_ which can be obtained from the callback function, the user-defined messages followed by "TTB\_HSHELL\_" are defined. See MessageDef.cpp in the project template.  
　The message number is set into _Msg_. In addition, _wParam_ and _lParam_ can also be obtained from the callback function.

[WH\_MOUSE]  
　This is notified only when _nCode_ which can be got by the callback function is _HC\_ACTION_.  
　_TTB\_HMOUSE\_ACTION_ will be set in _Msg_ (See MessageDef.cpp).  
　The sort of mouse message will be in _wParam_, the window handle that the event occured in will be set in _lParam_. Even though the actual _WH\_MOUSE_ sets a pointer to _MOUSEHOOKSTRUCT_ in _lParam_, you cannot get everything in your plugin.TTBase provides only window handle.

---

##API Functions

================================================================
###TTBPlugin\_GetPluginInfo
================================================================
```c
extern PLUGIN_INFO* (WINAPI* TTBPlugin_GetPluginInfo)(DWORD_PTR hPlugin);
```
　To get the information structure of the plugin which is designated by _hPlugin_. You can get the information that TTBase has at the time. Used this when you check the condition of the check state of the menu.  
　It is necessary to free the (PLUGIN\_INFO\*) type pointer with _TTBPlugin\_FreePluginInfo_ function.

================================================================
###TTBPlugin\_SetPluginInfo
================================================================
```c
extern void (WINAPI* TTBPlugin_SetPluginInfo)(DWORD_PTR hPlugin, PLUGIN_INFO* PluginInfo);
```
　Reset the plugin information structure of the plugin which is designated by _hPlugin_. Use this function when you want to change the information dynamically from the plugin side.  
　Allocate new memory for _PluginInfo_. **Never use the memory that is passed by _TTBPlugin\_GetPluginInfo_**.  
（In the plugin template, CopyPluginInfo function is served as an utility routine. You can use it to copy the information structure which is got by _TTBPlugin\_GetPluginInfo_. Off course you can copy by yourself without using it）  
　After the function is called, TTBase will copy the information structure and then use it inside. So **free the memory of _PluginInfo_ explicitly that is allocated in the plugin side**.

================================================================
###TTBPlugin\_FreePluginInfo
================================================================
```c
extern void (WINAPI* TTBPlugin_FreePluginInfo)(PLUGIN_INFO* PluginInfo);
```
　Let the (PLUGIN\_INFO\*) type memory free that is allocated by TTBase.  
　Use this API function to free the memory that is got by _TTBPlugin\_GetPluginInfo_.

================================================================
###TTBPlugin_SetMenuProperty
================================================================
```c
extern void (WINAPI* TTBPlugin_SetMenuChecked)(DWORD_PTR hPlugin, INT32 CommandID, DWORD ChagneFlag, DWORD Flag);
```
　Change the attribute of the command which is indicated by _CommandID_ of the plugin which is designated by _hPlugin_.

_**※in TTBase 1.1.0, there is a bug that CommandID is not understood as the command ID but as the command index in the plugin.**_

ChangeFlag: to determine the attribute to change. Use logical sum when more than one flag are to be set.

    DISPMENU_MENU   : Change the sort of system menu / tool menu  
                      If this flag is set, it is not displayed
                      unless both of dmToolMenu and dmSystemMenu is to be set.
    DISPMENU_ENABLED: To determine to gray out the menu or not
    DISPMENU_CHECKED: To determine to check the menu item or not

Flag: set the value as a logical sum of the values below.

    dmNone       =  0; // Show nothing
    dmSystemMenu =  1; // System Menu
    dmToolMenu   =  2; // Tool Menu
    dmUnChecked  =  0; // without a Checkmark
    dmChecked    =  8; // with a Checkmark
    dmEnabled    =  0; // the menu is enabled
    dmDisabled   = 16; // the menu is grayed out

　In combination with _ChangeFlag_, it is decided whether it will take effect or not. For example, if you set only _DISPMENU\_ENABLED_, it won't take effect even if you set _dmToolMenu_ in the _Flag_.

================================================================
###TTBPlugin\_GetAllPluginInfo
================================================================
```c
PLUGIN_INFO** WINAPI TTBPlugin_GetAllPluginInfo(void);
```
　Returns the pointer of the pointer array that consists of all the information of the pluguin information structure in TTBase.  
The last value is set to be nullptr. You can detect the tail of the array with it.

================================================================
###TTBPlugin\_FreePluginInfoArray
================================================================
```c
void WINAPI TTBPlugin_FreePluginInfoArray(PLUGIN_INFO** PluginInfoArray);
```
　Releases PLUGIN\_INFO\_ARRAY that is got by _TTBPlugin\_GetAllPluginInfo_.

***

####_TTBase v1.0.15 and later_


================================================================
###TTBPlugin\_SetTaskTrayIcon
================================================================
```c
void WINAPI TTBPlugin_SetTaskTrayIcon(HICON hIcon, LPCTSTR Tips);
```
　Change the system tray icon of TTBase.  

※_peach_ does not plan to implement this feature

***

####_TTBase v1.0.17 alpha 2 and later_

================================================================
###TTBPlugin\_WriteLog
================================================================
```c
void WINAPI TTBPlugin_WriteLog(DWORD_PTR hPlugin, INT32 logLevel, LPCTSTR msg);
```
　Outputs logs from plugins.  

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
　Specify _PluginFilename_ as a relative path from TTBase. You can get the path by _TTBPlugin\_GetPluginInfo_ or _TTBPlugin\_GetAllPluginInfo_.  
　The return value is to be returned from the plugin that executes the command. If the system could not find the plugin you specified, it returns FALSE.

---

Thanks for reading.

**Original Introduction by K2:**  
[http://pc2.2ch.net/test/read.cgi/tech/1042029896/1-28](http://pc2.2ch.net/test/read.cgi/tech/1042029896/1-28)

######modified and translated by tapetums 2014-2016