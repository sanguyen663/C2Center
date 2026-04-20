// DlgMonitor.cpp : implementation file
//

#include "stdafx.h"
#include "C2Center.h"
#include "DlgMonitor.h"
#include "afxdialogex.h"


// CDlgMonitor dialog

IMPLEMENT_DYNAMIC(CDlgMonitor, CDialogEx)

CDlgMonitor::CDlgMonitor(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MONITOR_DIALOG, pParent)
{

}

CDlgMonitor::~CDlgMonitor()
{
}

void CDlgMonitor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
}


BEGIN_MESSAGE_MAP(CDlgMonitor, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_START, &CDlgMonitor::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CDlgMonitor::OnBnClickedBtnPause)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CDlgMonitor::OnBnClickedBtnClear)
END_MESSAGE_MAP()


// CDlgMonitor message handlers

void CDlgMonitor::AddLog(CString strLog)
{
	// 1. Lấy thời gian hiện tại
	CTime currentTime = CTime::GetCurrentTime();
	CString strTime = currentTime.Format(_T("[%H:%M:%S] "));

	// 2. Thiết lập độ dài tối đa cho mỗi dòng (có thể tùy chỉnh)
	int nMaxLength = 85;
	int nLength = strLog.GetLength();

	if (m_listLog.GetSafeHwnd() == NULL) return;

	// TRƯỜNG HỢP 1: Chuỗi ngắn -> In trên 1 dòng
	if (nLength + strTime.GetLength() <= nMaxLength)
	{
		m_listLog.AddString(strTime + strLog);
	}
	// TRƯỜNG HỢP 2: Chuỗi dài -> Cắt thành nhiều dòng
	else
	{
		int nOffset = 0;
		bool bFirstLine = true;

		while (nOffset < nLength)
		{
			if (bFirstLine)
			{
				// Dòng đầu tiên: Bị chiếm mất 11 ký tự bởi chuỗi thời gian
				int nChunkSize = nMaxLength - strTime.GetLength();
				CString strChunk = strLog.Mid(nOffset, nChunkSize);

				m_listLog.AddString(strTime + strChunk);
				nOffset += nChunkSize;
				bFirstLine = false;
			}
			else
			{
				// Các dòng tiếp theo: Bỏ thụt lề, lấy trọn vẹn chiều rộng màn hình (nMaxLength)
				CString strChunk = strLog.Mid(nOffset, nMaxLength);

				m_listLog.AddString(strChunk); // Ghi thẳng, không thêm khoảng trắng
				nOffset += nMaxLength;
			}
		}
	}

	// 3. Tự động cuộn xuống dòng mới nhất
	int nCount = m_listLog.GetCount();
	if (nCount > 0)
	{
		m_listLog.SetTopIndex(nCount - 1);
	}
}

void CDlgMonitor::OnBnClickedBtnStart()
{
	// TODO: Add your control notification handler code here
	m_bIsPaused = false;
	CButton* pBtnPause = (CButton*)GetDlgItem(IDC_BTN_PAUSE);
	if (pBtnPause != NULL)
	{
		pBtnPause->SetWindowText(_T("Tạm dừng (Pause)"));
	}

	CTime time = CTime::GetCurrentTime();
	CString strTime = time.Format(_T("[%H:%M:%S] "));
	m_listLog.AddString(strTime + _T("[SYSTEM] ===== BẮT ĐẦU GIÁM SÁT DỮ LIỆU ====="));
}


void CDlgMonitor::OnBnClickedBtnPause()
{
	// TODO: Add your control notification handler code here
	m_bIsPaused = !m_bIsPaused;
	CButton* pBtnPause = (CButton*)GetDlgItem(IDC_BTN_PAUSE);
	if (pBtnPause != NULL)
	{
		if (m_bIsPaused)
			pBtnPause->SetWindowText(_T("Tiếp tục (Resume)"));
		else
			pBtnPause->SetWindowText(_T("Tạm dừng (Pause)"));
	}
}


void CDlgMonitor::OnBnClickedBtnClear()
{
	// TODO: Add your control notification handler code here
	m_listLog.ResetContent();
}
