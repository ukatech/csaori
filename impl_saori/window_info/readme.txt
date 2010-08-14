----------------------------------------------------------------------
■「window_info」：現在開いているウインドウを列挙・情報取得するSAORI
■Written by CSaori Project
　http://code.google.com/p/csaori/
----------------------------------------------------------------------

■これは何をするものか

　現在開いているウインドウを列挙・情報取得するためのSAORIです。

■動作環境

・Win98/NT4以上

■使用方法

・呼び出し：
　Argument0　　指定なし、もしくはenum

・結果
　Result：列挙できたウインドウの枚数
　Value0～：順番にウインドウタイトル


・呼び出し：
　Argument0　　info
　Argument1　　hwndの値

・結果
　Result：(なし)
　Value0：ウインドウタイトル
　Value1：クラス名
　Value2：ウインドウ座標(左,上,右,下)
　Value3：クライアント座標(左,上,右,下)
　Value4：ウインドウが属するワーク領域の座標(左,上,右,下)
　Value5：ウインドウスタイル
　Value6：ウインドウ拡張スタイル
　Value7：親ウインドウのhwnd トップレベルなら0
　Value8：normal(通常)/minimized(最小化)/maximized(最大化)

■配布条件等

license.txtを見てください。

■更新履歴

・2008/8/16 Initial Release (櫛ヶ浜やぎ氏のenumwindowsの代替品として)
・2010/8/14 機能拡張：info

