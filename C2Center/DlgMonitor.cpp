// DlgMonitor.cpp : implementation file
//

#include "stdafx.h"
#include "C2Center.h"
#include "DlgMonitor.h"
#include "afxdialogex.h"


// CDlgMonitor dialog

IMPLEMENT_DYNAMIC(CDlgMonitor, CDialogEx)

CDlgMonitor::CDlgMonitor(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MONITOR_DIALOG, pParent)
{

}

CDlgMonitor::~CDlgMonitor()
{
}

void CDlgMonitor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
}


BEGIN_MESSAGE_MAP(CDlgMonitor, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_START, &CDlgMonitor::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CDlgMonitor::OnBnClickedBtnPause)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CDlgMonitor::OnBnClickedBtnClear)
END_MESSAGE_MAP()


// CDlgMonitor message handlers

void CDlgMonitor::AddLog(CString strLog)
{
	// NẾU ĐANG TẠM DỪNG -> KHÔNG LÀM GÌ CẢ
	if (m_bIsPaused) return;

	CTime time = CTime::GetCurrentTime();
	CString strTime = time.Format(_T("[%H:%M:%S] "));

	m_listLog.AddString(strTime + strLog);

	int nCount = m_listLog.GetCount();
	if (nCount > 0)
	{
		m_listLog.SetCurSel(nCount - 1);
	}
}

void CDlgMonitor::OnBnClickedBtnStart()
{
	// TODO: Add your control notification handler code here
	m_bIsPaused = false;
	CButton* pBtnPause = (CButton*)GetDlgItem(IDC_BTN_PAUSE);
	if (pBtnPause != NULL)
	{
		pBtnPause->SetWindowText(_T("Tạm dừng (Pause)"));
	}

	CTime time = CTime::GetCurrentTime();
	CString strTime = time.Format(_T("[%H:%M:%S] "));
	m_listLog.AddString(strTime + _T("[SYSTEM] ===== BẮT ĐẦU GIÁM SÁT DỮ LIỆU ====="));
}


void CDlgMonitor::OnBnClickedBtnPause()
{
	// TODO: Add your control notification handler code here
	m_bIsPaused = !m_bIsPaused;
	CButton* pBtnPause = (CButton*)GetDlgItem(IDC_BTN_PAUSE);
	if (pBtnPause != NULL)
	{
		if (m_bIsPaused)
			pBtnPause->SetWindowText(_T("Tiếp tục (Resume)"));
		else
			pBtnPause->SetWindowText(_T("Tạm dừng (Pause)"));
	}
}


void CDlgMonitor::OnBnClickedBtnClear()
{
	// TODO: Add your control notification handler code here
	m_listLog.ResetContent();
}
