----------------------------------------------------------------------
■「gainer」：Gainerまたはその互換用ボードを制御するSAORI
■Written by CSaori Project
　http://code.google.com/p/csaori/
----------------------------------------------------------------------

■これは何をするものか

　Gainerまたはその互換用ボードを制御するSAORIです。

■動作環境

・Win95/NT4以上

■使用方法

■汎用コマンド
必ず1パラメータ以上必要です。
1つめのパラメータ(Argument0)は「コマンド」です。以降の項目名を設定して下さい。


□search
・機能
　Gainerが接続されたポート番号を返します。

・結果
　Result：列挙できたポートの総数 / 0は見つからなかった
　Value0～：順番にポート番号


□hwnd
・機能
　Gainerからの結果をSSTPで通知するためのhwndです（ボタン状態など）。
　不要な場合は設定する必要はありません。
　（イベント内容後述）

・パラメータ
　Argument1：SSTPを受け付けるウインドウのHWND

・結果
　Result：OK


■Gainer用コマンド群
必ず2パラメータ以上必要です。
1つめのパラメータ(Argument0)は「コマンド」です。以降の項目名を設定して下さい。
2つめのパラメータ(Argument1)は「シリアルポート番号」です。
1から128までの数値を指定可能です。
また、0を指定すると勝手にGainerを探索し、最初に見つかったものを制御します。


□configuration
・機能
　Gainerのモードを設定します(KONFIGURATION_n命令)
　
・パラメータ
　Argument2：モード番号(1～8をサポート)

・結果
　Result：OK/NG:理由:理由


□version
・機能
　Gainerのファームウェアのバージョンを取得します。
　
・結果
　Result：ファームウェアのバージョン番号


□reboot
・機能
　Gainerを再初期化します（モード0）。
　
・結果
　Result：OK/NG:理由


□disconnect
・機能
　Gainerから切断します。
　
・結果
　Result：OK


□peekAnalogInput
・機能
　アナログ入力ピンの入力電圧をすべて取得します

・結果
　Result：OK/NG:理由
　Value0～：ピン0から順番に 0～255


□beginAnalogInput
・機能
　アナログ入力ピンの入力電圧をすべて取得します
　hwndで指定したウインドウに取得ごとにイベントを送ります。（イベント内容後述）

・パラメータ
　Argument2：取得間隔（ミリ秒）　省略かあるいは0を指定すると「ありったけ」

・結果
　Result：OK/NG:理由


□endAnalogInput
・機能
　beginAnalogInputを終了します。

・結果
　Result：OK/NG:理由


□analogOutput
・機能
　アナログ出力ピンへの出力電圧を変更します。

・パラメータ
　Argument2～：ピン0からのデータ～

・結果
　Result：OK/NG:理由


□analogOutputSingle
・機能
　1つのアナログ出力ピンへの出力電圧を変更します。

・パラメータ
　Argument2：ピン番号
　Argument3：値(0～255)

・結果
　Result：OK/NG:理由


□peekDigitalInput
・機能
　デジタル入力ピンの状態をすべて取得します

・結果
　Result：OK/NG:理由
　Value0～：ピン0から順番に 0/1


□beginDigitalInput
・機能
　デジタル入力ピンの状態をすべて取得します
　hwndで指定したウインドウに取得ごとにイベントを送ります。（イベント内容後述）

・パラメータ
　Argument2：取得間隔（ミリ秒）　省略かあるいは0を指定すると「ありったけ」

・結果
　Result：OK/NG:理由


□endDigitalInput
・機能
　beginDigitalInputを終了します。

・結果
　Result：OK/NG:理由


□digitalOutput
・機能
　デジタル出力ピンの状態をすべて変更します

・パラメータ
　Argument2～：ピン0から順番に0/1～


□digitalOutputSingle
□setLow
□setHigh
・機能
　1つのデジタル出力ピンへの出力フラグを変更します。

・パラメータ
　Argument2：ピン番号
　Argument3：0/1（digitalOutputSingleの場合）／なし（setLow／setHighの場合）

・結果
　Result：OK/NG:理由


□servoOutput
・機能
　1つのRCサーボ制御ピンへの出力情報を変更します。
　[注意] Gainer mini config=8 のみのサポートです。

・パラメータ
　Argument2～：ピン0から順番に0-255～

・結果
　Result：OK/NG:理由


□servoOutputSingle
・機能
　1つのRCサーボ制御ピンへの出力情報を変更します。
　[注意] Gainer mini config=8 のみのサポートです。

・パラメータ
　Argument2：ピン番号
　Argument3：0～255

・結果
　Result：OK/NG:理由


□turnOnLED
□turnOffLED
・機能
　Gainer基盤上のLEDを光らせます/消灯させます

・結果
　Result：OK/NG:理由


□ampGainDGND
・機能
　0v基準で入力アナログ信号の倍率を設定します。

・パラメータ
　Argument2：（1.0～48.0）
　1.00,1.14,1.33,1.46,1.60,1.78,2.00,2.27
　2.67,3.20,4.00,5.33,8.00,16.0,24.0,48.0　から選択します。
　中間の値を設定すると適当な近い値を選びます。

・結果
　Result：OK/NG:理由


□ampGainAGND
・機能
　2.5v基準で入力アナログ信号の倍率を設定します。

・パラメータ
　Argument2：倍率（1.0～48.0）
　1.00,1.14,1.33,1.46,1.60,1.78,2.00,2.27
　2.67,3.20,4.00,5.33,8.00,16.0,24.0,48.0　から選択します。
　中間の値を設定すると適当な近い値を選びます。

・結果
　Result：OK/NG:理由


□scanLine
・機能
　LEDアレイへの出力を1行分セットします。
　[注意] Gainer config=7 のみのサポートです。

・パラメータ
　Argument2：行番号0～8
　Argument3：0～15の数値をカンマ区切りで8つ（例：1,3,5,7,9,11,13,15）

・結果
　Result：OK/NG:理由


□scanMatrix
・機能
　LEDアレイへの出力を全部セットします。
　[注意] Gainer config=7 のみのサポートです。

・パラメータ
　Argument2～9：0～15の数値をカンマ区切りで8つ（例：1,3,5,7,9,11,13,15）

・結果
　Result：OK/NG:理由


□scan7segLED
・機能
　LEDアレイに7セグLED(数字を表示するLED)をつけた際に、指定した数字を表示します。
　[注意] Gainer config=7 のみのサポートです。
　　　　 
　　　　 row0～6にカソードコモン7セグLEDのa～gピン、row7に*ピン、
　　　　 col0～7にそれぞれ1～8桁目のカソード端子をぶら下げてドライブした際に、
　　　　 8桁目(右端)を数値1桁目として表示します。頭の0は消灯します。
　　　　 上記仕様のため、8桁に満たない場合はcol7側に詰めて接続して下さい。

・パラメータ
　Argument2：数字列 8桁まで

・結果
　Result：OK/NG:理由



■イベント通知
beginDigitalInput / AnalogInputで連続取得中のデータは、SSTPを使い本体に通知されます。
hwndコマンドであらかじめSSTPを受信可能なウインドウハンドルを設定してください。

[注意]送信元hwndが存在しませんのでHWnd: 0になっています。
　　　動作はそれをサポートする本体(SSPなど)に限定されます。


□OnGainerAnalogInput
アナログピン入力データを受信しました。

イベントID：OnGainerAnalogInput
Reference0：COMポート番号
Reference1：データをカンマ区切りで

　サンプル：
　　NOTIFY SSTP/1.1
　　Charset: Shift_JIS
　　Sender: GainerSAORI
　　HWnd: 0
　　Event: OnGainerAnalogInput
　　Reference0: 10
　　Reference1: 66,56,47,38


□OnGainerDigitalInput
デジタルピン入力データを受信しました。

イベントID：OnGainerDigitalInput
Reference0：COMポート番号
Reference1：データをカンマ区切りで

　サンプル：
　　NOTIFY SSTP/1.1
　　Charset: Shift_JIS
　　Sender: GainerSAORI
　　HWnd: 0
　　Event: OnGainerDigitalInput
　　Reference0: 10
　　Reference1: 0,0,0,0


□OnGainerButtonPressed
Gainer本体のボタンが押されました。

イベントID：OnGainerButtonPressed
Reference0：COMポート番号

　サンプル：
　　NOTIFY SSTP/1.1
　　Charset: Shift_JIS
　　Sender: GainerSAORI
　　HWnd: 0
　　Event: OnGainerButtonPressed
　　Reference0: 10


□OnGainerButtonReleased
Gainer本体のボタンが離されました。

イベントID：OnGainerButtonReleased
Reference0：COMポート番号

　サンプル：
　　NOTIFY SSTP/1.1
　　Charset: Shift_JIS
　　Sender: GainerSAORI
　　HWnd: 0
　　Event: OnGainerButtonReleased
　　Reference0: 10


■配布条件等

license.txtを見てください。

■更新履歴

Version 1.0.0.0 2009/7/10

    * 新規作成 (ponapalt) 

Version 1.0.1.0 2009/7/11

    * OnGainerDigitalInput? のRef1が0/1のはずがそれ以外の値も出していた問題を修正 

Version 1.1.0.0 2009/7/12

    * disconnect と scan7segLED を追加 

Version 1.1.1.0 2009/7/14

    * 通信パラメータの誤りを修正
    * configureが一発しか打てない問題を修正

Version 1.1.2.0 2009/7/16

    * COMポートをポーリングするコードを修正
    * SecurityLevelヘッダの解釈に問題があった点を修正 (CSAORI Core) 

