#TTBaseHost

An application that is compatible with TTBase.  
Compilable with: **Visual Studio Community 2015 (C++11, 32/64-bit)**

---

##Description

　This is an application compatible with [**TTBase**](https://osdn.jp/projects/ttbase/). The source code is written partially based on [**peach**](http://white2.php.xdomain.jp/?page_id=27) which is produced by U.  
　This is nothing but a compatible application, so it does not function the same as TTBase. I don't guarantee it as an alternative for TTBase or peach because some functions are not implemented. I hope you could find something from this sample as an implementation in C++11.  
　The biggest feature of this program is that **you can use both 32/64-bit plugins with one 64-bit host application**.

---

##Features

###Implemented
- Loading plugins
- Executing commands
- Providing GUI
- Indicating task-tray icons
- Changing task-tray icons by plugins
- System Menu / Tool Menu
- Logging
- Saving the settings to the .ini file
- Loading and executing 32/64-bit plugins with one 64-bit host application

###Not Implemented
- ~~Hotkey function~~ [HotkeyCpp](https://github.com/tapetums/HotkeyCpp/releases) is now available
- Timer function
- ~~Windows Hook~~ **Hook.dll** is included in the package
- Command line options

　The concept is baed on _peach_. This program does not provide some functions that TTBase does as the host services, and let them go with plugins.  
　The main difference from _peach_ is serving the task-tray icon as a function of the host application. You can switch the visibility of the icon by a command.  
　If you want to show the UI window when the icon is invisible, call the command with the hotkey of plugins or simply double-start the application.

###Screen Shots
![System](https://raw.githubusercontent.com/tapetums/TTBase/master/TTBaseHost/ss/System.png)  
![Plugins](https://raw.githubusercontent.com/tapetums/TTBase/master/TTBaseHost/ss/Plugins.png)  
![Commands](https://raw.githubusercontent.com/tapetums/TTBase/master/TTBaseHost/ss/Commands.png)  
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

####Edited by
tapetums

######This document is under the license of NYSL Version 0.9982.  
######[http://www.kmonos.net/nysl/](http://www.kmonos.net/nysl/)
