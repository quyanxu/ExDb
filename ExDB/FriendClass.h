// FriendClass.h
#pragma once
#include "..\\Include\\define.h"

struct _FRIEND_INFO_STRUCT//948
{
	int GUID;
	short Number;
	short sIndex;
	char Name[MAX_IDSTRING + 1];
	char Names[50][MAX_IDSTRING + 1];
	short Use[50];
	BYTE Count;
	short RoomNumber[50];
	short ServerIndex[50];
	BYTE pServer[50];
	int ChatVeto;
	BYTE State;
	int MailCount;
	int WaitFriendCommand;
	_FRIEND_INFO_STRUCT* back;
	_FRIEND_INFO_STRUCT* next;
};

class CFriendClass
{
public:
	CFriendClass();
	virtual ~CFriendClass();

	void Init();
	_FRIEND_INFO_STRUCT* Add(int number, char* Name, unsigned char pServer, short serverindex);
	void AddTail(_FRIEND_INFO_STRUCT* pNewNode);
	void Print();
	void AllDelete();
	int Delete(char* friendname, char* commader);
	_FRIEND_INFO_STRUCT* Search(char* Name);
	void ServerDownDelMember(int server);
	int SearchMemberRoom(_FRIEND_INFO_STRUCT* lpNode, char* FriendName, int* UserIndex, short* pServer, int* room);
	int IsSearchMember(char* Name, char* FriendName);
	int IsSearchMember(_FRIEND_INFO_STRUCT* lpNode, char* FriendName);
	_FRIEND_INFO_STRUCT* Search_Number(int number);
	_FRIEND_INFO_STRUCT* Search_NumberAndId(int number, char* name);
	_FRIEND_INFO_STRUCT* AddMember(_FRIEND_INFO_STRUCT* lpNode, char* FrindName, unsigned char pServer, short number, short serverindex);
	_FRIEND_INFO_STRUCT* AddMember(char* Name, char* FrindName, unsigned char pServer, short number, short serverindex);
	_FRIEND_INFO_STRUCT* DelMember(char* Name, char* FriendName);
	int ConnectMember(char* Name, char* FriendName, unsigned char pServer);
	int SetConnectMember(_FRIEND_INFO_STRUCT* lpNode, char* FriendName, short number, unsigned char pServer, short sIndex);
	_FRIEND_INFO_STRUCT* GetHead();
	int DisconnectMember(char* Name, char* FriendName);
	void SetState(_FRIEND_INFO_STRUCT* lpNode, unsigned char State);
	int GetState(_FRIEND_INFO_STRUCT* lpNode);
	void SetChatVeto(_FRIEND_INFO_STRUCT* lpNode, int flag);
	int GetChatveto(_FRIEND_INFO_STRUCT* lpNode);
	int SetMemberRoom(char* Name, char* FriendName, short room);

private:
	_FRIEND_INFO_STRUCT* head;
	_FRIEND_INFO_STRUCT* tail;

	int m_count;
	int m_OnOffCount;
};

extern CFriendClass CFriendManager;