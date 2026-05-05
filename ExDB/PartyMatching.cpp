#include "stdafx.h"
#include "PartyMatching.h"
#include "GameServerMng.h"
#include "dbset/PartyMatchingDBSet.h"


CPartyMatching g_PartyMatching;

CPartyMatching::CPartyMatching()
{
    this->InitPartyMatching();
}

CPartyMatching::~CPartyMatching()
{
    this->AllDelete();
    DeleteCriticalSection(&this->m_partyMatching_cs);
}

void CPartyMatching::InitPartyMatching()
{
    this->head = NULL;
    this->tail = NULL;

    this->m_nPartyCnt = 0;
    this->m_nPartyNumberIndex = 0;

    InitializeCriticalSection(&this->m_partyMatching_cs);
}

_PARTY_INFO_STRUCT* CPartyMatching::CreateParty(int nServerChannel, int nServerIndex, int nLeaderUserIndex, char* szLeaderName, int bUsePartyMatching, int nLevel, BYTE btClass)
{
    HANDLE ProcessHeap;

    if (SearchParty(szLeaderName))
    {
	    return false;
    }

    EnterCriticalSection(&this->m_partyMatching_cs);
    ProcessHeap = GetProcessHeap();
    this->pNewNode = (_PARTY_INFO_STRUCT*)HeapAlloc(ProcessHeap, 8u, 0xE8u);
    if (this->pNewNode)
    {
        this->pNewNode->stPartyMember[0].bUse = 1;
        this->pNewNode->stPartyMember[0].bLogOn = 1;
        this->pNewNode->stPartyMember[0].nServerChannel = nServerChannel;
        this->pNewNode->stPartyMember[0].nUserIndex = nLeaderUserIndex;
        this->pNewNode->stPartyMember[0].nServerIndex = nServerIndex;
        this->pNewNode->bUsePartyMatching = bUsePartyMatching;
        memcpy(this->pNewNode->stPartyMember[0].Name, (unsigned __int8*)szLeaderName, 0xAu);
        this->pNewNode->stPartyMember[0].nLevel = nLevel;
        this->pNewNode->stPartyMember[0].btClass = btClass;
        this->pNewNode->next = 0;
        this->pNewNode->nCount = 1;
        AddTail(this->pNewNode);
        ++this->m_nPartyCnt;
        if (this->m_nPartyNumberIndex < 0)
        {
	        this->m_nPartyNumberIndex = 0;
        }
        this->pNewNode->nPartyNumber = this->m_nPartyNumberIndex++;
        cLog.AddTD("[PMATCHING][CreateParty]Leader:%s TotalPartyCnt:%d - Success",szLeaderName,this->m_nPartyCnt);
        LeaveCriticalSection(&this->m_partyMatching_cs);
        return this->pNewNode;
    }
    else
    {
        cLog.AddTD("[PMATCHING][CreateParty]Leader:%s - Fail", szLeaderName);
        LeaveCriticalSection(&this->m_partyMatching_cs);
        return 0;
    }
}

void CPartyMatching::DelParty(char* szLeaderName)
{
    HANDLE ProcessHeap; // eax
    _PARTY_INFO_STRUCT* v3; // [esp-4h] [ebp-118h]
    char szLogName[20]; // [esp+D0h] [ebp-44h] BYREF
    _PARTY_INFO_STRUCT* pNext; // [esp+E4h] [ebp-30h]
    _PARTY_INFO_STRUCT* pPrev; // [esp+F0h] [ebp-24h]
    _PARTY_INFO_STRUCT* pNode; // [esp+FCh] [ebp-18h]
    CPartyMatching* thisa; // [esp+108h] [ebp-Ch]

    memset(szLogName, 0, 11);
    memcpy((unsigned __int8*)szLogName, (unsigned __int8*)szLeaderName, 0xAu);
    pNode = SearchParty(szLeaderName);
    if (pNode)
    {
        EnterCriticalSection(&thisa->m_partyMatching_cs);
        pPrev = pNode->back;
        pNext = pNode->next;
        if (pPrev || pNext)
        {
            if (pPrev || !pNext)
            {
                if (!pPrev || pNext)
                {
                    if (pPrev && pNext)
                    {
                        pPrev->next = pNext;
                        pNext->back = pPrev;
                    }
                }
                else
                {
                    pPrev->next = 0;
                    thisa->tail = pPrev;
                }
            }
            else
            {
                pNext->back = 0;
                thisa->head = pNext;
            }
        }
        v3 = pNode;
        ProcessHeap = GetProcessHeap();
        HeapFree(ProcessHeap, 0, v3);
        if (!pPrev && !pNext)
        {
            thisa->head = 0;
            thisa->tail = 0;
        }
        cLog.AddTD("[PMATCHING][DelParty]Leader:%s, TotalPartyCnt:%d", szLogName, m_nPartyCnt);
        LeaveCriticalSection(&thisa->m_partyMatching_cs);
    }
}


int CPartyMatching::AddPartyMember(int nServerChannel,int nServerIndex,int nUserIndex,char* szName,char* szLeaderName,int nLevel,BYTE btClass)
{
    _PARTY_INFO_STRUCT* pNode;

    pNode = SearchParty(szLeaderName);
    if (!pNode)
        return -1;
    if (SearchPartyMemberName(szName))
        return -4;
    switch (gsm.m_Obj[nServerIndex].btGameServerType)
    {
    case 1:
        nServerChannel = 199;
        break;
    case 2:
        nServerChannel = 200;
        break;
    case 3:
        nServerChannel = 201;
        break;
    }
    EnterCriticalSection(&this->m_partyMatching_cs);
    for (int i = 0; i < 5; ++i)
    {
        if (!pNode->stPartyMember[i].bUse)
        {
            ++pNode->nCount;
            pNode->stPartyMember[i].bLogOn = 1;
            pNode->stPartyMember[i].bUse = 1;
            pNode->stPartyMember[i].nServerChannel = nServerChannel;
            pNode->stPartyMember[i].nServerIndex = nServerIndex;
            pNode->stPartyMember[i].nUserIndex = nUserIndex;
            memcpy((unsigned __int8*)pNode->stPartyMember[i].Name, (unsigned __int8*)szName, 0xAu);
            pNode->stPartyMember[i].nLevel = nLevel;
            pNode->stPartyMember[i].btClass = btClass;
            LeaveCriticalSection(&this->m_partyMatching_cs);
            cLog.AddTD("[PMATCHING][AddPartyMember]Leader:%s, Member:%s, MemberCnt:%d",szLeaderName,szName,pNode->nCount);
            return 0;
        }
    }
    LeaveCriticalSection(&this->m_partyMatching_cs);
    return -2;
}


int CPartyMatching::DelPartyMember(char* szName, char* szLeaderName)
{
    _PARTY_INFO_STRUCT* pNode = SearchParty(szLeaderName);
    if (!pNode)
        return -1;  // Party not found

    int nResult = -2;  // Member not found in party

    EnterCriticalSection(&m_partyMatching_cs);

    for (int i = 0; i < 5; ++i)
    {
        if (pNode->stPartyMember[i].bUse)
        {
            if (strcmp(pNode->stPartyMember[i].Name, szName) == 0)
            {
                --pNode->nCount;

                pNode->stPartyMember[i].bLogOn = 0;
                pNode->stPartyMember[i].bUse = 0;
                pNode->stPartyMember[i].nServerChannel = -1;
                pNode->stPartyMember[i].nServerIndex = -1;
                pNode->stPartyMember[i].nUserIndex = -1;
                memset(pNode->stPartyMember[i].Name, 0, MAX_IDSTRING);  // 10 bytes

                cLog.AddTD("[PMATCHING][DelPartyMember] Leader:%s, Member:%s, MemberCnt:%d",
                    szLeaderName, szName, pNode->nCount);

                nResult = 0;  // Success
            }
        }
    }

    LeaveCriticalSection(&m_partyMatching_cs);
    return nResult;
}


_PARTY_INFO_STRUCT* CPartyMatching::SearchParty(char* szLeaderName)
{
    EnterCriticalSection(&m_partyMatching_cs);

    for (_PARTY_INFO_STRUCT* pNode = this->head; pNode; pNode = pNode->next)
    {
        if (strcmp(pNode->stPartyMember[0].Name, szLeaderName) == 0)
        {
            LeaveCriticalSection(&m_partyMatching_cs);
            return pNode;
        }
    }

    LeaveCriticalSection(&m_partyMatching_cs);
    return NULL;
}


_PARTY_INFO_STRUCT* CPartyMatching::SearchParty(int nPartyIndex)
{
    EnterCriticalSection(&m_partyMatching_cs);

    for (_PARTY_INFO_STRUCT* pNode = this->head; pNode; pNode = pNode->next)
    {
        if (pNode->nPartyNumber == nPartyIndex)
        {
            LeaveCriticalSection(&m_partyMatching_cs);
            return pNode;
        }
    }

    LeaveCriticalSection(&m_partyMatching_cs);
    return NULL;
}


_PARTY_INFO_STRUCT* CPartyMatching::SearchPartyMemberName(char* szName)
{
    EnterCriticalSection(&m_partyMatching_cs);

    for (_PARTY_INFO_STRUCT* pNode = this->head; pNode; pNode = pNode->next)
    {
        for (int i = 0; i < 5; ++i)
        {
            if (strcmp(pNode->stPartyMember[i].Name, szName) == 0 && pNode->stPartyMember[i].bUse)
            {
                LeaveCriticalSection(&m_partyMatching_cs);
                return pNode;
            }
        }
    }

    LeaveCriticalSection(&m_partyMatching_cs);
    return NULL;
}


void CPartyMatching::AddTail(_PARTY_INFO_STRUCT* lpNewNode)
{
    EnterCriticalSection(&m_partyMatching_cs);

    lpNewNode->back = this->tail;
    lpNewNode->next = NULL;

    if (this->head)
        this->tail->next = lpNewNode;
    else
        this->head = lpNewNode;

    this->tail = lpNewNode;

    LeaveCriticalSection(&m_partyMatching_cs);
}


void CPartyMatching::AllDelete()
{
    EnterCriticalSection(&m_partyMatching_cs);

    _PARTY_INFO_STRUCT* pNode = head;
    while (pNode)
    {
        _PARTY_INFO_STRUCT* pTemp = pNode;
        pNode = pNode->next;
        HeapFree(GetProcessHeap(), 0, pTemp);
    }

    m_nPartyCnt = 0;

    LeaveCriticalSection(&m_partyMatching_cs);
}


int CPartyMatching::IsSameServerLeader(char* szName, char* szLeaderName)
{
    if (!szLeaderName || !szName)
        return 0;

    _PARTY_INFO_STRUCT* lpParty = SearchParty(szLeaderName);
    if (!lpParty)
        return 0;

    EnterCriticalSection(&m_partyMatching_cs);

    for (int i = 0; i < 5; ++i)
    {
        if (strcmp(lpParty->stPartyMember[i].Name, szName) == 0)
        {
            if (lpParty->stPartyMember[0].nServerChannel == lpParty->stPartyMember[i].nServerChannel)
            {
                LeaveCriticalSection(&m_partyMatching_cs);
                return 1;  // Same server
            }
        }
    }

    LeaveCriticalSection(&m_partyMatching_cs);
    return 0;  // Different server or not found
}


int CPartyMatching::ConnectMember(char* szMemberName,int nServerChannel,int nServerIndex,int nUserIndex)
{
    _PARTY_INFO_STRUCT* lpParty = SearchPartyMemberName(szMemberName);
    if (!lpParty)
        return -1;

    EnterCriticalSection(&m_partyMatching_cs);

    int i;
    for (i = 0; i < 5; ++i)
    {
        if (strcmp(lpParty->stPartyMember[i].Name, szMemberName) == 0)
            break;
    }

    if (i >= 5)
    {
        LeaveCriticalSection(&m_partyMatching_cs);
        return 0;  // Not found in party (shouldn't happen after SearchPartyMemberName)
    }

    // Handle special game server types for channel assignment
    switch (gsm.m_Obj[nServerIndex].btGameServerType)
    {
    case 1:
        nServerChannel = 199;
        break;
    case 2:
        nServerChannel = 200;
        break;
    case 3:
        nServerChannel = 201;
        break;
    }

    lpParty->stPartyMember[i].nServerChannel = nServerChannel;
    lpParty->stPartyMember[i].nServerIndex = nServerIndex;
    lpParty->stPartyMember[i].nUserIndex = nUserIndex;
    lpParty->stPartyMember[i].bLogOn = 1;

    cLog.AddTD("[PMATCHING][ConnectMember] LeaderName:%s, ConnectMemberName:%s",
        lpParty->stPartyMember[0].Name, szMemberName);

    LeaveCriticalSection(&m_partyMatching_cs);
    return 1;
}


int CPartyMatching::DisConnectMember(char* szMemberName)
{
    _PARTY_INFO_STRUCT* lpParty = SearchPartyMemberName(szMemberName);
    if (!lpParty)
        return -1;

    char szLeaderName[MAX_IDSTRING + 1];
    memset(szLeaderName, 0, MAX_IDSTRING + 1);
    memcpy(szLeaderName, lpParty->stPartyMember[0].Name, MAX_IDSTRING);

    int nReturn = 0;
    int bDestroyFlag = 1;

    EnterCriticalSection(&m_partyMatching_cs);

    // Find and disconnect the member
    for (int i = 0; i < 5; ++i)
    {
        if (strcmp(lpParty->stPartyMember[i].Name, szMemberName) == 0)
        {
            lpParty->stPartyMember[i].nServerChannel = -1;
            lpParty->stPartyMember[i].nServerIndex = -1;
            lpParty->stPartyMember[i].bLogOn = 0;

            cLog.AddTD("[PMATCHING][DisConnectMember] LeaderName:%s, ConnectMemberName:%s",szLeaderName,szMemberName);

            nReturn = 1;
        }

        // Check if any member is still online (used + logged on)
        if (lpParty->stPartyMember[i].bUse && lpParty->stPartyMember[i].bLogOn == 1)
            bDestroyFlag = 0;
    }

    // If only leader remains (or fewer), destroy the party
    if (lpParty->nCount <= 1)
        bDestroyFlag = 1;

    if (bDestroyFlag)
    {
        // Delete from database
        g_PartyMatching_DBSet.DeletePartyMatching(szLeaderName);

        // Get wait list and clear it
        _PARTY_MEMBER_WAIT_LIST stList[10];
        memset(stList, 0, sizeof(stList));
        int nListCount = g_PartyMatching_DBSet.GetPartyMatchingWaitListForLeader(szLeaderName, stList);

        // Remove party from memory
        DelParty(szLeaderName);

        // Clear all wait list entries
        if (nListCount > 0)
        {
            for (int j = 0; j < nListCount && j < 10; ++j)
            {
                g_PartyMatching_DBSet.DeleteWaitList(stList[j].szName);
            }
        }
    }

    LeaveCriticalSection(&m_partyMatching_cs);
    return nReturn;
}


_PARTY_INFO_STRUCT* CPartyMatching::SearchPartyForLeaderServerIndex(int nServerIndex)
{
    EnterCriticalSection(&m_partyMatching_cs);

    for (_PARTY_INFO_STRUCT* pNode = this->head; pNode; pNode = pNode->next)
    {
        if (pNode->stPartyMember[0].nServerIndex == nServerIndex)
        {
            LeaveCriticalSection(&m_partyMatching_cs);
            return pNode;
        }
    }

    LeaveCriticalSection(&m_partyMatching_cs);
    return NULL;
}

int CPartyMatching::ChangeLeader(char* szLeaderName,char* szNewLeaderName,int bIsSwap)
{
    _PARTY_INFO_STRUCT* lpParty = this->SearchParty(szLeaderName);

    if (lpParty == NULL)
    {
        return 0;
    }

    EnterCriticalSection(&this->m_partyMatching_cs);

    // validate current leader
    if (strcmp(lpParty->stPartyMember[0].Name, szLeaderName) == 0)
    {
        for (int i = 1; i < 5; i++)
        {
            if (bIsSwap)
            {
                if (strcmp(lpParty->stPartyMember[i].Name, szNewLeaderName) != 0)
                {
                    continue;
                }
            }

            if (lpParty->stPartyMember[i].bLogOn &&
                lpParty->stPartyMember[i].bUse)
            {
                _PARTY_MEMBER_INFO swapLeader;

                memset(&swapLeader, 0, sizeof(swapLeader));

                // backup new leader info
                swapLeader.bLogOn = lpParty->stPartyMember[i].bLogOn;
                swapLeader.bUse = lpParty->stPartyMember[i].bUse;
                swapLeader.nServerChannel = lpParty->stPartyMember[i].nServerChannel;
                swapLeader.nServerIndex = lpParty->stPartyMember[i].nServerIndex;
                swapLeader.nUserIndex = lpParty->stPartyMember[i].nUserIndex;
                swapLeader.nLevel = lpParty->stPartyMember[i].nLevel;
                swapLeader.btClass = lpParty->stPartyMember[i].btClass;

                memcpy(swapLeader.Name,lpParty->stPartyMember[i].Name,MAX_IDSTRING);

                // move current leader -> member slot
                lpParty->stPartyMember[i].bLogOn =lpParty->stPartyMember[0].bLogOn;
                lpParty->stPartyMember[i].bUse =lpParty->stPartyMember[0].bUse;
                lpParty->stPartyMember[i].nServerChannel =lpParty->stPartyMember[0].nServerChannel;
                lpParty->stPartyMember[i].nServerIndex =lpParty->stPartyMember[0].nServerIndex;
                lpParty->stPartyMember[i].nUserIndex =lpParty->stPartyMember[0].nUserIndex;
                lpParty->stPartyMember[i].nLevel =lpParty->stPartyMember[0].nLevel;
                lpParty->stPartyMember[i].btClass =lpParty->stPartyMember[0].btClass;
                memcpy(lpParty->stPartyMember[i].Name,lpParty->stPartyMember[0].Name,MAX_IDSTRING);

                // move selected member -> leader slot
                lpParty->stPartyMember[0].bUse = swapLeader.bUse;
                lpParty->stPartyMember[0].bLogOn = swapLeader.bLogOn;
                lpParty->stPartyMember[0].nServerChannel = swapLeader.nServerChannel;
                lpParty->stPartyMember[0].nServerIndex = swapLeader.nServerIndex;
                lpParty->stPartyMember[0].nUserIndex = swapLeader.nUserIndex;
                lpParty->stPartyMember[0].nLevel = swapLeader.nLevel;
                lpParty->stPartyMember[0].btClass = swapLeader.btClass;

                memcpy(lpParty->stPartyMember[0].Name,swapLeader.Name,MAX_IDSTRING);
                memcpy(szNewLeaderName,swapLeader.Name,MAX_IDSTRING);

                cLog.AddC(LOGC_BLUE,"[PMATCHING][ChangeLeader]Leader: %s -> %s",lpParty->stPartyMember[i].Name,swapLeader.Name);

                g_PartyMatching_DBSet.UpdatePartyMatchingList(szLeaderName,swapLeader.Name,swapLeader.nServerChannel,swapLeader.nLevel,swapLeader.btClass);

                // different server/channel
                if (lpParty->stPartyMember[i].nServerChannel !=
                    lpParty->stPartyMember[0].nServerChannel)
                {
                    DGFixPartyMember(lpParty->stPartyMember[0].nUserIndex,lpParty->stPartyMember[0].nUserIndex,lpParty->stPartyMember[0].nServerIndex);

                    for (int j = 1; j < 5; j++)
                    {
                        if (lpParty->stPartyMember[j].bLogOn &&
                            lpParty->stPartyMember[j].bUse &&
                            lpParty->stPartyMember[j].nServerIndex ==
                            swapLeader.nServerIndex)
                        {
                            DGFixPartyMember(lpParty->stPartyMember[0].nUserIndex,lpParty->stPartyMember[j].nUserIndex,lpParty->stPartyMember[0].nServerIndex);
                        }
                    }
                }

                LeaveCriticalSection(&this->m_partyMatching_cs);

                return 1;
            }
        }
    }

    LeaveCriticalSection(&this->m_partyMatching_cs);

    return 0;
}