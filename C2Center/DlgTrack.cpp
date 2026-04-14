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
	// Tắt vẽ lại (Redraw) tạm thời để nạp dữ liệu nhanh hơn và không bị giật
	m_ListTrack.SetRedraw(FALSE);

	m_ListTrack.DeleteAllItems(); // Xóa sạch dữ liệu cũ trên bảng

	int nItem = 0;
	// Duyệt qua toàn bộ quỹ đạo đang lưu trong map (từ bước trước)
	for (auto const& pair : m_mapTracks)
	{
		AsterixTrack t = pair.second;

		// --- 1. Định dạng dữ liệu dạng chuỗi (CString) ---
		CString strTN, strPos, strSpeedHdg, strAlti, strType, strIden, strStatus;

		strTN.Format(_T("%02d"), t.nTrackNumber);
		strPos.Format(_T("%.4f - %.4f"), t.fLat, t.fLon);
		strSpeedHdg.Format(_T("%.1f - %.1f"), t.fHeading, t.fSpeed);
		strAlti.Format(_T("%.0f m"), t.fAltitude);
		strType.Format(_T("%d"), t.nType);

		// Xử lý chuỗi char[] sang CString (Đề phòng chuỗi szIden không có ký tự kết thúc null)
		strIden = CString(t.szIden, strnlen_s(t.szIden, 8));

		// Diễn giải ký tự trạng thái (N: New, U: Update, D: Delete)
		if (t.cStatus == 'N') strStatus = _T("Mới (New)");
		else if (t.cStatus == 'U') strStatus = _T("Đang bám");
		else if (t.cStatus == 'D') strStatus = _T("Mất dấu");
		else strStatus = CString(t.cStatus);

		// --- 2. Đẩy lên bảng UI ---
		m_ListTrack.InsertItem(nItem, _T("RDR_01"));	   // [0] Nguồn
		m_ListTrack.SetItemText(nItem, 1, strTN);          // [1] TN
		m_ListTrack.SetItemText(nItem, 2, strPos);         // [2] Vị trí
		m_ListTrack.SetItemText(nItem, 3, strSpeedHdg);    // [3] Hướng - Tốc độ
		m_ListTrack.SetItemText(nItem, 4, strAlti);        // [4] Độ cao
		m_ListTrack.SetItemText(nItem, 5, strType);        // [5] Loại
		m_ListTrack.SetItemText(nItem, 6, strIden);        // [6] Nhận dạng
		m_ListTrack.SetItemText(nItem, 7, strStatus);      // [7] Trạng thái

		nItem++;
	}

	// Bật lại việc vẽ màn hình
	m_ListTrack.SetRedraw(TRUE);
}

BOOL CDlgTrack::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 1. Cấu hình giao diện cho Bảng (List Control)
	// LVS_EX_FULLROWSELECT: Click vào 1 ô là chọn bôi xanh cả dòng
	// LVS_EX_GRIDLINES: Hiện đường kẻ lưới như Excel
	// LVS_EX_DOUBLEBUFFER: Chống giật (flickering) khi cập nhật dữ liệu liên tục
	m_ListTrack.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

	// 2. Tạo các cột (Giống hệt RadSim, thêm cột Nguồn)
	// Cú pháp: InsertColumn(Cột_số, Tên_cột, Căn_lề, Độ_rộng)
	m_ListTrack.InsertColumn(0, _T("Nguồn"), LVCFMT_CENTER, 60);
	m_ListTrack.InsertColumn(1, _T("TN"), LVCFMT_CENTER, 40);
	m_ListTrack.InsertColumn(2, _T("Vị trí (Lat - Lon)"), LVCFMT_CENTER, 100);
	m_ListTrack.InsertColumn(3, _T("Hướng - Vận tốc"), LVCFMT_CENTER, 120);
	m_ListTrack.InsertColumn(4, _T("Độ cao"), LVCFMT_CENTER, 50);
	m_ListTrack.InsertColumn(5, _T("Loại"), LVCFMT_CENTER, 50);
	m_ListTrack.InsertColumn(6, _T("Nhận dạng"), LVCFMT_CENTER, 80);
	m_ListTrack.InsertColumn(7, _T("Trạng thái"), LVCFMT_CENTER, 90);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDlgTrack::ProcessNewTrack(AsterixTrack track)
{
	// Cập nhật hoặc chèn mới vào Map bằng số hiệu quỹ đạo
	m_mapTracks[track.nTrackNumber] = track;

	// Gọi hàm cập nhật giao diện
	UpdateTrackList();
}