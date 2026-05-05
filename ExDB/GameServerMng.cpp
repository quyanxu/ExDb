// CGameServerMng.cpp
#include "stdafx.h"
#include "GameServerMng.h"
#include "FriendClass.h"

CGameServerMng::CGameServerMng()
{
	this->m_CSIndex = -1;
}

CGameServerMng::~CGameServerMng()
{
}

int CGameServerMng::NotUsedSearch()
{
	for (int n = 0; n < MAX_GAMESERVER; n++)
	{
		if (this->m_Obj[n].m_Used == 0)
		{
			return n;
		}
	}
	return -1;
}

int CGameServerMng::Add(_PER_HANDLE_DATA* lpPerHandleData)
{
	int number = this->NotUsedSearch();

	if (number < 0)
	{
		return -1;
	}

	this->m_Obj[number].m_Used = 1;
	this->m_Obj[number].m_count = 0;
	this->m_Obj[number].m_Type = 0;
	this->m_Obj[number].lpPHD = lpPerHandleData;

	cLog.Add("Server Connect : index: (%d)", number);
	CGuildManager.ClearSendGuildInfo();

	return number;
}

int CGameServerMng::Del(int number)
{
	this->m_Obj[number].m_Used = 0;
	int Type = this->m_Obj[number].m_Type;

	if (Type == SERVER_TYPE_GAME)
	{
		cLog.Add("Game Server Disconnect : (%s) index: (%d)", this->m_Obj[number].m_ServerName, number);

		CFriendManager.ServerDownDelMember(number);
		GServerDownDelParty(number);
	}
	else if (Type == SERVER_TYPE_CHAT)
	{
		this->m_CSIndex = -1;

		cLog.Add("Chating Server Disconnect : index: (%d)", number);
	}
	else
	{
		cLog.Add("Server Disconnect : index: (%d)", number);
	}

	return TRUE;
}

int CGameServerMng::Set(int aIndex, WORD port, int type, char* ServerName, short ServerCode, BYTE btGameServerType)
{
	if (this->m_Obj[aIndex].m_Used < 1)
	{
		cLog.AddTD("Error : not connectd %d", aIndex);
		return FALSE;
	}

	if (this->m_Obj[aIndex].m_Used == 2)
	{
		cLog.AddTD("Error : server info reset error (%d)", this->m_Obj[aIndex].m_Used);
		return FALSE;
	}

	this->m_Obj[aIndex].m_Used = 2;
	this->m_Obj[aIndex].m_Type = type;
	this->m_Obj[aIndex].pServer = ServerCode % 20;
	this->m_Obj[aIndex].btGameServerType = btGameServerType;

	strcpy(this->m_Obj[aIndex].m_ServerName, ServerName);

	if (type == SERVER_TYPE_CHAT)
	{
		this->m_CSIndex = aIndex;
		char* ip = inet_ntoa(this->m_Obj[aIndex].lpPHD->ClientAddr);
		cLog.AddTD("ChatServer Connected : (%s) %d", ip, aIndex);
	}
	else if (type == SERVER_TYPE_GAME)
	{
		cLog.AddTD("GameServer Connected (%s) : %d", ServerName, aIndex);
	}

	return TRUE;
}