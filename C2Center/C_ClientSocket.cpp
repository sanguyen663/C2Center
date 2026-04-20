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
						bool frnPresent[30] = { false }; // Ta chỉ cần check đến FRN 21 cho 3 byte FSPEC

						// Đọc FSPEC (tối đa 3 byte theo thiết kế hiện tại)
						while (fspecLen < 4) {
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

						// FRN 11: I062/180 - Calculated Track Velocity Polar (4 bytes)
						if (frnPresent[11]) {
							uint16_t speedRaw = ((BYTE)buffer[offset] << 8) | (BYTE)buffer[offset + 1];
							uint16_t headingRaw = ((BYTE)buffer[offset + 2] << 8) | (BYTE)buffer[offset + 3];

							// Đổi ngược lại thành số thập phân dựa theo LSB [cite: 567, 570]
							trackData.fSpeed = (float)(speedRaw * 0.1f);
							trackData.fHeading = (float)(headingRaw * (360.0f / 65536.0f));

							offset += 4;
						}

						// FRN 12: I062/040 - Track Number (2 bytes)
						if (frnPresent[12]) {
							trackData.nTrackNumber = ((BYTE)buffer[offset] << 8) | (BYTE)buffer[offset + 1];
							offset += 2;
						}

						// FRN 13: I062/080 - Track Status (Variable)
						if (frnPresent[13]) {
							BYTE octet1 = (BYTE)buffer[offset++];

							if (octet1 & 1) { // Đọc Octet 2
								BYTE octet2 = (BYTE)buffer[offset++];
								if (octet2 & 0x20) trackData.cStatus = 'N';
								else if (octet2 & 0x40) trackData.cStatus = 'D';
								else trackData.cStatus = 'U';

								if (octet2 & 1) { // Đọc Octet 3
									BYTE octet3 = (BYTE)buffer[offset++];

									if (octet3 & 1) { // Đọc Octet 4 (Chứa Loại Mục Tiêu) [cite: 586, 587]
										BYTE octet4 = (BYTE)buffer[offset++];
										// Giải mã Target Type từ bit 5->2
										trackData.nType = (octet4 >> 1) & 0x0F;

										// Bỏ qua các Octet mở rộng thừa nếu có
										while (octet4 & 1) {
											octet4 = (BYTE)buffer[offset++];
										}
									}
								}
							}
							else {
								trackData.cStatus = 'U';
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
						
						// FRN 24: I062/245 - Target Identification (7 bytes)
						if (frnPresent[24]) {
							offset++; // Nhảy qua byte STI
							uint64_t encoded = 0;
							for (int i = 0; i < 6; i++) {
								encoded = (encoded << 8) | (BYTE)buffer[offset++];
							}

							char temp[9];
							memset(temp, 0, 9); // BẮT BUỘC: Xóa trắng biến tạm

							for (int i = 7; i >= 0; i--) {
								uint8_t val = (uint8_t)(encoded & 0x3F);
								encoded >>= 6;
								if (val >= 1 && val <= 26) temp[i] = val + 64; // A-Z
								else if (val >= 48 && val <= 57) temp[i] = val; // 0-9
								else temp[i] = ' '; // Khoảng trắng
							}

							// Cắt bỏ khoảng trắng thừa ở cuối
							for (int i = 7; i >= 0; i--) {
								if (temp[i] == ' ') temp[i] = '\0';
								else break;
							}

							// Xóa sạch trường szIden trong struct trước khi copy
							memset(trackData.szIden, 0, sizeof(trackData.szIden));
							strcpy_s(trackData.szIden, temp);
						}

						// FRN 26: I062/501 - Track Quality (1 byte)
						if (frnPresent[26]) {
							// Đọc 1 byte ra và gán vào biến nQuality
							trackData.nQuality = (BYTE)buffer[offset++];
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