-----------------------------------------------------------------------------------
■「CrystalDiskInfoEvent」：常駐しているCrystalDiskInfoから情報を拝借するプラグイン
■Written by CSaori Project
　http://code.google.com/p/csaori/
-----------------------------------------------------------------------------------

■これは何をするものか

☆建前
　CrystalDiskInfo 5.X系から、「ガジェット対応」なる機能が追加されていると知り、
　
　ガジェットで取れるんなら他のアプリでも取れるんじゃ→とりあえずどこから取れるか検索だ→
　練れば練るほど色が変わって→こうやってつけて→うまい！（テーレッテレー）
　
　のような経緯でできたものです。
　常駐するCrystalDiskInfoの情報をゴーストで拝借することができます。

★本音
　野郎ども！時代の先端(笑)を走る萌えDA集団として、水晶 雫ちゃんとガチバトルだちくしょう！

■動作環境

・SSPと同じ

■使い方

CrystalDiskInfoの[機能]メニューから、

・[上級者向け機能]→[ガジェット対応]

をONにしてください。また、お好みに応じて下記の設定をどうぞ。

・[常駐]
・[スタートアップ]

■ゴーストからの利用方法

・OnCrystalDiskInfoEvent
　ゴースト起動時に情報がある場合/情報が更新された時 に通知されます。

　Reference0: (Good|Caution|Bad|Unknown),YYYY,NN,DD,HH,MM,SS
　　ディスク群全体のステータス（＝最も悪いものの情報）と、更新日付です。カンマ区切り。
　　例：Good,2012,6,22,5,17,10

　Reference1～: 名前,ドライブレター,(Good|Caution|Bad|Unknown),容量,温度,temperature(Good|Caution|Bad|Unknown)
　　ディスク個別ステータスです。カンマ区切り。
　　3番目はディスク全体の診断情報、6番目は温度警告です。
　　例：TOSHIBA MK6476GSX,C: D:,Good,640.1GB,31 ℃,temperatureGood


・OnCrystalDiskInfoClear
　今まで情報があったのに消えてしまった時に通知されます。Referenceはありません。
　CrystalDiskInfoが終了した等で情報が取れなくなった、という意味になります。


■配布条件等

license.txtを見てください。

■更新履歴

・2012/6/22 1.0
　First Release

