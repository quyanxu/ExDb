#pragma once
#include "stdafx.h"
#include "..\\include\\ProDef.h"
#include "FriendClass.h"
#include "GuildClass.h"
#include "PartyMatching.h"


// ----------------------------------------------------------------------
// Server Join Info
// ----------------------------------------------------------------------

struct SDHP_SERVERINFO
{
	PBMSG_HEAD h;
	BYTE Type;
	WORD Port;
	char ServerName[50];
	WORD ServerCode;
	BYTE btGameServerType;
};

// ----------------------------------------------------------------------
// Generic Result Packet
// ----------------------------------------------------------------------

struct SDHP_RESULT
{
	PBMSG_HEAD h;
	BYTE Result;
	DWORD ItemCount;
};

// ----------------------------------------------------------------------
// Protocol Dispatcher
// ----------------------------------------------------------------------


struct SDHP_USERCLOSE
{
	PBMSG_HEAD h;
	char CharName[MAX_IDSTRING];
	char GuildName[MAX_GUILDNAMESTRING];
	BYTE Type;
};

/* 1174 */
struct _PARTY_INFO_LIST
{
	char szLeaderName[MAX_IDSTRING + 1];
	char szTitle[MAX_PMATCHING_TITLESTR + 1];
	short nMinLevel;
	short nMaxLevel;
	short nHuntingGround;
	int nLeaderLevel;
	BYTE btLeaderClass;
	BYTE btWantedClass;
	BYTE btCurPartyMemberCnt;
	BYTE btUsePassword;
	BYTE btWantedClassDetailInfo[7];
	int nServerChennel;
	BYTE btGensType;
};

/* 1175 */
struct _PARTY_MEMBER_WAIT_LIST//16
{
	char szName[MAX_IDSTRING + 1];
	BYTE btClass;
	int  nLevel;
};

/* 1177 */
struct _SEND_PARTYMEMBER_INFO//28
{
	_SEND_PARTYMEMBER_INFO()
	{
		this->bUse = 0;
		this->bFlag = 0;
		this->nUserIndex = -1;
		this->nServerChannel = -1;
	}
	int  bUse;
	int  bFlag;
	char Name[MAX_IDSTRING + 1];
	int  nUserIndex;
	int  nServerChannel;
};

/* 1737 */
struct FHP_FRIEND_MEMO_LIST_REQ
{
	PBMSG_HEAD h; // C1:71 GS->ExDB
	WORD Number;
	char Name[MAX_IDSTRING];
};

/* 1738 */
struct FHP_FRIEND_MEMO_RECV_REQ
{
	PBMSG_HEAD h; // C1:72 GS->ExDB
	short Number;
	WORD MemoIndex;
	char Name[MAX_IDSTRING];
};

/* 1739 */
struct FHP_FRIEND_MEMO_DEL_REQ
{
	PBMSG_HEAD h; // C1:73 GS->ExDB
	short Number;
	WORD MemoIndex;
	char Name[MAX_IDSTRING];
};

/* 1740 */
struct _stGuildMatchingList//84
{
	char szMemo[MAX_GMATCHING_MEMOSTR + 1];                 // 0x00: 41 bytes - Guild memo/description
	char szRegistrant[MAX_IDSTRING + 1];          // 0x29: 11 bytes - Registrant name (GuildMasterName)
	char szGuildName[MAX_GUILDNAMESTRING + 1];            // 0x34: 9 bytes - Guild name (8 chars + null)
	BYTE btGuildMemberCnt;    // 0x3D: 1 byte - Guild member count
	BYTE btGuildMasterClass;  // 0x3E: 1 byte - Guild master class
	BYTE btInterestType;      // 0x3F: 1 byte - Interest type
	BYTE btLevelRange;        // 0x40: 1 byte - Level range
	BYTE btClassType;         // 0x41: 1 byte - Class type
	int nGuildMasterLevel;          // 0x44: 4 bytes (aligned) - Guild master level
	DWORD nBoardNumber;      // 0x48: 4 bytes - Board number (identNo)
	int nGuildNumber;               // 0x4C: 4 bytes - Guild number
	BYTE btGensType;     // 0x50: 1 byte - Gens type
	// Total: 0x51 = 81 bytes per entry
};

/* 1741 */
struct _stGuildMatchingAllowListDB
{
	int nGuildNumber;
	char szApplicantName[MAX_IDSTRING + 1];
	BYTE btApplicantClass;
	int nApplicantLevel;
	BYTE btState;
};

/* 1742 */
struct _stGuildMatchingAllowList
{
	char szName[MAX_IDSTRING + 1];
	BYTE btClass;
	int nLevel;
};

/* 1890 */
struct _tagGensMemberInfo//24
{
	_tagGensMemberInfo()
	{
		memset(szCharName, 0, sizeof(szCharName));
		btInfluence = 0;
		iContributePoint = 0;
		iGensRanking = 0;
		iGensClass = 0;
	}

	char szCharName[MAX_IDSTRING + 1];             // 0x00: 11 bytes
	BYTE btInfluence;     // 0x0B: 1 byte
	int iContributePoint;            // 0x0C: 4 bytes
	int iGensRanking;                // 0x10: 4 bytes
	int iGensClass;                  // 0x14: 4 bytes
	// Total: 11 + 1 + 4 + 4 + 4 = 24 bytes
};

/* 1892 */
struct _tagPMSG_REQ_SAVE_ABUSING_KILLUSER_EXDB
{
	_tagPMSG_REQ_SAVE_ABUSING_KILLUSER_EXDB()
	{
		memset(szAccount, 0, sizeof(szAccount));
		memset(szCharName, 0, sizeof(szCharName));
		memset(szKillUserName, 0, sizeof(szKillUserName));
		memset(iKillUserCnt, 0, sizeof(iKillUserCnt));
	}

	PBMSG_HEAD2 h; // C1:F8:08 ExDB->GS
	char szAccount[MAX_IDSTRING + 1];
	char szCharName[MAX_IDSTRING + 1];
	char szKillUserName[MAX_GENSABUSEINFO][MAX_IDSTRING + 1];
	int iKillUserCnt[MAX_GENSABUSEINFO];
};

/* 1893 */
#pragma pack (1)
struct _tagPMSG_ANS_ABUSING_INFO
{
	_tagPMSG_ANS_ABUSING_INFO()
	{
		memset(this->szKillUserName, 0, sizeof(this->szKillUserName));
		memset(this->iKillUserCnt, 0, sizeof(this->iKillUserCnt));
		this->NumberH = 0;
		this->NumberL = 0;
	}
	PBMSG_HEAD2 h; // C1:F8:0A ExDB->GS
	char szKillUserName[MAX_GENSABUSEINFO][MAX_IDSTRING + 1];
	int iKillUserCnt[MAX_GENSABUSEINFO];
	BYTE NumberH;
	BYTE NumberL;
};
#pragma pack ()

/* 1894 */
struct _tagPMSG_REQ_SECEDE_GENS_DATE_MODIFY
{
	PBMSG_HEAD2 h; // C1:F8:0B GS->ExDB
	char szCharName[MAX_IDSTRING + 1];
	int iDateModify;
};

/* 1895 */
// Request: GS -> ExDB (C1:F8:0C)
struct _tagPMSG_REQ_GENS_REWARD_EXDB
{
	_tagPMSG_REQ_GENS_REWARD_EXDB()
	{
		memset(szCharName, 0, sizeof(szCharName));
		iInfluence = 0;
		NumberH = 0;
		NumberL = 0;
	}

	PBMSG_HEAD2 h;               // 0x00: 4 bytes: C1:F8:0C
	char szCharName[MAX_IDSTRING + 1];         // 0x04: 11 bytes
	int iInfluence;              // 0x10: 4 bytes (aligned)
	BYTE NumberH;     // 0x14: 1 byte
	BYTE NumberL;     // 0x15: 1 byte
	// Total: 4 + 11 + 1(pad) + 4 + 1 + 1 = 22 bytes (+ 2 padding = 24 with align(4))
};

// Response: ExDB -> GS (C1:F8:0D) - 16 bytes
/* 1896 */
struct _tagPMSG_ANS_GENS_REWARD_EXDB
{
	_tagPMSG_ANS_GENS_REWARD_EXDB()
	{
		btResult = 0;
		iGensClass = 0;
		NumberH = 0;
		NumberL = 0;
	}

	PBMSG_HEAD2 h;               // 0x00: 4 bytes: C1:F8:0D
	BYTE btResult;    // 0x04: 1 byte
	int iGensClass;              // 0x08: 4 bytes (aligned)
	BYTE NumberH;     // 0x0C: 1 byte
	BYTE NumberL;     // 0x0D: 1 byte
	// Total: 4 + 1 + 3(pad) + 4 + 1 + 1 = 14 bytes (+ 2 padding = 16 with align(4))
};

/* 1897 */
typedef struct _tagPMSG_ANS_GENS_MEMBER_COUNT_EXDB
{
	_tagPMSG_ANS_GENS_MEMBER_COUNT_EXDB()
	{
		iGensMemberCount1 = 0;
		iGensMemberCount2 = 0;
		NumberH = 0;
		NumberL = 0;
	}
	PBMSG_HEAD2 h; // C1:F8:10 ExDB->GS
	int iGensMemberCount1;
	int iGensMemberCount2;
	BYTE NumberH;
	BYTE NumberL;
} *LPPMSG_ANS_GENS_MEMBER_COUNT_EXDB;

/* 1898 */
typedef struct _tagPMSG_ANS_GENS_REWARD_DAY_CHECK_EXDB
{
	PBMSG_HEAD2 h; // C1:F8:14 ExDB->GS
	BYTE NumberH;
	BYTE NumberL;
	int iResult;
} *LPPMSG_ANS_GENS_REWARD_DAY_CHECK_EXDB;


/* 1900 */
struct _tagPMSG_REQ_REG_GENS_MEMBER_EXDB//34
{
	_tagPMSG_REQ_REG_GENS_MEMBER_EXDB()
	{
		memset(szAccount, 0, MAX_IDSTRING + 1);
		memset(szCharName, 0, MAX_IDSTRING + 1);
		btInfluence = 0;
		GuildNumH = 0;
		GuildNumL = 0;
		NumberH = 0;
		NumberL = 0;
	}
	PBMSG_HEAD2 h; // C1:F8:03 GS->ExDB
	char szAccount[MAX_IDSTRING + 1];
	char szCharName[MAX_IDSTRING + 1];
	BYTE btInfluence;
	WORD GuildNumH;
	WORD GuildNumL;
	BYTE NumberH;
	BYTE NumberL;
};
/* 1901 */
typedef struct _tagPMSG_REQ_SECEDE_GENS_MEMBER_EXDB//22
{
	_tagPMSG_REQ_SECEDE_GENS_MEMBER_EXDB()
	{
		memset(szCharName, 0, sizeof(szCharName));
		GuildNumH = 0;
		GuildNumL = 0;
		NumberH = 0;
		NumberL = 0;
	}

	PBMSG_HEAD2 h; // C1:F8:05 GS->ExDB
	char szCharName[MAX_IDSTRING + 1];
	WORD GuildNumH;
	WORD GuildNumL;
	BYTE NumberH;
	BYTE NumberL;
} *LPPMSG_REQ_SECEDE_GENS_MEMBER_EXDB;

/* 1902 */
struct _tagPMSG_REQ_GENS_INFO_EXDB//17
{
	PBMSG_HEAD2 h; // C1:F8:01 GS->ExDB
	char szCharName[MAX_IDSTRING + 1];
	BYTE NumberH;
	BYTE NumberL;
};

/* 1903 */

struct _tagPMSG_REQ_SAVE_CONTRIBUTE_POINT_EXDB
{
	_tagPMSG_REQ_SAVE_CONTRIBUTE_POINT_EXDB()
	{
		memset(szCharName, 0, sizeof(szCharName));
		iContributePoint = 0;
		NumberH = 0;
		NumberL = 0;
		iGensClass = 0;
	}

	PBMSG_HEAD2 h; // C1:F8:07 GS->ExDB
	char szCharName[MAX_IDSTRING + 1];
	int iContributePoint;
	BYTE NumberH;
	BYTE NumberL;
	int iGensClass;
};


/* 1904 */
struct _tagPMSG_REQ_ABUSING_INFO
{
	_tagPMSG_REQ_ABUSING_INFO()
	{
		memset(szCharName, 0, sizeof(szCharName));
		NumberH = 0;
		NumberL = 0;
	}

	PBMSG_HEAD2 h; // C1:F8:09 GS->ExDB
	char szCharName[MAX_IDSTRING + 1];
	BYTE NumberH;
	BYTE NumberL;
};

/* 1905 */
struct _tagPMSG_REQ_GENS_REWARD_COMPLETE_EXDB
{
	PBMSG_HEAD2 h; // C1:F8:0E GS->ExDB
	char szCharName[MAX_IDSTRING + 1];
	BYTE NumberH;
	BYTE NumberL;
};

/* 1906 */
struct _tagPMSG_REQ_GENS_MEMBER_COUNT_EXDB
{
	_tagPMSG_REQ_GENS_MEMBER_COUNT_EXDB()
	{
		memset(szCharName, 0, sizeof(szCharName));
		NumberH = 0;
		NumberL = 0;
	}
	PBMSG_HEAD2 h; // C1:F8:0F GS->ExDB
	char szCharName[MAX_IDSTRING + 1];
	BYTE NumberH;
	BYTE NumberL;
};

/* 1907 */
struct _tagPMSG_REQ_SET_GENS_REWARD_DAY_EXDB
{
	PBMSG_HEAD2 h; // C1:F8:11 GS->ExDB
};

/* 1908 */
struct _tagPMSG_REQ_SET_GENS_RANKING_EXDB
{
	PBMSG_HEAD2 h; // C1:F8:12 GS->ExDB
};

/* 1909 */
struct _tagPMSG_REQ_GENS_REWARD_DAY_CHECK_EXDB
{
	PBMSG_HEAD2 h; // C1:F8:13 GS->ExDB
	BYTE NumberH;
	BYTE NumberL;
};

/* 5333 */
struct SDHP_GUILDCREATE
{
	PBMSG_HEAD	h; // C1:30 GS->ExDB
	char		GuildName[MAX_GUILDNAMESTRING + 1];
	char		Master[MAX_IDSTRING + 1];
	BYTE		Mark[MAX_GUILDMARKBUFFER];
	BYTE		NumberH;
	BYTE		NumberL;
	BYTE		btGuildType;
};

/* 5334 */
struct SDHP_GUILDCREATE_RESULT
{
	PBMSG_HEAD h; // C1:30 ExDB->GS
	BYTE Result;
	BYTE Flag;
	DWORD GuildNumber;
	BYTE NumberH;
	BYTE NumberL;
	char Master[MAX_IDSTRING + 1];
	char GuildName[MAX_GUILDNAMESTRING + 1];
	BYTE Mark[MAX_GUILDMARKBUFFER];
	BYTE btGuildType;
};

/* 3948 */
struct _tagPMSG_ANS_SECEDE_GENS_MEMBER_EXDB
{
	_tagPMSG_ANS_SECEDE_GENS_MEMBER_EXDB()
	{
		btResult = 0;
		NumberH = 0;
		NumberL = 0;
	}

	PBMSG_HEAD2 h; // C1:F8:06 ExDB->GS
	BYTE btResult;
	BYTE NumberH;
	BYTE NumberL;
};

/* 5385 */
struct EXSDHP_UNION_RELATIONSHIP_LIST
{
	PWMSG_HEAD h; // C2:E7 ExDB->GS
	BYTE btFlag;
	BYTE btRelationShipType;
	BYTE btRelationShipMemberCount;
	char szUnionMasterGuildName[MAX_GUILDNAMESTRING + 1];
	int iUnionMasterGuildNumber;
	int iRelationShipMember[100];
};

/* 4311 */
struct _tagPMSG_ANS_REG_GENS_MEMBER_EXDB//8
{
	_tagPMSG_ANS_REG_GENS_MEMBER_EXDB()
	{
		btResult = 0;
		btInfluence = 0;
		NumberH = 0;
		NumberL = 0;
	}
	PBMSG_HEAD2 h; // C1:F8:04 ExDB->GS
	BYTE btResult;
	BYTE btInfluence;
	BYTE NumberH;
	BYTE NumberL;
};

/* 4732 */
struct _tagPMSG_ANS_GENS_INFO_EXDB//28
{
	_tagPMSG_ANS_GENS_INFO_EXDB()
	{
		this->btInfluence = 0;
		this->iContributePoint = 0;
		this->NumberH = 0;
		this->NumberL = 0;
		this->btResult = 0;
		this->iGensRanking = 0;
		this->iGensClass = 0;
		this->iGensMemberCount = 0;
	}
	PBMSG_HEAD2 h; // C1:F8:02 ExDB->GS
	BYTE btInfluence;
	int iContributePoint;
	BYTE NumberH;
	BYTE NumberL;
	BYTE btResult;
	int iGensRanking;
	int iGensClass;
	int iGensMemberCount;
};

/* 5358 */
struct FHP_FRIEND_DEL_RESULT//28
{
	PBMSG_HEAD h; // C1:65 ExDB->GS
	short Number;
	BYTE Result;
	char Name[MAX_IDSTRING];
	char FriendName[MAX_IDSTRING];
};

/* 5355 */
struct FHP_FRIEND_STATE
{
	PBMSG_HEAD h;        // 3 bytes (typical: C1 + size + headcode)
	short Number;        // 2 bytes
	char Name[MAX_IDSTRING];       // player name
	char FriendName[MAX_IDSTRING]; // friend name
	BYTE State; // online/offline/etc
};

/* 5356 */
struct FHP_FRIEND_ADD_RESULT
{
	PBMSG_HEAD h; // C1:63 ExDB->GS
	short Number;
	BYTE Result;
	char Name[MAX_IDSTRING];
	char FriendName[MAX_IDSTRING];
	BYTE Server;
};

/* 5357 */
struct FHP_WAITFRIEND_ADD_RESULT
{
	PBMSG_HEAD h; // C1:64 ExDB->GS
	short Number;
	BYTE Result;
	char Name[MAX_IDSTRING];
	char FriendName[MAX_IDSTRING];
	BYTE pServer;
};

/* 5361 */
struct FHP_FRIEND_MEMO_SEND_RESULT//24
{
	PBMSG_HEAD h; // C1:70 ExDB->GS
	short Number;
	char Name[MAX_IDSTRING];
	BYTE Result;
	DWORD WindowGuid;
};

/* 5363 */
struct FHP_FRIEND_MEMO_RECV
{
	PWMSG_HEAD h; // C2:72 ExDB->GS
	short Number;
	char Name[MAX_IDSTRING];
	WORD MemoIndex;
	short MemoSize;
#ifdef ITEM_INDEX_EXTEND_20050706
	BYTE		Photo[MAX_PREVIEWCHARSET + 9];
#else
#ifdef DARKLORD_WORK
	BYTE		Photo[MAX_PREVIEWCHARSET + 4];
#else
	BYTE		Photo[MAX_PREVIEWCHARSET + 3];
#endif
#endif
	BYTE Dir;
	BYTE Action;
	char Memo[MAX_MEMO];
};

/* 5384 */
struct EXSDHP_RELATIONSHIP_BREAKOFF_RESULT//20
{
	PBMSG_HEAD h;
	BYTE btFlag;
	short wRequestUserIndex;
	short wTargetUserIndex;
	BYTE btResult;
	BYTE btRelationShipType;
	int iRequestGuildNum;
	int iTargetGuildNum;
};

/* 5386 */
struct EXSDHP_NOTIFICATION_RELATIONSHIP
{
	PWMSG_HEAD h; // C2:E8 ExDB->GS
	BYTE btFlag;
	BYTE btUpdateFlag;
	BYTE btGuildListCount;
	int iGuildList[100];
};

/* 5345 */
struct SDHP_GUILDMEMBER_INFO
{
	PBMSG_HEAD h; // C1:35 ExDB->GS
	char GuildName[MAX_GUILDNAMESTRING + 1];
	char MemberID[MAX_IDSTRING + 1];
	BYTE btGuildStatus;
	BYTE btGuildType;
	short pServer;
};

/* 5346 */
struct SDHP_GUILDALL_COUNT
{
	PWMSG_HEAD h; // C2:36 ExDB->GS
	int	Number;
	char GuildName[MAX_GUILDNAMESTRING + 1];
	char Master[MAX_IDSTRING + 1];
	BYTE Mark[MAX_GUILDMARKBUFFER];
	int score;
	BYTE btGuildType;
	int iGuildUnion;
	int iGuildRival;
	char szGuildRivalName[MAX_GUILDNAMESTRING + 1];
	BYTE Count;
};

/* 5347 */
struct SDHP_GUILDALL
{
	char MemberID[MAX_IDSTRING + 1];
	BYTE btGuildStatus;
	short pServer;
};

/* 5348 */
struct SDHP_GUILDSCOREUPDATE
{
	PBMSG_HEAD h;
	char GuildName[MAX_GUILDNAMESTRING + 1];
	int Score;
};

/* 5349 */
struct SDHP_GUILDNOTICE
{
	PBMSG_HEAD h; // C1:38 GS->ExDB / ExDB->GS
	char GuildName[MAX_GUILDNAMESTRING + 1];
	char szGuildNotice[MAX_GUILDNOTICE];
};

/* 5350 */
struct SDHP_GUILDCREATED
{
	PBMSG_HEAD h; // C1:40 ExDB->GS
	WORD Number;
	char GuildName[MAX_GUILDNAMESTRING + 1];
	char Master[MAX_IDSTRING + 1];
	BYTE Mark[MAX_GUILDMARKBUFFER];
	int score;
};

/* 5351 */
struct SDHP_GUILDLISTSTATE
{
	PBMSG_HEAD h; // 3 bytes
	BYTE State;  // 1 byte
	int Count;   // 4 bytes
};

/* 5352 */
struct FHP_FRIENDLIST_COUNT
{
	PWMSG_HEAD h; // C2:60 ExDB->GS
	short Number;
	char Name[MAX_IDSTRING];
	BYTE Count;
	BYTE MailCount;
	BYTE Server;
};

/* 5353 */
#pragma pack(push, 1)

struct FHP_FRIENDLIST
{
	char Name[MAX_IDSTRING];
	BYTE Server;
};

#pragma pack(pop)

/* 5354 */
struct FHP_WAITFRIENDLIST_COUNT
{
	PBMSG_HEAD h; // C1:61 ExDB->GS
	short Number;
	char Name[MAX_IDSTRING];
	char FriendName[MAX_IDSTRING];
};

/* 5335 */
struct SDHP_GUILDDESTROY
{
	PBMSG_HEAD h;
	BYTE NumberH;
	BYTE NumberL;
	char GuildName[MAX_GUILDNAMESTRING];
	char Master[MAX_IDSTRING];
};

/* 5336 */
struct SDHP_GUILDDESTROY_RESULT
{
	PBMSG_HEAD h; // C1:31 ExDB->GS
	BYTE Result;
	BYTE Flag;
	BYTE NumberH;
	BYTE NumberL;
	char GuildName[MAX_GUILDNAMESTRING + 1];
	char Master[MAX_IDSTRING + 1];
};

/* 5337 */
struct SDHP_GUILDMEMBERADD
{
	PBMSG_HEAD h; // C1:32 GS->ExDB
	char GuildName[MAX_GUILDNAMESTRING + 1];
	char MemberID[MAX_IDSTRING + 1];
	BYTE NumberH;
	BYTE NumberL;
};

/* 5338 */
struct SDHP_GUILDMEMBERADD_RESULT
{
	PBMSG_HEAD h; // C1:32 ExDB->GS
	BYTE Result;
	BYTE Flag;
	BYTE NumberH;
	BYTE NumberL;
	char GuildName[MAX_GUILDNAMESTRING + 1];
	char MemberID[MAX_IDSTRING + 1];
	short pServer;
};

/* 5339 */
struct SDHP_GUILDMEMBERADD_WITHOUT_USERINDEX
{
	PBMSG_HEAD h; // C1:39 GS->ExDB
	char GuildName[MAX_GUILDNAMESTRING + 1];
	char MemberID[MAX_IDSTRING + 1];
};

/* 5340 */
struct SDHP_GUILDMEMBERADD_RESULT_WITHOUT_USERINDEX
{
	PBMSG_HEAD h; // C1:39 ExDB->GS
	BYTE Result;
	BYTE Flag;
	char GuildName[MAX_GUILDNAMESTRING + 1];
	char MemberID[MAX_IDSTRING + 1];
	short pServer;
	WORD Number;
};

/* 5341 */
struct SDHP_GUILDMEMBERDEL
{
	PBMSG_HEAD h; // C1:33 GS->ExDB
	BYTE NumberH;
	BYTE NumberL;
	char GuildName[MAX_GUILDNAMESTRING];
	char MemberID[MAX_IDSTRING];
};

/* 5342 */
struct SDHP_GUILDMEMBERDEL_RESULT
{
	PBMSG_HEAD h; // C1:33 ExDB->GS
	BYTE Result;
	BYTE Flag;
	BYTE NumberH;
	BYTE NumberL;
	char GuildName[MAX_GUILDNAMESTRING + 1];
	char MemberID[MAX_IDSTRING + 1];
};

/* 5343 */
struct SDHP_GUILDMEMBER_INFO_REQUEST
{
	PBMSG_HEAD h; // C1:35 GS->ExDB
	BYTE NumberH;
	BYTE NumberL;
	char MemberID[MAX_IDSTRING];
};

/* 5344 */
struct SDHP_GUILDMEMBER_INFO_GUILDNAME_REQUEST
{
	PBMSG_HEAD h;
	char szGuildName[MAX_GUILDNAMESTRING+1];
};

/* 5416 */
struct _stReqJoinMemberPartyMatching//48
{
	PBMSG_HEAD2 h; // C1:A4:02 GS->ExDB
	char szMemberName[MAX_IDSTRING + 1];
	char szLeaderName[MAX_IDSTRING + 1];
	char szPassWord[MAX_PMATCHING_PASSWORDSTR + 1];
	int nUserIndex;
	int nUserDBNumber;
	int nLevel;
	BYTE btClass;
	BYTE btRandomParty;
	BYTE btGensType;
	BYTE btChangeUpClass;
};

/* 5417 */
struct _stAnsJoinMemberPartyMatching
{
	PBMSG_HEAD2 h;
	int nUserIndex;
	int nResult;
};

/* 5418 */
struct _stReqJoinMemberStateListPartyMatching
{
	PBMSG_HEAD2 h;
	int nUserIndex;
	char szMemberName[MAX_IDSTRING + 1];
};

/* 5419 */
struct _stAnsJoinMemberStateListPartyMatching
{
	PBMSG_HEAD2 h; // C1:A4:03 ExDB->GS
	int nUserIndex;
	int nResult;
	BYTE btLeaderChannel;
	char szLeaderName[MAX_IDSTRING + 1];
};

/* 5420 */
struct _stReqWaitListPartyMatching
{
	PBMSG_HEAD2 h;
	int nUserIndex;
	char szLeaderName[MAX_IDSTRING + 1];
};

/* 5421 */
struct _stAnsWaitListPartyMatching
{
	PWMSG_HEAD2 h;
	int nUserIndex;
	int nListCount;
	int nResult;
	_PARTY_MEMBER_WAIT_LIST stList[10];
};

/* 5422 */
struct __stReqAddPartyMember
{
	PBMSG_HEAD2 h; // C1:A4:05 GS->ExDB
	int nUserIndex;
	int nMemberIndex;
	BYTE btType;
	BYTE btManualJoin;
	BYTE btAlradyParty;
	char szLeaderName[MAX_IDSTRING + 1];
	char szMemberName[MAX_IDSTRING + 1];
	int nLeaderLevel;
	int nMemberLevel;
	BYTE btLeaderClass;
	BYTE btMemeberClass;
};

/* 5423 */
struct __stAnsAddPartyMember
{
	PBMSG_HEAD2 h; // C1:A4:05 ExDB->GS
	int nResult;
	int nUserIndex;
	int nMemberIndex;
	BYTE btType;
	BYTE btSendType;
	BYTE btManualJoin;
	char szMemberName[MAX_IDSTRING + 1];
};


/* 5424 */
struct _stReqCancePartyMatching
{
	PBMSG_HEAD2 h; // C1:A4:06 GS->ExDB
	BYTE btType;
	char szName[MAX_IDSTRING + 1];
	int nUserIndex;
};


/* 5425 */
struct _stAnsCancePartyMatching
{
	PBMSG_HEAD2 h; // C1:A4:06 ExDB->GS
	BYTE btType;
	int nUserIndex;
	int nResult;
};

/* 5426 */
struct _stReqDelPartyUserPartyMatching
{
	PBMSG_HEAD2 h; // C1:A4:07 GS->ExDB
	char szTargetName[MAX_IDSTRING + 1];
	int nUserIndex;
	BYTE btDelType;
};

/* 5427 */
struct _stAnsDelPartyUserPartyMatching
{
	PBMSG_HEAD2 h; // C1:A4:07 ExDB->GS
	int nUserIndex;
	int nTargetIndex;
	int nResult;
	BYTE btType;
};

/* 5428 */
struct _stAnsRequestJoinPartyMatchingNoti
{
	PBMSG_HEAD2 h;
	int nUserIndex;
};

/* 5429 */
struct __stAnsAddRealPartyMember
{
	PBMSG_HEAD2 h;
	int nLeaderUserIndex;
	int nMemberUserIndex;
};

/* 5430 */
struct _stReqChattingPartyMatching
{
	PBMSG_HEAD2 h; // C1:A4:12 GS->ExDB
	int nPartyIndex;
	char szChat[MAX_CHAT + 1];
	char szSendCharName[MAX_IDSTRING + 1];
};

/* 5431 */
struct _stAnsChattingPartyMatching
{
	PBMSG_HEAD2 h; // C1:A4:12 ExDB->GS
	int nRecvUserIndex;
	int nPartyIndex;
	char szChat[MAX_CHAT + 1];
	char szSendCharName[MAX_IDSTRING + 1];
};

/* 5432 */
struct __stReqSendPartyMemberList
{
	PBMSG_HEAD2 h;
	char szLeaderName[MAX_IDSTRING + 1];
};

/* 5433 */
struct __stAnsSendPartyMemberList//156
{
	__stAnsSendPartyMemberList()
	{
		// Initialize header
		memset(&h, 0, sizeof(h));
		nUserIndex = 0;
		nMemberCount = 0;
		nPartyMatchingIndex = 0;

		// Initialize all 5 member entries
		for (int i = 0; i < 5; ++i)
		{
			memset(&stList[i], 0, sizeof(_SEND_PARTYMEMBER_INFO));
		}
	}
	PBMSG_HEAD2 h;
	int nUserIndex;
	int nMemberCount;
	int nPartyMatchingIndex;
	_SEND_PARTYMEMBER_INFO stList[5];
};

/* 5359 */
struct FHP_FRIEND_CHATROOM_CREATE_REQ
{
	PBMSG_HEAD h; // C1:66 GS->ExDB
	short Number;
	char Name[MAX_IDSTRING];
	char fName[MAX_IDSTRING];
};

/* 5360 */
struct FHP_FRIEND_CHATROOM_CREATE_RESULT
{
	PBMSG_HEAD h; // C1:66 ExDB->GS
	BYTE Result;
	short Number;
	char Name[MAX_IDSTRING];
	char FriendName[MAX_IDSTRING];
	char ServerIp[15];
	WORD RoomNumber;
	DWORD Ticket;
	BYTE Type;
};

/* 5364 */
struct FHP_FRIEND_MEMO_DEL_RESULT
{
	PBMSG_HEAD h; // C1:73 ExDB->GS
	BYTE Result;
	WORD MemoIndex;
	short Number;
	char Name[MAX_IDSTRING];
};

/* 5365 */
struct FHP_FRIEND_INVITATION_REQ
{
	PBMSG_HEAD h; // C1:74 GS->ExDB
	short Number;
	char Name[MAX_IDSTRING];
	char FriendName[MAX_IDSTRING];
	WORD RoomNumber;
	DWORD WindowGuid;
};

/* 5366 */
struct FHP_FRIEND_INVITATION_RET
{
	PBMSG_HEAD h; // C1:74 ExDB->GS
	BYTE Result;
	short Number;
	char Name[MAX_IDSTRING];
	DWORD WindowGuid;
};

/* 5367 */
struct FHP_CHAT_ROOMCREATE
{
	PBMSG_HEAD h;
	char Name[MAX_IDSTRING];
	char FriendName[MAX_IDSTRING];
	short UserNumber;
	short ServerNumber;
	short FriendNumber;
	short FriendServerNumber;
};

/* 5368 */
struct FHP_CHAT_ROOMCREATE_RESULT
{
	PBMSG_HEAD h;
	BYTE Result;
	short RoomNumber;
	char Name[MAX_IDSTRING];
	char FriendName[MAX_IDSTRING];
	short UserNumber;
	short ServerNumber;
	DWORD Ticket;
	DWORD FriendTicket;
	BYTE Type;
};

/* 5369 */
struct FHP_CHAT_JOINUSER_REQ
{
	PBMSG_HEAD h;
	WORD RoomNumber;
	char Name[MAX_IDSTRING];
	short UserNumber;
	short ServerNumber;
	BYTE Type;
};

/* 5371 */
struct FHP_CHAT_ROOMINVITATION//120
{
	PBMSG_HEAD h;
	WORD RoomNumber;
	char Name[MAX_IDSTRING];
	DWORD Ticket;
	char Msg[100];
};

/* 5372 */
struct EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND
{
	PBMSG_HEAD h; // C1:50 GS->ExDB
	int iGuildNum;
	char szCharacterName[MAX_IDSTRING];
	char szChattingMsg[MAX_CHAT];
};
/* 5373 */
struct EXSDHP_SERVERGROUP_UNION_CHATTING_SEND
{
	PBMSG_HEAD h; // C1:51 GS->ExDB
	int iUnionNum;
	char szCharacterName[MAX_IDSTRING];
	char szChattingMsg[MAX_CHAT];
};
/* 5374 */
struct EXSDHP_SERVERGROUP_GENS_CHATTING_SEND
{
	PBMSG_HEAD h;
	int iInfluence;
	char szCharacterName[MAX_IDSTRING];
	char szChattingMsg[MAX_CHAT];
};
/* 5375 */
struct PMSG_REQ_GUILD_PERIODBUFF_INSERT
{
	PBMSG_HEAD2 head;
	char szGuildName[MAX_GUILDNAMESTRING + 1];
	WORD wBuffIndex;
	BYTE btEffectType1;
	BYTE btEffectType2;
	DWORD dwDuration;
	long lExpireDate;
};
/* 5376 */
struct PMSG_REQ_GUILD_PERIODBUFF_DELETE
{
	PBMSG_HEAD2 head;
	BYTE btGuildCnt;
	WORD wBuffIndex[5];
};
/* 5377 */
struct EXSDHP_GUILD_ASSIGN_STATUS_REQ
{
	PBMSG_HEAD h; // C1:E1 GS->ExDB
	WORD wUserIndex;
	BYTE btType;
	BYTE btGuildStatus;
	char szGuildName[MAX_GUILDNAMESTRING + 1];
	char szTargetName[MAX_IDSTRING + 1];
};
/* 5378 */
struct EXSDHP_GUILD_ASSIGN_STATUS_RESULT
{
	PBMSG_HEAD h; // C1:E1 ExDB->GS
	BYTE btFlag;
	WORD wUserIndex;
	BYTE btType;
	BYTE btResult;
	BYTE btGuildStatus;
	char szGuildName[MAX_GUILDNAMESTRING + 1];
	char szTargetName[MAX_IDSTRING + 1];
};
/* 5379 */
struct EXSDHP_GUILD_ASSIGN_TYPE_REQ
{
	PBMSG_HEAD h; // C1:E2 GS->ExDB
	WORD wUserIndex;
	BYTE btGuildType;
	char szGuildName[MAX_GUILDNAMESTRING + 1];
};
/* 5380 */
struct EXSDHP_GUILD_ASSIGN_TYPE_RESULT
{
	PBMSG_HEAD h; // C1:E2 ExDB->GS
	BYTE btFlag;
	WORD wUserIndex;
	BYTE btGuildType;
	BYTE btResult;
	char szGuildName[MAX_GUILDNAMESTRING + 1];
};

/* 5381 */
struct EXSDHP_RELATIONSHIP_JOIN_REQ//20
{
	PBMSG_HEAD h; // C1:E5 GS->ExDB
	WORD wRequestUserIndex;
	WORD wTargetUserIndex;
	BYTE btRelationShipType;
	int iRequestGuildNum;
	int iTargetGuildNum;
};

/* 5382 */
struct EXSDHP_RELATIONSHIP_JOIN_RESULT  // 40 bytes
{
	PBMSG_HEAD h; // C1:E5 ExDB->GS
	BYTE btFlag;
	WORD wRequestUserIndex;
	WORD wTargetUserIndex;
	BYTE btResult;
	BYTE btRelationShipType;
	int iRequestGuildNum;
	int iTargetGuildNum;
	char szRequestGuildName[MAX_GUILDNAMESTRING + 1];
	char szTargetGuildName[MAX_GUILDNAMESTRING + 1];
};

/* 5383 */
struct EXSDHP_RELATIONSHIP_BREAKOFF_REQ//20
{
	PBMSG_HEAD h; // C1:E6 GS->ExDB
	WORD wRequestUserIndex;
	WORD wTargetUserIndex;
	BYTE btRelationShipType;
	int iRequestGuildNum;
	int iTargetGuildNum;
};

/* 5387 */
struct EXSDHP_UNION_LIST_REQ//12
{
	PBMSG_HEAD h; // C1:E9 GS->ExDB
	WORD wRequestUserIndex;
	int iUnionMasterGuildNumber;
};

/* 5388 */
struct EXSDHP_UNION_LIST_COUNT
{
	PWMSG_HEAD h;                // 0x00
	BYTE btCount;                // 0x04
	BYTE btResult;               // 0x05
	WORD wRequestUserIndex;      // 0x06
	DWORD iTimeStamp;            // 0x08
	BYTE btRivalMemberNum;       // 0x0C
	BYTE btUnionMemberNum;       // 0x0D
};

/* 5389 */
struct EXSDHP_UNION_LIST
{
	BYTE btMemberNum;
	BYTE Mark[MAX_GUILDMARKBUFFER];
	char szGuildName[MAX_GUILDNAMESTRING];
};

/* 5390 */
struct EXSDHP_KICKOUT_UNIONMEMBER_REQ//24
{
	PBMSG_HEAD2 h; // C1:EB:01 GS->ExDB
	WORD wRequestUserIndex;
	BYTE btRelationShipType;
	char szUnionMasterGuildName[MAX_GUILDNAMESTRING];
	char szUnionMemberGuildName[MAX_GUILDNAMESTRING];
};

/* 5391 */
struct EXSDHP_KICKOUT_UNIONMEMBER_RESULT//28
{
	PBMSG_HEAD2 h; // C1:EB:01 ExDB->GS
	BYTE btFlag;
	WORD wRequestUserIndex;
	BYTE btRelationShipType;
	BYTE btResult;
	char szUnionMasterGuildName[MAX_GUILDNAMESTRING + 1];
	char szUnionMemberGuildName[MAX_GUILDNAMESTRING + 1];
};

/* 5392 */
typedef struct _stReqGuildMatchingList
{
	PBMSG_HEAD2 h; // C1:A3:00 GS->ExDB
	int nUserIndex;
	int nPage;
} *LPEXSDHP_REQ_GUILDMATCHINGLIST;

/* 5393 */
struct _stAnsGuildMatchingList// 784
{
	PWMSG_HEAD2 h;
	int nUserIndex;
	int nPage;
	int nTotalPage;
	int nListCount;
	int nResult;
	_stGuildMatchingList stGuildMatchingList[MAX_GUILDNAMESTRING + 1];
};

/* 5394 */
struct _stReqGuildMatchingListSearchWord//24
{
	PBMSG_HEAD2 h;
	int nUserIndex;
	int nPage;
	char szSearchWord[MAX_PMATCHING_SEARCHWORDSTR + 1];
};

/* 5395 */
struct _stReqDelGuildMatchingList
{
	PBMSG_HEAD2 h; // C1:A3:03 GS->ExDB
	int nUserIndex;
	int nGuildNumber;
};

/* 5396 */
struct _stAnsDelGuildMatchingList
{
	PBMSG_HEAD2 h; // C1:A3:03 ExDB->GS
	int nUserIndex;
	int nResult;
};

/* 5397 */
struct _stReqGuildMatchingData
{
	PBMSG_HEAD2 h; // C1:A3:02 GS->ExDB
	int nUserIndex;
	_stGuildMatchingList stGuildMatchingList;
};

/* 5398 */
struct _stAnsGuildMatchingData
{
	PBMSG_HEAD2 h; // C1:A3:02 ExDB->GS
	int nUserIndex;
	int nResult;
};

/* 5399 */
struct _stRegWaitGuildMatching
{
	PBMSG_HEAD2 h;
	int nUserIndex;
	_stGuildMatchingAllowListDB stAllowList;
};

/* 5400 */
struct _stAnsWaitGuildMatching
{
	PBMSG_HEAD2 h; // C1:A3:04 ExDB->GS
	int nUserIndex;
	int nResult;
};

/* 5401 */
struct _stReqDelWaitGuildMatchingList
{
	PBMSG_HEAD2 h; // C1:A3:05 GS->ExDB
	int nUserIndex;
	int nType;
	char szName[MAX_IDSTRING + 1];
};

/* 5402 */
struct _stAnsDelWaitGuildMatchingList
{
	PBMSG_HEAD2 h;
	int nUserIndex;
	int nType;
	int nResult;
};

/* 5403 */
struct _stReqAllowJoinGuildMatching
{
	PBMSG_HEAD2 h;
	int nUserIndex;
	int nAllowType;
	char szName[MAX_IDSTRING + 1];
	char szGuildName[MAX_GUILDNAMESTRING + 1];
};

/* 5404 */
struct _stAnsAllowJoinGuildMatching
{
	PBMSG_HEAD2 h;
	int nUserIndex;
	int nAllowType;
	char szMemberName[MAX_IDSTRING + 1];
	char szGuildName[MAX_GUILDNAMESTRING + 1];
	int nResult;
};

/* 5405 */
struct _stReqWaitGuildMatchingList
{
	PBMSG_HEAD2 h;
	int nUserIndex;
	int nGuildNumber;
};

/* 5406 */
struct _stAnsWaitGuildMatchingList
{
	PWMSG_HEAD2 h;
	int nUserIndex;
	int nListCount;
	int nResult;
	_stGuildMatchingAllowList stAllowList[MAX_GUILD];
};

/* 5407 */
struct _stReqWaitStateListGuildMatching
{
	PBMSG_HEAD2 h; // C1:A3:08 GS->ExDB
	int nUserIndex;
	char szName[MAX_IDSTRING + 1];
};

/* 5408 */
struct _stAnsWaitStateListGuildMatching
{
	PBMSG_HEAD2 h;
	char szGuildMasterName[MAX_IDSTRING + 1];
	char szGuildName[MAX_GUILDNAMESTRING + 1];
	int nUserIndex;
	int nResult;
};

/* 5409 */
struct _stAnsNotiGuildMatching
{
	PBMSG_HEAD2 h; // C1:A3:09 ExDB->GS
	int nUserIndex;
	int nResult;
};

/* 5412 */
struct _stReqRegWantedPartyMember
{
	PBMSG_HEAD2 h; // C1:A4:00 GS->ExDB
	int nUserIndex;
	char szPartyLeaderName[MAX_IDSTRING + 1];
	char szTitle[MAX_PMATCHING_TITLESTR + 1];
	char szPassWord[MAX_PMATCHING_PASSWORDSTR + 1];
	short nMinLevel;
	short nMaxLevel;
	short nHuntingGround;
	short nLeaderLevel;
	BYTE btWantedClass;
	BYTE btUsePassWord;
	BYTE btCurrentPartyCnt;
	BYTE btApprovalType;
	BYTE btWantedClassDetailInfo[MAX_PMATCHING_CLASSDETAIL];
	BYTE btGensType;
	BYTE btLeaderClass;
};

/* 5413 */
struct _stAnsRegWantedPartyMember
{
	PBMSG_HEAD2 h;
	int nUserIndex;
	int nResult;
};

/* 5414 */
struct _stReqGetPartyMatchingList
{
	PBMSG_HEAD2 h; // C1:A4:01 GS->ExDB
	int nUserIndex;
	int nPage;
	int nType;
	int nLevel;
	BYTE btClass;
	BYTE btGens;
	char szSearchWord[MAX_PMATCHING_SEARCHWORDSTR + 1];
};

/* 5415 */
struct _stAnsGetPartyMatchingList
{
	PWMSG_HEAD2 h;
	int nUserIndex;
	int nListCount;
	int nPage;
	int nTotalPage;
	int nResult;
	_PARTY_INFO_LIST stPartyInfoList[6];
};

struct FHP_FRIEND_MEMO_LIST
{
	PWMSG_HEAD h; // C2:71 ExDB->GS
	WORD Number;
	WORD MemoIndex;
	char SendName[MAX_IDSTRING];
	char RecvName[MAX_IDSTRING];
	char Date[MAX_DATE];
	char Subject[MAX_MEMO_SUBJECT];
	BYTE read;
};

// Packet structures
struct _stAnsUseGuildMatchingGuild
{
	PBMSG_HEAD2 h;           // 4 bytes: header (0xA3, 0x11, 8)
	int nUserIndex;           // 4 bytes: user index
	// Total: 8 bytes
};

struct _stAnsNotiGuildMatchingForGuildMaster
{
	PBMSG_HEAD2 h;           // 4 bytes: header (0xA3, 0x10, 0x0C)
	int nUserIndex;           // 4 bytes: user index
	int nResult;              // 4 bytes: result code
	// Total: 12 bytes
};


// ----------------------------------------------------------------------
// Friend Protocol Structures
// ----------------------------------------------------------------------


struct FHP_FRIENDLIST_REQ//18
{
	PBMSG_HEAD h; // C1:60 GS->ExDB
	short Number;
	char Name[MAX_IDSTRING];
	BYTE pServer;
};

struct FHP_FRIEND_STATE_C//18
{
	PBMSG_HEAD h; // C1:62 GS->ExDB
	short Number;
	char Name[MAX_IDSTRING];
	BYTE State;
};

struct FHP_FRIEND_ADD_REQ
{
	PBMSG_HEAD h;
	short Number;
	char Name[MAX_IDSTRING];
	char FriendName[MAX_IDSTRING];
};

struct FHP_WAITFRIEND_ADD_REQ
{
	PBMSG_HEAD h;
	BYTE Result;
	short Number;
	char Name[MAX_IDSTRING];
	char FriendName[MAX_IDSTRING];
};

struct FHP_FRIEND_MEMO_SEND
{
	PWMSG_HEAD h;
	short Number;
	DWORD WindowGuid;
	char Name[MAX_IDSTRING];
	char ToName[MAX_IDSTRING];
	char Subject[MAX_MEMO_SUBJECT];
	BYTE Dir;
	BYTE Action;
	short MemoSize;
#ifdef ITEM_INDEX_EXTEND_20050706
	BYTE		Photo[MAX_PREVIEWCHARSET + 9];
#else
#ifdef DARKLORD_WORK
	BYTE		Photo[MAX_PREVIEWCHARSET + 4];
#else
	BYTE		Photo[MAX_PREVIEWCHARSET + 3];
#endif
#endif
	char Memo[MAX_MEMO];
};


/* 8322 */
struct _stReqChangePartyLeader
{
	PBMSG_HEAD2 h;
	int nOldUserIndex;
	char szOldLeaderName[MAX_IDSTRING + 1];
	char szNewLeaderName[MAX_IDSTRING + 1];
};

/* 8323 */
struct _stAnsChangePartyLeader
{
	PBMSG_HEAD2 h; // C1:A4:13 ExDB->GS
	int nOldLeaderIndex;
	int nNewLeaderIndex;
	int nResult;
	BYTE btChangeType;
};

/* 8324 */
struct _stAnsPartyMemberDelType
{
	PBMSG_HEAD2 h; // C1:A4:14 ExDB->GS
	int nRecvUserIndex;
	BYTE btDelType;
	char szDelName[MAX_IDSTRING + 1];
};


int ProtocolCore(BYTE ProtocolNumber, LPBYTE RecvBuffer, SIZE_T RecvLength, int Index);
int cSend(BYTE* Buffer, int size);
void GetJoinInfo(SDHP_SERVERINFO* lpMsg, short aIndex);
void GDCharacterClose(SDHP_USERCLOSE* lpMsg, short aIndex);
void GSGuildCreate(SDHP_GUILDCREATE* lpMsg, int aIndex);
void DGGuildMemberInfoRequest(SDHP_GUILDMEMBER_INFO_REQUEST* lpMsg, int aIndex);
void DGGuildMemberInfoSend(int aIndex, char* name, short pServer);
void GDGuildAllSend(int aIndex, char* GuildName);
void GDGuildAllSend(int aIndex, SDHP_GUILDMEMBER_INFO_GUILDNAME_REQUEST* lpRecv);
int DGGuildInfoSend(int aIndex, char* guildName, char* guildMaster, BYTE* mark, int score, WORD number);
void DGGuildInfoSendState(int aIndex, BYTE state, int count);
void GDGuildListRequest();
void DGGuildInfoAllSend(char* guildname, char* guildmaster, unsigned char* mark, int score, unsigned short number);
void GDGuildDestroy(SDHP_GUILDDESTROY* lpMsg, int aIndex);
void GDGuildMemberAdd(SDHP_GUILDMEMBERADD* lpMsg, int aIndex);
void GDGuildMemberAddWithoutUserIndex(SDHP_GUILDMEMBERADD_WITHOUT_USERINDEX* lpMsg, int aIndex);
void GDGuildMemberDel(SDHP_GUILDMEMBERDEL* lpMsg, int aIndex);
void GDGuildScoreUpdateRecv(SDHP_GUILDSCOREUPDATE* lpMsg);
void DGGuildScoreUpdate(char* guildname, int score);
void GDGuildNoticeRecv(SDHP_GUILDNOTICE* lpMsg);
void DGFriendList(_FRIEND_INFO_STRUCT* lpNode, int aIndex);
int DGWaitFriendList(int GUID, char* Name, __int16 Number, int aIndex);
void GDFrinedListReq(FHP_FRIENDLIST_REQ* lpMsg, int aIndex);
void DGFriendStateSend(char* Name, char* FriendName, unsigned char State, short Number, int aIndex);
void DGFriendOfflineSend(char* szName);
void DGFriendStateAllSend(_FRIEND_INFO_STRUCT* lpNode);
void DGFriendStateAllSend(_FRIEND_INFO_STRUCT* lpNode, __int16 number, int aIndex, unsigned __int8 state);
void GDFriendStateChange(FHP_FRIEND_STATE_C* lpMsg, int aIndex);
void GDFriendAddReq(FHP_FRIEND_ADD_REQ* lpMsg, int aIndex);
int FriendAddFunc(char* Name, char* friendName, unsigned __int8 server, int index, unsigned __int8* FriendConectServer, __int16 serverindex);
void GDWaitFriendAddReq(FHP_WAITFRIEND_ADD_REQ* lpMsg, int aIndex);
void GDFriendDelReq(FHP_FRIEND_ADD_REQ* lpMsg, int aIndex);
void GDFriendMemoSend(FHP_FRIEND_MEMO_SEND* lpMsg, int aIndex);
void GDFriendMemoListReq(FHP_FRIEND_MEMO_LIST_REQ* lpMsg, int aIndex);
void DGFriendMemoList(int aIndex, unsigned __int16 Number, unsigned __int16 MemoNumber, char* SendName, char* RecvName, char* Date, char* Subject, unsigned __int8 read);
void GDFriendMemoRead(FHP_FRIEND_MEMO_RECV_REQ* lpMsg, int aIndex);
void GDFriendMemoDel(FHP_FRIEND_MEMO_DEL_REQ* lpMsg, int aIndex);
void GDFriendChatRoomCreateRecv(FHP_FRIEND_CHATROOM_CREATE_REQ* lpMsg, int Index);
void GDFriendInvitationRecv(FHP_FRIEND_INVITATION_REQ* lpMsg, int aIndex);
void DCCreateChatRoom(char* szName, int usernumber, char* szFriendName, int friendnumber, __int16 servernumber, __int16 friendservernumber);
void DCChatRoomInvitation(FHP_CHAT_ROOMINVITATION* lpMsg);
void CDCreateChatRoomResult(FHP_CHAT_ROOMCREATE_RESULT* lpMsg);
void DGRoomInvitationReq(char* Name, signed __int16 room, __int16 server, __int16 usernumber, unsigned __int8 Type);
void DGRoomInvitationRecv();
void GDGuildReqAssignStatus(EXSDHP_GUILD_ASSIGN_STATUS_REQ* lpMsg, int iServerIndex);
void GDGuildReqAssignType(EXSDHP_GUILD_ASSIGN_TYPE_REQ* lpMsg, int iServerIndex);
void GDRelationShipReqJoin(EXSDHP_RELATIONSHIP_JOIN_REQ* lpMsg, int iServerIndex);
void GDRelationShipReqBreakOff(EXSDHP_RELATIONSHIP_BREAKOFF_REQ* lpMsg, int iServerIndex);
void GDUnionListReq(EXSDHP_UNION_LIST_REQ* lpRecvMsg, int iServerIndex);
void DGRelationShipJoin(int iRequestGuildNum, int iTargetGuildNum, BYTE btRelationShipType, BYTE btTranFlag);
void DGRelationShipBreakOff(int iRequestGuildNum, int iTargetGuildNum, BYTE iRelationShipType, BYTE btTranFlag);
void DGRelationShipNotificationSend(BYTE iNotificationFlag, int iGuildListCount, int* iGuildList, BYTE btTranFlag);
void DGRelationShipListSend(_GUILD_INFO_STRUCT* lpGuildInfo, int iRelationShipType, BYTE btTranFlag, int iServerIndex);
void GDRelationShipReqKickOutUnionMember(EXSDHP_KICKOUT_UNIONMEMBER_REQ* lpMsg, int iServerIndex);
void GDGuildServerGroupChatting(EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND* lpMsg, int iServerIndex);
void GDUnionServerGroupChatting(EXSDHP_SERVERGROUP_UNION_CHATTING_SEND* lpMsg, int iServerIndex);
void GDGensServerGroupChatting(EXSDHP_SERVERGROUP_GENS_CHATTING_SEND* lpMsg, int iServerIndex);
void GDRewardGuildPeriodBuffInsert(PMSG_REQ_GUILD_PERIODBUFF_INSERT* lpMsg);
void GDRewardGuildPeriodBuffDelete(PMSG_REQ_GUILD_PERIODBUFF_DELETE* lpMsg);
void GDReqRegGensMember(_tagPMSG_REQ_REG_GENS_MEMBER_EXDB* lpMsg, int aIndex);
void GDReqSecedeGensMember(_tagPMSG_REQ_SECEDE_GENS_MEMBER_EXDB* lpMsg, int aIndex);
void GDReqGensInfo(_tagPMSG_REQ_GENS_INFO_EXDB* lpMsg, int aIndex);
void GDReqSaveContributePoint(_tagPMSG_REQ_SAVE_CONTRIBUTE_POINT_EXDB* lpMsg);
void GDReqSaveContributePoint(_tagPMSG_REQ_SAVE_ABUSING_KILLUSER_EXDB* lpMsg);
void GDReqAbusingInfo(_tagPMSG_REQ_ABUSING_INFO* lpMsg, int aIndex);
void GDReqGensSecedeDateModify(_tagPMSG_REQ_SECEDE_GENS_DATE_MODIFY* lpMsg);
void GDReqGensReward(_tagPMSG_REQ_GENS_REWARD_EXDB* lpMsg, int aIndex);
void GDReqGensRewardComplete(_tagPMSG_REQ_GENS_REWARD_COMPLETE_EXDB* lpMsg);
void GDReqGensMemberCount(_tagPMSG_REQ_GENS_MEMBER_COUNT_EXDB* lpMsg, int aIndex);
void GDReqGensSetRewardDay(_tagPMSG_REQ_SET_GENS_REWARD_DAY_EXDB* lpMsg);
void GDReqGensSetRanking(_tagPMSG_REQ_SET_GENS_RANKING_EXDB* lpMsg);
void GDReqGensRewardDayCheck(_tagPMSG_REQ_GENS_REWARD_DAY_CHECK_EXDB* lpMsg, int aIndex);
void GDReqGuildMatchingList(_stReqGuildMatchingList* lpMsg, int iServerIndex);
void GDReqGuildMatchingListSearchWord(_stReqGuildMatchingListSearchWord* lpMsg, int iServerIndex);
void GDInsertGuildMatchingList(_stReqGuildMatchingData* lpMsg, int iServerIndex);
void GDDeleteGuildMatching(_stReqDelGuildMatchingList* lpMsg, int iServerIndex);
void GDReqJoinGuildMatching(_stRegWaitGuildMatching* lpMsg, int iServerIndex);
void GDReqCancelJoinGuildMatching(_stReqDelWaitGuildMatchingList* lpMsg, int iServerIndex);
void GDReqWaitGuildMatchingList(_stReqWaitGuildMatchingList* lpMsg, int iServerIndex);
void GDReqGetWaitStateListGuildMatching(_stReqWaitStateListGuildMatching* lpMsg, int iServerIndex);
void GDReqAllowJoinGuildMatching(_stReqAllowJoinGuildMatching* lpMsg, int iServerIndex);
void DGAnsSendNotiGuildMatching(char* szName, int nUserIndex, int nServerIndex);
void DGAnsUseGuildMatchingGuild(int nGuildNumber);
void DGAnsSendNotiGuildMatchingForGuildMaster(int nGuildNumber);
void GDReqRegPartyMatchingList(_stReqRegWantedPartyMember* lpMsg, int iServerIndex);
void GDReqGetPartyMatchingList(_stReqGetPartyMatchingList* lpMsg, int iServerIndex);
void GDReqMemeberJoinPartyMatching(_stReqJoinMemberPartyMatching* lpMsg, int iServerIndex);
void GDReqMemberJoinStateList(_stReqJoinMemberStateListPartyMatching* lpMsg, int iServerIndex);
void GDReqMemberJoinStateListLeader(_stReqWaitListPartyMatching* lpMsg, int iServerIndex);
void GDReqCancelPartyMatching(_stReqCancePartyMatching* lpMsg, int iServerIndex);
void GDAutoMemberJoin(char* szLeaderName, char* szMemberName, int nMemberIndex, int iServerIndex, int nLevel, BYTE btClass);
void GDReqAddPartyMember(__stReqAddPartyMember* lpMsg, int iServerIndex);
void DGFixPartyMember(int nLeaderIndex, int nMemberIndex, int nServerIndex);
void DGSendPartyMember(char* szLeaderName, int bEmptyList);
void DGSendEmptyPartyMemberList(char* szMemberName);
void GDDeletePartyUser(_stReqDelPartyUserPartyMatching* lpMsg, int iServerIndex);
void GDReqSendPartyMember(__stReqSendPartyMemberList* lpMsg);
void AddFriendListForPartyMatching(int nNumber, char* szName, __int16 nServerindex);
void GServerDownDelParty(int nServer);
void GDSendChattingMsgPartyMatching(_stReqChattingPartyMatching* lpMsg);
void GDReqSwapPartyLeader(_stReqChangePartyLeader* lpMsg, int iServerIndex);
void GDReqSendPartyMemberList(__stReqSendPartyMemberList* lpMsg);
void DGClearPartyWaitListForOldLeader(char* szLeaderName);
void DGSendDelTypePartyMember(char* szLeaderName, char* szTargetName, BYTE btDelType);
