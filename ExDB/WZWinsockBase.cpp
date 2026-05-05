#include "stdafx.h"
#include "WZWinsockBase.h"

CWZWinsockBase::CWZWinsockBase()
{
    this->m_socket = INVALID_SOCKET;
    this->m_iMaxSockets = 0;
    this->m_Connect = FALSE;
    this->m_recvbufsize = 0;
    this->m_sendbufsize = 0;
    this->m_hWnd = NULL;

    this->Startup();
}

CWZWinsockBase::~CWZWinsockBase()
{
    WSACleanup();
}

int CWZWinsockBase::Startup()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }

    if (LOBYTE(wsaData.wVersion) == 2 &&
        HIBYTE(wsaData.wVersion) == 2)
    {
        this->m_socket = INVALID_SOCKET;
        this->m_iMaxSockets = wsaData.iMaxSockets;
        this->m_Connect = FALSE;
        return 0;
    }

    WSACleanup();
    return 2;
}

int CWZWinsockBase::CreateSocket(HWND hWnd)
{
    this->m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (this->m_socket == INVALID_SOCKET)
    {
        return FALSE;
    }

    this->m_hWnd = hWnd;
    return TRUE;
}

int CWZWinsockBase::CloseSocket()
{
    if (this->m_socket != INVALID_SOCKET)
    {
        closesocket(this->m_socket);
        this->m_socket = INVALID_SOCKET;
    }

    this->m_Connect = FALSE;
    return TRUE;
}

int CWZWinsockBase::CloseSocket(SOCKET socket)
{
    if (socket != INVALID_SOCKET)
    {
        closesocket(socket);
    }

    this->m_socket = INVALID_SOCKET;
    this->m_Connect = FALSE;
    return TRUE;
}

SOCKET CWZWinsockBase::GetSocket()
{
    return this->m_socket;
}

int CWZWinsockBase::GetRecvBuffSize()
{
    return this->m_recvbufsize;
}

int CWZWinsockBase::GetSendBuffSize()
{
    return this->m_sendbufsize;
}

int CWZWinsockBase::GetConnect()
{
    return this->m_Connect;
}

void CWZWinsockBase::SetConnect(int connected)
{
    this->m_Connect = connected;
}