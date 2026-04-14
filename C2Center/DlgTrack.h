#pragma once
#include "afxcmn.h"
#include <map>
#include "C_ClientSocket.h"

// CDlgTrack dialog

class CDlgTrack : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTrack)

public:
	CDlgTrack(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgTrack();
	CString m_strCurrentRadarFilter = _T("All");
	void UpdateTrackList();
	std::map<int, AsterixTrack> m_mapTracks;
	void ProcessNewTrack(AsterixTrack track);
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TRACK_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSplitRadar();
	afx_msg void OnLvnItemchangedListTrack(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl m_ListTrack;
};
