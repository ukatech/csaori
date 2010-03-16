----------------------------------------------------------------------
■「advanced_sysinfo」：要るのかわからないシステム情報取得SAORI
■Written by CSaori Project
　http://code.google.com/p/csaori/
----------------------------------------------------------------------

■これは何をするものか

　×ド変態系システム情報SAORI
　○特殊なシステム情報類をいろいろ取得できるSAORI

■動作環境

　Win95/NT4以上

■使用方法

□パラメータ
　Argument0　　コマンド
　このコマンド文字列によって取得するシステム情報の内容を指定します。
　以下はこのArgument0に入力することで使えるコマンド内容です。

□get_special_folder_path
　マイドキュメント、デスクトップ等の特別なシステムフォルダを取得します。
　
　・パラメータ
　　Argument1には以下のリストのうちどれかを指定します。
　　
　　ApplicationData       アプリケーションデータ（ユーザ固有）
　　CommonApplicationData 共有アプリケーションデータ
　　LocalApplicationData  ローカルのみのアプリケーションデータ（キャッシュなど）
　　Cookies               インターネット cookie
　　Favorites             お気に入り
　　History               インターネットの履歴
　　InternetCache         一時インターネットファイル
　　Programs              [スタート] メニュー->プログラム 
　　MyMusic               マイ ミュージック
　　MyPictures            マイ ピクチャ
　　Recent                最近使用したドキュメント
　　SendTo                [送る] メニュー
　　StartMenu             [スタート] メニュー
　　Startup               [スタート] メニュー -> スタートアップ
　　System                System ディレクトリ。 
　　Windows               Windows ディレクトリ。 
　　Templates             テンプレート
　　DesktopDirectory      デスクトップ
　　MyDocuments           マイ ドキュメント
　　ProgramFiles          プログラム ファイル
　　CommonProgramFiles    アプリケーション間で共有されるプログラムファイル
　
　・返り値
　　正常に取得できた場合：200 OK ／ Result: 取得できたパス
　　対応していない場合：204 No Contents ／ Result: 空
　　その他ひどいことをした場合：400 Bad Request

■配布条件等

license.txtを見てください。

■更新履歴

・2009/2/17 Initial Release

