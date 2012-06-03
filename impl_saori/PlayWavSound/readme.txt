----------------------------------------------------------------------
■「PlayWavSound」：効果音再生
■Written by CSaori Project
　http://code.google.com/p/csaori/
----------------------------------------------------------------------

■これは何をするものか

　効果音再生用に、WAVEファイルを再生するSAORIです。
　システムの標準機能をただ呼び出すだけですが…
　WAVEしか使えない代わりに、効果音として使える程度のレスポンスが期待できます。

■動作環境

・Win95/NT4以上

■使用方法

・呼び出し
　Argument0
　　system:ではじまるもの＝システム登録済み警告音
　　　例：system:SystemExclamation -> 警告ダイアログの音
　　　　　レジストリエディタで HKEY_CURRENT_USER\AppEvents\EventLabels 以下を見ると幸せになれるかもしれません。

　　その他＝WAVEファイルのパス
　　　環境変数を展開可能です。例：%SystemRoot%\Media\chimes.wav
　　　本DLLからの相対パスでも指定可能です。例：..\sound\type1.wav

・結果
　Result
　　1：成功　0：失敗

■配布条件等

license.txtを見てください。

■更新履歴

・2012/6/4 Initial Release

