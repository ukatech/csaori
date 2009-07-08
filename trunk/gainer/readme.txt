----------------------------------------------------------------------
■「gainer」：現在開いているウインドウを列挙するSAORI
■Written by CSaori Project / original by 櫛ヶ浜やぎ
　http://code.google.com/p/csaori/
----------------------------------------------------------------------

■これは何をするものか

　現在開いているウインドウを列挙するためのSAORIです。

■動作環境

・Win95/NT4以上

■使用方法

[汎用コマンド]
必ず1パラメータ以上必要です。
1つめのパラメータ(Argument0)は「コマンド」です。以降の項目名を設定して下さい。

□search
・機能
　Gainerが接続されたポート番号を返します。

・結果
　Result：列挙できたポートの総数 / 0は見つからなかった
　Value0～：順番にポート番号

□set.hwnd
・機能
　Gainerからの結果をSSTPで通知するためのhwndです（ボタン状態など）。
　不要な場合は設定する必要はありません。

・パラメータ
　Argument1：SSTPを受け付けるウインドウのHWND

・結果
　Result：OK

[Gainer用コマンド群]
必ず2パラメータ以上必要です。
1つめのパラメータ(Argument0)は「コマンド」です。以降の項目名を設定して下さい。
2つめのパラメータ(Argument1)は「シリアルポート番号」です。
1から128までの数値を指定するか、あるいは0を指定すると勝手にGainerを探索します。

□set.config
・機能
　Gainerのモードを設定します(KONFIGURATION_n命令)
　
・パラメータ
　Argument2：モード番号(1～8をサポート)

・結果
　Result：OK/NG

□get.version
・機能
　Gainerのファームウェアのバージョンを取得します。
　
・結果
　Result：ファームウェアのバージョン番号

□in.analog.all
・機能
　アナログ入力ピンの状態をすべて取得します

・結果
　Result：OK/NG
　Value0～：

□out.analog.single
・機能
　1つのアナログ出力ピンへの出力電圧を変更します。

・パラメータ
　Argument2：ピン番号
　Argument3：値(0～255)

・結果
　Result：OK/NG

□in.digital.all
□in.digital.all.bit
・機能
　デジタル入力ピンの状態をすべて取得します

・結果
　Result：OK/NG
　Value0：ピン0を最下位ビットとして解釈した場合の数値
　Value0～：(.bit)ピン0から順番に0/1

□out.digital.all
□out.digital.all.bit
・機能
　デジタル出力ピンの状態をすべて変更します

・パラメータ
　Argument2：ピン0を最下位ビットとして解釈した場合の数値
　Argument2～：(.bit)ピン0から順番に0/1

□out.digital.single
・機能
　1つのデジタル出力ピンへの出力フラグを変更します。

・パラメータ
　Argument2：ピン番号
　Argument3：0/1

・結果
　Result：OK/NG

□out.servo.single
・機能
　1つのRCサーボ制御ピンへの出力情報を変更します。
　[注意] Gainer mini config=8 のみのサポートです。

・パラメータ
　Argument2：ピン番号
　Argument3：0～255

・結果
　Result：OK/NG

□out.led
・機能
　Gainer基盤上のLEDを光らせます

・パラメータ
　Argument2：1で点灯、0で消灯

・結果
　Result：OK/NG

□set.pga.dgnd
・機能
　0v基準で入力アナログ信号の倍率を設定します。

・パラメータ
　Argument2：（1.0～48.0）
　1.00,1.14,1.33,1.46,1.60,1.78,2.00,2.27
　2.67,3.20,4.00,5.33,8.00,16.0,24.0,48.0　から選択します。
　中間の値を設定すると適当な近い値を選びます。

・結果
　Result：OK/NG

□set.pga.agnd
・機能
　2.5v基準で入力アナログ信号の倍率を設定します。

・パラメータ
　Argument2：倍率（1.0～48.0）
　1.00,1.14,1.33,1.46,1.60,1.78,2.00,2.27
　2.67,3.20,4.00,5.33,8.00,16.0,24.0,48.0　から選択します。
　中間の値を設定すると適当な近い値を選びます。

・結果
　Result：OK/NG

■配布条件等

license.txtを見てください。

■更新履歴

・2008/8/16 Initial Release

