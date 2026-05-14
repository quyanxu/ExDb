// WZIocpEngine.h
// Visual Studio 2008 compatible

#ifndef WZIOCPENGINE_H
#define WZIOCPENGINE_H


#include <winsock2.h>
#include <windows.h>

#include "WZWinsockBase.h"

#pragma comment(lib, "ws2_32.lib")

#define MAX_IO_BUFFER_SIZE 8192
#define MAX_OBJECTS        1000

#define IO_SEND 0
#define IO_RECV 1

struct _PerIoData : public OVERLAPPED
{
	WSABUF wsabuf;
	char Buffer[MAX_IO_BUFFER_SIZE];
	int BufferLen;
	int bOfs;
	int OperationType;
};

struct _PER_HANDLE_DATA
{
	SOCKET m_socket;
	in_addr ClientAddr;

	_PerIoData PerIoDataRecv;
	_PerIoData PerIoDataSend;

	int m_Index;
	int m_IOCount;
	BYTE m_State;
	int m_This;
};

class CWZIocpEngine : public CWZWinsockBase
{
public:
	CWZIocpEngine();
	virtual ~CWZIocpEngine();

public:
	BOOL Create(WORD server_port, int max_listen = 5);
	void Destroy();

	int Listen();

	int WorkerThreadCreate();
	int WorkerThreadDestroy();

	_PER_HANDLE_DATA* Accept();
	int Recv(_PER_HANDLE_DATA* lpPerData);
	int send(_PER_HANDLE_DATA* lpPerData, BYTE* SendBuff, int nSize);
	int SendMore(int TransBytes, _PER_HANDLE_DATA* lpPerData);
	int RecvDataParse(_PER_HANDLE_DATA* lpPerData);
	int CloseHandleData(_PER_HANDLE_DATA* lpPerData);
	BOOL UpdateCompletionPort(HANDLE AcceptSocket, _PER_HANDLE_DATA* lpPerData);

	int NotUsedHandleData();

	HANDLE GetCP();
	int SetCP(HANDLE cp);

	SOCKET GetHDSocket(_PER_HANDLE_DATA* lpPerData);

	void SetListenCount(int max_listen);

	int CheckFunc();

public:
	void AcceptFuncRegister(int (*lpFunc)(_PER_HANDLE_DATA*));
	void CloseFuncRegister(int (*lpFunc)(_PER_HANDLE_DATA*));
	void ProtocolCoreFuncRegister(int (*lpFunc)(BYTE, BYTE*, int, int));

public:
	void wz_printf(char* szlog, ...);
	void wz_Bigprintf(char* szlog, ...);

public:
	WORD m_ServerPort;
	DWORD m_ThreadID;
	HANDLE m_hThreadHandle;
	int m_ThreadCount;
	int m_MaxListen;
	HANDLE m_cp;
	int m_Blank;
	CRITICAL_SECTION m_cs;
	_PER_HANDLE_DATA* lpPerHandleData[MAX_OBJECTS];
public:
	int (*WZIocpEngine_AcceptFunc)(_PER_HANDLE_DATA*);
	int (*WZIocpEngine_CloseFunc)(_PER_HANDLE_DATA*);
	int (*WZIocpEngine_ProtocolCoreFunc)(BYTE, BYTE*, int, int);
};

extern CWZIocpEngine* WzIoEngine;

DWORD WINAPI WZIocpEngine_AcceptThread(LPVOID lpParam);
DWORD WINAPI WZIocpEngine_WorkerThreadServer(LPVOID lpParam);

void WZIOCPMsgBox(char* szlog, ...);
int GS_Add(_PER_HANDLE_DATA* lpPerHandleData);
int GS_Del(_PER_HANDLE_DATA* lpPerHandleData);

#endif