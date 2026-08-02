// WZAsyncEngine.cpp
#include "stdafx.h"
#include "WZAsyncEngine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ----------------------------------------------------------------------
// CWZAsyncEngine
// ----------------------------------------------------------------------

CWZAsyncEngine::CWZAsyncEngine()
{
	for (int n = 0; n < 100; n++)
	{
		memset(&sRecvStack[n], 0, sizeof(STRUCT_RECVSTACK));
	}

	memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
	memset(m_SendBuf, 0, sizeof(m_SendBuf));

	m_nRecvBufLen = 0;
	m_nSendBufLen = 0;

	m_LogPrint = 0;
	m_logfp = NULL;

	WZAsyncEngine_ProtocolCoreFunc = NULL;
	WZAsyncEngine_CloseFunc = NULL;
	WZAsyncEngine_AcceptFunc = NULL;
}

CWZAsyncEngine::~CWZAsyncEngine()
{
	LogPrintOff();
}

// ----------------------------------------------------------------------
// Connect
// ----------------------------------------------------------------------

int CWZAsyncEngine::Connect(HWND hWnd, char* ip_addr, u_short port, u_int WinMsgNum)
{
	if (m_socket != INVALID_SOCKET)
	{
		CloseSocket();
	}

	CreateSocket(hWnd);

	if (!hWnd)
	{
		MessageBoxA(0, "윈도우 핸들 에러", "Error", 0);
		return 0;
	}

	sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip_addr);

	if (addr.sin_addr.s_addr == INADDR_NONE)
	{
		hostent* host = gethostbyname(ip_addr);

		if (!host)
		{
			return 2;
		}
		memcpy(&addr.sin_addr, *host->h_addr_list, host->h_length);
	}

	if (::connect(m_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		int Error = WSAGetLastError();
		LogPrint("Connect error (%d)", Error);

		if (Error != WSAEWOULDBLOCK)
		{
			CloseSocket(m_socket);
			return 0;
		}
	}

	if (WSAAsyncSelect(m_socket, m_hWnd, WinMsgNum, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
	{
		CloseSocket();
		return 0;
	}

	SetConnect(1);

	return 1;
}

// ----------------------------------------------------------------------
// CreateServer
// ----------------------------------------------------------------------

int CWZAsyncEngine::CreateServer(char* ip_addr, u_short port, u_int WinServerMsg)
{
	UNREFERENCED_PARAMETER(ip_addr);

	if (m_socket != INVALID_SOCKET)
	{
		CloseSocket(m_socket);
	}

	CreateSocket(m_hWnd);

	if (!m_hWnd)
	{
		MsgBox((char*)"윈도우 핸들 에러");
		return 0;
	}

	sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(m_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		LogPrint("Server bind error %d", WSAGetLastError());
		CloseSocket(m_socket);
		return 0;
	}

	if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		LogPrint("Server listen error %d", WSAGetLastError());
		CloseSocket(m_socket);
		return 0;
	}

	if (WSAAsyncSelect(m_socket, m_hWnd, WinServerMsg, FD_READ | FD_WRITE | FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
	{
		LogPrint("Server WSAAsyncSelect error %d", WSAGetLastError());
		CloseSocket(m_socket);
		return 0;
	}
	return 1;
}

// ----------------------------------------------------------------------
// Send
// ----------------------------------------------------------------------

int CWZAsyncEngine::send(SOCKET socket, char* buf, int len)
{
	int nLeft = len;
	int Ofs = 0;

	while (1)
	{
		int nResult = ::send(socket, &buf[Ofs], len - Ofs, 0);

		if (nResult == SOCKET_ERROR)
		{
			break;
		}

		if (nResult > 0)
		{
			Ofs += nResult;
			nLeft -= nResult;

			if (nLeft > 0)
			{
				continue;
			}
		}

		return 1;
	}

	if (WSAGetLastError() == WSAEWOULDBLOCK)
	{
		if ((len + m_nSendBufLen) <= 4096)
		{
			memcpy(&m_SendBuf[m_nSendBufLen], buf, nLeft);
			m_nSendBufLen += nLeft;
		}
		else
		{
			CloseSocket();
		}
		return 0;
	}
	CloseSocket();
	return 0;
}

int CWZAsyncEngine::send(char* buf, int len)
{
	send(m_socket, buf, len);

	return 1;
}

// ----------------------------------------------------------------------
// FDWriteSend
// ----------------------------------------------------------------------

int CWZAsyncEngine::FDWriteSend()
{
	int Ofs = 0;

	while (m_nSendBufLen > 0)
	{
		int nResult = ::send(m_socket, (const char*)&m_SendBuf[Ofs], m_nSendBufLen - Ofs, 0);
		if (nResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				LogPrint("FDWriteSend error %d", WSAGetLastError());
				CloseSocket();
				return 0;
			}
			return 1;
		}

		if (nResult <= 0)
		{
			CloseSocket();
			return 0;
		}

		Ofs += nResult;
		m_nSendBufLen -= nResult;
	}

	return 1;
}

// ----------------------------------------------------------------------
// Logging
// ----------------------------------------------------------------------

void CWZAsyncEngine::LogPrintOn()
{
	m_LogPrint = 1;

	m_logfp = fopen("wsctlc.log", "wt");
}

void CWZAsyncEngine::LogPrintOff()
{
	if (m_LogPrint)
	{
		m_LogPrint = 0;

		if (m_logfp)
		{
			fclose(m_logfp);
			m_logfp = NULL;
		}
	}
}

void CWZAsyncEngine::LogHexPrint(unsigned char* buf, int size)
{
	if (!m_LogPrint)
	{
		return;
	}

	for (int n = 0; n < size; n++)
	{
		fprintf(m_logfp, "%02X ", buf[n]);
	}

	fprintf(m_logfp, "\n");
}

void CWZAsyncEngine::LogPrint(char* szlog, ...)
{
	if (!m_LogPrint)
	{
		return;
	}

	char szBuffer[260];

	memset(szBuffer, 0, sizeof(szBuffer));

	va_list arg;
	va_start(arg, szlog);

	vsprintf(szBuffer, szlog, arg);

	va_end(arg);

	fprintf(m_logfp, "%s\n", szBuffer);
}

// ----------------------------------------------------------------------
// Recv
// ----------------------------------------------------------------------

int CWZAsyncEngine::recv()
{
	int nResult = ::recv(m_socket, (char*)&m_RecvBuf[m_nRecvBufLen], 4096 - m_nRecvBufLen, 0);

	if (m_LogPrint)
	{
		LogHexPrint(m_RecvBuf, nResult);
	}

	if (nResult == 0)
	{
		return 1;
	}

	if (nResult == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			LogPrint("recv error %d", WSAGetLastError());
		}
		return 1;
	}

	m_nRecvBufLen += nResult;

	if (m_nRecvBufLen < 3)
	{
		return 3;
	}

	int bOfs = 0;

	while (true)
	{
		int size;
		unsigned char headcode;

		size = 0;
		headcode = 0;

		if (m_RecvBuf[bOfs] == 0xC1)
		{
			size = m_RecvBuf[bOfs + 1];
			headcode = m_RecvBuf[bOfs + 2];
		}
		else if (m_RecvBuf[bOfs] == 0xC2)
		{
			size = m_RecvBuf[bOfs + 2] + (m_RecvBuf[bOfs + 1] << 8);
			headcode = m_RecvBuf[bOfs + 3];
		}
		else
		{
			m_nRecvBufLen = 0;
			return 0;
		}

		if (size <= 0)
		{
			return 0;
		}

		if (size > m_nRecvBufLen)
		{
			break;
		}

		if (WZAsyncEngine_ProtocolCoreFunc)
		{
			WZAsyncEngine_ProtocolCoreFunc(headcode, &m_RecvBuf[bOfs], size);
		}

		bOfs += size;
		m_nRecvBufLen -= size;

		if (m_nRecvBufLen <= 0)
		{
			return 0;
		}
	}

	if (bOfs > 0 && m_nRecvBufLen >= 1)
	{
		memcpy(m_RecvBuf, &m_RecvBuf[bOfs], m_nRecvBufLen);
	}

	return 0;
}

// ----------------------------------------------------------------------
// Stack
// ----------------------------------------------------------------------

int CWZAsyncEngine::Push(unsigned char* buf, int len)
{
	if (len > 4096)
	{
		return 2;
	}

	for (int n = 0; n < 100; n++)
	{
		if (!sRecvStack[n].used)
		{
			sRecvStack[n].used = 1;
			sRecvStack[n].len = len;
			memcpy(sRecvStack[n].buf, buf, len);
			return 0;
		}
	}

	return 1;
}

unsigned char* CWZAsyncEngine::Pop()
{
	for (int n = 0; n < 100; n++)
	{
		if (sRecvStack[n].used == 1)
		{
			sRecvStack[n].used = 0;
			return sRecvStack[n].buf;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------
// WinMsgProc
// ----------------------------------------------------------------------

int CWZAsyncEngine::WinMsgProc(SOCKET wParam, short lParam)
{
	switch (lParam)
	{
	case FD_READ:
		recv();
		break;

	case FD_WRITE:
		FDWriteSend();
		break;

	case FD_ACCEPT:

		if (WZAsyncEngine_AcceptFunc)
		{
			WZAsyncEngine_AcceptFunc(this);
		}

		break;

	case FD_CLOSE:

		closesocket(wParam);

		SetConnect(0);

		if (WZAsyncEngine_CloseFunc)
		{
			WZAsyncEngine_CloseFunc();
		}

		break;

	default:
		return 1;
	}

	return 1;
}

// ----------------------------------------------------------------------
// Callback Register
// ----------------------------------------------------------------------

void CWZAsyncEngine::ProtocolCoreFuncRegister(int (*lpFunc)(BYTE, BYTE*, int))
{
	WZAsyncEngine_ProtocolCoreFunc = lpFunc;
}

void CWZAsyncEngine::CloseFuncRegister(int(*lpFunc)())
{
	WZAsyncEngine_CloseFunc = lpFunc;
}

void CWZAsyncEngine::AcceptFuncRegister(int(*lpFunc)(void*))
{
	WZAsyncEngine_AcceptFunc = lpFunc;
}
