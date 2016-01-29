#TTBasePlugin

TTBase プラグインのためのスケルトンプロジェクトです。  
**Visual Studio Community 2015 (C++11, 32/64-bit)** でコンパイルを確認しています。

---

##内容

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
