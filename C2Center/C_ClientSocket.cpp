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
						if (pMainDlg->m_dlgSess.GetSafeHwnd() != NULL) {
							pMainDlg->m_dlgSess.UpdateSessList(); // Gọi bảng cập nhật
						}
					}
				}
			}
		}
		// TRƯỜNG HỢP 2: Đã kết nối và nhận được dữ liệu (Kiểm tra CAT 062)
		else if (m_nConnectionState == 2)
		{
			// Kiểm tra nếu byte đầu tiên là 62 (CAT 062)
			if (nBytes >= 5 && buffer[0] == 62)
			{
				// Tính toán tổng chiều dài gói tin từ LEN
				int packetLen = (buffer[1] << 8) | buffer[2];

				if (nBytes >= packetLen)
				{
					// --- ĐOẠN 1: IN LOG HEX LÊN MONITOR ---
					CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
					if (pMainDlg != NULL)
					{
						CString strHex = _T("");
						CString strTemp;
						for (int i = 0; i < packetLen; i++)
						{
							strTemp.Format(_T("%02X "), buffer[i]);
							strHex += strTemp;
						}
						CString strLog;
						strLog.Format(_T("[RX from %s] %s"), m_strRadarIP, strHex);
						pMainDlg->AddToMonitor(strLog);

						// --- ĐOẠN 2: BÓC TÁCH DỮ LIỆU CAT 062 ---
						// Tái tạo lại struct AsterixTrack để đưa vào hàm ProcessReceivedTrack
						AsterixTrack trackData;
						memset(&trackData, 0, sizeof(AsterixTrack));

						// Kiểm tra FSPEC có đúng chuẩn ta vừa đóng gói (0xB9 0x80) không
						if (buffer[3] == 0xB9 && buffer[4] == 0x80)
						{
							int offset = 5;

							// Bỏ qua FRN1 (Nguồn) và FRN3 (Thời gian)
							offset += 2;
							offset += 3;

							// Lấy FRN 4: Số hiệu quỹ đạo (2 bytes)
							// Thay ".nTrackID" bằng biến ID thực tế trong struct của bạn
							int trackID = (buffer[offset] << 8) | buffer[offset + 1];
							offset += 2;
							// trackData.nTrackID = trackID; // Gán vào struct

							// Bỏ qua FRN 5 (Vị trí) - Trong thực tế bạn sẽ giải mã ở đây
							offset += 6;

							// Lấy FRN 8: Độ cao (2 bytes)
							int altitude = (buffer[offset] << 8) | buffer[offset + 1];
							offset += 2;
							// trackData.nAltitude = altitude; // Gán vào struct
						}

						// Đưa vào xử lý đồ họa
						pMainDlg->ProcessReceivedTrack(m_strRadarIP, trackData);
					}
				}
			}
		}
	}

	CAsyncSocket::OnReceive(nErrorCode);
}