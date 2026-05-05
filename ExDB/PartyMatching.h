// PartyMatching.h

#pragma once
#include "Protocol.h"

// Party Matching Define
#define MAX_PMATCHING_TITLESTR		40
#define MAX_PMATCHING_PASSWORDSTR	4
#define MAX_PMATCHING_SEARCHWORDSTR	10
#define MAX_PMATCHING_CLASSDETAIL	7

/* 1176 */
struct _PARTY_MEMBER_INFO//40
{
    _PARTY_MEMBER_INFO()
    {
        this->bUse = 0;
        this->bLogOn = 0;
        this->nUserIndex = -1;
        this->nServerChannel = -1;
        this->nServerIndex = -1;
    }
    int  bUse;
    int  bLogOn;
    int  nUserIndex;
    int  nServerChannel;
    int  nServerIndex;
    char Name[11];
    int  nLevel;
    BYTE btClass;
};

/* 1178 */
struct _PARTY_INFO_STRUCT
{
    int bUse;
    int bUsePartyMatching;
    int nCount;
    int nPartyNumber;
    int nMinLevel;
    int nMaxLevel;
    _PARTY_MEMBER_INFO stPartyMember[5];
    _PARTY_INFO_STRUCT* back;
    _PARTY_INFO_STRUCT* next;
};

class CPartyMatching
{
public:

    CPartyMatching();
    virtual ~CPartyMatching();
    void InitPartyMatching();
    _PARTY_INFO_STRUCT* CreateParty(int nServerChannel,int nServerIndex,int nLeaderUserIndex,char* szLeaderName,int bUsePartyMatching,int nLevel,BYTE btClass);
    void DelParty(char* szLeaderName);
    int AddPartyMember(int nServerChannel,int nServerIndex,int nUserIndex,char* szName,char* szLeaderName,int nLevel,BYTE btClass);
    int DelPartyMember(char* szName,char* szLeaderName);
    _PARTY_INFO_STRUCT* SearchParty(char* szLeaderName);
    _PARTY_INFO_STRUCT* SearchParty(int nPartyIndex);
    _PARTY_INFO_STRUCT* SearchPartyMemberName(char* szName);
    void AddTail(_PARTY_INFO_STRUCT* lpNewNode);
    void AllDelete();
    int IsSameServerLeader(char* szName,char* szLeaderName);
    int ConnectMember(char* szMemberName,int nServerChannel,int nServerIndex,int nUserIndex);
    int DisConnectMember(char* szMemberName);
    _PARTY_INFO_STRUCT* SearchPartyForLeaderServerIndex(int nServerIndex);
    int ChangeLeader(char* szLeaderName, char* szNewLeaderName, int bIsSwap);
public:

    int m_nPartyCnt;
    int m_nPartyNumberIndex;

    _PARTY_INFO_STRUCT* head;
    _PARTY_INFO_STRUCT* tail;
    _PARTY_INFO_STRUCT* pNewNode;

    CRITICAL_SECTION m_partyMatching_cs;
};

extern CPartyMatching g_PartyMatching;