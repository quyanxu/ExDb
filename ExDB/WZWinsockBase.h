#pragma once
#pragma comment(lib,"ws2_32.lib")

class CWZWinsockBase
{
public:

    SOCKET m_socket;
    int m_iMaxSockets;
    int m_Connect;
    int m_recvbufsize;
    int m_sendbufsize;
    HWND m_hWnd;

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