【ソフト名】 PopTwit
【著作権者】 Copyright(C) 2014 tapetums
【種　　別】 TTBase プラグイン
【動作環境】 Windows Vista/7/8/8.1 32/64-bit
【開発環境】 Visual C++ 2013 Express Edition

【概要】
ツイッターに投稿するだけのプラグインです。マルチアカウント／マルチユーザーに対応しています。
TTBase もしくは 互換プログラムのプラグインとして動作します。

作者の環境では Windows 7 64-bit で動作確認を行っています。
Windows XP では起動しません。


【依存関係】
Microsoft Visual C++ 2008 および 2013 ランタイムが必要です。
また、以下のライブラリを使用しています。

・OpenSSL
http://slproweb.com/products/Win32OpenSSL.html

・libssh2
http://www.libssh2.org/download/

・cURL
http://curl.haxx.se/download.html


【アーカイブ内容】
├─README.TXT
├─NYSL.TXT
├─x64
│　├─PopTwit.dll
│　└─PopTwit
│　　　├─PopTwit.exe
│　　　├─libeay32.dll
│　　　├─ssleay32.dll
│　　　├─libssh2.dll
│　　　└─libcurl.dll
└─x86
　　├─PopTwit.dll
　　└─PopTwit
　　　　├─PopTwit.exe
　　　　├─libeay32.dll
　　　　├─ssleay32.dll
　　　　├─libssh2.dll
　　　　└─libcurl.dll

計 14 ファイル


【インストール】
本体となるプログラムによります。
TTBase の場合は、本体のインストールフォルダ以下の場所に配置してください。

32ビット版 は x86 フォルダ一式を、
64ビット版 は x64 フォルダ一式を それぞれお使いください。
PopTwit.dll と PopTwit の位置関係は崩さないでください。


【アンインストール】
DLLファイルおよびPopTwitフォルダ以下を削除してください。レジストリは使用していません。

 なお、%LOCALAPPDATA%\PopTwit に一時ファイルを保存しています。
 ( Windows 7 であれば C:\Users\アカウント名\AppData\Local\PopTwit\ )
お手数ですが手動での削除をお願いします。


【使い方】
1. ウィンドウ右上にあるボタンをクリックするか、Ctrl+U でユーザーリスト編集画面を開き、
   自分の Twitter のユーザー名を入力します。
2. 初回投稿時にパスワードを入力します。
   以後は取得したアクセストークンだけを使って書き込めるようになります。
3. Ctrl+↑ / Ctrl+↓ でユーザーを切り替えます。
   最大 8 アカウントまでに対応しています。


【既知の不具合】
・ 文字列を選択状態にすると透明になってしまう
・ Ctrl + A で 文字列の全選択ができない
・ Ctrl+Enter から送信した場合、キャンセルすると改行が入力されてしまう


【ライセンス】
NYSL です。このプラグラムを使用したことによる一切の影響は関知しません。
http://www.kmonos.net/nysl/

なお、各ライブラリはそれぞれのライセンスに従います。


【更新履歴】
2014.10.11 v0.2.0.0
・常駐プラグインからコマンドプラグインに変更

2014.10.11 v0.1.0.2
・32ビット版を ANSI に
・メッセージ送信時のエンコードバグを修正

2014.10.11 v0.1.0.1
・ウィンドウの二重起動を防止
・ウィンドウを開いた時に最前面に移動するようにした

2014.10.10 v0.1.0.0
・初版作成


【文責】
tapetums
https://github.com/tapetums
