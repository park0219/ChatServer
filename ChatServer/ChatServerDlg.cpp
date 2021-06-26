
// ChatServerDlg.cpp: 구현 파일
//
#include "pch.h"
#include "framework.h"
#include "ChatServer.h"
#include "ChatServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChatServerDlg 대화 상자

CChatServerDlg::CChatServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_client_count = 0;
}

void CChatServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_event_list);
}

BEGIN_MESSAGE_MAP(CChatServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CChatServerDlg::OnBnClickedButton1)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CChatServerDlg 메시지 처리기

BOOL CChatServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	//DB 연결 확인 코드.
	try { 
		BOOL bOpen = db.OpenEx(_T("DSN=chatDB;SERVER=192.168.0.148;PORT=1443;UID=chat;PWD=1;DATABASE=chatDB;"), CDatabase::noOdbcDialog); 
		if (bOpen) 
			record = new CRecordset(&db);
	}
	catch (CException* e) {
		e->ReportError(); 
	}

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CChatServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CChatServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CChatServerDlg::AcceptProcess(SOCKET parm_h_socket) {
	if (MAX_CLIENT_COUNT > m_client_count) {

		struct sockaddr_in client_addr;
		int sockaddr_in_size = sizeof(client_addr);

		mh_client_list[m_client_count] = accept(parm_h_socket, (SOCKADDR*)&client_addr, &sockaddr_in_size);
		WSAAsyncSelect(mh_client_list[m_client_count], m_hWnd, 27002, FD_READ | FD_CLOSE);
		
		//TODO 클라이언트 IP 주소 DB 저장
		CString ip_address;
		ip_address = inet_ntoa(client_addr.sin_addr);
		wcscpy(m_client_ip[m_client_count], ip_address);

		AddEventString(L"새로운 클라이언트가 접속했습니다: " + ip_address);
		//MessageBox(ip_address, L"새로운 클라이언트가 접속했습니다.", MB_OK);

		try {
			CString sql;
			sql.Format(L"INSERT INTO access_list (IP, TYPE, REG_TIME) VALUES ('%s', '1', getdate())", ip_address);
			db.ExecuteSQL(sql);

			BOOL result = record->Open(CRecordset::snapshot, L"SELECT CHAT_CONTENT, IP, DATEPART(HOUR, REG_TIME) AS HOUR, DATEPART(MINUTE, REG_TIME) AS MIN FROM chat WHERE convert(varchar(8), REG_TIME, 112) = convert(varchar(8), getdate(), 112) ORDER BY CHAT_SEQ");
			if (result == true) {
				if (record->GetRecordCount() > 0) {
					CString str = L"====이전 채팅 내용 시작 지점입니다====";
					SendFrameData(mh_client_list[m_client_count], 1, (const wchar_t*)str, (str.GetLength() + 1) * 2);
					for (int p = 0; p < 2; p++)
					{
						while (!record->IsEOF())
						{
							CString tmp;
							CString chat_content;
							CString ip;
							CString hour;
							CString min;
							record->GetFieldValue((short)0, chat_content);
							record->GetFieldValue((short)1, ip);
							record->GetFieldValue((short)2, hour);
							record->GetFieldValue((short)3, min);

							tmp.Format(L"%s/%s:%s -> %s", ip, hour, min, chat_content);
							SendFrameData(mh_client_list[m_client_count], 1, (const wchar_t*)tmp, (tmp.GetLength() + 1) * 2);
							record->MoveNext();
						}
					}
					str = L"====이전 채팅 내용 종료 지점입니다====";
					SendFrameData(mh_client_list[m_client_count], 1, (const wchar_t*)str, (str.GetLength() + 1) * 2);
				}
			}
			record->Close();
		}
		catch (CDBException* e) {
			e->ReportError();
		}

		m_client_count++;
	}
	else {

	}
}

void CChatServerDlg::AddEventString(CString parm_string) {
	int index = m_event_list.InsertString(-1, parm_string);
	m_event_list.SetCurSel(index);
}

void CChatServerDlg::ClientCloseProcess(SOCKET parm_h_socket, char parm_force_flag) {
	if (parm_force_flag == 1) {
		LINGER temp_linger = { TRUE, 0 };
		setsockopt(parm_h_socket, SOL_SOCKET, SO_LINGER, (char*)&temp_linger, sizeof(temp_linger));
	}
	closesocket(parm_h_socket);
	for (int i = 0; i < m_client_count; i++) {
		if (mh_client_list[i] == parm_h_socket) {
			m_client_count--;
			if (i != m_client_count) {
				mh_client_list[i] = mh_client_list[m_client_count];
				wcscpy(m_client_ip[i], m_client_ip[m_client_count]);
			}
		}
	}
}

void CChatServerDlg::ReveiveData(SOCKET parm_h_socket, char *parm_p_buffer, int parm_size) {
	int current_size, total_size = 0, retry_count = 0;

	while (total_size < parm_size) {
		current_size = recv(parm_h_socket, parm_p_buffer + total_size, parm_size - total_size, 0);

		if (current_size == SOCKET_ERROR) {
			retry_count++;
			Sleep(50);
			if (retry_count > 5) {
				break;
			}
		}
		else {
			retry_count = 0;
			total_size = total_size + current_size;
		}
	}
}

void CChatServerDlg::SendFrameData(SOCKET parm_h_socket, unsigned char parm_id, const void* parm_p_data, int parm_size) {
	char* p_send_data = new char[parm_size + 4];

	*p_send_data = 27;
	*(unsigned short int*)(p_send_data + 1) = parm_size;
	*(p_send_data + 3) = parm_id;

	memcpy(p_send_data + 4, parm_p_data, parm_size);

	send(parm_h_socket, p_send_data, parm_size + 4, 0);

	delete[] p_send_data;
}

LRESULT CChatServerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if (27001 == message) {
		AcceptProcess((SOCKET)wParam);
	}
	else if (27002 == message) { //FD_READ, FDCLOSE
		SOCKET h_socket = (SOCKET)wParam; //mh_listen_socket 핸들값과 동일
		if (WSAGETSELECTEVENT(lParam) == FD_READ) {
			WSAAsyncSelect(h_socket, m_hWnd, 27002, FD_CLOSE);

			char key, network_message_id;
			recv(h_socket, &key, 1, 0);
			if (key == 27) {
				
				unsigned short int body_size;
				recv(h_socket, (char*)&body_size, sizeof(body_size), 0);
				recv(h_socket, (char*)&network_message_id, 1, 0);

				if (body_size > 0) {
					char* p_body_data = new char[body_size];

					ReveiveData(h_socket, p_body_data, body_size);

					if (network_message_id == 1) { //채팅
						//실제로 클라이언트가 보내준 데이터를 처리(p_body_data)
						//TODO DB 
						
						CString str;
						CString sql;
						CTime cTime = CTime::GetCurrentTime();
						CString strTime;
						strTime.Format(L"%d:%d", cTime.GetHour(), cTime.GetMinute());

						for (int i = 0; i < m_client_count; i++) {
							if (h_socket == mh_client_list[i]) {
								str.Format(L"%s/%s -> %s", m_client_ip[i], strTime, p_body_data);
								sql.Format(L"INSERT INTO chat (CHAT_CONTENT, IP, REG_TIME) VALUES ('%s', '%s', getdate());", p_body_data, m_client_ip[i]);
								break;
							}
						}

						AddEventString(str);
						
						for (int i = 0; i < m_client_count; i++) {
							SendFrameData(mh_client_list[i], 1, (const wchar_t*)str, (str.GetLength() + 1) * 2);
						}

						try {
							db.ExecuteSQL(sql);
						}
						catch (CDBException* e) {
							e->ReportError();
						}

					}
					delete[] p_body_data;
				}

				WSAAsyncSelect(h_socket, m_hWnd, 27002, FD_CLOSE | FD_READ);
			}
			else {
				ClientCloseProcess(h_socket, 1);
			}
		}
		else { //FD_CLOSE
			CString ip_address;
			for (int i = 0; i < m_client_count; i++) {
				if (h_socket == mh_client_list[i]) {
					ip_address = m_client_ip[i];
					break;
				}
			}
			ClientCloseProcess(h_socket, 0);
			AddEventString(L"클라이언트가 접속을 해제했습니다: " + ip_address);
			try {
				CString sql;
				sql.Format(L"INSERT INTO access_list (IP, TYPE, REG_TIME) VALUES ('%s', '2', getdate())", ip_address);
				db.ExecuteSQL(sql);
			}
			catch (CDBException* e) {
				e->ReportError();
			}
		}
	}

	return CDialogEx::WindowProc(message, wParam, lParam);
}


void CChatServerDlg::OnBnClickedButton1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	
	CString str;
	GetDlgItemText(IDC_IP_EDIT, str);

	if (!str.IsEmpty()) {
		mh_listen_socket = socket(AF_INET, SOCK_STREAM, 0); //TCP

		struct sockaddr_in srv_addr;
		srv_addr.sin_family = AF_INET;
		srv_addr.sin_addr.s_addr = inet_addr((CStringA)str); //ipconfig
		srv_addr.sin_port = htons(20001);

		bind(mh_listen_socket, (LPSOCKADDR)&srv_addr, sizeof(srv_addr));
		listen(mh_listen_socket, 1);
		AddEventString(L"클라이언트의 접속을 허락합니다...");
		WSAAsyncSelect(mh_listen_socket, m_hWnd, 27001, FD_ACCEPT);
		GetDlgItem(IDC_IP_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	}
}


void CChatServerDlg::OnDestroy()
{
	db.Close();
	CDialogEx::OnDestroy();
}



BOOL CChatServerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (WM_KEYDOWN == pMsg->message)
	{
		if (VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam)
		{
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
