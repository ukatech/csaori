// SaoriTester.cpp : アプリケーション用クラスの定義を行います。
//

#include "stdafx.h"
#include "SaoriTester.h"
#include "SaoriTesterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaoriTesterApp

BEGIN_MESSAGE_MAP(CSaoriTesterApp, CWinApp)
	//{{AFX_MSG_MAP(CSaoriTesterApp)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaoriTesterApp クラスの構築

CSaoriTesterApp::CSaoriTesterApp()
{
	// TODO: この位置に構築用のコードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CSaoriTesterApp オブジェクト

CSaoriTesterApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSaoriTesterApp クラスの初期化

BOOL CSaoriTesterApp::InitInstance()
{
	free((void*)m_pszProfileName);

	char myPath[MAX_PATH];
	::GetModuleFileName(NULL,myPath,sizeof(myPath));
	char *lastPeriod = (char*)_mbsrchr((unsigned char*)myPath,'.');
	strcpy(lastPeriod,".ini");

	m_pszProfileName = (const char*)malloc(strlen(myPath) + 1);
	strcpy((char*)m_pszProfileName,myPath);

	m_pMainWnd = new CSaoriTesterDlg;

	return TRUE;
}
