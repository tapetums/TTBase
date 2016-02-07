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

##License


NYSL Version 0.9982
```
A. 本ソフトウェアは Everyone'sWare です。このソフトを手にした一人一人が、
   ご自分の作ったものを扱うのと同じように、自由に利用することが出来ます。

  A-1. フリーウェアです。作者からは使用料等を要求しません。
  A-2. 有料無料や媒体の如何を問わず、自由に転載・再配布できます。
  A-3. いかなる種類の 改変・他プログラムでの利用 を行っても構いません。
  A-4. 変更したものや部分的に使用したものは、あなたのものになります。
       公開する場合は、あなたの名前の下で行って下さい。

B. このソフトを利用することによって生じた損害等について、作者は
   責任を負わないものとします。各自の責任においてご利用下さい。

C. 著作者人格権は tapetums に帰属します。著作権は放棄します。

D. 以上の３項は、ソース・実行バイナリの双方に適用されます。
```

NYSL Version 0.9982 (en) (Unofficial)
```
A. This software is "Everyone'sWare". It means:
  Anybody who has this software can use it as if he/she is
  the author.

  A-1. Freeware. No fee is required.
  A-2. You can freely redistribute this software.
  A-3. You can freely modify this software. And the source
      may be used in any software with no limitation.
  A-4. When you release a modified version to public, you
      must publish it with your name.

B. The author is not responsible for any kind of damages or loss
  while using or misusing this software, which is distributed
  "AS IS". No warranty of any kind is expressed or implied.
  You use AT YOUR OWN RISK.

C. Copyrighted to tapetums

D. Above three clauses are applied both to source and binary
  form of this software.
```

[http://www.kmonos.net/nysl/](http://www.kmonos.net/nysl/)

---

##Acknowledgements

I express deep respect and appreciation to K2 who has created TTBase, U who producted and published peach, the author of TTBaseCpp,  
and the authors who produced the great number of plugins for TTBase.

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
