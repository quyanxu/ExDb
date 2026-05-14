#pragma once
#pragma comment(lib,"ws2_32.lib")

class CWZWinsockBase
{
public:
    int m_iMaxSockets;
    HWND m_hWnd;
    SOCKET m_socket;
    int m_sendbufsize;
    int m_recvbufsize;
    int m_Connect;
public:

    CWZWinsockBase();
    virtual ~CWZWinsockBase();

public:

    int Startup();
    int CreateSocket(HWND hWnd);
    int CloseSocket();
    int CloseSocket(SOCKET socket);
    SOCKET GetSocket();
    int GetRecvBuffSize();
    int GetSendBuffSize();
    int GetConnect();
    void SetConnect(int connected);
};