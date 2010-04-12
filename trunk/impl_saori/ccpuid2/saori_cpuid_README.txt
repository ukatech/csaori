#######################################################
SAORI	CPU/OS情報取得プラグイン	saori_cpuid.dll

	Copyright 2002-2003 Ewi Nanase (nanase@himawari-c.com)
#######################################################

######################################################
#		概要
######################################################
saori_cpuid.dllはCPU/OSの情報を取得するSAORIモジュールです。


######################################################
#		使用方法
######################################################
下記のパラメータを渡してやると、それについての情報を
返してきます。

美坂での使用例
{$os_name={$saori("saori_cpuid.dll","os.name")}}
にて{$os_name}にオペレーティングシステムの名称が代入されます。


######################################################
#		引数
#
######################################################
#		オペレーティングシステム関連
#
os.name		オペレーティングシステムの名称
			例: Windows XP Home Edition
				Windows 98

os.version	オペレーティングシステムのバージョン番号
			例：Windows XP = 5.1
				Windows 2000 = 5.0
				Windows Me = 4.90
				Windows 98=  4.10

os.build	オペレーティングシステムのビルド番号


#####################################################
#		ＣＰＵ関連情報
#

cpu.num		ＣＰＵの個数

cpu.vender	ＣＰＵのベンダー名
			例:GenuineIntel

cpu.name	ＣＰＵの名称
			※必ずしも詳しく名称が出るとは限らない。

cpu.ptype		ＣＰＵプロセッサータイプコード

cpu.family		ＣＰＵファミリコード

cpu.model		ＣＰＵモデルコード

cpu.stepping	ＣＰＵステッピングコード

cpu.mmx		MMX機能の有無
			有り: Ready
			無し: Not Ready

cpu.sse		SSE機能の有無
			有り: Ready
			無し: Not Ready

cpu.sse2	SSE2機能の有無
			有り: Ready
			無し: Not Ready

cpu.tdn		3DNow!機能の有無
			有り: Ready
			無し: Not Ready

cpu.mmx+	MMX+機能の有無
			有り: Ready
			無し: Not Ready

cpu.tdn+	3DNow!+機能の有無
			有り: Ready
			無し: Not Ready


cpu.htt		Hyper Threading Tecnology機能の有無
			有り: Ready
			無し: Not Ready

cpu.clock	ＣＰＵのクロックスピード
			MH単位で取得

cpu.clockex	ＣＰＵのクロックスピード
			MH単位で取得（小数点３桁まで）


#####################################################
#		メモリ関連情報
#

mem.os		メモリの使用率

mem.phyt	物理メモリ 全体(MB) 

mem.phya	物理メモリ 空き(MB)

mem.pagt	ページファイルサイズ 全体(MB)

mem.paga	ページファイルサイズ 空き(MB) 

mem.virt	仮想メモリ 全体(MB) 

mem.vira	仮想メモリ 空き(MB) 


#####################################################
#		その他
#
platform	プラットフォーム名称
			何か: materia.exe,embryo.exe等
			SSP:  ssp.exe
			何かみたいな何か: duet.exe


#####################################################

<< 改変履歴 >>
Version 2.1.0	2004/04/17	STLライブラリをスタティックリンクしていなかった不具合を修正

Version 2.0.0	2004/04/09	3GHz以上のCPUクロックが正常に検出できない不具合を修正

Version 1.0.5	2003/08/05	"cpu.htt"でCPUがHyper Threading Technology機能
							の有無を取得できるようにしました。

Version 1.0.4	2002/04/01	"cpu.clock"、"cpu.clockex"でCPUのクロックスピード
							を取得できるようにしました。

Version 1.0.3	2002/03/31	cpu.famiry →　cpu.familyに修正（スペルミス）
							cpu.steppingが取得できない不具合を修正
							ドキュメントにcpu.pnameとあるが実際にはcpu.ptype（ドキュメントの修正）

Version 1.0.2	2002/03/31	"platform"の追加。
							デバッグでsaori.txtを出力していた不具合を修正。

Version 1.0.1	2002/03/30  Pentium以外のCPUのCPU名称の不具合を修正しました。

Version 1.0.0	2002/03/30	ファーストリリース



#####################################################
<<連絡先>>
七瀬いーうぃ (nanase@himawari-c.com)
http://www.himawari-c.com/

