// SaoriTester.h : SAORITESTER アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_SAORITESTER_H__E4B2BE86_2530_48C9_8F0D_4B2D71B97AAA__INCLUDED_)
#define AFX_SAORITESTER_H__E4B2BE86_2530_48C9_8F0D_4B2D71B97AAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CSaoriTesterApp:
// このクラスの動作の定義に関しては SaoriTester.cpp ファイルを参照してください。
//

class CSaoriTesterApp : public CWinApp
{
public:
	CSaoriTesterApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CSaoriTesterApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CSaoriTesterApp)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SAORITESTER_H__E4B2BE86_2530_48C9_8F0D_4B2D71B97AAA__INCLUDED_)
