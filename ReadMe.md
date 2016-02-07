#TTBasePlugin

TTBase プラグインのためのスケルトンプロジェクトです。  
**Visual Studio Community 2015 (C++11, 32/64-bit)** でコンパイルを確認しています。

##内容

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

##説明

###TTBase.md
TTBase プラグインの仕様書です。  
オリジナルの作者である K2 さんが 2ch に書き込んだ内容を、加筆・修正したものです。

###TTBase.sln
VC++用のソリューションファイルです。  
このファイルをダブルクリックして開いてください。

###Plugin.hpp
TTBase プラグイン インターフェイスの定義ファイルです。

###Plugin.cpp
プラグインテンプレートの共通処理をまとめたものです。  
通常は変更する必要はありません。

###MessageDef.hpp
TTBase で使用される各種メッセージの定義です。

###MessageDef.cpp
TTBase で使用される各種メッセージを Windows に登録しています。  
通常は変更する必要はありません。

###Utility.hpp / Utility.cpp
プラグインテンプレートの処理のうち、よく使われるものをまとめています。

###x86.manifest / amd64.manifest
バイナリにビジュアルスタイルを適用するためのマニフェストファイルです。  
将来の Windows のバージョンでは、修正が必要になる可能性があります。

###NYSL.TXT
このファイル群全体のライセンスを定義しています。

---

_TTBasePlugin フォルダ 以下_

###TTBasePlugin.vcxproj
VC++用のプロジェクトファイルです。  
TTBase.sln を開くと、自動で読み込まれます。

###TTBasePlugin.def
TTBase プラグイン のエクスポート関数の定義ファイルです。  
TTBase の API バージョンに応じて、将来的に変更の必要が生じるかもしれません。

###TTBasePlugin.res
DLL のリソースファイルです。  
ResEdit などの対応アプリケーションで編集してください。

###Main.hpp / Main.cpp
プラグインの処理を記述します。  
この部分を改造して、あなたのプラグインを作ってください。  

---

なお、Main.cpp 中で

```c
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#pragma comment(linker, "/entry:DllMain")
```

を定義しているため、C ランタイム が使用できません。  
C ランタイム および C++ 標準ライブラリ をお使いになる場合は、  
【プロジェクト -> C/C++ -> プリプロセッサ】の設定から  

```
_NODEFLIB  
```

を削除してください。

---

##ライセンス


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

##謝辞

TTBase を生み出された K2 さん、peach を 公開されている U さん、TTBaseCpp の作者さんを始め、  
数多くのプラグインを作られたそれぞれの作者さんたちに深い敬意と感謝を表します。

---

###変更履歴

2016.01.30  
- 開発環境を VS2015 および C++11 に変更  

2014.09.30  
- 初版作成

---

以上です。

####文責
tapetums

######この文書のライセンスは NYSL Version 0.9982 とします。  
######[http://www.kmonos.net/nysl/](http://www.kmonos.net/nysl/)
