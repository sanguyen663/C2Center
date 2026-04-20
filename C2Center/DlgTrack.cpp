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
	CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
	if (pMainDlg == NULL) return;

	// Lặp qua danh sách mục tiêu nhận được từ Kho chứa
	for (size_t i = 0; i < pMainDlg->m_listReceivedTracks.size(); i++)
	{
		AsterixTrack track = pMainDlg->m_listReceivedTracks[i].trackData;
		CString strIP = pMainDlg->m_listReceivedTracks[i].strRadarIP;

		// 1. Format dữ liệu
		CString strTN, strPos, strHdgSpd, strAlti, strType, strIden, strStatus, strTQ;
		strTN.Format(_T("%02d"), track.nTrackNumber);
		strPos.Format(_T("%.3f - %.3f"), track.fLat, track.fLon);
		strHdgSpd.Format(_T("%.0f - %.0f"), track.fHeading, track.fSpeed);
		strAlti.Format(_T("%.0f"), track.fAltitude);
		strType.Format(_T("%d"), track.nType);
		strTQ.Format(_T("%d"), track.nQuality);
		strIden = CA2T(track.szIden);

		if (track.cStatus == 'N') strStatus = _T("New");
		else if (track.cStatus == 'D') strStatus = _T("Del");
		else strStatus = _T("Upd");

		// 2. KIỂM TRA MỤC TIÊU NÀY ĐÃ CÓ TRÊN BẢNG CHƯA
		bool bFound = false;
		int nRowCount = m_ListTrack.GetItemCount();

		for (int row = 0; row < nRowCount; row++)
		{
			CString currentIP = m_ListTrack.GetItemText(row, 0);
			CString currentTN = m_ListTrack.GetItemText(row, 1);
			// Phải khớp cả IP của Radar và Số hiệu mục tiêu
			if (currentIP == strIP && currentTN == strTN)
			{
				m_ListTrack.SetItemText(row, 0, strIP);
				m_ListTrack.SetItemText(row, 1, strTN);
				m_ListTrack.SetItemText(row, 2, strPos);
				m_ListTrack.SetItemText(row, 3, strHdgSpd);
				m_ListTrack.SetItemText(row, 4, strAlti);
				m_ListTrack.SetItemText(row, 5, strType);
				m_ListTrack.SetItemText(row, 6, strIden);
				m_ListTrack.SetItemText(row, 7, strTQ);
				m_ListTrack.SetItemText(row, 8, strStatus);
				bFound = true; // Đánh dấu là đã tìm thấy và sửa xong
				break;         // Thoát vòng lặp tìm kiếm
			}
		}
		// 3. NẾU CHƯA CÓ TRÊN BẢNG -> IN THÊM DÒNG MỚI (InsertItem)
		if (!bFound)
		{
			// Thêm vào dòng cuối cùng của bảng (nRowCount)
			int nItem = m_ListTrack.InsertItem(nRowCount, strIP);
			m_ListTrack.SetItemText(nItem, 1, strTN);
			m_ListTrack.SetItemText(nItem, 2, strPos);
			m_ListTrack.SetItemText(nItem, 3, strHdgSpd);
			m_ListTrack.SetItemText(nItem, 4, strAlti);
			m_ListTrack.SetItemText(nItem, 5, strType);
			m_ListTrack.SetItemText(nItem, 6, strIden);
			m_ListTrack.SetItemText(nItem, 7, strTQ);
			m_ListTrack.SetItemText(nItem, 8, strStatus);
		}
	}
}


BOOL CDlgTrack::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_ListTrack.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

	m_ListTrack.InsertColumn(0, _T("Nguồn"), LVCFMT_CENTER, 60);
	m_ListTrack.InsertColumn(1, _T("TN"), LVCFMT_CENTER, 40);
	m_ListTrack.InsertColumn(2, _T("Vị trí (Lat - Lon)"), LVCFMT_CENTER, 100);
	m_ListTrack.InsertColumn(3, _T("Hướng - Vận tốc"), LVCFMT_CENTER, 120);
	m_ListTrack.InsertColumn(4, _T("Độ cao"), LVCFMT_CENTER, 50);
	m_ListTrack.InsertColumn(5, _T("Loại"), LVCFMT_CENTER, 50);
	m_ListTrack.InsertColumn(6, _T("Nhận dạng"), LVCFMT_CENTER, 80);
	m_ListTrack.InsertColumn(7, _T("TQ"), LVCFMT_CENTER, 20);
	m_ListTrack.InsertColumn(8, _T("Trạng thái"), LVCFMT_CENTER, 90);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDlgTrack::ProcessNewTrack(AsterixTrack track)
{
	// Cập nhật hoặc chèn mới vào Map bằng số hiệu quỹ đạo
	m_mapTracks[track.nTrackNumber] = track;

	// Gọi hàm cập nhật giao diện
	UpdateTrackList();
}

