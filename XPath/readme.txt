----------------------------------------------------------------------
■「XAORIK（ざおりく）」：XML解析SAORI
■Written by CSaori Project
　http://code.google.com/p/csaori/
----------------------------------------------------------------------

■これは何をするものか

　XMLを読み込み、解析し、中の情報をXPathを使って指定して取り出すSAORIです。

■動作環境

・Win95/NT4以上

■使用方法

□ファイルを開く
・呼び出し
　Argument0　　open
　Argument1　　開きたいXMLファイルのフルパス

・結果
　成功時200 OK、失敗時400エラー
　Result　　ファイルID（解析＋閉じる時に使いますのでどこかに保存してください）

□ファイルを閉じる
・呼び出し
　Argument0　　close
　Argument1　　ファイルID

・結果
　成功時204、失敗時400エラー

□内容を取得する
・呼び出し
　Argument0　　get
　Argument1　　ファイルID
　Argument2　　XPath

・結果
　成功時200 OK、妙なXPathを食わせると500、そのほかの失敗時400エラー
　Result　　解析結果を文字列で

XPathやその他XMLの概念については
http://www6.airnet.ne.jp/manyo/xml/
このあたりが詳しいです。

■配布条件等

license.txtを見てください。

■更新履歴

・2008/12/30 1.0
　Initial Release

・2009/02/23 1.1
　/hoge/@fuga のようにAttributeノードを指定した場合、自動で内容テキストを取るようにした
　今まではきちんと /hoge/@fuga/text() のように指定しないとだめだった

・2009/02/23 1.1.1
　上記変更で通常のテキストノード自動選択処理が動かなくなっていたエンバグ修正

