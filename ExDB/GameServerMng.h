#pragma once

#include <winsock2.h>
#include <windows.h>

#include "WZIocpEngine.h"

#define MAX_GAMESERVER 100

#define SERVER_TYPE_NONE   0
#define SERVER_TYPE_GAME   1
#define SERVER_TYPE_CHAT   2

struct SockObj
{
    SockObj()
    {
        this->m_Used = 0;
        this->m_Type = 0;
        this->m_count = 0;
        this->lpPHD = 0;
        this->btGameServerType = 0;
    }
    int m_Used;
    int m_count;
    _PER_HANDLE_DATA* lpPHD;
    unsigned __int8 pServer;
    int m_Type;
    char m_ServerName[50];
    int btGameServerType;
};

class CGameServerMng
{
public:

    SockObj m_Obj[MAX_GAMESERVER];
    int m_CSIndex;

public:

    CGameServerMng();
    virtual ~CGameServerMng();

public:

    int NotUsedSearch();
    int Add(_PER_HANDLE_DATA* lpPerHandleData);
    int Del(int number);
    int Set(int aIndex, WORD port, int type, char* ServerName, short ServerCode, BYTE btGameServerType);
};