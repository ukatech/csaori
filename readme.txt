----------------------------------------------------------------------
■「CSAORI」：SAORIサンプルクラス
■By CSAORI Project Team
http://code.google.com/p/csaori/
----------------------------------------------------------------------

■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■これは何をするものか
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

　SAORIのサンプルクラスです。

■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■動作環境
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

・VisualStudio.NET2005　など

■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■使用方法
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

■１．基本的構造■

CSAORIは、以下の構成からなっています。（→は派生）

CSaori_base クラス　　「SAORI的」DLLインタフェースを規定するクラス
→CSaori クラス　　　 SAORI DLLを規定するクラス 
→CPlugin クラス　　　Plugin DLLを規定するクラス

CSAORIInput クラス　　ベースウェアからSAORIへの呼び出しパラメータを
　　　　　　　　　　　表現するクラス
CSAORIOutputクラス　　SAORIからベースウェアへの返却値を表現するクラス

SAORI_FUNC名前空間　　SAORI作成に有用と思われるユーティリティ関数を
　　　　　　　　　　　集めたクラス

■２．最も基本的な利用法■

最も簡単なCSAORIを使ったSAORI DLLのソースコードは、下記のようになります。
（同梱のSAORITESTプロジェクトを参考にしてください）
単純な処理であれば、CSAORIクラスからクラスの派生を行う必要はありません。

----------------------------------------------------------------------
SaoriTest.cpp
----------------------------------------------------------------------
#include "csaori.h"
bool CSAORI::load(){
	return true;
}
bool CSAORI::unload(){
	return true;
}
void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out){
	out.result_code = SAORIRESULT_OK;

	out.result = getModulePath();
	out.result += in.args[0];
	out.result += in.args[1];
}
----------------------------------------------------------------------

（１）おまじない

CSAORIを利用するには、開発環境でcsaori.cpp、csaori_base.cpp、及び両フ
ァイルの.hを参照設定した上で、自分で作るソースファイル（hoge.cppとしま
す）の先頭で、csaori.hをincludeしてください。
また、リンクの設定で、user32.libを参照設定してください。

（２）利用者が実装しなければならない関数

利用者が実装しなければならない関数は、load、unload、execです。

① load

SAORI DLLがロードされた際に呼び出されます。
初期化などの処理を行って下さい。処理に成功したら、trueを返して下さい。

② unload

SAORI DLLがアンロードされた際に呼び出されます。
終了などの処理を行って下さい。処理に成功したら、trueを返して下さい。

③ exec

SAORI DLLが（SHIORIなどの実行系から）実際にリクエストされた際に
呼び出されます。
引数として CSAORIInputへの参照、返却値としてCSAORIOutputへの参照を
持っています。
CSAORIInputを見て処理を行ったら、CSAORIOutputのresult_code、及び
resultを適切に設定して下さい。

■３．CSAORIが行ってくれること■

CSAORIは、以下の処理を行います。

・SAORI DLLのインタフェースの実装
・SHIORIなどの実行系からの呼び出し文字列をCSAORIInputに纏める
・execした後のCSAORIOutputをSHIORIに返す

なお、CSAORI内部では、文字コードはUNICODE（wchar_t）を用いています。
実行系からの呼び出しに応じて、UNICODEとの相互変換を行いますので、
利用者側は、load,unload,execの実装にあたって文字コードを意識する必要は
あまりありません。全てUNICODEでやり取りされると思って下さい。

■４．CSAORI　簡易リファレンス■

本項は、CSAORIのソースを見るのにあたって、何がどこにあるかを理解する手
助けの為に、利用者が知っておくべきだと思われる事項について記載したもの
です。完全なリファレンスは現在存在しませんので、ソースコードを確認して
ください。

----------------------------------------------------------------------
■string_t,char_t（CSaori_Base.h）
----------------------------------------------------------------------

CSAORIでは文字及び文字列を表すのに、char_t 及び string_t という名前を
利用しています。

----------------------------------------------------------------------
■SAORIRESULT（CSaori_Base.h）
----------------------------------------------------------------------

SAORIのリザルトコードを定義しています。execの処理では、必ずCSAORIOutput
のresult_codeに、この列挙体のどれかを設定してください。

----------------------------------------------------------------------
■グローバル pSaori オブジェクト（CSaori_base.cpp及びCSaori.h）
----------------------------------------------------------------------

CSAORIBase* pSaori;
	CSAORIの唯一のオブジェクトが格納されます。DLLがロードされている間
	このオブジェクトが存在し、モジュールパス等の情報を保持します。csa
	ori.h で extern されているため、利用者側からは、例えば pSaori->get
	ModulePath() 等とすることで、CSAORIオブジェクトの機能にアクセスで
	きます。

----------------------------------------------------------------------
■SAORI_FUNC名前空間の関数
----------------------------------------------------------------------

UnicodeToMultiByte
	string_t をShift-JIS等のマルチバイト文字列に変換します。

MultiByteToUnicode
	上記の逆です。

CodePagetoString
	Windowsコードページを、文字コードを表す文字列に変換します。

StringtoCodePage
	上記の逆です。

CP番号	CODEPAGE		文字列（大文字小文字は無視されます）
932		CP_SJIS			Shift_JIS または x-sjis
50220	CP_ISO2022JP	ISO-2022-JP
20932	CP_EUCJP		EUC-JP または x-euc-jp
65001	CP_UTF8			UTF-8


getResultString
	SAORIリザルトコード番号を、それを表す文字列に変換します。
	（例）	200	→	OK
			204	→	No Content

getLine
	文字列を行単位に切り出します。

intToString
	数値を文字列に変換します。

AsyncMessageBox
	非同期にメッセージBOXを表示します。

CCriticalSection
	クリティカルセクションを扱うクラスです。

----------------------------------------------------------------------
■CSAORIInputクラス（Csaori_base.h　及び .cpp）
----------------------------------------------------------------------

vector_str args;
	リクエストの引数がstring_t（UNICODE文字列）の配列で入っています。
	args[0] 等としてアクセスできます。


------ 以下のメンバは、利用者はあまり意識する必要はありません。 ------

unsigned int codepage;
	リクエストの元の文字コードのコードページ番号が入っています。

string_t cmd;
	GET Version SAORI/1.0やEXECUTE SAORI/1.0といった、リクエストの最初
	の行が入っています。

string_t id;
	リクエストに ID: があれば、それが入っています。

map_strpair opts;
	その他のリクエストヘッダが、key-valueペアで入っています。

bool parseString(const string_t &src);
	リクエスト文字列を解析し、argsやopts等のメンバーを設定する処理です。

----------------------------------------------------------------------
■CSAORIOutputクラス（Csaori_base.h　及び .cpp）
----------------------------------------------------------------------

SAORIRESULT result_code;
	SAORI/1.0 200 OK 等のリザルトコードを返すために、SAORIRESULT列挙体
	のどれかを設定してください。

string_t result;
	Result: 行で返す内容を設定してください。

vector_str values;
	Values: 行で返す内容を設定してください。

------ 以下のメンバは、利用者はあまり意識する必要はありません。 ------

map_strpair opts;
	ResultやValues以外のリザルトヘッダを設定したい場合に用います。

unsigned int codepage;
	自動的にリクエストの文字コードにあわせて設定されます。

void setResultEmpty();	
	このCSAORIOutputオブジェクトを、「SAORI処理で何も返さない」を表す
	リザルト状態にします。
	result、valuesはクリアされ、result_codeは（何も設定がなければ）
	204 No Content になります。

string_t toString();
	このCSAORIOutputオブジェクトを、実行系に返す文字列にします。

----------------------------------------------------------------------
■CSAORIBaseクラス（Csaori_base.h　及び .cpp）
----------------------------------------------------------------------

virtual bool load()
	load関数です。利用者側で実装してください。

virtual bool unload()
	unload関数です。利用者側で実装してください。

virtual void exec(const CSAORIInput& in,CSAORIOutput& out)
	実際のリクエストを処理するexec関数です。利用者側で実装してください。

const string_t& getModulePath(void) 
	実行系から渡される、DLLが存在するディレクトリパスを取得します。

HANDLE getModuleHandle(void) 
	DLLがアタッチされた際にDLLMainで渡されるHANDLEを取得します。

std::string checkAndModifyPath(const std::string &path);
string_t checkAndModifyPathW(const string_t &path);
	パス表記の正規化を行います。WのついているのはUNICODEバージョンです。

	・環境変数は展開されます。
	・DLLのある位置からの相対パス表記は、getModulePathの返却値と結合され、
	　フルパスになります。

------ 以下のメンバは、利用者はあまり意識する必要はありません。 ------

virtual bool exec_insecure(const CSAORIInput& in,CSAORIOutput& out)
	SecLevel Remote でリクエストが来た場合のexec関数です。
	デフォルトで何もしないようにしていますので、そのまま何もしないことを
	推奨します。

virtual void exec_before(const CSAORIInput& in,CSAORIOutput& out)
	exec関数が実行される前の処理があればここに記載します。

virtual void exec_after(const CSAORIInput& in,CSAORIOutput& out)
	exec関数が実行された後の処理があればここに記載します。


■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■配布条件等
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

以下の配布条件で規定するライセンスは、CSAORIサンプルクラス本体に適用
されるライセンスであり、CSAORIを用いて作成された派生SAORIは、CSAORI
とは異なるライセンスになっている場合があります。各SAORIのライセンスを
確認してください。

以下の文章において、CSAORIとは以下のファイルを示すものとします。

csaori.cpp
csaori.h
csaori_base.cpp
csaori_base.h
cplugin.cpp
cplugin.h
readme.txt（このファイル）


CSAORIは、以下の2.で明記したライセンスに従って利用できます。
ただし、「伺かゴースト」とその周辺プログラムに組み込んで利用する場合は、
1.のライセンスを適用するものとします。

要するに、ゴーストやそれに関係するユーティリティに組み込んで使う場合は、
何も気にしなくていいです。

----------------------------------------------------------------
1.伺かゴースト用ライセンス

使途を限定せず自由に利用することが出来ます。

配布パッケージに含まれているすべてのファイルを自由に取り扱うことが出来ます。
一部のファイル、もしくはそのファイルの内容の一部を抜き出して、
他の構成に含めてもかまいません。 
このプログラムを使用した結果、あなたに何らかの損害が発生しても、
その責任を作者は負いません。 

----------------------------------------------------------------
2.修正BSDライセンス

Copyright (C) 2008-, CSaori Project All rights reserved.
http://code.google.com/p/csaori/

ソースコード形式かバイナリ形式か、変更するかしないかを問わず、
以下の条件を満たす場合に限り、再頒布および使用が許可されます。

・ソースコードを再頒布する場合、上記の著作権表示、本条件一覧、
　および下記免責条項を含めること。
・バイナリ形式で再頒布する場合、頒布物に付属のドキュメント等の資料に、
　上記の著作権表示、本条件一覧、および下記免責条項を含めること。
・書面による特別の許可なしに、本ソフトウェアから派生した製品の宣伝または
　販売促進に、「CSaori Project」の名前または貢献者の名前を使用してはならない。

本ソフトウェアは、著作権者および貢献者によって「現状のまま」提供されており、
明示黙示を問わず、商業的な使用可能性、および特定の目的に対する適合性に関する
暗黙の保証も含め、またそれに限定されない、いかなる保証もありません。著作権者も
貢献者も、事由のいかんを問わず、 損害発生の原因いかんを問わず、かつ責任の根拠
が契約であるか厳格責任であるか（過失その他の）不法行為であるかを問わず、
仮にそのような損害が発生する可能性を知らされていたとしても、本ソフトウェアの
使用によって発生した（代替品または代用サービスの調達、使用の喪失、データの喪失、
利益の喪失、業務の中断も含め、またそれに限定されない）直接損害、間接損害、
偶発的な損害、特別損害、懲罰的損害、または結果損害について、一切責任を
負わないものとします。

■更新履歴

http://code.google.com/p/csaori/wiki/ChangeLog
