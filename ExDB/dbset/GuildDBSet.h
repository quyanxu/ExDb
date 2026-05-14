#pragma once
#include "Protocol.h"

struct ST_GUILD_COMMON_DBINFO
{
	char m_szGuildName[MAX_GUILDNAMESTRING+1];
	int m_iGuildLevel;
	int m_iGuildMemberStatus;
};


class CGuildDBSet
{
public:
	CGuildDBSet();
	virtual ~CGuildDBSet();

public:
	int m_TotalCount;
	int Connect();
	int CheckDBID();
	int DeleteGuild(char* Name);
	int CreateGuild(char* GuildName, char* Master, unsigned char* Mark);
	int GetNumber(char* GuildName);
	int GetGuildCount();
	int ReadAllUnion();
	int ReadAllGuild();
	int ScoreUpdate(char* guildName, int score);
	int NoticeUpdate(char* guildName, char* guild_notice);
	int UpdateGuildMemberStatus(char* szGuildName, char* szMemberName, int iGuildStatus);
	int UpdateGuildType(char* szGuildName, int iGuildType);
	int UpdateGuildUnion(int iGuildNumber, int iUnion);
	int UpdateGuildRival(int iGuildNumber, int iRival);
	int GetDBGuildMemberInfo(char* szGuildMemberName, ST_GUILD_COMMON_DBINFO* stGuildCommonDBInfo);
	int DelDBGuild(char* szGuildName);
	int CheckDBGuildMember(char* szGuildMemberName);

	int CheckDBGuildMaster(char* szGuildMasterName);
	int AddDBGuildMember(char* szGuildMemberName, char* szGuildName);
	int DelDBGuildMember(char* szGuildMemberName);
	int DelAllDBGuildMember(char* szGuildName);
	int InsertGuildMemberBuff(char* szGuildName, unsigned short wBuffIndex, unsigned char btEffectType1, unsigned char btEffectType2, unsigned int dwDuration, int lExpireDate);
	int DeleteGuildMemberBuff(unsigned short* wBuffIndex, unsigned char btGuildCnt);

	int GetDBGuildMatchingList(int nPage, _stGuildMatchingList* lpstMatchingList);
	int GetDBGuildMatchingListSearchWord(int nPage, char* szSearchWord, _stGuildMatchingList* lpstMatchingList);
	int GetDBGuildMatchingListCount();
	int RegGuildMatchingData(_stGuildMatchingList stMatchingList);
	int GuildMatchingRegCheck(int nGuildNum);
	int DeleteGuildMatching(int nGuildNum);
	int InsertWaitGuildMatching(_stGuildMatchingAllowListDB stAllowList);
	int GetGuildMatchingWaitState(char* szName, int nState);
	int DeleteWaitGuildMatching(char* szName);
	int GetWaitGuildMatching(int nGuildNumber, int nState, _stGuildMatchingAllowList* lpAllowList);
	int SetWaitGuildMatching(char* szName, int nState);
	int GetWaitStateListGuildMatching(char* szApplicantName, char* szGuildName, char* szGuildMasterName);
	int GetGuildMatchingAcceptNRejectInfo(char* szName);
	int GetIsApplicantListGuildMatching(int nGuildNumber);
	int SetGuildMatchingMemberCount(char* szGuildName, int nMemberCount);
	int CleanGuildMatchingData();

	CRITICAL_SECTION m_csUnionQry;
	CRITICAL_SECTION m_csGuildMemberQry;
	CRITICAL_SECTION m_csGuildMarkQry;
	CRITICAL_SECTION m_csDBQuery;

	CQuery m_UnionQry;
	CQuery m_GuildMemberQry;
	CQuery m_GuildMarkQry;
	CQuery m_DBQuery;
};

extern CGuildDBSet* GuildDbSet;