#pragma once


// CDlgSess dialog

class CDlgSess : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSess)

public:
	CDlgSess(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSess();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SESS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnCon();
};
