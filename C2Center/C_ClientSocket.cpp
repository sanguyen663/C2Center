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
			if (nBytes >= 5 && (BYTE)buffer[0] == 62)
			{
				int packetLen = ((BYTE)buffer[1] << 8) | (BYTE)buffer[2];

				if (nBytes >= packetLen)
				{
					CC2CenterDlg* pMainDlg = (CC2CenterDlg*)AfxGetMainWnd();
					if (pMainDlg != NULL)
					{
						// 1. In Log Hex lên Monitor
						CString strHex = _T(""), strTemp;
						for (int i = 0; i < packetLen; i++) {
							strTemp.Format(_T("%02X "), (BYTE)buffer[i]);
							strHex += strTemp;
						}
						CString strLog;
						strLog.Format(_T("[RX from %s] %s"), m_strRadarIP, strHex);
						pMainDlg->AddToMonitor(strLog);

						// 2. Bắt đầu giải mã cấu trúc FSPEC
						int fspecLen = 0;
						bool frnPresent[22] = { false }; // Ta chỉ cần check đến FRN 21 cho 3 byte FSPEC

														 // Đọc FSPEC (tối đa 3 byte theo thiết kế hiện tại)
						while (fspecLen < 3) {
							BYTE b = (BYTE)buffer[3 + fspecLen];
							for (int i = 1; i <= 7; i++) {
								if ((b >> (8 - i)) & 1) frnPresent[(fspecLen * 7) + i] = true;
							}
							fspecLen++;
							if ((b & 1) == 0) break; // FX bit = 0 thì dừng
						}

						int offset = 3 + fspecLen;
						AsterixTrack trackData;
						memset(&trackData, 0, sizeof(AsterixTrack));

						// 3. Giải mã chi tiết từng trường dựa trên cờ báo FSPEC
						// FRN 1: I062/010 - Data Source Identifier (2 bytes)
						if (frnPresent[1]) {
							BYTE sac = (BYTE)buffer[offset];
							BYTE sic = (BYTE)buffer[offset + 1];
							// Kiểm tra SAC có phải 0x94 không (tùy chọn lọc dữ liệu)
							offset += 2;
						}

						// FRN 4: I062/070 - Time of Track Information (3 bytes)
						if (frnPresent[4]) offset += 3;

						// FRN 5: I062/105 - Position in WGS-84 (8 bytes)
						if (frnPresent[5]) {
							int32_t latRaw = ((BYTE)buffer[offset] << 24) | ((BYTE)buffer[offset + 1] << 16) |
								((BYTE)buffer[offset + 2] << 8) | (BYTE)buffer[offset + 3];
							int32_t lonRaw = ((BYTE)buffer[offset + 4] << 24) | ((BYTE)buffer[offset + 5] << 16) |
								((BYTE)buffer[offset + 6] << 8) | (BYTE)buffer[offset + 7];

							double scalePos = 180.0 / 33554432.0; // 180 / 2^25
							trackData.fLat = (float)(latRaw * scalePos);
							trackData.fLon = (float)(lonRaw * scalePos);
							offset += 8;
						}

						// FRN 7: I062/185 - Calculated Track Velocity (4 bytes)
						if (frnPresent[7]) {
							int16_t vxRaw = ((BYTE)buffer[offset] << 8) | (BYTE)buffer[offset + 1];
							trackData.fSpeed = (float)(vxRaw * 0.25); // LSB = 0.25 m/s
							offset += 4;
						}

						// FRN 12: I062/040 - Track Number (2 bytes)
						if (frnPresent[12]) {
							trackData.nTrackNumber = ((BYTE)buffer[offset] << 8) | (BYTE)buffer[offset + 1];
							offset += 2;
						}

						// FRN 13: I062/080 - Track Status (Variable)
						if (frnPresent[13]) {
							BYTE octet1 = (BYTE)buffer[offset];
							if (octet1 & 1) { // Nếu FX=1, đọc tiếp Octet 2 để lấy cStatus
								BYTE octet2 = (BYTE)buffer[offset + 1];
								if (octet2 & 0x20) trackData.cStatus = 'N';      // TSB bit (bit 6)
								else if (octet2 & 0x40) trackData.cStatus = 'D'; // TSE bit (bit 7)
								else trackData.cStatus = 'U';
								offset += 2;
							}
							else {
								trackData.cStatus = 'U';
								offset += 1;
							}
						}

						// FRN 14: I062/290 - System Track Update Ages (Compound)
						if (frnPresent[14]) {
							// Trong code RadSim ta gửi 2 byte (Map 0x80 + 1 byte data)
							offset += 2;
						}

						// FRN 18: I062/130 - Calculated Geometric Altitude (2 bytes)
						if (frnPresent[18]) {
							int16_t altRaw = ((BYTE)buffer[offset] << 8) | (BYTE)buffer[offset + 1];
							trackData.fAltitude = (float)(altRaw * 6.25); // LSB = 6.25 ft
							offset += 2;
						}

						// 4. Đưa dữ liệu đã giải mã vào kho lưu trữ và hiển thị
						pMainDlg->ProcessReceivedTrack(m_strRadarIP, trackData);
					}
				}
			}
		}
	}
	CAsyncSocket::OnReceive(nErrorCode);
}