#pragma once
#include "afxcmn.h"


// CDlgSess dialog

class CDlgSess : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSess)

public:
	CDlgSess(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSess();
	void UpdateSessList();
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SESS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnCon();
	CListCtrl m_listSess;
};
