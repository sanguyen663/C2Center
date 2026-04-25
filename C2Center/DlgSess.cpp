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
	// 1. Khai báo biến và lấy dữ liệu từ các ô nhập liệu (Edit Control)
	CString strIP;
	CString strPort;
	CString strUserID;

	// Đọc dữ liệu từ giao diện (Nhớ thay các IDC_ tương ứng với ID bạn đã đặt trên Dialog)
	GetDlgItemText(IDC_EDIT_IP, strIP);
	GetDlgItemText(IDC_EDIT_PORT, strPort);
	GetDlgItemText(IDC_EDIT_ID, strUserID);

	// 2. Kiểm tra xem người dùng đã nhập đủ thông tin chưa
	if (strIP.IsEmpty() || strPort.IsEmpty() || strUserID.IsEmpty())
	{
		MessageBox(_T("Vui lòng nhập đầy đủ IP, Port và UserID trước khi kết nối!"), _T("Lỗi dữ liệu"), MB_ICONWARNING);
		return; // Dừng lại, không thực hiện kết nối
	}

	// Chuyển đổi Port từ chuỗi (CString) sang số nguyên (UINT)
	UINT nPort = _ttoi(strPort);

	// Kiểm tra tính hợp lệ của Port
	if (nPort <= 0 || nPort > 65535)
	{
		MessageBox(_T("Cổng (Port) không hợp lệ! Vui lòng nhập số từ 1 đến 65535."), _T("Lỗi dữ liệu"), MB_ICONWARNING);
		return;
	}

	// 3. Thực hiện gửi yêu cầu kết nối
	CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
	if (pMainDlg != NULL)
	{
		if (pMainDlg->m_ClientSocket.RequestConnect(strIP, nPort, strUserID))
		{
			MessageBox(_T("Đã gửi yêu cầu kết nối!"), _T("Thông báo"), MB_OK);
		}
		else
		{
			MessageBox(_T("Gửi yêu cầu kết nối thất bại! Vui lòng kiểm tra lại mạng."), _T("Lỗi"), MB_ICONERROR);
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
	// Lấy con trỏ về Main Dialog
	CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
	if (pMainDlg == NULL) return;

	// Kiểm tra trạng thái: Nếu đang kết nối (trạng thái 2) hoặc đang chờ (trạng thái 1)
	if (pMainDlg->m_ClientSocket.m_nConnectionState != 0)
	{
		// 1. Đóng kết nối mạng
		pMainDlg->m_ClientSocket.Close();
		pMainDlg->m_ClientSocket.m_nConnectionState = 0;

		// 2. Ghi log thông báo
		pMainDlg->AddToMonitor(_T("[SYSTEM] Đã ngắt kết nối với Radar."));

		// 3. Xóa dữ liệu cũ trên giao diện
		pMainDlg->m_listReceivedTracks.clear();

		// Cập nhật lại bảng Quỹ đạo (nếu đang mở)
		if (pMainDlg->m_dlgTrack.GetSafeHwnd() != NULL) {
			pMainDlg->m_dlgTrack.UpdateTrackList();
		}

		// 4. Cập nhật lại chính bảng Session này
		UpdateSessList();

		AfxMessageBox(_T("Đã ngắt kết nối!"));
	}
}
