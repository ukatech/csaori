----------------------------------------------------------------------
■「kisaragi」：MeCabラッパーSAORI
■Written by CSaori Project (C.Ponapalt)
　http://code.google.com/p/csaori/
----------------------------------------------------------------------

■これは何をするものか

　日本語形態素解析ソフト"MeCab"をSAORI規格DLLとして使えるようにしたものです。
　
　さとー氏製作のmutsuki.dllをもっと手軽に扱えるように改良する目的で生まれました。
　睦月の次ですから、旧月名の「如月」というわけです。
　
　mutsuki.dllからの移行は、おおむねそのまま置き換えだけで済むようになっています。

■動作環境

・Win2000以上

■使用方法

kisaragi.dll
libmecab.dll
dicフォルダの中身(単語辞書)

以上3つの要素を同じフォルダの中に入れて、kisaragi.dllをSAORIとして呼び出してください。

ゴーストの一部として配布する際は、license.txtを確認し、中身をreadmeなどに明記してください。

○parse

  引数
    Argument0: parse
    Argument1: 解析する文字列
    Argument2: 出力フォーマット(省略可能)

  戻り値(Result)
    -1:    出力フォーマットが異常だった
    0以上: 出力結果の行数

  戻り値(Value*)
    解析結果

  機能
    文字列をMeCabに渡し、形態素解析を行います。出力フォーマットはChaSen互換です。
    Argument2で出力フォーマットを指定できます。
    出力結果はValue*に行数ことに分解して返します。戻り値は行数です。

  例
    Argument0: parse
    Argument1: 如月と申します。お側に置いてくださいね。
    　↓
    Result: 13
    Value0: 如月	キサラギ	如月	名詞-一般		
    Value1: と	ト	と	助詞-格助詞-引用		
    Value2: 申し	モウシ	申す	動詞-自立	五段・サ行	連用形
    Value3: ます	マス	ます	助動詞	特殊・マス	基本形
    Value4: 。	。	。	記号-句点		
    Value5: お側	オソバ	お側	名詞-一般		
    Value6: に	ニ	に	助詞-格助詞-一般		
    Value7: 置い	オイ	置く	動詞-自立	五段・カ行イ音便	連用タ接続
    Value8: て	テ	て	助詞-接続助詞		
    Value9: ください	クダサイ	くださる	動詞-非自立	五段・ラ行特殊	命令ｉ
    Value10: ね	ネ	ね	助詞-終助詞		
    Value11: 。	。	。	記号-句点		
    Value12: EOS


○parse-mecab

  parseと同じですが、出力フォーマットがMeCabネイティブとなります。
  フォーマットの違いについては、例えば下記が参考になります。
  http://d.hatena.ne.jp/Kshi_Kshi/20110102/1293920002


○wakati

  引数
    Argument0: parse
    Argument1: 解析する文字列

  戻り値(Result)
    -1:    出力フォーマットが異常だった
    0以上: 出力結果の行数

  戻り値(Value*)
    解析結果

  機能
    半角空白区切りで分かち書きをします。

  例
    Argument0: wakati
    Argument1: 見て見て、この輝く肌。あはっ、もっと近くで見てよ。
    　↓
    Result: 1
    Value0: 見 て 見 て 、 この 輝く 肌 。 あ はっ 、 もっと 近く で 見 て よ 。 


○yomi

  引数
    Argument0: yomi
    Argument1: 解析する文字列

  戻り値(Result)
    -1:    出力フォーマットが異常だった
    0以上: 出力結果の行数

  戻り値(Value*)
    解析結果

  機能
    カタカナで読み仮名を振ります。

  例
    Argument0: yomi
    Argument1: 如月、出撃します。
    　↓
    Result: 1
    Value0: キサラギ、シュツゲキシマス。


■注意事項

dic以下の辞書を別のものに置き換える際は、現状必ずUTF-8でコンパイルしたものを使ってください。
(v1.1でSJISから変更しました)

NEologdを使いたい場合は以下のURLからコンパイル済辞書をダウンロードできます。
http://ssp.shillest.net/etc/mecab-ipadic-neologd-dic.zip
中身をdicフォルダ以下にそのまま上書きすることで使用できます。
ただし、とても大きなファイルになるため、使用する際は十分検討を重ねた上でどうぞ。

NEologdについての詳細な情報は以下をご覧ください。
https://github.com/neologd/mecab-ipadic-neologd/blob/master/README.ja.md


■配布条件等

license.txtを見てください。


■更新履歴

・2016/11/26 初版

・2017/5/27 v1.1
　Mecab未インストール環境でうまく動かない場合がある問題を修正
　辞書の文字コードをUTF-8に変更
　NEologdで動作検証

・2017/9/10 v1.1.1
　dicrcまでのパスに2バイト文字が含まれるとうまく読み込まない問題を修正

