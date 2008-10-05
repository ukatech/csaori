----------------------------------------------------------------------
■「XPath」：ファイルのバージョン情報取得SAORI
■Written by CSaori Project
　http://code.google.com/p/csaori/
----------------------------------------------------------------------

■これは何をするものか

　エクスプローラでファイルのプロパティを開くと出てくるバージョン情報を
　取得するためのSAORIです。

■動作環境

・Win95/NT4以上

■使用方法

・呼び出し
　Argument0　　バージョンリソースを含むファイルのパス
　環境変数を展開可能です。例：%SystemRoot%\system32\Macromed\Flash\npswf32.dll

・結果
　Result
　　ERROR_OPEN　ファイルない！
　　ERROR_NORESOURCE　バージョン情報ない！
　　ERROR_NOINFO　情報取得失敗！
　　
　　数字.数字.数字.数字　バージョン番号　例：9.0.124.0
　
　以下成功時のみ
　Value0　製品名
　Value1　説明
　Value2　コメント
　Value3　著作権
　Value4　商標

■配布条件等

license.txtを見てください。

■更新履歴

・2008/7/2 Initial Release

