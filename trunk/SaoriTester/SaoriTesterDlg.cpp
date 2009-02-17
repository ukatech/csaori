// SaoriTesterDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "SaoriTester.h"
#include "SaoriTesterDlg.h"

#include <mbstring.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaoriTesterDlg ダイアログ

CSaoriTesterDlg::CSaoriTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSaoriTesterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaoriTesterDlg)
	m_a1 = _T("");
	m_a2 = _T("");
	m_a3 = _T("");
	m_a4 = _T("");
	m_a5 = _T("");
	m_a6 = _T("");
	m_a7 = _T("");
	m_a8 = _T("");
	m_path = _T("");
	m_request = _T("");
	m_response = _T("");
	//}}AFX_DATA_INIT

	Create(CSaoriTesterDlg::IDD,pParent);
	m_hModule = NULL;
}

void CSaoriTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaoriTesterDlg)
	DDX_Text(pDX, IDC_ARG1, m_a1);
	DDX_Text(pDX, IDC_ARG2, m_a2);
	DDX_Text(pDX, IDC_ARG3, m_a3);
	DDX_Text(pDX, IDC_ARG4, m_a4);
	DDX_Text(pDX, IDC_ARG5, m_a5);
	DDX_Text(pDX, IDC_ARG6, m_a6);
	DDX_Text(pDX, IDC_ARG7, m_a7);
	DDX_Text(pDX, IDC_ARG8, m_a8);
	DDX_Text(pDX, IDC_PATH, m_path);
	DDX_Text(pDX, IDC_REQUEST, m_request);
	DDX_Text(pDX, IDC_RESPONSE, m_response);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSaoriTesterDlg, CDialog)
	//{{AFX_MSG_MAP(CSaoriTesterDlg)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_EXECUTE, OnExecute)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaoriTesterDlg メッセージ ハンドラ

BOOL CSaoriTesterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	return TRUE;  // TRUE を返すとコントロールに設定したフォーカスは失われません。
}


void CSaoriTesterDlg::PostNcDestroy() 
{
	delete this;
	::PostQuitMessage(0);
}

void CSaoriTesterDlg::OnOK()
{
	DestroyWindow();
}

void CSaoriTesterDlg::OnCancel()
{
	if ( m_hModule ) {
		unload();
		::FreeLibrary(m_hModule);
	}
	DestroyWindow();
}

void CSaoriTesterDlg::OnDropFiles(HDROP hDropInfo) 
{
	char path[MAX_PATH+1];
	if ( ::DragQueryFile(hDropInfo,0,path,sizeof(path)-1) ) {
		if ( m_hModule ) {
			unload();
			::FreeLibrary(m_hModule);
		}

		m_hModule = ::LoadLibrary(path);
		m_path = path;
		UpdateData(FALSE);

		if ( m_hModule ) {
			//proc
			load = (SPM_load)::GetProcAddress(m_hModule,"load");
			if ( ! load ) {
				load = (SPM_load)::GetProcAddress(m_hModule,"_load");
			}

			unload = (SPM_unload)::GetProcAddress(m_hModule,"unload");
			if ( ! load ) {
				unload = (SPM_unload)::GetProcAddress(m_hModule,"_unload");
			}

			request = (SPM_request)::GetProcAddress(m_hModule,"request");
			if ( ! load ) {
				request = (SPM_request)::GetProcAddress(m_hModule,"_request");
			}

			if ( ! load || ! unload || ! request ) {
				::FreeLibrary(m_hModule);
				m_hModule = NULL;
			}

			//load
			char *pc = (char*)_mbsrchr((unsigned char*)path,'\\');
			*(pc+1) = 0;

			HGLOBAL h = ::GlobalAlloc(GMEM_FIXED,strlen(path)+1);
			strcpy((char*)h,path);
			
			long len = strlen(path);
			load(h,len);

			//get version
			char *c = "GET Version SAORI/1.0\r\nCharset: Shift_JIS\r\nSender: SAORI_TESTER\r\nSecurityLevel: Local\r\n\r\n";
			h = ::GlobalAlloc(GMEM_FIXED,strlen(c)+1);
			strcpy((char*)h,c);
			len = strlen(c);
			h = request(h,&len);
			if ( h ) {
				::GlobalFree(h);
			}
		}
	}
	
	CDialog::OnDropFiles(hDropInfo);
}

void CSaoriTesterDlg::OnExecute() 
{
	if ( m_hModule && request ) {
		UpdateData(TRUE);

		m_request = "EXECUTE SAORI/1.0\r\nCharset: Shift_JIS\r\nSender: SAORI_TESTER\r\nSecurityLevel: Local\r\n";
		if ( m_a1.GetLength() ) {
			m_request += "Argument0: " + m_a1 + "\r\n";
		}
		if ( m_a2.GetLength() ) {
			m_request += "Argument1: " + m_a2 + "\r\n";
		}
		if ( m_a3.GetLength() ) {
			m_request += "Argument2: " + m_a3 + "\r\n";
		}
		if ( m_a4.GetLength() ) {
			m_request += "Argument3: " + m_a4 + "\r\n";
		}
		if ( m_a5.GetLength() ) {
			m_request += "Argument4: " + m_a5 + "\r\n";
		}
		if ( m_a6.GetLength() ) {
			m_request += "Argument5: " + m_a6 + "\r\n";
		}
		if ( m_a7.GetLength() ) {
			m_request += "Argument6: " + m_a7 + "\r\n";
		}
		if ( m_a8.GetLength() ) {
			m_request += "Argument7: " + m_a8 + "\r\n";
		}
		m_request += "\r\n";

		HGLOBAL h = ::GlobalAlloc(GMEM_FIXED,m_request.GetLength()+1);
		strcpy((char*)h,m_request.GetBuffer(1));

		long l = m_request.GetLength();

		h = request(h,&l);

		if ( h ) {
			char *pc = m_response.GetBuffer(l+1);
			memcpy(pc,h,l);
			pc[l] = 0;
			m_response.ReleaseBuffer(l);
		}
		else {
			m_response = "ERROR!!!";
		}

		UpdateData(FALSE);
	}
}
