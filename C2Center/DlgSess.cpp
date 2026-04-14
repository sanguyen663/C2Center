// DlgSess.cpp : implementation file
//

#include "stdafx.h"
#include "C2Center.h"
#include "C2CenterDlg.h"
#include "DlgSess.h"
#include "afxdialogex.h"


// CDlgSess dialog

IMPLEMENT_DYNAMIC(CDlgSess, CDialogEx)

CDlgSess::CDlgSess(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SESS_DIALOG, pParent)
{

}

CDlgSess::~CDlgSess()
{
}

void CDlgSess::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSess, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_CON, &CDlgSess::OnBnClickedBtnCon)
END_MESSAGE_MAP()


// CDlgSess message handlers


void CDlgSess::OnBnClickedBtnCon()
{
	// TODO: Add your control notification handler code here
	CString strIP = _T("127.0.0.1"); // Nhập IP của máy chạy RadSim (127.0.0.1 nếu chạy cùng máy)
	UINT nPort = 10000;
	CString strUserID = _T("TT_01");

	CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
	if (pMainDlg != NULL)
	{
		if (pMainDlg->m_ClientSocket.RequestConnect(strIP, nPort, strUserID))
		{
			MessageBox(_T("Đã gửi yêu cầu kết nối!"), _T("Thông báo"), MB_OK);
		}
	}
}
