#include "stdafx.h"
#include "C_ClientSocket.h"
#include "C2Center.h"
#include "C2CenterDlg.h"

CC_ClientSocket::CC_ClientSocket()
{
}


CC_ClientSocket::~CC_ClientSocket()
{
}

// 1. Hàm bắt đầu xin kết nối
BOOL CC_ClientSocket::RequestConnect(CString strIP, UINT nPort, CString strUserID)
{
	m_strRadarIP = strIP;
	m_nRadarPort = nPort;
	m_strMyUserID = strUserID;

	// SỬA Ở ĐÂY: Đóng socket cũ trước khi tạo mới để tránh lỗi Bind
	Close();

	// Tạo cổng ngẫu nhiên ở máy cục bộ để kết nối đi
	if (!Create(0, SOCK_DGRAM))
	{
		AfxMessageBox(_T("Lỗi khởi tạo mạng. Vui lòng thử lại!"));
		return FALSE;
	}

	// Gửi gói tin [REQ_CONNECT]
	CString strReq = _T("[REQ_CONNECT]");
	char buffer[256];
	strcpy_s(buffer, CT2A(strReq));

	SendTo(buffer, strlen(buffer), m_nRadarPort, m_strRadarIP);
	m_nConnectionState = 1; // Chuyển sang trạng thái Đang chờ

	return TRUE;
}

// 2. Hàm gửi Nhịp tim duy trì kết nối
void CC_ClientSocket::SendHeartbeat()
{
	if (m_nConnectionState == 2)
	{
		char buffer[] = "[I_AM_ALIVE]";
		// Gửi vào cổng mới (Ví dụ: 10001) thay vì 10000
		SendTo(buffer, strlen(buffer), m_nRadarPort, m_strRadarIP);
	}
}

// 3. Hàm hứng mọi dữ liệu bay tới
void CC_ClientSocket::OnReceive(int nErrorCode)
{
	char buffer[4096];
	memset(buffer, 0, sizeof(buffer));
	CString strSenderIP;
	UINT nSenderPort;

	int nBytes = ReceiveFrom(buffer, 4096, strSenderIP, nSenderPort);

	if (nBytes > 0)
	{
		// TRƯỜNG HỢP 1: Đang chờ Bắt tay và nhận được chữ
		if (m_nConnectionState == 1)
		{
			CString strMsg(buffer);
			// Nếu nhận được gói tin bắt đầu bằng [ACCEPT:
			if (strMsg.Find(_T("[ACCEPT:")) == 0)
			{
				int nCloseBracket = strMsg.Find(_T("]"));
				if (nCloseBracket != -1)
				{
					// Cắt lấy số cổng mới (Ví dụ lấy "10001" từ "[ACCEPT:10001]")
					CString strNewPort = strMsg.Mid(8, nCloseBracket - 8);

					// Cập nhật lại cổng đích để từ giờ nói chuyện qua đường hầm này
					m_nRadarPort = _ttoi(strNewPort);
					m_nConnectionState = 2; // KẾT NỐI THÀNH CÔNG!

					// Báo cáo lên màn hình Monitor
					CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
					if (pMainDlg != NULL)
					{
						// Thêm IP của Radar vừa kết nối vào danh sách quản lý
						pMainDlg->m_listConnectedIPs.push_back(m_strRadarIP);

						CString strLog;
						strLog.Format(_T("[SYSTEM] Radar %s đã sẵn sàng."), m_strRadarIP);
						pMainDlg->AddToMonitor(strLog);
					}
				}
			}
		}
		// TRƯỜNG HỢP 2: Đã kết nối và nhận được dữ liệu Quỹ đạo (Đúng kích thước struct)
		else if (m_nConnectionState == 2 && nBytes == sizeof(AsterixTrack))
		{
			AsterixTrack* pTrack = (AsterixTrack*)buffer;

			// Gửi dữ liệu này lên Main Dialog để xử lý hiển thị
			CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
			if (pMainDlg != NULL)
			{
				// Truyền thêm m_strRadarIP vào trước *pTrack
				pMainDlg->ProcessReceivedTrack(m_strRadarIP, *pTrack);
			}
		}
	}

	CAsyncSocket::OnReceive(nErrorCode);
}