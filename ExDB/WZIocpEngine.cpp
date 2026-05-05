// WZIocpEngine.cpp
#include "stdafx.h"
#include "WZIocpEngine.h"
#include "GameServerMng.h"

CLogToFile IOCP_ERROR_LOG("IOCP_ERROR_LOG", ".\\IOCP_ERROR_LOG", TRUE);
CWZIocpEngine* WzIoEngine;

CWZIocpEngine::CWZIocpEngine()
{
	m_ServerPort = 0;
	m_MaxListen = 5;
	m_Blank = 0;

	for (int n = 0; n < MAX_OBJECTS; n++)
	{
		lpPerHandleData[n] = new _PER_HANDLE_DATA;

		if (lpPerHandleData[n])
		{
			memset(lpPerHandleData[n], 0, sizeof(_PER_HANDLE_DATA));

			lpPerHandleData[n]->m_socket = INVALID_SOCKET;
			lpPerHandleData[n]->m_State = 0;
		}
		else
		{
			WZIOCPMsgBox("WZIOCP Memory allocation error");
		}
	}

	WZIocpEngine_AcceptFunc = NULL;
	WZIocpEngine_CloseFunc = NULL;
	WZIocpEngine_ProtocolCoreFunc = NULL;
}

CWZIocpEngine::~CWZIocpEngine()
{
	Destroy();
}

void CWZIocpEngine::SetListenCount(int max_listen)
{
	m_MaxListen = max_listen;
}

HANDLE CWZIocpEngine::GetCP()
{
	return m_cp;
}

int CWZIocpEngine::SetCP(HANDLE cp)
{
	m_cp = cp;
	return true;
}

SOCKET CWZIocpEngine::GetHDSocket(_PER_HANDLE_DATA* lpPerData)
{
	return lpPerData->m_socket;
}

void CWZIocpEngine::wz_printf(char* szlog, ...)
{
	char szBuffer[3076];

	va_list ap;
	va_start(ap, szlog);

	EnterCriticalSection(&m_cs);
	memset(szBuffer, 0, sizeof(szBuffer));
	vsprintf(szBuffer, szlog, ap);
	IOCP_ERROR_LOG.Output(szBuffer);
	LeaveCriticalSection(&m_cs);

	va_end(ap);
}

void CWZIocpEngine::wz_Bigprintf(char* szlog, ...)
{
	char string[49156];

	va_list ap;
	va_start(ap, szlog);
	EnterCriticalSection(&m_cs);
	memset(string, 0, sizeof(string));

	if (vsprintf(string, szlog, ap) < 0)
	{
		IOCP_ERROR_LOG.Output("Error");
	}

	IOCP_ERROR_LOG.Output(string);
	LeaveCriticalSection(&m_cs);

	va_end(ap);
}

BOOL CWZIocpEngine::Create(WORD server_port, int max_listen)
{
	SYSTEM_INFO SystemInfo;

	InitializeCriticalSection(&m_cs);
	GetSystemInfo(&SystemInfo);
	m_ThreadCount = SystemInfo.dwNumberOfProcessors * 2;
	m_ServerPort = server_port;

	HANDLE completionport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (!completionport)
	{
		return FALSE;
	}

	SetCP(completionport);

	if (max_listen != 5)
	{
		SetListenCount(max_listen);
	}

	m_hThreadHandle = CreateThread(NULL, 0, WZIocpEngine_AcceptThread, this, 0, &m_ThreadID);

	return (m_hThreadHandle != NULL);
}

void CWZIocpEngine::Destroy()
{
	WorkerThreadDestroy();

	if (m_hThreadHandle)
	{
		TerminateThread(m_hThreadHandle, 0);
	}

	for (int n = 0; n < MAX_OBJECTS; n++)
	{
		if (lpPerHandleData[n])
		{
			delete lpPerHandleData[n];
			lpPerHandleData[n] = NULL;
		}
	}

	DeleteCriticalSection(&m_cs);
}

int CWZIocpEngine::CheckFunc()
{
	if (!WZIocpEngine_AcceptFunc)
	{
		MessageBoxA(0, "WZIocpEngine AcceptFunc NULL", "IocpEngine", 0);
		return 0;
	}

	if (!WZIocpEngine_CloseFunc)
	{
		MessageBoxA(0, "WZIocpEngine CloseFunc NULL", "IocpEngine", 0);
		return 0;
	}

	if (!WZIocpEngine_ProtocolCoreFunc)
	{
		MessageBoxA(0, "WZIocpEngine ProtocolCoreFunc NULL", "IocpEngine", 0);
		return 0;
	}

	return 1;
}

void CWZIocpEngine::AcceptFuncRegister(int (*lpFunc)(_PER_HANDLE_DATA*))
{
	this->WZIocpEngine_AcceptFunc = lpFunc;
}

void CWZIocpEngine::CloseFuncRegister(int (*lpFunc)(_PER_HANDLE_DATA*))
{
	this->WZIocpEngine_CloseFunc = lpFunc;
}

void CWZIocpEngine::ProtocolCoreFuncRegister(int (*lpFunc)(BYTE, BYTE*, int, int))
{
	this->WZIocpEngine_ProtocolCoreFunc = lpFunc;
}

int CWZIocpEngine::Listen()
{
	sockaddr_in InternetAddr;

	m_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (m_socket == INVALID_SOCKET)
	{
		return 10;
	}

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(m_ServerPort);

	if (bind(m_socket, (sockaddr*)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		MsgBox("Bind error!!");
		return 11;
	}

	if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		return 12;
	}

	return 0;
}

int CWZIocpEngine::WorkerThreadCreate()
{
	DWORD ThreadID;

	for (int n = 0; n < m_ThreadCount; n++)
	{
		HANDLE ThreadHandle = CreateThread(NULL, 0, WZIocpEngine_WorkerThreadServer, this, 0, &ThreadID);
		if (!ThreadHandle)
		{
			return 0;
		}
		CloseHandle(ThreadHandle);
	}

	return 1;
}

int CWZIocpEngine::WorkerThreadDestroy()
{
	if (m_cp)
	{
		for (int n = 0; n < m_ThreadCount; n++)
		{
			PostQueuedCompletionStatus((HANDLE)m_cp, 0, 0, NULL);
		}
	}
	return 1;
}

_PER_HANDLE_DATA* CWZIocpEngine::Accept()
{
	sockaddr_in Addr;
	int Addrlen = sizeof(Addr);
	SOCKET AcceptSocket = WSAAccept(this->m_socket, (sockaddr*)&Addr, &Addrlen, NULL, NULL);

	if (AcceptSocket == INVALID_SOCKET)
	{
		return NULL;
	}

	EnterCriticalSection(&this->m_cs);

	int Result = this->NotUsedHandleData();

	if (Result < 0)
	{
		LeaveCriticalSection(&this->m_cs);

		closesocket(AcceptSocket);

		return NULL;
	}

	_PER_HANDLE_DATA* lpPerData = this->lpPerHandleData[Result];
	memset(lpPerData, 0, sizeof(_PER_HANDLE_DATA));
	lpPerData->m_This = Result;
	lpPerData->m_Index = GS_Add(lpPerData);

	if (lpPerData->m_Index < 0)
	{
		this->wz_printf("[AcceptError] Index Error");

		closesocket(AcceptSocket);
		LeaveCriticalSection(&this->m_cs);

		return NULL;
	}

	if (this->UpdateCompletionPort((HANDLE)AcceptSocket, lpPerData) == FALSE)
	{
		this->wz_printf("[AcceptError] CreateIoCompletionPort Error");

		closesocket(AcceptSocket);
		LeaveCriticalSection(&this->m_cs);

		return NULL;
	}

	memcpy(&lpPerData->ClientAddr, &Addr.sin_addr, sizeof(in_addr));

	lpPerData->m_socket = AcceptSocket;
	lpPerData->m_IOCount = 0;
	lpPerData->m_State = 0;

	LeaveCriticalSection(&this->m_cs);

	return lpPerData;
}

int CWZIocpEngine::Recv(_PER_HANDLE_DATA* lpPerData)
{
	if (!lpPerData)
	{
		wz_printf("(index:NULL) RECV buffer NULL ");
		return 0;
	}

	if (lpPerData->m_State == 1)
	{
		wz_printf("(index:%d) Recv State HDS_CLOSE ", lpPerData->m_Index);
		return 0;
	}

	_PerIoData* lpPerIoData = &lpPerData->PerIoDataRecv;

	// Zero the control fields (first 0x14 = 20 bytes after OVERLAPPED)
	//memset(&lpPerData->PerIoDataRecv, 0, 0x14u);
	memset((unsigned char*)&lpPerData->PerIoDataRecv,0,sizeof(OVERLAPPED));

	//lpPerData->PerIoDataRecv.wsabuf.len = MAX_IO_BUFFER_SIZE - lpPerData->PerIoDataRecv.bOfs;
	//lpPerData->PerIoDataRecv.wsabuf.buf = &lpPerData->PerIoDataRecv.Buffer[lpPerData->PerIoDataRecv.bOfs];
	//lpPerData->PerIoDataRecv.OperationType = 1;
	lpPerIoData->wsabuf.len = MAX_IO_BUFFER_SIZE - lpPerIoData->bOfs;
	lpPerIoData->wsabuf.buf = &lpPerIoData->Buffer[lpPerIoData->bOfs];
	lpPerIoData->OperationType = IO_RECV;

	DWORD RecvBytes = 0;
	DWORD Flags = 0;

	int Result = WSARecv(lpPerData->m_socket, &lpPerData->PerIoDataRecv.wsabuf, 1u, &RecvBytes, &Flags, &lpPerData->PerIoDataRecv, 0);

	if (Result != SOCKET_ERROR || WSAGetLastError() == WSA_IO_PENDING)
	{
		return 1;
	}
	else
	{
		wz_printf("WSARecv() failed with error %d", WSAGetLastError());
		CloseHandleData(lpPerData);
		return 0;
	}
}

int CWZIocpEngine::send(_PER_HANDLE_DATA* lpPerData, BYTE* SendBuff, int nSize)
{
	DWORD NumberOfBytesSent = 0;
	int Result;
	int Error;

	if (lpPerData == NULL)
	{
		wz_printf("buffer NULL ");
		return 0;
	}

	if (lpPerData->m_State == 1)
	{
		wz_printf("(index:%d) Send State HDS_CLOSE ", lpPerData->m_Index);
		return 0;
	}

	if (lpPerData->m_socket == INVALID_SOCKET)
	{
		wz_printf("(index:%d) INVALID_SOCKET error", lpPerData->m_Index);
		return 0;
	}

	EnterCriticalSection(&m_cs);

	_PerIoData* lpPerIoData = &lpPerData->PerIoDataSend;

	//
	// Already sending data?
	// Queue new packet into existing send buffer
	//
	if (lpPerIoData->bOfs > 0)
	{
		//
		// Overflow protection
		//
		if ((lpPerIoData->bOfs + nSize) > sizeof(lpPerIoData->Buffer) - 1)
		{
			wz_printf("(index:%d) send() max buffer overflow ", lpPerData->m_Index);

			char szTemp[3072];
			memset(szTemp, 0, sizeof(szTemp));

			for (int i = 0; i < 4 && i < nSize; i++)
			{
				char szHex[32];
				wsprintf(szHex, "0x%02X ", SendBuff[i]);
				strcat(szTemp, szHex);
			}

			char szInfo[128];
			wsprintf(szInfo, "( bOfs : %d nSize : %d )", lpPerIoData->bOfs, nSize);

			strcat(szTemp, szInfo);

			wz_printf(szTemp);

			//
			// Dump current send buffer
			//
			char* szBigLog = new char[49152];

			if (szBigLog)
			{
				memset(szBigLog, 0, 49152);

				for (int j = 0; j < MAX_IO_BUFFER_SIZE; j++)
				{
					char szHex[16];
					wsprintf(szHex, "0x%02X ", (BYTE)lpPerIoData->Buffer[j]);
					strcat(szBigLog, szHex);
				}
				wz_Bigprintf(szBigLog);
				delete[] szBigLog;
			}
			CloseHandleData(lpPerData);
			LeaveCriticalSection(&m_cs);

			return 0;
		}

		//
		// Append data to queued send buffer
		//
		memcpy(&lpPerIoData->Buffer[lpPerIoData->bOfs], SendBuff, nSize);
		lpPerIoData->bOfs += nSize;
		LeaveCriticalSection(&m_cs);

		return 1;
	}

	//
	// No pending send
	// Start async WSASend immediately
	//
	//memcpy(lpPerIoData->Buffer, SendBuff, nSize);

	//lpPerIoData->wsabuf.buf = lpPerIoData->Buffer;
	//lpPerIoData->wsabuf.len = nSize;

	//lpPerIoData->OperationType = 0;
	//lpPerIoData->bOfs = nSize;

	//memset((OVERLAPPED*)lpPerIoData, 0, sizeof(OVERLAPPED));
	memset(static_cast<OVERLAPPED*>(lpPerIoData),0,sizeof(OVERLAPPED));

	memcpy(lpPerIoData->Buffer, SendBuff, nSize);

	lpPerIoData->wsabuf.buf = lpPerIoData->Buffer;
	lpPerIoData->wsabuf.len = nSize;

	lpPerIoData->OperationType = IO_SEND;
	lpPerIoData->bOfs = nSize;

	Result = WSASend(lpPerData->m_socket, &lpPerIoData->wsabuf, 1, &NumberOfBytesSent, 0, lpPerIoData, NULL);

	Error = WSAGetLastError();

	if (Result == SOCKET_ERROR && Error != WSA_IO_PENDING)
	{
		wz_printf("(index:%d) (SendBytes:%d) send error %d Result : %d", lpPerData->m_Index, NumberOfBytesSent, Error, Result);
		//
		// Packet dump
		//
		char szLog[3072];
		memset(szLog, 0, sizeof(szLog));
		int DumpSize = nSize;

		if (DumpSize > 512)
		{
			DumpSize = 512;
		}

		for (int k = 0; k < DumpSize; k++)
		{
			char szHex[16];
			wsprintf(szHex, "0x%02X ", lpPerIoData->Buffer[k]);
			strcat(szLog, szHex);
		}

		wz_printf(szLog);
		CloseHandleData(lpPerData);
		LeaveCriticalSection(&m_cs);

		return 0;
	}

	++lpPerData->m_IOCount;
	LeaveCriticalSection(&m_cs);

	return 1;
}

int CWZIocpEngine::SendMore(int TransBytes, _PER_HANDLE_DATA* lpPerData)
{
	DWORD SendBytes[3];
	_PerIoData* lpPerIoData;

	if (lpPerData->m_State == 1)
	{
		wz_printf(" (index:%d) Sendmore State HDS_CLOSE ", lpPerData->m_Index);
		return false;
	}
	else
	{
		lpPerIoData = &lpPerData->PerIoDataSend;
		if (lpPerData->PerIoDataSend.bOfs > 0)
		{
			lpPerIoData->wsabuf.buf = lpPerIoData->Buffer;
			lpPerIoData->wsabuf.len = lpPerIoData->bOfs;
			lpPerIoData->OperationType = 0;
			if (WSASend(lpPerData->m_socket, &lpPerIoData->wsabuf, 1u, SendBytes, 0, lpPerIoData, 0)
				!= SOCKET_ERROR || WSAGetLastError() == WSA_IO_PENDING)
			{
				++lpPerData->m_IOCount;
			}
			else
			{
				wz_printf("sendmore(%d) error %d", lpPerData->m_Index, WSAGetLastError());
			}
		}
		return true;
	}
}

int CWZIocpEngine::RecvDataParse(_PER_HANDLE_DATA* lpPerHandleData)
{
	BYTE RecvBuffer[8200];
	_PerIoData* p_PerIoDataRecv;
	BYTE headcode;
	BYTE xcode = 0;
	int count = 0;
	int lOfs = 0;
	LPBYTE recvbuf;

	if (lpPerHandleData->PerIoDataRecv.bOfs < 3)
	{
		return true;
	}

	memset(RecvBuffer, 0, MAX_IO_BUFFER_SIZE);
	p_PerIoDataRecv = &lpPerHandleData->PerIoDataRecv;
	recvbuf = (LPBYTE)lpPerHandleData->PerIoDataRecv.Buffer;
	while (true)
	{
		if (*recvbuf == 0xC1 || *recvbuf == 0xC3)
		{
			count = recvbuf[1];
			headcode = recvbuf[2];
			xcode = *recvbuf;
		}
		else
		{
			if (*recvbuf != 0xC2 && *recvbuf != 0xC4)
			{
				wz_printf("Header error %s %d", __FILE__, __LINE__);
				return 0;
			}
			count = recvbuf[1] << 8;
			count |= recvbuf[2];
			headcode = recvbuf[3];
			xcode = *recvbuf;
		}
		if (count <= 0)
		{
			wz_printf("error-L1 : size %d", count);
			return 0;
		}
		if (count > p_PerIoDataRecv->bOfs)
			break;

		memcpy(RecvBuffer, recvbuf, count);
		memmove(recvbuf, &recvbuf[count], p_PerIoDataRecv->bOfs - count);

		if (lpPerHandleData->m_Index >= 0)
		{
			//ProtocolCore(headcode, (SDHP_SERVERINFO*)RecvBuffer, count, lpPerHandleData->m_Index);
			ProtocolCore(headcode, RecvBuffer, count, lpPerHandleData->m_Index);
		}

		lOfs += count;
		p_PerIoDataRecv->bOfs -= count;

		if (p_PerIoDataRecv->bOfs <= 0)
		{
			return 1;
		}
	}
	if (lOfs > 0 && p_PerIoDataRecv->bOfs < 1)
	{
		wz_printf("error-L1 : recvbuflen 1 %s %d", __FILE__, __LINE__);
	}
	return true;
}

int CWZIocpEngine::CloseHandleData(_PER_HANDLE_DATA* lpPerData)
{
	EnterCriticalSection(&this->m_cs);

	wz_printf("[Close] lpPerData->m_Index %d WSAGetLastError() %d", lpPerData->m_Index, WSAGetLastError());
	if (!lpPerData->m_State)
	{
		lpPerData->m_State = 1;
	}
	if (lpPerData->m_Index >= 0)
	{
		if (lpPerData->m_IOCount > 0)
		{
			wz_printf("[CloseError] (index:%d) IOCount %d", lpPerData->m_Index, lpPerData->m_IOCount);
		}
		closesocket(lpPerData->m_socket);
		GS_Del(lpPerData);
		lpPerData->m_socket = -1;
		lpPerData->m_Index = -1;
		this->m_Blank = lpPerData->m_This;
		wz_printf("[Close] closesocket");
		LeaveCriticalSection(&this->m_cs);
		return 1;
	}
	else
	{
		wz_printf("[CloseError] index : %d", lpPerData->m_Index);
		LeaveCriticalSection(&this->m_cs);
		return 0;
	}
}

BOOL CWZIocpEngine::UpdateCompletionPort(void* AcceptSocket, _PER_HANDLE_DATA* lpPerData)
{
	return CreateIoCompletionPort(AcceptSocket, this->m_cp, (ULONG_PTR)lpPerData, 0) != NULL;
}

int CWZIocpEngine::NotUsedHandleData()
{
	for (int n = 0; n < MAX_OBJECTS; n++)
	{
		if (this->lpPerHandleData[n]->m_socket
			== INVALID_SOCKET)
		{
			return n;
		}
	}

	return -1;
}

DWORD WINAPI WZIocpEngine_AcceptThread(LPVOID lpThreadParameter)
{
	CWZIocpEngine* pEngine = (CWZIocpEngine*)lpThreadParameter;

	if (pEngine->WorkerThreadCreate() == FALSE)
	{
		MessageBoxA(NULL, "WorkerThreadCreate() failed", "Error", MB_OK);
		return 0;
	}

	int Result = pEngine->Listen();

	if (Result != 0)
	{
		return Result;
	}

	while (true)
	{
		_PER_HANDLE_DATA* lpPerData = NULL;
		do
		{
			lpPerData = pEngine->Accept();
		} while (lpPerData == NULL);
		pEngine->Recv(lpPerData);
	}
	return 0;
}

DWORD WINAPI WZIocpEngine_WorkerThreadServer(LPVOID lpParam)
{
	CWZIocpEngine* pEngine = (CWZIocpEngine*)lpParam;
	HANDLE CompletionPort = (HANDLE)pEngine->GetCP();
	DWORD BytesTransferred = 0;
	_PER_HANDLE_DATA* lpPerData = NULL;
	_PerIoData* lpPerIoData = NULL;

	while (true)
	{
		BOOL Result = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (PULONG_PTR)&lpPerData, (LPOVERLAPPED*)&lpPerIoData, INFINITE);
		//
		// Shutdown signal
		//
		if (lpPerData == NULL && lpPerIoData == NULL)
		{
			break;
		}

		//
		// GQCS failure
		//
		if (Result == FALSE)
		{
			DWORD dwError = WSAGetLastError();

			if (lpPerData)
			{
				pEngine->wz_printf("GetQueuedCompletionStatus failed with error %d (index:%d)", dwError, lpPerData->m_Index);

				if (dwError == WSAECONNRESET || dwError == WSAECONNABORTED)
				{
					pEngine->CloseHandleData(lpPerData);
				}
			}

			continue;
		}

		//
		// Invalid overlapped
		//
		if (lpPerIoData == NULL)
		{
			continue;
		}

		//
		// Zero bytes transferred usually means disconnect
		//
		if (BytesTransferred == 0 &&
			lpPerIoData->OperationType <= 1)
		{
			EnterCriticalSection(&pEngine->m_cs);

			pEngine->wz_printf("GetQueuedCompletionStatus Transferred error %d (index:%d) OperationType %d", WSAGetLastError(), lpPerData ? lpPerData->m_Index : -1, lpPerIoData->OperationType);

			if (lpPerData)
			{
				pEngine->CloseHandleData(lpPerData);
			}

			LeaveCriticalSection(&pEngine->m_cs);

			continue;
		}

		//
		// ==========================================
		// SEND COMPLETION
		// ==========================================
		//
		if (lpPerIoData->OperationType == 0)
		{
			EnterCriticalSection(&pEngine->m_cs);

			//
			// Remaining queued bytes
			//
			int cSize = lpPerIoData->bOfs - BytesTransferred;

			if (cSize < 0)
			{
				pEngine->wz_printf("error-L1 : cSize < 0");
				lpPerIoData->bOfs = 0;
			}
			else
			{
				//
				// Move remaining bytes
				//
				if (cSize > 0 && cSize < (int)sizeof(lpPerIoData->Buffer))
				{
					memmove(lpPerIoData->Buffer, &lpPerIoData->Buffer[BytesTransferred], cSize);
				}

				//
				// Update queued size
				//
				lpPerIoData->bOfs = cSize;
			}

			//
			// One async IO completed
			//
			//--lpPerData->m_IOCount;
			if (lpPerData->m_IOCount > 0)//fix?
			{
				--lpPerData->m_IOCount;
			}
			//
			// More pending queued bytes?
			//
			if (lpPerIoData->bOfs > 0)
			{
				pEngine->SendMore(BytesTransferred, lpPerData);
			}

			//
			// Delayed close support
			//
			if (lpPerData->m_State == 1)
			{
				pEngine->CloseHandleData(lpPerData);
			}
			LeaveCriticalSection(&pEngine->m_cs);

			continue;
		}

		//
		// ==========================================
		// RECV COMPLETION
		// ==========================================
		//
		if (lpPerIoData->OperationType == 1)
		{
			EnterCriticalSection(&pEngine->m_cs);

			//
			// Append received bytes
			//
			lpPerIoData->bOfs += BytesTransferred;

			//
			// Parse stream packets
			//
			pEngine->RecvDataParse(lpPerData);

			//
			// Continue receiving
			//
			if (lpPerData->m_Index >= 0 && lpPerData->m_socket != INVALID_SOCKET)
			{
				pEngine->Recv(lpPerData);
			}

			LeaveCriticalSection(&pEngine->m_cs);

			continue;
		}

		//
		// Unknown operation type
		//
		pEngine->wz_printf("Unknown IO OperationType %d", lpPerIoData->OperationType);
	}

	return 0;
}
void WZIOCPMsgBox(char* szlog, ...)
{
	char szBuffer[512];
	va_list arg;

	va_start(arg, szlog);
	memset(szBuffer, 0, sizeof(szBuffer));
	vsprintf(szBuffer, szlog, arg);
	MessageBoxA(0, szBuffer, "error", 0);
	va_end(arg);
}

int GS_Add(_PER_HANDLE_DATA* lpPerHandleData)
{
	return gsm.Add(lpPerHandleData);
}

int GS_Del(_PER_HANDLE_DATA* lpPerHandleData)
{
	return gsm.Del(lpPerHandleData->m_Index);
}