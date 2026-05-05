// FriendClass.cpp
#include "stdafx.h"
#include "FriendClass.h"

CFriendClass::CFriendClass()
{
	this->head = 0;
	this->tail = 0;

	this->m_count = 0;
	this->m_OnOffCount = 0;
}

CFriendClass::~CFriendClass()
{
	this->AllDelete();
}

void CFriendClass::Init()
{
	this->m_count = 0;
	this->m_OnOffCount = 0;
}

_FRIEND_INFO_STRUCT* CFriendClass::Add(int number,char* Name,unsigned char pServer,short serverindex)
{
	_FRIEND_INFO_STRUCT* pSearchNode = this->Search(Name);

	if (pSearchNode)
	{
		pSearchNode->Number = number;
		pSearchNode->pServer[0] = pServer;

		return pSearchNode;
	}

	HANDLE hHeap = GetProcessHeap();

	_FRIEND_INFO_STRUCT* pTempNode = (_FRIEND_INFO_STRUCT*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(_FRIEND_INFO_STRUCT));

	if (!pTempNode)
	{
		cLog.Add("error-L1 : memory allocation error");
		return 0;
	}

	strcpy(pTempNode->Name, Name);
	strcpy(pTempNode->Names[0], Name);

	pTempNode->Number = number;
	pTempNode->Count = 1;

	pTempNode->next = 0;
	pTempNode->back = 0;

	pTempNode->ChatVeto = 1;

	for (int n = 1; n < 50; n++)
	{
		pTempNode->Use[n] = -1;
		pTempNode->pServer[n] = (unsigned char)-1;
		pTempNode->RoomNumber[n] = -1;
		pTempNode->ServerIndex[n] = -1;
	}

	pTempNode->Use[0] = number;
	pTempNode->pServer[0] = pServer;
	pTempNode->ServerIndex[0] = serverindex;

	this->AddTail(pTempNode);

	cLog.Add("[%d] New Friend Master = %s", number, Name);

	++this->m_count;

	return pTempNode;
}

void CFriendClass::AddTail(_FRIEND_INFO_STRUCT* pNewNode)
{
	pNewNode->back = this->tail;
	pNewNode->next = 0;

	if (this->head)
	{
		this->tail->next = pNewNode;
	}
	else
	{
		this->head = pNewNode;
	}

	this->tail = pNewNode;
}

void CFriendClass::Print()
{
	for (_FRIEND_INFO_STRUCT* pNode = this->head;
		pNode;
		pNode = pNode->next)
	{
		cLog.Add("Friend print : %s  %s  ",
			pNode->Name,
			pNode->Names[0]);

		for (int n = 1; n < 50; n++)
		{
			if (pNode->Use[n] >= 0)
			{
				cLog.Add("(%s) ", pNode->Names[n]);
			}
		}
	}
}

void CFriendClass::AllDelete()
{
	_FRIEND_INFO_STRUCT* pNode = this->head;

	while (pNode)
	{
		_FRIEND_INFO_STRUCT* pTemp = pNode;

		pNode = pNode->next;

		HeapFree(GetProcessHeap(), 0, pTemp);
	}

	this->head = 0;
	this->tail = 0;
}

int CFriendClass::Delete(char* friendname, char* commader)
{
	_FRIEND_INFO_STRUCT* pNode = this->Search(friendname);

	if (!pNode)
	{
		return 0;
	}

	_FRIEND_INFO_STRUCT* pprev = pNode->back;
	_FRIEND_INFO_STRUCT* pnext = pNode->next;

	if (pprev)
	{
		pprev->next = pnext;
	}
	else
	{
		this->head = pnext;
	}

	if (pnext)
	{
		pnext->back = pprev;
	}
	else
	{
		this->tail = pprev;
	}

	HeapFree(GetProcessHeap(), 0, pNode);

	--this->m_count;

	return 1;
}

_FRIEND_INFO_STRUCT* CFriendClass::Search(char* Name)
{
	for (_FRIEND_INFO_STRUCT* pNode = this->head;
		pNode;
		pNode = pNode->next)
	{
		if (pNode->Name[0] == Name[0] &&
			pNode->Name[1] == Name[1])
		{
			if (strcmp(pNode->Name, Name) == 0)
			{
				return pNode;
			}
		}
	}

	return 0;
}

void CFriendClass::ServerDownDelMember(int server)
{
	_FRIEND_INFO_STRUCT* pNode = this->head;

	if (!pNode)
	{
		cLog.AddTD("CFriendClass::ServerDownDelMember pNode is NULL Server:%d ", server);

		return;
	}

	while (pNode)
	{
		char Name[20] = { 0 };

		memcpy(Name, pNode->Name, sizeof(pNode->Name));

		int sindex = pNode->sIndex;

		pNode = pNode->next;

		if (sindex == server)
		{
			DGFriendOfflineSend(Name);
		}
	}
}

int CFriendClass::SearchMemberRoom(_FRIEND_INFO_STRUCT* lpNode, char* FriendName, int* UserIndex, short* pServer, int* room)
{
	if (!lpNode)
	{
		return -1;
	}

	for (int n = 0; n < 50; n++)
	{
		if (lpNode->Use[n] >= 0 &&
			lpNode->Names[n][0] == FriendName[0])
		{
			if (strcmp(lpNode->Names[n], FriendName) == 0)
			{
				*UserIndex = n;
				*pServer = lpNode->pServer[n];
				*room = lpNode->RoomNumber[n];

				return n;
			}
		}
	}

	return -1;
}

int CFriendClass::IsSearchMember(char* Name, char* FriendName)
{
	_FRIEND_INFO_STRUCT* lpNode = this->Search(Name);

	return this->IsSearchMember(lpNode, FriendName);
}

int CFriendClass::IsSearchMember(_FRIEND_INFO_STRUCT* lpNode, char* FriendName)
{
	if (!lpNode)
	{
		return 0;
	}

	for (int n = 1; n < 50; n++)
	{
		if (lpNode->Use[n] >= 0 &&
			lpNode->Names[n][0] == FriendName[0])
		{
			if (strcmp(lpNode->Names[n], FriendName) == 0)
			{
				return 1;
			}
		}
	}

	return 0;
}

_FRIEND_INFO_STRUCT* CFriendClass::Search_Number(int number)
{
	for (_FRIEND_INFO_STRUCT* pNode = this->head;
		pNode;
		pNode = pNode->next)
	{
		if (pNode->Number == number)
		{
			return pNode;
		}
	}

	return 0;
}

_FRIEND_INFO_STRUCT* CFriendClass::Search_NumberAndId(
	int number,
	char* name)
{
	_FRIEND_INFO_STRUCT* pNode = this->Search_Number(number);

	if (!pNode)
	{
		return 0;
	}

	for (int n = 0; n < 50; n++)
	{
		if (strcmp(pNode->Names[n], name) == 0)
		{
			return pNode;
		}
	}

	return 0;
}

_FRIEND_INFO_STRUCT* CFriendClass::AddMember(_FRIEND_INFO_STRUCT* lpNode, char* FrindName, unsigned char pServer, short number, short serverindex)
{
	if (!lpNode)
	{
		cLog.AddTD("Msg-L3 : Friend Add member null pointer %s", FrindName);
		return 0;
	}

	if (lpNode->Count > 50)
	{
		cLog.AddTD("Msg-L3 : Friend Add member max %d", lpNode->Count);
		return 0;
	}

	int blank = -1;

	for (int n = 0; n < 50; n++)
	{
		if (lpNode->Use[n] < 0)
		{
			if (blank < 0)
			{
				blank = n;
			}
		}
		else
		{
			if (strcmp(lpNode->Names[n], FrindName) == 0)
			{
				cLog.AddTD("Msg-L3 : Friend member %s : %s", lpNode->Names[n], FrindName);
				return 0;
			}
		}
	}

	if (blank < 0)
	{
		cLog.AddTD("Msg-L3 : Friend not blank %s : %s", lpNode->Name, FrindName);
		return 0;
	}

	strcpy(lpNode->Names[blank], FrindName);

	lpNode->Use[blank] = number;
	lpNode->pServer[blank] = pServer;
	lpNode->ServerIndex[blank] = serverindex;

	++lpNode->Count;

	cLog.Add("[%s] Frined Member Add (%d) %s", lpNode->Name, blank, FrindName);

	return lpNode;
}

_FRIEND_INFO_STRUCT* CFriendClass::AddMember(char* Name, char* FrindName, unsigned char pServer, short number, short serverindex)
{
	_FRIEND_INFO_STRUCT* pNode = this->Search(Name);

	return this->AddMember(pNode, FrindName, pServer, number, serverindex);
}

_FRIEND_INFO_STRUCT* CFriendClass::DelMember(char* Name, char* FriendName)
{
	_FRIEND_INFO_STRUCT* pNode = this->Search(Name);

	if (!pNode)
	{
		return 0;
	}

	for (int n = 0; n < 50; n++)
	{
		if (pNode->Use[n] >= 0)
		{
			if (strcmp(pNode->Names[n], FriendName) == 0)
			{
				pNode->Use[n] = -1;
				--pNode->Count;
				return pNode;
			}
		}
	}

	return 0;
}

int CFriendClass::ConnectMember(char* Name, char* FriendName, unsigned char pServer)
{
	_FRIEND_INFO_STRUCT* pNode = this->Search(Name);

	if (!pNode)
	{
		return 0;
	}

	for (int n = 0; n < 50; n++)
	{
		if (pNode->Use[n] >= 0)
		{
			if (strcmp(pNode->Names[n], FriendName) == 0)
			{
				if (pNode->Use[n] == 30000)
				{
					pNode->pServer[n] = (unsigned char)-1;
				}
				else
				{
					pNode->pServer[n] = pServer;
				}

				return 1;
			}
		}
	}

	return 0;
}

int CFriendClass::SetConnectMember(_FRIEND_INFO_STRUCT* lpNode, char* FriendName, short number, unsigned char pServer, short sIndex)
{
	if (!lpNode)
	{
		return 0;
	}

	for (int n = 0; n < 50; n++)
	{
		if (lpNode->Use[n] >= 0 && lpNode->Names[n][0] == FriendName[0])
		{
			if (strcmp(lpNode->Names[n], FriendName) == 0)
			{
				if (lpNode->Use[n] == 30000)
				{
					lpNode->pServer[n] = (unsigned char)-1;
				}
				else
				{
					lpNode->Use[n] = number;
					lpNode->pServer[n] = pServer;
				}

				lpNode->ServerIndex[n] = sIndex;
				return 1;
			}
		}
	}

	return 0;
}

_FRIEND_INFO_STRUCT* CFriendClass::GetHead()
{
	return this->head;
}

int CFriendClass::DisconnectMember(char* Name, char* FriendName)
{
	_FRIEND_INFO_STRUCT* pNode = this->Search(Name);

	if (!pNode)
	{
		return 0;
	}

	for (int n = 0; n < 50; n++)
	{
		if (pNode->Use[n] >= 0)
		{
			if (strcmp(pNode->Names[n], FriendName) == 0)
			{
				pNode->pServer[n] = (unsigned char)-1;
				return 1;
			}
		}
	}

	return 0;
}

void CFriendClass::SetState(_FRIEND_INFO_STRUCT* lpNode, unsigned char State)
{
	lpNode->State = State;
}

int CFriendClass::GetState(_FRIEND_INFO_STRUCT* lpNode)
{
	return lpNode->State;
}

void CFriendClass::SetChatVeto(_FRIEND_INFO_STRUCT* lpNode, int flag)
{
	lpNode->ChatVeto = flag;
}

int CFriendClass::GetChatveto(_FRIEND_INFO_STRUCT* lpNode)
{
	return lpNode->ChatVeto;
}

int CFriendClass::SetMemberRoom(char* Name, char* FriendName, short room)
{
	_FRIEND_INFO_STRUCT* pNode = this->Search(Name);

	if (!pNode)
	{
		return 0;
	}

	for (int n = 0; n < 50; n++)
	{
		if (pNode->Use[n] >= 0)
		{
			if (strcmp(pNode->Names[n], FriendName) == 0)
			{
				pNode->RoomNumber[n] = room;
				return 1;
			}
		}
	}

	return 0;
}