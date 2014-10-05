#TTBasePlugin

TTBase プラグインのためのスケルトンプロジェクトです。  
VC++2013(32/64-bit) でコンパイルを確認しています。

##内容

- TTBase.md
- TTBase.sln
- Plugin.h
- Plugin.cpp
- MessageDef.h
- MessageDef.cpp
- Utility.h
- Utility.cpp
- x86.manifest
- amd64.manifest
- NYSL.TXT
- TTBasePlugin
 - TTBasePlugin.vcxproj
 - TTBasePlugin.vcxproj.filters
 - TTBasePlugin.def
 - TTBasePlugin.res
 - Main.h
 - Main.cpp

##説明

###TTBase.md
TTBase プラグインの仕様書です。  
オリジナルの作者であるK2 さんが 2ch に書き込んだ内容を、加筆・修正したものです。

###TTBase.sln
VC++用のソリューションファイルです。  
このファイルをダブルクリックして開いてください。

###Plugin.h
TTBase プラグイン インターフェイスの定義ファイルです。

###Plugin.cpp
プラグインテンプレートの共通処理をまとめたものです。  
通常は変更する必要はありません。

###MessageDef.h
TTBase で使用される各種メッセージの定義です。

###MessageDef.cpp
TTBase で使用される各種メッセージを Windows に登録しています。  
通常は変更する必要はありません。

###Utility.h / Utility.cpp
プラグインテンプレートの処理のうち、よく使われるものをまとめています。

###x86.manifest / amd64.manifest
バイナリにビジュアルスタイルを適用するためのマニフェストファイルです。  
将来の Windows のバージョンでは 修正が必要になる可能性があります。

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

###Main.h / Main.cpp
プラグインの処理を記述します。  
この部分を改造して、あなたのプラグインを作ってください。  

なお、

```c
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#pragma comment(linker, "/entry:DllMain")
```

を定義しているため、C ランタイム が使用できません。  
C ランタイム をお使いの場合は、この箇所と  

```c
void* __cdecl operator new(size_t size)
{
    return ::HeapAlloc(::GetProcessHeap(), 0, size);
}

void __cdecl operator delete(void* p)
{
    if ( p != nullptr ) ::HeapFree(::GetProcessHeap(), 0, p);
}

void* __cdecl operator new[](size_t size)
{
    return ::HeapAlloc(::GetProcessHeap(), 0, size);
}

void __cdecl operator delete[](void* p)
{
    if ( p != nullptr ) ::HeapFree(::GetProcessHeap(), 0, p);
}
```

の箇所を削除してください。

---

以上です。

####文責
tapetums

######この文書のライセンスは NYSL とします。  
[http://www.kmonos.net/nysl/](http://www.kmonos.net/nysl/)
