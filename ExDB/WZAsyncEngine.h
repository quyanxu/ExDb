// WZAsyncEngine.h
#pragma once
#include "WZWinsockBase.h"


// ----------------------------------------------------------------------
// Receive Stack
// ----------------------------------------------------------------------

struct STRUCT_RECVSTACK
{
	int used;
	int len;
	unsigned char buf[4096];
};

// ----------------------------------------------------------------------
// CWZAsyncEngine
// ----------------------------------------------------------------------

class CWZAsyncEngine : public CWZWinsockBase
{
public:

	CWZAsyncEngine();
	virtual ~CWZAsyncEngine();

public:

	unsigned char m_RecvBuf[4096];
	int m_nSendBufLen;
	unsigned char m_SendBuf[4096];
	int m_nRecvBufLen;
	int   m_LogPrint;
	FILE* m_logfp;
	STRUCT_RECVSTACK sRecvStack[100];
public:
	int(*WZAsyncEngine_ProtocolCoreFunc)(BYTE, BYTE*, int);
	int(*WZAsyncEngine_CloseFunc)();
	int(*WZAsyncEngine_AcceptFunc)(void*);

public:

	int Connect(HWND hWnd, char* ip_addr, u_short port, u_int WinMsgNum);
	int CreateServer(char* ip_addr, u_short port, u_int WinServerMsg);
	int send(SOCKET socket, char* buf, int len);
	int send(char* buf, int len);
	int FDWriteSend();
	int recv();
	int Push(unsigned char* buf, int len);
	unsigned char* Pop();
	int WinMsgProc(SOCKET wParam, short lParam);
	void ProtocolCoreFuncRegister(int (*lpFunc)(BYTE, BYTE*, int));
	void CloseFuncRegister(int(*lpFunc)());
	void AcceptFuncRegister(int(*lpFunc)(void*));

public:

	void LogPrintOn();
	void LogPrintOff();
	void LogHexPrint(unsigned char* buf, int size);
	void LogPrint(char* szlog, ...);
};

extern CWZAsyncEngine WzAsycSock; // idb