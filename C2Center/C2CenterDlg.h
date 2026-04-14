
// C2CenterDlg.h : header file
//

#pragma once
#include "DlgTrack.h"
#include "DlgSess.h"
#include "DlgMonitor.h"
#include "C_ClientSocket.h"
#include <vector>

// CC2CenterDlg dialog
class CC2CenterDlg : public CDialogEx
{
// Construction
public:
	CC2CenterDlg(CWnd* pParent = NULL);	// standard constructor
	CDlgMonitor m_dlgMonitor;
	CDlgTrack   m_dlgTrack;
	CDlgSess    m_dlgSess;
	// BIẾN QUẢN LÝ KẾT NỐI MẠNG
	CC_ClientSocket m_ClientSocket;

	// HÀM NHẬN DỮ LIỆU TỪ SOCKET
	void AddToMonitor(CString strLog);
	void ProcessReceivedTrack(AsterixTrack track);

	// Hàm dọn dẹp và Timer
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	std::vector<CString> m_listConnectedIPs;
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_C2CENTER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnSess();
	afx_msg void OnBnClickedBtnTrack();
	afx_msg void OnBnClickedBtnMonitor();
};
