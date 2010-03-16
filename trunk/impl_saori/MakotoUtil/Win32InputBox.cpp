#include "Win32InputBox.h"
#include <stdio.h>

#pragma warning (disable: 4312)

/*
History
----------
03/02/2006 
- Initial version development

03/04/2006
- Lessened the complexity of the class, made it less generic (since its purpose is to be simple)
- Updated the dialog template and made OK button as default button

*/

typedef struct _MSDN_DLGTEMPLATEEX 
{
  WORD dlgVer;
  WORD signature;
  DWORD helpID;
  DWORD exStyle;
  DWORD style;
  WORD cDlgItems;
  short x;
  short y;
  short cx;
  short cy;
  BYTE  _rest[1]; // rest of structure
} MSDN_DLGTEMPLATEEX;

static bool IsDlgTemplateExtended(DLGTEMPLATE *dlgTemplate)
{
  MSDN_DLGTEMPLATEEX *dgExTemplate = (MSDN_DLGTEMPLATEEX *) dlgTemplate;

  // MSDN excerpt:
  //* dlgVer
  //  Specifies the version number of the extended dialog box template. This member must be 1. 
  //* signature
  //  Indicates whether a template is an extended dialog box template. 
  // If signature is 0xFFFF, this is an extended dialog box template. 
  // In this case, the dlgVer member specifies the template version number. 
  // If signature is any value other than 0xFFFF, this is a standard dialog box template that uses the DLGTEMPLATE and DLGITEMTEMPLATE structures. 

  return (dgExTemplate->dlgVer == 1) && (dgExTemplate->signature == 0xFFFF);
}

static LPCTSTR definputbox_buttonnames[] = { _T("OK"), _T("Cancel") };
static const INT_PTR definputbox_buttonids[] = { IDOK, IDCANCEL };

static const INT 
  definputbox_id_prompt = 1000,
  definputbox_id_edit1 = 1001,
  definputbox_id_edit2 = 1002;

WIN32INPUTBOX_PARAM::WIN32INPUTBOX_PARAM()
{
  bMultiline = false;
  hwndOwner = 0;
  DlgTemplateName = 0;
  hInstance = (HINSTANCE) ::GetModuleHandle(0);
  DlgTemplateData = definputbox_dlg;

  bCenter = true;

  dwStylesPlus = 0;
  dwExStylesPlus = 0;
  dwStylesMinus = 0xFFFFFFFF;
  dwExStylesMinus = 0xFFFFFFFF;

  xPos = yPos = -1;

  szResult = 0;
  nResultSize = 0;
}

CWin32InputBox::CWin32InputBox(WIN32INPUTBOX_PARAM *param)
{
  _param = param;
}

CWin32InputBox::~CWin32InputBox()
{

}

void CWin32InputBox::SetParam(WIN32INPUTBOX_PARAM *param)
{
  _param = param;
}

WIN32INPUTBOX_PARAM *CWin32InputBox::GetParam()
{
  return _param;
}

INT_PTR CWin32InputBox::InputBoxEx(WIN32INPUTBOX_PARAM *param)
{
  // Check mandatory parameters
  if (param->szResult == 0)
  {
    ::SetLastError(ERROR_INVALID_PARAMETER);
    return 0;
  }

  LPDLGTEMPLATE dlgTemplate;

  if (param->DlgTemplateName != 0)
  {
    HMODULE hModule = (HMODULE)param->hInstance;
    HRSRC rcDlg = ::FindResource(hModule, MAKEINTRESOURCE(param->DlgTemplateName), RT_DIALOG);
    if (rcDlg == NULL)
      return 0;

    HGLOBAL hglobalDlg = ::LoadResource(hModule, rcDlg);
    if (hglobalDlg == NULL)
      return 0;

    dlgTemplate = (LPDLGTEMPLATE) hglobalDlg;
  }
  else if (param->DlgTemplateData != 0)
  {
    dlgTemplate = (LPDLGTEMPLATE) param->DlgTemplateData;
  }

  MSDN_DLGTEMPLATEEX *dlgTemplateEx = 
    IsDlgTemplateExtended((LPDLGTEMPLATE) dlgTemplate) ? (MSDN_DLGTEMPLATEEX *) dlgTemplate : 0;

  if (dlgTemplateEx != 0)
  {
    dlgTemplateEx->exStyle |= param->dwExStylesPlus;
    dlgTemplateEx->style   |= param->dwStylesPlus;
    dlgTemplateEx->exStyle &= param->dwExStylesMinus;
    dlgTemplateEx->style   &= param->dwStylesMinus;

    if (param->bCenter)
      dlgTemplateEx->style |= DS_CENTER;

    if (param->xPos != -1)
      dlgTemplateEx->x = param->xPos;
    if (param->yPos != -1)
      dlgTemplateEx->y = param->yPos;
  }
  else
  {
    dlgTemplate->dwExtendedStyle  |= param->dwExStylesPlus;
    dlgTemplate->style            |= param->dwStylesPlus;
    dlgTemplate->dwExtendedStyle  &= param->dwExStylesMinus;
    dlgTemplate->style            &= param->dwStylesMinus;

    if (param->bCenter)
      dlgTemplate->style |= DS_CENTER;

    if (param->xPos != -1)
      dlgTemplate->x = param->xPos;

    if (param->yPos != -1)
      dlgTemplate->y = param->yPos;
  }

  CWin32InputBox inputbox(param);

  // Resize dialog and SHOW or HIDE multiline
  INT_PTR r = ::DialogBoxIndirectParam(param->hInstance, dlgTemplate, param->hwndOwner, (DLGPROC)DlgProc, (LPARAM)&inputbox);

  return r;
}

INT_PTR CWin32InputBox::InputBox(
  LPCTSTR szTitle, 
  LPCTSTR szPrompt, 
  LPTSTR szResult, 
  DWORD nResultSize,
  int flags,
  HWND hwndParent)
{
  WIN32INPUTBOX_PARAM param;

  if(flags==2) // NUMERIC
  {
    definputbox_dlg[149] =32;
    definputbox_dlg[148] =128;
    param.bMultiline=false;
  }
  else if(flags==3) // PASSWORD
  {
    definputbox_dlg[149] =0;
    definputbox_dlg[148] =160;
    param.bMultiline=false;
  }
  else
  {
    definputbox_dlg[149] =0;
    definputbox_dlg[148] =128;
    param.bMultiline= (flags==1) ? true : false;  // MULTILINE
  }

  param.szTitle = szTitle;
  param.szPrompt = szPrompt;
  param.szResult = szResult;
  param.nResultSize = nResultSize;

  return InputBoxEx(&param);
}

void CWin32InputBox::InitDialog()
{
  // Set the button captions
  for (size_t i=0;i<sizeof(definputbox_buttonids)/sizeof(definputbox_buttonids[0]);i++)
    ::SetDlgItemText(_param->hDlg, (int) definputbox_buttonids[i], definputbox_buttonnames[i]);

  // Set other controls
  ::SetWindowText(_param->hDlg, _param->szTitle);
  ::SetDlgItemText(_param->hDlg, definputbox_id_prompt, _param->szPrompt);

  HWND hwndEdit1 = ::GetDlgItem(_param->hDlg, definputbox_id_edit1);
  HWND hwndEdit2 = ::GetDlgItem(_param->hDlg, definputbox_id_edit2);

  if (_param->bMultiline)
    _hwndEditCtrl = hwndEdit2;
  else
    _hwndEditCtrl = hwndEdit1;

  ::SetWindowText(_hwndEditCtrl, _param->szResult);

  RECT rectDlg, rectEdit1, rectEdit2;

  ::GetWindowRect(_param->hDlg, &rectDlg);
  ::GetWindowRect(hwndEdit1, &rectEdit1);
  ::GetWindowRect(hwndEdit2, &rectEdit2);

  if (_param->bMultiline)
  {
    ::ShowWindow(hwndEdit1, SW_HIDE);
    ::SetWindowPos(
      hwndEdit2, 
      HWND_NOTOPMOST, 
      rectEdit1.left - rectDlg.left, 
      (rectEdit1.top - rectDlg.top) - (rectEdit1.bottom - rectEdit1.top), 
      0, 
      0, 
      SWP_NOSIZE | SWP_NOZORDER);

    ::SetWindowPos(
      _param->hDlg, 
      HWND_NOTOPMOST, 
      0, 
      0, 
      rectDlg.right - rectDlg.left, 
      rectDlg.bottom - rectDlg.top - (rectEdit1.bottom - rectEdit1.top), 
      SWP_NOMOVE);

  }
  else
  {
    ::SetWindowPos(
      _param->hDlg, 
      HWND_NOTOPMOST, 
      0, 
      0, 
      rectDlg.right - rectDlg.left, 
      rectEdit1.bottom - rectDlg.top + 5,
      SWP_NOMOVE);

    ::ShowWindow(hwndEdit2, SW_HIDE);
  }
}

// Message handler for about box.
LRESULT CALLBACK CWin32InputBox::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  CWin32InputBox *_this = (CWin32InputBox *) ::GetWindowLong(hDlg, GWL_USERDATA);
  WIN32INPUTBOX_PARAM *param = _this ? _this->GetParam() : 0;

  switch (message)
  {
    case WM_INITDIALOG:
    {
      ::SetWindowLong(hDlg, GWL_USERDATA, (LONG) lParam);
      
      _this = (CWin32InputBox *)  lParam;
      _this->_param->hDlg = hDlg;
      _this->InitDialog();
      return TRUE;
    }

    case WM_COMMAND:
    {
#ifdef _MY_DEBUG
      CHAR buf[1024];
      static int i=0;
      sprintf(buf, "WM_COMMAND: %09d wParam=%08X lParam=%08X\n", i++, wParam, lParam);
      OutputDebugString(buf);
#endif
      INT_PTR buttonId = LOWORD(wParam);
      for (size_t i=0;
           i<sizeof(definputbox_buttonids)/sizeof(definputbox_buttonids[0]);
           i++)
      {
        if (buttonId == definputbox_buttonids[i]) 
        {

          if (buttonId == IDOK)
		  {
            ::GetWindowText(
              _this->_hwndEditCtrl, 
              _this->_param->szResult, 
              _this->_param->nResultSize);
		  }

          ::EndDialog(hDlg, buttonId);
          return TRUE;
        }
      }
    }
    break;
  }
  return FALSE;
}