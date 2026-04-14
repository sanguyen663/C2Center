#pragma once
#include "afxsock.h"
#pragma pack(push, 1) 
struct AsterixTrack {
	int nTrackNumber;      // Số hiệu quỹ đạo (TN)
	float fLat;            // Vĩ độ
	float fLon;            // Kinh độ
	float fSpeed;          // Vận tốc
	float fHeading;        // Hướng đi
	float fAltitude;       // Độ cao
	int nType;             // Kiểu loại
	char szIden[8];        // Nhận dạng (Ví dụ: "VN01")
	char cStatus;          // Trạng thái: 'N' (New), 'U' (Upd), 'D' (Del)
};
#pragma pack(pop)

class CC_ClientSocket :
	public CAsyncSocket
{
public:
	CC_ClientSocket();
	~CC_ClientSocket();
	CString m_strRadarIP;
	UINT m_nRadarPort; // Ban đầu là 10000, sau sẽ đổi thành cổng riêng
	CString m_strMyUserID;

	// Trạng thái: 0 = Chưa kết nối, 1 = Đang chờ Accept, 2 = Đã kết nối thành công
	int m_nConnectionState = 0;

	// Hàm xin kết nối
	BOOL RequestConnect(CString strIP, UINT nPort, CString strUserID);

	// Hàm gửi Nhịp tim
	void SendHeartbeat();

	// Hàm hứng mọi thứ bay tới (Cả Text Bắt tay lẫn Byte Quỹ đạo)
	virtual void OnReceive(int nErrorCode);
};

