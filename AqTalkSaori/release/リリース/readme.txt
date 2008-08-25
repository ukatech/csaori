----------------------------------------------------------------------
■「AquesTalkSaori」：AquesTalkによる音声合成SAORI
■Written by ukiya
　mailto:ukiya@s.mail-box.ne.jp
　http://ukiya.sakura.ne.jp/
----------------------------------------------------------------------

■これは何をするものか

　株式会社アクエストの開発した、AquesTalkという音声合成エンジンを利用して、
　音声合成を行うSAORIです。

■動作環境

　・AqTalkSaori.dll、AquesTalk.dll、を同じディレクトリにいれてください。
　・デフォルトでは女性の声ですが、AquesTalk.dll を差し替えることによって
　　別の声になります。同梱の「他の声」ディレクトリを参照してください。

　・（参考）開発環境：Windows Vista Ultimate + Core2 2.13GHz + 2GB Memory
　　　　　  C++ on VisualStudio.NET2005


■使用方法

いずれの機能も、音声記号列の入力が必要となります。
音声記号列とは、主にひらがなで構成された、発音の文字列です。
詳しくは同梱の「音声記号列仕様.pdf」を参照してください。

（１）PLAYSYNC

【機　　能】
　　入力した音声記号列をその場で再生する。再生完了まで実行がストップします。
【引　　数】
　　Argument0: PLAYSYNC
　　Argument1: 音声記号列
　　Argument2: スピード（省略可能）（50～300、省略時100）
【注意事項】
　　・再生がおわるまで制御が戻りません。
【例】
　	_result=FUNCTIONEX("AqTalkSaori.dll","PLAYSYNC","ほげほげへもへも");

（２）CREATEWAV

【機　　能】
　　入力した音声記号列をWAVファイルにする。
【引　　数】
　　Argument0: CREATEWAV
　　Argument1: 音声記号列
　　Argument2: スピード（省略できません）（50～300、普通は100）
　　Argument3: ファイル名（拡張子含む）
【注意事項】
　　・ファイルはDLLのあるディレクトリに作成されます。
　　・作成されるWAVファイルは以下のフォーマットです。
　　　8kHz 16bit モノラル リニアPCM WAV形式
　　・そのファイルが既に開かれている場合は作成できずにエラーとなります。
【例】
//	_result=FUNCTIONEX("AqTalkSaori.dll","CREATEWAV","ほげへもほげへも",100,"hoge.wav");

（３）PLAYASYNC

【機　　能】
　　入力した音声記号列をその場で再生する。再生完了を待たずに制御を返します。
【引　　数】
　　Argument0: PLAYASYNC
　　Argument1: 音声記号列
　　Argument2: スピード（省略可能）（50～300、省略時100）
【注意事項】
　　・再生をとめたい場合はSTOPコマンドを利用してください。
　　・再生中にPLAYSYNC,PLAYASYNC,CREATEWAVコマンドを実行すると再生が止まります。
【例】
　	_result=FUNCTIONEX("AqTalkSaori.dll","PLAYASYNC","ほげほげへもへも");

（４）STOP

【機　　能】
　　PLAYASYNCで再生中の音声を停止する。
【引　　数】
　　Argument0: STOP
【注意事項】
【例】
　	_result=FUNCTIONEX("AqTalkSaori.dll","STOP);

（５）CHANGEVOICE

【機　　能】
　　実行中に利用するDLLを差し替える。
【引　　数】
　　Argument0: CHANGEVOICE
　　Argument1: DLLファイル名
【注意事項】
　　・差し替え対象のファイル名は AqTalkSaori.dll と同じディレクトリにおいてください。
【例】
　	_result=FUNCTIONEX("AqTalkSaori.dll","CHANGEVOICE","m1.dll");


■エラーコード

SAORIが返す可能性のあるエラーを列挙します。

ERROR001：引数がありません
ERROR301：DLLファイルが指定されていません
ERROR302：DLLファイルのロードに失敗しました（DLLファイル名）
ERROR003：不明な命令です
ERROR999：AquesDLLロードに失敗しています
ERROR002：最低２つ以上の引数を指定してください
ERROR100：Syntheのエラー(エラーコード)
ERROR101：mallocのエラー
ERROR201：ファイル名が指定されていません
ERROR202：ファイルを開くのに失敗しました


「ERROR100：Syntheのエラー(エラーコード)」が返った場合は、
AquesTalk内部のエラーです。以下のエラーコードを参考にしてください。
	100	その他のエラー
	101	メモリ不足
	102	音声記号列に未定義の読み記号が指定された
	103	韻律データの時間長がマイナスになっている
	104	内部エラー（未定義の区切りコード検出）
	105	音声記号列に未定義の読み記号が指定された
	106	音声記号列のタグの指定が正しくない
	107	タグの長さが制限を越えている
	108	タグ内の値の指定が正しくない
	109	WAVE再生ができない（サウンドドライバ関連の問題）
	110	本プログラムでは未使用
	111	発声すべきデータがない
	200	音声記号列が長すぎる
	201	１つのフレーズ中の読み記号が多すぎる
	202	音声記号列が長い（内部バッファオーバー１）
	203	ヒープメモリ不足
	204	音声記号列が長い（内部バッファオーバー１）

■配布条件等

以下のDLLファイル、ドキュメントは株式会社アクエストの著作物です。
それらのファイルの配布条件は、同梱のAqLicence.txtに従って下さい。

・AqLicence.txt
・AquesTalk.dll および「他の声」ディレクトリにあるDLL
・音声記号列仕様.pdf

それ以外のファイルは、浮子屋の著作物です。
それらについてはお好きなようにどうぞ。

■■■■■■■■■■■■■■　注意　■■■■■■■■■■■■■■■■■■■■■
このセットをゴーストに利用して配布する際は、DLLと同じ場所にAqLicence.txtを含め、
以下の文をゴーストのReadMeかどこかに記載してください。
それでライセンスを満たすと思います。

・このゴーストは、株式会社アクエストの規則音声合成ライブラリAquesTalkを使用しています。
・AquesTalk 規則音声合成ライブラリ AquesTalk.dll AquesTalkDa.dll は、株式会社アクエスト の著作物です。
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■


■更新履歴

2008/02/02	ver1.0.0.0	新規作成
2008/02/02	ver1.0.1.0	里々で辞書リロードを行うとライブラリがロードできない問題に対応
2008/02/02	ver1.0.2.0	ファイル出力のパスを明示
2008/03/01	ver1.1.0.0	PLAYASYNC、STOP、CHANGEVOICEを追加。CREATEWAVの引数順序変更。
2008/03/01	ver1.1.1.0	CREATEWAVが正常に動かない問題を修正、エラー時に落ちる問題を修正

■Thanks to

　・このプログラムは、音声合成に AquesTalk 規則音声合成ライブラリ を使用しています。
　・本プログラムでは AquesTalk.dll を利用しています。
　・AquesTalk 規則音声合成ライブラリ AquesTalk.dll は、株式会社アクエスト の著作物です。
　・AquesTalk 規則音声合成ライブラリのライセンスは AqLicence.txt を参照ください。
　・http://www.a-quest.com/aquestalk/index_win.html

　素晴しいソフトウェアを開発・公開されている皆様に深く感謝致します。

