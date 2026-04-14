// DlgTrack.cpp : implementation file
//

#include "stdafx.h"
#include "C2Center.h"
#include "DlgTrack.h"
#include "afxdialogex.h"


// CDlgTrack dialog

IMPLEMENT_DYNAMIC(CDlgTrack, CDialogEx)

CDlgTrack::CDlgTrack(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TRACK_DIALOG, pParent)
{

}

CDlgTrack::~CDlgTrack()
{
}

void CDlgTrack::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TRACK, m_ListTrack);
}


BEGIN_MESSAGE_MAP(CDlgTrack, CDialogEx)
	ON_BN_CLICKED(IDC_SPLIT_RADAR, &CDlgTrack::OnBnClickedSplitRadar)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TRACK, &CDlgTrack::OnLvnItemchangedListTrack)
END_MESSAGE_MAP()


// CDlgTrack message handlers


void CDlgTrack::OnBnClickedSplitRadar()
{
	// TODO: Add your control notification handler code here
}


void CDlgTrack::OnLvnItemchangedListTrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	// Kiểm tra nếu có một dòng VỪA ĐƯỢC CHỌN (Trạng thái chuyển sang LVIS_SELECTED)
	if ((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uNewState & LVIS_SELECTED))
	{
		int nSelectedRow = pNMLV->iItem; // Vị trí dòng vừa click

										 // Lấy số hiệu TN của quỹ đạo (Giả sử cột 0 là Cột Nguồn Radar, Cột 1 là TN)
		CString strRadarSource = m_ListTrack.GetItemText(nSelectedRow, 0);
		CString strTN = m_ListTrack.GetItemText(nSelectedRow, 1);

		// TODO: Tạm thời bạn có thể dùng strTN này để bôi đậm, 
		// hoặc đẩy thông tin ra một Label "Chi tiết mục tiêu" bên ngoài.
	}

	*pResult = 0;
}
