// DlgTrack.cpp : implementation file
//

#include "stdafx.h"
#include "C2Center.h"
#include "DlgTrack.h"
#include "afxdialogex.h"
#include "C2CenterDlg.h"


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
	CMenu menu;
	menu.CreatePopupMenu();

	// 1. Luôn có mục "Tất cả" ở đầu với ID là 1000
	menu.AppendMenu(MF_STRING, 1000, _T("Tất cả (All)"));

	// 2. Lấy danh sách từ Main Dialog
	CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
	if (pMainDlg != NULL)
	{
		// Dùng vòng lặp để đưa các IP đã kết nối vào Menu
		for (size_t i = 0; i < pMainDlg->m_listConnectedIPs.size(); ++i)
		{
			// Cấp ID cho các mục Radar bắt đầu từ 1001 trở đi
			menu.AppendMenu(MF_STRING, 1001 + i, pMainDlg->m_listConnectedIPs[i]);
		}
	}

	// 3. Hiển thị Menu tại vị trí nút bấm
	CRect rect;
	GetDlgItem(IDC_SPLIT_RADAR)->GetWindowRect(&rect);
	int nSelected = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, rect.left, rect.bottom, this);

	// 4. Xử lý lựa chọn bằng ID
	if (nSelected == 1000)
	{
		m_strCurrentRadarFilter = _T("All");
		SetDlgItemText(IDC_SPLIT_RADAR, _T("Lọc theo Radar: [ Tất cả (All) ]"));
	}
	else if (nSelected > 1000)
	{
		// Tính toán lại chỉ số (index) trong mảng dựa trên ID menu đã chọn
		int nIndex = nSelected - 1001;
		m_strCurrentRadarFilter = pMainDlg->m_listConnectedIPs[nIndex];

		SetDlgItemText(IDC_SPLIT_RADAR, _T("Lọc theo Radar: [ ") + m_strCurrentRadarFilter + _T(" ]"));
	}

	// Cập nhật lại bảng dữ liệu sau khi lọc
	UpdateTrackList();
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

void CDlgTrack::UpdateTrackList()
{
	// 1. Tắt tính năng vẽ tạm thời để chống giật (flicker) khi nạp nhiều dữ liệu
	m_ListTrack.SetRedraw(FALSE);

	// 2. Xóa sạch bảng cũ
	m_ListTrack.DeleteAllItems();

	CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
	if (pMainDlg != NULL)
	{
		int nRow = 0;
		// 3. Duyệt qua Kho chứa của Main Dialog
		for (size_t i = 0; i < pMainDlg->m_listReceivedTracks.size(); i++)
		{
			CenterTrack ct = pMainDlg->m_listReceivedTracks[i];

			// 4. BỘ LỌC KỲ DIỆU: Chỉ vẽ nếu chọn "All" hoặc IP trùng với bộ lọc
			if (m_strCurrentRadarFilter == _T("All") || m_strCurrentRadarFilter == ct.strRadarIP)
			{
				AsterixTrack& trk = ct.trackData;

				// Chuyển đổi dữ liệu sang dạng chuỗi
				CString strTN, strPos, strHdgSpd, strAlti, strType, strIden;
				strTN.Format(_T("%02d"), trk.nTrackNumber);
				strPos.Format(_T("%.4f - %.4f"), trk.fLat, trk.fLon);
				strHdgSpd.Format(_T("%.0f - %.0f"), trk.fHeading, trk.fSpeed);
				strAlti.Format(_T("%.0f"), trk.fAltitude);
				strType.Format(_T("%d"), trk.nType);
				strIden = CA2T(trk.szIden);

				// Đẩy lên bảng hiển thị (8 Cột theo thiết kế của bạn)
				int nItem = m_ListTrack.InsertItem(nRow, ct.strRadarIP); // Cột 0: Radar
				m_ListTrack.SetItemText(nItem, 1, strTN);                // Cột 1: TN
				m_ListTrack.SetItemText(nItem, 2, strPos);               // Cột 2: Position
				m_ListTrack.SetItemText(nItem, 3, strHdgSpd);            // Cột 3: Hdg-Spd
				m_ListTrack.SetItemText(nItem, 4, strAlti);              // Cột 4: Alti
				m_ListTrack.SetItemText(nItem, 5, strType);              // Cột 5: Type
				m_ListTrack.SetItemText(nItem, 6, strIden);              // Cột 6: Iden

																		 // Cột 7: TQ (Tự làm giả một con số chất lượng tín hiệu)
				m_ListTrack.SetItemText(nItem, 7, _T("99"));

				nRow++;
			}
		}

		// Cập nhật dòng Text tổng số mục tiêu
		CString strTotal;
		strTotal.Format(_T("Tổng số mục tiêu đang bám: %d"), nRow);
		SetDlgItemText(IDC_STATIC_INFO, strTotal); // Nhớ tạo ID IDC_STATIC_INFO cho dòng text này nhé
	}

	// 5. Bật lại tính năng vẽ
	m_ListTrack.SetRedraw(TRUE);
}
