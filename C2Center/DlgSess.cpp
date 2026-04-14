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
	DDX_Control(pDX, IDC_LIST_SESS, m_listSess);
}


BEGIN_MESSAGE_MAP(CDlgSess, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_CON, &CDlgSess::OnBnClickedBtnCon)
	ON_BN_CLICKED(IDC_BTN_DISCON, &CDlgSess::OnBnClickedBtnDiscon)
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

void CDlgSess::UpdateSessList()
{
	CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
	if (pMainDlg == NULL) return;

	m_listSess.DeleteAllItems();

	// Kiểm tra nếu Socket đang ở trạng thái đã kết nối
	if (pMainDlg->m_ClientSocket.m_nConnectionState == 2)
	{
		CString strIP = pMainDlg->m_ClientSocket.m_strRadarIP;
		CString strPort;
		strPort.Format(_T("%d"), pMainDlg->m_ClientSocket.m_nRadarPort);

		int nItem = m_listSess.InsertItem(0, _T("1"));
		m_listSess.SetItemText(nItem, 1, strIP);        // Cột IP
		m_listSess.SetItemText(nItem, 2, strPort);      // Cột Cổng đích
		m_listSess.SetItemText(nItem, 3, _T("ACTIVE")); // Trạng thái
	}
}

BOOL CDlgSess::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_listSess.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
	m_listSess.InsertColumn(0, _T("STT"), LVCFMT_CENTER, 50);
	m_listSess.InsertColumn(1, _T("IP Radar"), LVCFMT_LEFT, 130);
	m_listSess.InsertColumn(2, _T("Port"), LVCFMT_CENTER, 135);
	m_listSess.InsertColumn(3, _T("Trạng thái"), LVCFMT_LEFT, 120);
	return TRUE;
}

void CDlgSess::OnBnClickedBtnDiscon()
{
	// TODO: Add your control notification handler code here
	// 1. Lấy con trỏ đến cửa sổ chính của C2Center
	CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
	if (pMainDlg == NULL) return;

	// 2. Kiểm tra xem Socket có đang hoạt động hay không (Trạng thái khác 0)
	if (pMainDlg->m_ClientSocket.m_nConnectionState != 0)
	{
		// Ghi log lên màn hình Monitor
		CString strLog;
		strLog.Format(_T("[SYSTEM] Chủ động ngắt kết nối với Radar %s"), pMainDlg->m_ClientSocket.m_strRadarIP);
		pMainDlg->AddToMonitor(strLog);

		// 3. Đóng Socket và reset trạng thái về 0 (Chưa kết nối)
		pMainDlg->m_ClientSocket.Close();
		pMainDlg->m_ClientSocket.m_nConnectionState = 0;
		pMainDlg->m_ClientSocket.m_strRadarIP = _T(""); // Xóa IP cũ

														// 4. Dọn dẹp kho chứa Quỹ đạo và cập nhật bảng DlgTrack cho trắng đi
		pMainDlg->m_listReceivedTracks.clear();
		if (pMainDlg->m_dlgTrack.GetSafeHwnd() != NULL)
		{
			pMainDlg->m_dlgTrack.UpdateTrackList();
		}

		// 5. Vẽ lại bảng Phiên kết nối (lúc này sẽ không còn dòng nào vì trạng thái = 0)
		UpdateSessList();

		AfxMessageBox(_T("Đã ngắt kết nối thành công!"), MB_ICONINFORMATION);
	}
	else
	{
		AfxMessageBox(_T("Hiện tại không có kết nối nào để ngắt!"), MB_ICONWARNING);
	}
}
