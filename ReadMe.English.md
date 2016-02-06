#TTBasePlugin

This is a template project for a TTBase plugin.  
Compilable with: **Visual Studio Community 2015 (C++11, 32/64-bit)**

##Contents

---

- TTBase.md
- TTBase.sln
- Plugin.hpp
- Plugin.cpp
- MessageDef.hpp
- MessageDef.cpp
- Utility.hpp
- Utility.cpp
- x86.manifest
- amd64.manifest
- NYSL.TXT
- TTBasePlugin
 - TTBasePlugin.vcxproj
 - TTBasePlugin.vcxproj.filters
 - TTBasePlugin.vcxproj.user
 - TTBasePlugin.def
 - TTBasePlugin.res
 - Main.hpp
 - Main.cpp  

---

##Description

###TTBase.md
Specifications for TTBase plugin.  
Original introduction was written in 2ch by K2, who has originally created TTBase.  
I'm sorry but only Japanese version is available.  
Any translations are appreciated.

###TTBase.sln
Solution file for Visual C++ 2015.  
Please open this by double-clicking.

###Plugin.hpp
Header file for the TTBase plugin interfaces.

###Plugin.cpp
Common procedures for this plugin template.  
Usually you don't have to change it.

###MessageDef.hpp
Definitions of messages which are used in the TTBase system.

###MessageDef.cpp
Registering messages for the TTBase system to Windows.  
Usually you don't have to change it.

###Utility.hpp / Utility.cpp
Utilities for copying strings, getting version info from the DLL file and so on.  
Usually you don't have to change it.

###x86.manifest / amd64.manifest
Manifest file for Windows Visual Style.  
You might have to modify this for the future versions of Windows.

###NYSL.TXT
License for these files above.

---

_Under [TTBasePlugin] folder_

###TTBasePlugin.vcxproj
Project file for Visual C++2015.  
This will be automatically loaded when you open TTBase.sln.

###TTBasePlugin.def
.def file for export functions of the plugin.  
You may have to update it if the API version of the TTBase system is revised in the future.

###TTBasePlugin.res
Resource file for the DLL.  
You can edit it with applications such as ResEdit.

###Main.hpp / Main.cpp
Main part of this template.  
Make your own plugin with modifying this!!  

---

Here, for the sake of reducing the DLL's size, in Main.cpp

```c
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#pragma comment(linker, "/entry:DllMain")
```

is defined. So you cannot use C runtime.  
If you want to use C runtime and/or C++ standard libraries,  
Please delete the definition below  

```
_NODEFLIB  
```

from **[ Project -> C/C++ -> Preprocessors ]**.

---

###Change Log

2016.01.30  
- Changed development environment to VS2015 and C++11

2014.09.30  
- First Edition

---

Thanks for reading.

####Edited by
tapetums

######This document is under the license of NYSL Version 0.9982.  
######[http://www.kmonos.net/nysl/](http://www.kmonos.net/nysl/)
