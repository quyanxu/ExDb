// FriendDBSet.h
#pragma once

class CFriendDBSet
{
public:
	CFriendDBSet();
	virtual ~CFriendDBSet();

public:
	int Connect();

	int CreateGuid(char* szName);
	int GetGuid(char* szName, int* memoCount);
	int GetFriendList(char* szName, _FRIEND_INFO_STRUCT* lpNode);
	int FriendAdd(char* szName, char* szFriendName);
	unsigned char FriendWaitAdd(char* szName, char* szFriendName);
	int FriendDel(char* szName, char* szFriendName);
	int WaitFriendDel(char* szName, char* szFriendName);
	int GetWaitFriend(int GUID, char* szFriendName);
	int ReadMemoSubject(int aIndex, char* UserName, unsigned short UserIndex, int guid);
	int WriteMemo(char* SendName, char* RecvName, char* subject, char* memo, unsigned char* lpPhoto, unsigned char Dir, unsigned char Action);
	int ReadMemo(int aIndex, int memoIndex, int guid, char* Memo, BYTE* Photo, BYTE* Dir, BYTE* Act, int* strsize);
	int DelMemo(char* szName, int memoIndex);

	CRITICAL_SECTION m_csDBQueryFriend;
	CRITICAL_SECTION m_csDBQueryMail;

	CQuery m_DBQueryFriend;
	CQuery m_DBQueryMail;
};

extern CFriendDBSet* FriendDbSet;