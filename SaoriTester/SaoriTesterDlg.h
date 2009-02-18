// SaoriTesterDlg.h : ヘッダー ファイル
//

#if !defined(AFX_SAORITESTERDLG_H__152C015F_AB75_495A_A460_F724B449DDBB__INCLUDED_)
#define AFX_SAORITESTERDLG_H__152C015F_AB75_495A_A460_F724B449DDBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////
// CSaoriTesterDlg ダイアログ

typedef BOOL (__cdecl *SPM_load)(HGLOBAL h, long len);
typedef BOOL (__cdecl *SPM_unload)();
typedef HGLOBAL (__cdecl *SPM_request)(HGLOBAL h, long* len);

class CSaoriTesterDlg : public CDialog
{
// 構築
public:
	virtual void OnCancel();
	virtual void OnOK();
	CSaoriTesterDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CSaoriTesterDlg)
	enum { IDD = IDD_SAORITESTER_DIALOG };
	CString	m_a1;
	CString	m_a2;
	CString	m_a3;
	CString	m_a4;
	CString	m_a5;
	CString	m_a6;
	CString	m_a7;
	CString	m_a8;
	CString	m_path;
	CString	m_request;
	CString	m_response;
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CSaoriTesterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	HMODULE m_hModule;
	SPM_load load;
	SPM_unload unload;
	SPM_request request;

	CArray<CString,CString&> m_fileHistory;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CSaoriTesterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnExecute();
	afx_msg void OnUnload();
	afx_msg void OnReload();
	afx_msg void OnDestroy();
	afx_msg void OnHistory();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	bool LoadSAORI(const char *pPath);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SAORITESTERDLG_H__152C015F_AB75_495A_A460_F724B449DDBB__INCLUDED_)
