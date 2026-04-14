
// C2CenterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "C2Center.h"
#include "C2CenterDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CC2CenterDlg dialog



CC2CenterDlg::CC2CenterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_C2CENTER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CC2CenterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CC2CenterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SESS, &CC2CenterDlg::OnBnClickedBtnSess)
	ON_BN_CLICKED(IDC_BTN_TRACK, &CC2CenterDlg::OnBnClickedBtnTrack)
	ON_BN_CLICKED(IDC_BTN_MONITOR, &CC2CenterDlg::OnBnClickedBtnMonitor)
END_MESSAGE_MAP()


// CC2CenterDlg message handlers

BOOL CC2CenterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_dlgSess.Create(IDD_SESS_DIALOG, this);
	m_dlgTrack.Create(IDD_TRACK_DIALOG, this);
	m_dlgMonitor.Create(IDD_MONITOR_DIALOG, this);
	SetTimer(1, 5000, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CC2CenterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CC2CenterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CC2CenterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CC2CenterDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		// Cứ mỗi 5 giây, gọi Client Socket gửi chữ [I_AM_ALIVE]
		m_ClientSocket.SendHeartbeat();
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CC2CenterDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	m_ClientSocket.Close(); // Đóng mạng khi tắt app
}

void CC2CenterDlg::AddToMonitor(CString strLog)
{
	// Nếu cửa sổ Monitor đang mở thì in log
	if (m_dlgMonitor.GetSafeHwnd() != NULL)
	{
		// Lưu ý: Bạn cần phải định nghĩa m_listLog và hàm AddLog trong DlgMonitor.h / .cpp 
		// y hệt như lúc làm phần mềm RadSim nhé!
		m_dlgMonitor.AddLog(strLog);
	}
}

void CC2CenterDlg::ProcessReceivedTrack(CString strSourceIP, AsterixTrack track)
{
	// 1. In ra màn hình Giám sát
	CString strLog;
	strLog.Format(_T("[RX] Nguồn: %s | Nhận quỹ đạo: %02d | Vị trí: %.4f - %.4f"),
		strSourceIP, track.nTrackNumber, track.fLat, track.fLon);
	AddToMonitor(strLog);

	// 2. Tìm xem mục tiêu này của Radar này đã có trong Kho chứa chưa?
	bool bFound = false;
	for (size_t i = 0; i < m_listReceivedTracks.size(); i++)
	{
		if (m_listReceivedTracks[i].strRadarIP == strSourceIP &&
			m_listReceivedTracks[i].trackData.nTrackNumber == track.nTrackNumber)
		{
			// Đã có -> Cập nhật thông số mới
			m_listReceivedTracks[i].trackData = track;
			bFound = true;
			break;
		}
	}

	// 3. Nếu chưa có -> Thêm mới vào Kho chứa
	if (!bFound)
	{
		CenterTrack newTrack;
		newTrack.strRadarIP = strSourceIP;
		newTrack.trackData = track;
		m_listReceivedTracks.push_back(newTrack);
	}

	// 4. Báo cho cửa sổ DlgTrack biết để nó vẽ lại bảng ngay lập tức
	if (m_dlgTrack.GetSafeHwnd() != NULL)
	{
		m_dlgTrack.UpdateTrackList();
	}
}

void CC2CenterDlg::OnBnClickedBtnSess()
{
	// TODO: Add your control notification handler code here
	m_dlgSess.ShowWindow(SW_SHOW);
	m_dlgSess.SetForegroundWindow();
}


void CC2CenterDlg::OnBnClickedBtnTrack()
{
	// TODO: Add your control notification handler code here
	m_dlgTrack.ShowWindow(SW_SHOW);
	m_dlgTrack.SetForegroundWindow();
}


void CC2CenterDlg::OnBnClickedBtnMonitor()
{
	// TODO: Add your control notification handler code here
	m_dlgMonitor.ShowWindow(SW_SHOW);
	m_dlgMonitor.SetForegroundWindow();
}
