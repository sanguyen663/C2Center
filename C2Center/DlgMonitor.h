#pragma once
#include "afxwin.h"


// CDlgMonitor dialog

class CDlgMonitor : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMonitor)

public:
	CDlgMonitor(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMonitor();
	void AddLog(CString strLog);
	bool m_bIsPaused = false;
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MONITOR_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnPause();
	afx_msg void OnBnClickedBtnClear();
	CListBox m_listLog;
};
