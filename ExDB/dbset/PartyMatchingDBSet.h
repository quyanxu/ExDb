// PartyMatchingDBSet.h

#pragma once
#include "stdafx.h"

class CPartyMatchingDBSet
{
public:
	CPartyMatchingDBSet();
	virtual ~CPartyMatchingDBSet();

public:
	int Connect();

	int GetRandomMatching(int nUserLevel, BYTE btClass, BYTE btGensType, char* szLeaderNameOutput);
	int GetPartyMatchingList(int nPage, _PARTY_INFO_LIST* lpOutPutList);
	int GetPartyMatchingListSearchWord(int nPage, char* szSearchWord, _PARTY_INFO_LIST* lpOutPutList);
	int GetPartyMatchingListJoinAble(int nPage, BYTE btClass, int nLevel, BYTE btGens, _PARTY_INFO_LIST* lpOutPutList);
	int GetPartyMatchingListJoinAbleTotalCount(BYTE btClass, int nLevel, BYTE btGens);
	int GetPartyMatchingListCount();
	int InsertPartyMatchingList(char* szLeaderName, char* szTitle, char* szPassWord, short nMinLevel, short nMaxLevel, short nHuntingGround, BYTE btWantedClass, BYTE btCurMemberCount, BYTE btUsePassWord, BYTE btAcceptType, BYTE* lpbtWantedClassDetailInfo, int nServerChannel, BYTE btGensType, short nLeaderLevel, BYTE btLeaderClass);
	int GetPartyMatchingPasswordInfo(char* szLeaderName, char* szPassWordOut, BYTE* btUsePassWordOut, BYTE* btAcceptType);
	int InsertWaitPartyMatching(char* szLeaderName, char* szMemberName, BYTE btClass, int nLevel, int nUserDBNumber);
	int GetPartyMatchingWaitList(char* szMemberName, char* szLeaderName);
	int GetPartyMatchingWaitListForLeader(char* szLeaderName, _PARTY_MEMBER_WAIT_LIST* stList);
	void DeleteAllPartyMatching();
	int UpdatePartyMatchingList(char* szLeaderName, char* szNewLeaderName, int nServerChannel, short nLeaderLevel, BYTE btLeaderClass);
	int DeletePartyMatching(char* szLeaderName);
	int DeleteWaitList(char* szMemberName);
	int UpdatePartyMemberCount(char* szLeaderName, int nMemberCount);
	int GetPartyMatchingWaitMemberInfo(char* szMemberName, int* nMemberLevel, unsigned __int8* btMemberClass);

public:
	CQuery          m_PartyMatchingQry;
	CRITICAL_SECTION m_csDBQueryPartyMatching;
};

extern CPartyMatchingDBSet g_PartyMatching_DBSet;