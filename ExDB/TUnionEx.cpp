#include "stdafx.h"
// TUnionEx.cpp
#include "TUnionEx.h"
#include "GuildClass.h"
#include "./dbset/GuildDBSet.h"
#include "WZIocpEngine.h"
//#include "Logging.h"       // For TRACE_OUT
#include "Protocol.h"      // For DGRelationShip* functions

// Define global instance
TUnionEx UnionExManager;
//extern CLogProc cLog;

TUnionEx::TUnionEx()
{
    // m_mpUnion and m_vtUnion are default-constructed
    // m_Sync is default-constructed
}

TUnionEx::~TUnionEx()
{
    DelAllUnion();
    // m_Sync, m_vtUnion, m_mpUnion destructors called automatically
}

int TUnionEx::SetGuildClass(CGuildClass* pGuildClass)
{
    if (!pGuildClass)
        return -1;
    m_pGuildClass = pGuildClass;
    return 1;
}

int TUnionEx::SetGuildDBSet(CGuildDBSet* pGuildDBSet)
{
    if (!pGuildDBSet)
        return -1;
    m_pGuildDBSet = pGuildDBSet;
    return 1;
}

CGuildClass* TUnionEx::GetGuildClass()
{
    return m_pGuildClass;
}

CGuildDBSet* TUnionEx::GetGuildDBSet()
{
    return m_pGuildDBSet;
}

CWZIocpEngine* TUnionEx::GetWzIoEngine()
{
    return m_pIoEngine;
}

TUnionExInfo* TUnionEx::SearchUnion(int iMasterGuildNumber)
{
    m_Sync.Lock();

    std::map<int, TUnionExInfo*>::iterator it = this->m_mpUnion.find(iMasterGuildNumber);
    TUnionExInfo* pInfo = 0;

    if (it != this->m_mpUnion.end())
    {
        pInfo = it->second;
    }

    m_Sync.Unlock();

    return pInfo;
}

int TUnionEx::AddUnion(int iMasterGuildNumber, _GUILD_INFO_STRUCT* lpGuildInfo, int bWriteDB)
{
    if (SearchUnion(iMasterGuildNumber))
        return -1;

    TUnionExInfo* pInfo = new TUnionExInfo();
    if (!pInfo)
        return -1;

    if (pInfo->SetUnionMaster(iMasterGuildNumber, lpGuildInfo) != 1)
    {
        delete pInfo;
        return -1;
    }

    m_Sync.Lock();
    m_mpUnion[iMasterGuildNumber] = pInfo;
    if (bWriteDB && m_pGuildDBSet)
    {
        m_pGuildDBSet->UpdateGuildUnion(iMasterGuildNumber, iMasterGuildNumber);
    }
    m_Sync.Unlock();
    return 1;
}

int TUnionEx::AddUnionMemberInfo(int iMasterGuildNumber, _GUILD_INFO_STRUCT* lpGuildInfo, int bWriteDB, int bInitRival)
{
    TUnionExInfo* pUnionExInfo = SearchUnion(iMasterGuildNumber);
    if (!pUnionExInfo)
        return -1;

    if (pUnionExInfo->AddUnionMemberInfo(lpGuildInfo) != 1)
        return -1;

    if (bWriteDB && m_pGuildDBSet)
        m_pGuildDBSet->UpdateGuildUnion(lpGuildInfo->Number, iMasterGuildNumber);

    if (bInitRival)
        InitRivalMemberInfo(iMasterGuildNumber);

    return 1;
}

int TUnionEx::AddUnionMemberInfo(int iMasterGuildNumber, int iGuildNumber, int bWriteDB, int bInitRival)
{
    if (!m_pGuildClass)
        return -1;

    _GUILD_INFO_STRUCT* lpGuildInfo = m_pGuildClass->SearchGuild_Number(iGuildNumber);
    if (lpGuildInfo)
        return AddUnionMemberInfo(iMasterGuildNumber, lpGuildInfo, bWriteDB, bInitRival);
    else
        return -1;
}

int TUnionEx::DelUnion(int iMasterGuildNumber)
{
    TUnionExInfo* pUnionExInfo = SearchUnion(iMasterGuildNumber);
    if (!pUnionExInfo)
        return -1;

    m_Sync.Lock();
    m_mpUnion.erase(iMasterGuildNumber);
    delete pUnionExInfo;

    if (m_pGuildDBSet)
        m_pGuildDBSet->UpdateGuildUnion(iMasterGuildNumber, 0);
    m_Sync.Unlock();
    return 1;
}

int TUnionEx::DelUnionMemberInfo(int iMasterGuildNumber, int iGuildNumber, int bWriteDB, int bInitRival)
{
    TUnionExInfo* pUnionExInfo = SearchUnion(iMasterGuildNumber);
    if (!pUnionExInfo)
        return -1;

    if (pUnionExInfo->DelUnionMemberInfo(iGuildNumber) != 1)
        return -1;

    if (bWriteDB && m_pGuildDBSet)
        m_pGuildDBSet->UpdateGuildUnion(iGuildNumber, 0);

    if (bInitRival)
        InitRivalMemberInfo(iMasterGuildNumber);

    return 1;
}

int TUnionEx::DelAllUnion()
{
    std::map<int, TUnionExInfo*>::iterator it;

    for (it = m_mpUnion.begin(); it != m_mpUnion.end(); ++it)
    {
        TUnionExInfo* pInfo = it->second;

        if (pInfo)
        {
            delete pInfo;
        }
    }

    m_mpUnion.clear();

    return -1;
}

int TUnionEx::GetGuildRelationShip(int iMasterGuildNumber, int iTargetGuildNumber)
{
    TUnionExInfo* pUnionExInfo = SearchUnion(iMasterGuildNumber);
    if (!pUnionExInfo)
        return 0;

    if (pUnionExInfo->SearchUnionMember(iTargetGuildNumber))
        return 1;

    if (pUnionExInfo->SearchRivalMember(iTargetGuildNumber))
        return 2;

    return 0;
}

int TUnionEx::Init()
{
    if (this->m_pGuildClass == NULL || this->m_pGuildDBSet == NULL)
    {
        return -1;
    }

    std::map<int, _GUILD_INFO_STRUCT*>::iterator it;
    std::map<int, _GUILD_INFO_STRUCT*>::iterator end;

    //
    // Create all unions
    //
    it = this->m_pGuildClass->m_GuildNumberMap.begin();
    end = this->m_pGuildClass->m_GuildNumberMap.end();

    while (it != end)
    {
        _GUILD_INFO_STRUCT* lpGuild = it->second;

        if (lpGuild != NULL)
        {
            if (this->AddUnion(lpGuild->Number, lpGuild, 0) == 1)
            {
                TRACE_OUT(
                    3,
                    "Union [ %d ] : [ %s ] [ %d ] ",
                    lpGuild->iGuildUnion,
                    lpGuild->Name,
                    lpGuild->Number);
            }
            else
            {
                TRACE_OUT(
                    2,
                    "Union [ %d ] : [ %s ] [ %d ] ",
                    lpGuild->iGuildUnion,
                    lpGuild->Name,
                    lpGuild->Number);
            }
        }

        ++it;
    }

    //
    // Add union members
    //
    it = this->m_pGuildClass->m_GuildNumberMap.begin();

    while (it != end)
    {
        _GUILD_INFO_STRUCT* lpGuildInfo = it->second;

        if (lpGuildInfo != NULL)
        {
            if (lpGuildInfo->iGuildUnion != 0 &&
                lpGuildInfo->Number != lpGuildInfo->iGuildUnion)
            {
                if (this->AddUnionMemberInfo(
                    lpGuildInfo->iGuildUnion,
                    lpGuildInfo,
                    0,
                    1) == 1)
                {
                    TRACE_OUT(
                        3,
                        "Offical Union Member [ %s ] [ %d ] ",
                        lpGuildInfo->Name,
                        lpGuildInfo->Number);
                }
                else
                {
                    TRACE_OUT(
                        2,
                        "Offical Union Member [ %s ] [ %d ] ",
                        lpGuildInfo->Name,
                        lpGuildInfo->Number);
                }
            }
        }

        ++it;
    }

    this->InitAllRivalMemberInfo();

    return 1;
}

//int TUnionEx::Init()
//{
//    if (!m_pGuildClass || !m_pGuildDBSet)
//        return -1;
//
//    //
//    // Create all union containers
//    //
//    for (auto it = m_pGuildClass->m_GuildNumberMap.begin();
//        it != m_pGuildClass->m_GuildNumberMap.end();
//        ++it)
//    {
//        _GUILD_INFO_STRUCT* lpGuild = it->second;
//
//        if (!lpGuild)
//            continue;
//
//        if (AddUnion(lpGuild->Number, lpGuild, 0) != 1)
//        {
//            TRACE_OUT(3,"Union Create Failed [%d][%s][%d]",lpGuild->iGuildUnion,lpGuild->Name,lpGuild->Number);
//        }
//        else
//        {
//            TRACE_OUT(2,"Union Create Success [%d][%s][%d]",lpGuild->iGuildUnion,lpGuild->Name,lpGuild->Number);
//        }
//    }
//
//    //
//    // Add guilds into their unions
//    //
//    for (auto it = m_pGuildClass->m_GuildNumberMap.begin();
//        it != m_pGuildClass->m_GuildNumberMap.end();
//        ++it)
//    {
//        _GUILD_INFO_STRUCT* lpGuildInfo = it->second;
//
//        if (!lpGuildInfo)
//            continue;
//
//        if (lpGuildInfo->iGuildUnion &&
//            lpGuildInfo->Number != lpGuildInfo->iGuildUnion)
//        {
//            if (AddUnionMemberInfo(lpGuildInfo->iGuildUnion,lpGuildInfo,0,1) != 1)
//            {
//                TRACE_OUT(3,"Official Union Member Add Failed [%s][%d]",lpGuildInfo->Name,lpGuildInfo->Number
//                );
//            }
//            else
//            {
//                TRACE_OUT(2,"Official Union Member Add Success [%s][%d]",lpGuildInfo->Name,lpGuildInfo->Number
//                );
//            }
//        }
//    }
//
//    InitAllRivalMemberInfo();
//
//    return 1;
//}

int TUnionEx::InitAllRivalMemberInfo()
{
    std::map<int, TUnionExInfo*>::iterator it;
    std::map<int, TUnionExInfo*>::iterator end;

    it = this->m_mpUnion.begin();
    end = this->m_mpUnion.end();

    while (it != end)
    {
        TUnionExInfo* pUnionExInfo = it->second;

        if (pUnionExInfo != NULL)
        {
            if (this->InitRivalMemberInfo(pUnionExInfo->m_iMasterGuild) != 1)
            {
                TRACE_OUT(1,"Rival ",pUnionExInfo->m_szMasterGuild,pUnionExInfo->m_iMasterGuild);
            }
        }
        else
        {
            TRACE_OUT(2,"@ InitAllRivalMemberInfo() : pUnionExInfo = NULL");
        }

        ++it;
    }

    return 1;
}

//int TUnionEx::InitAllRivalMemberInfo()
//{
//    for (auto it = m_mpUnion.begin();
//        it != m_mpUnion.end();
//        ++it)
//    {
//        TUnionExInfo* pUnionExInfo = it->second;
//
//        if (!pUnionExInfo)
//        {
//            TRACE_OUT(2,"@ InitAllRivalMemberInfo() : pUnionExInfo = NULL");
//
//            continue;
//        }
//
//        if (InitRivalMemberInfo(
//            pUnionExInfo->m_iMasterGuild) != 1)
//        {
//            TRACE_OUT(1,"Rival Init Failed [%s][%d]",pUnionExInfo->m_szMasterGuild,pUnionExInfo->m_iMasterGuild);
//        }
//    }
//
//    return 1;
//}

int TUnionEx::InitRivalMemberInfo(int iMasterGuildNumber)
{
    TUnionExInfo* pUnionExInfo = this->SearchUnion(iMasterGuildNumber);

    if (pUnionExInfo == NULL)
    {
        return -1;
    }

    m_Sync.Lock();

    pUnionExInfo->m_mpRivalMember.clear();

    std::map<int, _GUILD_INFO_STRUCT*>::iterator it;
    std::map<int, _GUILD_INFO_STRUCT*>::iterator end;

    it = pUnionExInfo->m_mpUnionMember.begin();
    end = pUnionExInfo->m_mpUnionMember.end();

    while (it != end)
    {
        _GUILD_INFO_STRUCT* lpGuildInfo = it->second;

        if (lpGuildInfo != NULL)
        {
            _GUILD_INFO_STRUCT* lpRivalGuildInfo =
                this->m_pGuildClass->SearchGuild_Number(lpGuildInfo->iGuildRival);

            if (lpRivalGuildInfo != NULL)
            {
                TUnionExInfo* pRivalUnionExInfo = NULL;

                if (lpRivalGuildInfo->iGuildUnion)
                {
                    pRivalUnionExInfo =
                        this->SearchUnion(lpRivalGuildInfo->iGuildUnion);
                }
                else
                {
                    pRivalUnionExInfo =
                        this->SearchUnion(lpRivalGuildInfo->Number);
                }

                if (pRivalUnionExInfo != NULL)
                {
                    std::map<int, _GUILD_INFO_STRUCT*>::iterator rit;
                    std::map<int, _GUILD_INFO_STRUCT*>::iterator rend;

                    rit = pRivalUnionExInfo->m_mpUnionMember.begin();
                    rend = pRivalUnionExInfo->m_mpUnionMember.end();

                    while (rit != rend)
                    {
                        _GUILD_INFO_STRUCT* lpGuildGuildInfo = rit->second;

                        if (lpGuildGuildInfo != NULL)
                        {
                            pUnionExInfo->AddRivalMemberInfo(lpGuildGuildInfo);
                        }

                        ++rit;
                    }
                }
            }
        }

        ++it;
    }

    m_Sync.Unlock();

    return 1;
}

//int TUnionEx::InitRivalMemberInfo(int iMasterGuildNumber)
//{
//    TUnionExInfo* pUnionExInfo = this->SearchUnion(iMasterGuildNumber);
//
//    if (!pUnionExInfo)
//        return -1;
//
//    m_Sync.Lock();
//
//    // Clear old rival cache
//    pUnionExInfo->m_mpRivalMember.clear();
//
//    // Iterate all guilds in this union
//    for (auto it = pUnionExInfo->m_mpUnionMember.begin();
//         it != pUnionExInfo->m_mpUnionMember.end();
//         ++it)
//    {
//        _GUILD_INFO_STRUCT* lpGuildInfo = it->second;
//
//        if (!lpGuildInfo)
//            continue;
//
//        // Find rival guild
//        _GUILD_INFO_STRUCT* lpRivalGuildInfo = this->m_pGuildClass->SearchGuild_Number(lpGuildInfo->iGuildRival);
//
//        if (!lpRivalGuildInfo)
//            continue;
//
//        TUnionExInfo* pRivalUnionExInfo = 0;
//
//        // Rival belongs to a union
//        if (lpRivalGuildInfo->iGuildUnion)
//        {
//            pRivalUnionExInfo = this->SearchUnion(lpRivalGuildInfo->iGuildUnion);
//        }
//        else
//        {
//            // Rival is standalone
//            pRivalUnionExInfo = this->SearchUnion(lpRivalGuildInfo->Number);
//        }
//
//        if (!pRivalUnionExInfo)
//            continue;
//
//        // Add every guild from rival union
//        for (auto rit = pRivalUnionExInfo->m_mpUnionMember.begin();
//             rit != pRivalUnionExInfo->m_mpUnionMember.end();
//             ++rit)
//        {
//            _GUILD_INFO_STRUCT* lpGuildGuildInfo = rit->second;
//
//            if (lpGuildGuildInfo)
//            {
//                pUnionExInfo->AddRivalMemberInfo(lpGuildGuildInfo);
//            }
//        }
//    }
//
//    m_Sync.Unlock();
//    return 1;
//}

int TUnionEx::NotifyInitRelationShip(_GUILD_INFO_STRUCT* lpGuildInfo, int iRelationShipType)
{
    if (!lpGuildInfo)
    {
        TRACE_OUT(3, "NotifyInitRelationShip: lpGuildInfo is NULL");
        return -1;
    }

    if (iRelationShipType == 1)
        return NotifyInitRelationUnion(lpGuildInfo);
    if (iRelationShipType == 2)
        return NotifyInitRelationRival(lpGuildInfo);
    if (iRelationShipType)
        return 1;

    if (NotifyInitRelationUnion(lpGuildInfo) == -1)
        return NotifyInitRelationRival(lpGuildInfo);

    return -1;
}

int TUnionEx::NotifyInitRelationUnion(_GUILD_INFO_STRUCT* lpGuildInfo)
{
    if (lpGuildInfo == NULL)
    {
        TRACE_OUT(3, "@ NotifyInitRelationUnion() : ");
        return -1;
    }

    if (lpGuildInfo->iGuildUnion == 0)
    {
        TRACE_OUT(3, "@ NotifyInitRelationUnion() : ");
        return -1;
    }

    //
    // Master guild
    //
    if (lpGuildInfo->Number == lpGuildInfo->iGuildUnion)
    {
        TUnionExInfo* pUnionExInfo =
            this->SearchUnion(lpGuildInfo->Number);

        if (pUnionExInfo == NULL)
        {
            return -1;
        }

        if (pUnionExInfo->m_mpUnionMember.size() > 1)
        {
            return -1;
        }

		m_Sync.Lock();

        std::map<int, _GUILD_INFO_STRUCT*>::iterator it;
        std::map<int, _GUILD_INFO_STRUCT*>::iterator end;

        it = pUnionExInfo->m_mpUnionMember.begin();
        end = pUnionExInfo->m_mpUnionMember.end();

        while (it != end)
        {
            _GUILD_INFO_STRUCT* lpUnionMember = it->second;

            if (lpUnionMember != NULL)
            {
                if (lpUnionMember != lpGuildInfo)
                {
                    this->SetRelationUnion(
                        lpUnionMember->Number,
                        lpUnionMember->iGuildUnion,
                        2);

                    DGRelationShipBreakOff(
                        lpUnionMember->Number,
                        lpUnionMember->iGuildUnion,
                        1,
                        0);
                }
            }

            ++it;
        }

        m_Sync.Unlock();

        if (lpGuildInfo->iGuildUnion)
        {
            TRACE_OUT(3,"@ NotifyInitRelationUnion() : ",lpGuildInfo->iGuildUnion);

            return -1;
        }

        TRACE_OUT(2, "@ NotifyInitRelationUnion() : ",lpGuildInfo->iGuildUnion);
    }
    else
    {
        this->SetRelationUnion(lpGuildInfo->Number,lpGuildInfo->iGuildUnion,2);
        DGRelationShipBreakOff(lpGuildInfo->Number,lpGuildInfo->iGuildUnion,1,0);
    }

    return 1;
}

//int TUnionEx::NotifyInitRelationUnion(_GUILD_INFO_STRUCT* lpGuildInfo)
//{
//    if (!lpGuildInfo)
//    {
//        TRACE_OUT(3, "lpGuildInfo == NULL");
//        return -1;
//    }
//
//    if (!lpGuildInfo->iGuildUnion)
//    {
//        TRACE_OUT(3, "Guild has no union");
//        return -1;
//    }
//
//    // Guild is the master guild of the union
//    if (lpGuildInfo->Number == lpGuildInfo->iGuildUnion)
//    {
//        TUnionExInfo* pUnionExInfo = this->SearchUnion(lpGuildInfo->Number);
//
//        // Already has members -> invalid state
//        if (pUnionExInfo->m_mpUnionMember.size() > 1)
//            return -1;
//
//        if (pUnionExInfo)
//        {
//            m_Sync.Lock();
//
//            for (auto it = pUnionExInfo->m_mpUnionMember.begin();
//                it != pUnionExInfo->m_mpUnionMember.end();
//                ++it)
//            {
//                _GUILD_INFO_STRUCT* lpUnionMember = it->second;
//
//                if (!lpUnionMember)
//                    continue;
//
//                if (lpUnionMember == lpGuildInfo)
//                    continue;
//
//                // Break union relation
//                this->SetRelationUnion(lpUnionMember->Number,lpUnionMember->iGuildUnion,2);
//                DGRelationShipBreakOff(lpUnionMember->Number,lpUnionMember->iGuildUnion,1,0);
//            }
//
//            m_Sync.Unlock();
//
//            // Still in union after break?
//            if (lpGuildInfo->iGuildUnion)
//            {
//                TRACE_OUT(3,"Failed to break union %d",lpGuildInfo->iGuildUnion);
//
//                return -1;
//            }
//
//            TRACE_OUT(2,"Union reset success %d",lpGuildInfo->iGuildUnion);
//        }
//    }
//    else
//    {
//        // Non-master guild
//        this->SetRelationUnion(lpGuildInfo->Number,lpGuildInfo->iGuildUnion,2);
//        DGRelationShipBreakOff(lpGuildInfo->Number,lpGuildInfo->iGuildUnion,1,0);
//    }
//
//    return 1;
//}

int TUnionEx::NotifyInitRelationRival(_GUILD_INFO_STRUCT* lpGuildInfo)
{
    if (!lpGuildInfo)
    {
        TRACE_OUT(3, "NotifyInitRelationRival: lpGuildInfo is NULL");
        return -1;
    }

    if (lpGuildInfo->iGuildRival)
    {
        SetRelationRival(lpGuildInfo->Number, lpGuildInfo->iGuildRival, 2);
        DGRelationShipBreakOff(lpGuildInfo->Number, lpGuildInfo->iGuildUnion, 2, 0);
        return 1;
    }
    else
    {
        TRACE_OUT(3, "NotifyInitRelationRival: No Rival");
        return -1;
    }
}

void TUnionEx::NotifyList(_GUILD_INFO_STRUCT* lpGuildInfo,int iRelationShipType)
{
    int iGuildList[302];
    int iGuildListCount = 0;

    memset(iGuildList, 0, sizeof(iGuildList));

    if (lpGuildInfo == NULL)
    {
        TRACE_OUT(3, "RelationShip List");
        return;
    }

    int iGuildUnion = 0;

    if (lpGuildInfo->iGuildUnion)
    {
        iGuildUnion = lpGuildInfo->iGuildUnion;
    }
    else
    {
        iGuildUnion = lpGuildInfo->Number;
    }

    TUnionExInfo* pUnionExInfo = SearchUnion(iGuildUnion);

    if (pUnionExInfo == NULL)
    {
        return;
    }

    this->m_Sync.Lock();

    switch (iRelationShipType)
    {
        // Union members only
    case 1:
    {
        std::map<int, _GUILD_INFO_STRUCT*>::iterator it;

        for (it = pUnionExInfo->m_mpUnionMember.begin();
            it != pUnionExInfo->m_mpUnionMember.end();
            ++it)
        {
            _GUILD_INFO_STRUCT* lpUnionGuild = it->second;

            if (lpUnionGuild)
            {
                iGuildList[iGuildListCount++] =
                    lpUnionGuild->Number;
            }
        }
    }
    break;

    // Rival members only
    case 2:
    {
        std::map<int, _GUILD_INFO_STRUCT*>::iterator it;

        for (it = pUnionExInfo->m_mpRivalMember.begin();
            it != pUnionExInfo->m_mpRivalMember.end();
            ++it)
        {
            _GUILD_INFO_STRUCT* lpRivalGuild = it->second;

            if (lpRivalGuild)
            {
                iGuildList[iGuildListCount++] =
                    lpRivalGuild->Number;
            }
        }
    }
    break;

    // Both union + rival
    case 0:
    {
        std::map<int, _GUILD_INFO_STRUCT*>::iterator it;

        for (it = pUnionExInfo->m_mpUnionMember.begin();
            it != pUnionExInfo->m_mpUnionMember.end();
            ++it)
        {
            _GUILD_INFO_STRUCT* lpUnionGuild = it->second;

            if (lpUnionGuild)
            {
                iGuildList[iGuildListCount++] =
                    lpUnionGuild->Number;
            }
        }

        for (it = pUnionExInfo->m_mpRivalMember.begin();
            it != pUnionExInfo->m_mpRivalMember.end();
            ++it)
        {
            _GUILD_INFO_STRUCT* lpRivalGuild = it->second;

            if (lpRivalGuild)
            {
                iGuildList[iGuildListCount++] =
                    lpRivalGuild->Number;
            }
        }
    }
    break;
    }

    this->m_Sync.Unlock();

    DGRelationShipNotificationSend(1,iGuildListCount,iGuildList,0);
}

//std::vector<TUnionExInfo*> TUnionEx::ReportUnionInfoList()
//{
//    std::vector<TUnionExInfo*> vtUnionExInfoList;
//
//    for (auto it = this->m_mpUnion.begin();
//        it != this->m_mpUnion.end();
//        ++it)
//    {
//        vtUnionExInfoList.push_back(it->second);
//    }
//
//    return vtUnionExInfoList;
//}

std::vector<TUnionExInfo*> TUnionEx::ReportUnionInfoList()
{
    std::vector<TUnionExInfo*> vtUnionExInfoList;

    std::map<int, TUnionExInfo*>::iterator it;
    std::map<int, TUnionExInfo*>::iterator end;

    it = this->m_mpUnion.begin();
    end = this->m_mpUnion.end();

    while (it != end)
    {
        vtUnionExInfoList.push_back(it->second);
        ++it;
    }

    return vtUnionExInfoList;
}

//std::vector<TUnionExInfo*>* TUnionEx::ReportUnionInfoList(std::vector<TUnionExInfo*>* result)
//{
//	std::vector<TUnionExInfo*> vtUnionExInfoList;
//
//	for (auto it = this->m_mpUnion.begin();
//		it != this->m_mpUnion.end();
//		++it)
//	{
//		vtUnionExInfoList.push_back(it->second);
//	}
//
//	*result = vtUnionExInfoList;
//
//	return result;
//}

int TUnionEx::SetRelationRival(int iReqGuildNumber, int iTargetGuildNumber, int iRelationOperation)
{
    if (!m_pGuildClass || !m_pGuildDBSet)
        return -1;

    _GUILD_INFO_STRUCT* lpReqGuild = m_pGuildClass->SearchGuild_Number(iReqGuildNumber);
    _GUILD_INFO_STRUCT* lpTargetGuild = m_pGuildClass->SearchGuild_Number(iTargetGuildNumber);

    if (!lpReqGuild || !lpTargetGuild)
        return -1;

    m_Sync.Lock();
    if (iRelationOperation == 1) // Set rival
    {
        if (lpReqGuild->iGuildRival || lpTargetGuild->iGuildRival)
        {
            m_Sync.Unlock();
            return -1;
        }
        lpReqGuild->iGuildRival = lpTargetGuild->Number;
        lpTargetGuild->iGuildRival = lpReqGuild->Number;

        InitRivalMemberInfo(lpReqGuild->iGuildUnion ? lpReqGuild->iGuildUnion : lpReqGuild->Number);
        InitRivalMemberInfo(lpTargetGuild->iGuildUnion ? lpTargetGuild->iGuildUnion : lpTargetGuild->Number);

        m_pGuildDBSet->UpdateGuildRival(iReqGuildNumber, iTargetGuildNumber);
        m_pGuildDBSet->UpdateGuildRival(iTargetGuildNumber, iReqGuildNumber);

        DGRelationShipListSend(lpReqGuild, 2, 0, -1);
        DGRelationShipListSend(lpTargetGuild, 2, 0, -1);
        NotifyList(lpReqGuild, 0);
        NotifyList(lpTargetGuild, 0);
    }
    else if (iRelationOperation == 2) // Remove rival
    {
        lpReqGuild->iGuildRival = 0;
        lpTargetGuild->iGuildRival = 0;

        InitRivalMemberInfo(lpReqGuild->iGuildUnion ? lpReqGuild->iGuildUnion : lpReqGuild->Number);
        InitRivalMemberInfo(lpTargetGuild->iGuildUnion ? lpTargetGuild->iGuildUnion : lpTargetGuild->Number);

        m_pGuildDBSet->UpdateGuildRival(iReqGuildNumber, 0);
        m_pGuildDBSet->UpdateGuildRival(iTargetGuildNumber, 0);

        DGRelationShipListSend(lpReqGuild, 2, 0, -1);
        DGRelationShipListSend(lpTargetGuild, 2, 0, -1);
        NotifyList(lpReqGuild, 0);
        NotifyList(lpTargetGuild, 0);
    }
    m_Sync.Unlock();
    return 1;
}

int TUnionEx::SetRelationUnion(int iReqGuildNumber,int iTargetGuildNumber,int iRelationOperation)
{
    if (this->m_pGuildClass == NULL || this->m_pGuildDBSet == NULL)
    {
        return -1;
    }

    if (iReqGuildNumber == iTargetGuildNumber)
    {
        return -1;
    }

    _GUILD_INFO_STRUCT* lpReqGuild =this->m_pGuildClass->SearchGuild_Number(iReqGuildNumber);

    _GUILD_INFO_STRUCT* lpTargetGuild =this->m_pGuildClass->SearchGuild_Number(iTargetGuildNumber);

    //
    // UNION JOIN
    //
    if (iRelationOperation == 1)
    {
        if (lpReqGuild == NULL || lpTargetGuild == NULL)
        {
            return -1;
        }

        if (lpTargetGuild->iGuildUnion &&
            lpTargetGuild->Number != lpTargetGuild->iGuildUnion)
        {
            return -1;
        }

        if (this->AddUnionMemberInfo(iTargetGuildNumber,lpReqGuild->Number,1,1) == -1)
        {
            return -1;
        }

        if (lpTargetGuild->iGuildUnion == 0)
        {
            lpTargetGuild->iGuildUnion = iTargetGuildNumber;
            this->m_pGuildDBSet->UpdateGuildUnion(iTargetGuildNumber,iTargetGuildNumber);
        }

        DGRelationShipListSend(lpTargetGuild, 1, 0, -1);
        DGRelationShipListSend(lpTargetGuild, 2, 0, -1);

        this->NotifyList(lpTargetGuild, 0);

        m_Sync.Lock();

        TUnionExInfo* pUnionExInfo =
            this->SearchUnion(lpTargetGuild->iGuildUnion);

        if (pUnionExInfo)
        {
            std::map<int, _GUILD_INFO_STRUCT*>::iterator it;
            std::map<int, _GUILD_INFO_STRUCT*>::iterator end;

            it = pUnionExInfo->m_mpUnionMember.begin();
            end = pUnionExInfo->m_mpUnionMember.end();

            for (; it != end; ++it)
            {
                _GUILD_INFO_STRUCT* lpUnionGuild = it->second;

                if (lpUnionGuild && lpUnionGuild->iGuildRival)
                {
                    _GUILD_INFO_STRUCT* lpRivalGuild =this->m_pGuildClass->SearchGuild_Number(lpUnionGuild->iGuildRival);

                    if (lpRivalGuild)
                    {
                        if (lpRivalGuild->iGuildUnion)
                        {
                            this->InitRivalMemberInfo(lpRivalGuild->iGuildUnion);
                        }
                        else
                        {
                            this->InitRivalMemberInfo(lpRivalGuild->Number);
                        }

                        DGRelationShipListSend(lpRivalGuild, 2, 0, -1);
                        this->NotifyList(lpRivalGuild, 1);
                    }
                }
            }
        }

        m_Sync.Unlock();
    }
    //
    // UNION BREAK
    //
    else if (iRelationOperation == 2)
    {
        if (lpReqGuild == NULL || lpTargetGuild == NULL)
        {
            return -1;
        }

        TUnionExInfo* pUnionExInfo =this->SearchUnion(iTargetGuildNumber);

        if (pUnionExInfo == NULL)
        {
            return -1;
        }

        if (this->DelUnionMemberInfo(iTargetGuildNumber,lpReqGuild->Number,1,1) == -1)
        {
            return -1;
        }

        if (!pUnionExInfo->IsValidUnion())
        {
            this->m_pGuildDBSet->UpdateGuildUnion(iTargetGuildNumber,0);
            DGRelationShipNotificationSend(0x10,1,&lpTargetGuild->iGuildUnion,0);
            lpTargetGuild->iGuildUnion = 0;
        }

        DGRelationShipListSend(lpTargetGuild, 1, 0, -1);
        DGRelationShipListSend(lpTargetGuild, 2, 0, -1);

        this->NotifyList(lpTargetGuild, 0);

        DGRelationShipListSend(lpReqGuild, 1, 0, -1);
        DGRelationShipListSend(lpReqGuild, 2, 0, -1);

        this->NotifyList(lpReqGuild, 0);

        m_Sync.Lock();

        std::map<int, _GUILD_INFO_STRUCT*>::iterator it;
        std::map<int, _GUILD_INFO_STRUCT*>::iterator end;

        it = pUnionExInfo->m_mpUnionMember.begin();
        end = pUnionExInfo->m_mpUnionMember.end();

        for (; it != end; ++it)
        {
            _GUILD_INFO_STRUCT* lpGuildInfo = it->second;

            if (lpGuildInfo && lpGuildInfo->iGuildRival)
            {
                _GUILD_INFO_STRUCT* lpRivalGuild =this->m_pGuildClass->SearchGuild_Number(lpGuildInfo->iGuildRival);

                if (lpRivalGuild)
                {
                    if (lpRivalGuild->iGuildUnion)
                    {
                        this->InitRivalMemberInfo(lpRivalGuild->iGuildUnion);
                    }
                    else
                    {
                        this->InitRivalMemberInfo(lpRivalGuild->Number);
                    }

                    DGRelationShipListSend(lpRivalGuild, 2, 0, -1);

                    this->NotifyList(lpRivalGuild, 1);
                }
            }
        }

        m_Sync.Unlock();

        //
        // Refresh rivals for request guild
        //
        if (lpReqGuild->iGuildRival)
        {
            if (lpReqGuild->iGuildUnion)
            {
                this->InitRivalMemberInfo(lpReqGuild->iGuildUnion);
            }
            else
            {
                this->InitRivalMemberInfo(lpReqGuild->Number);
            }

            DGRelationShipListSend(lpReqGuild, 2, 0, -1);
            this->NotifyList(lpReqGuild, 1);
            _GUILD_INFO_STRUCT* lpRivalGuild =this->m_pGuildClass->SearchGuild_Number(lpReqGuild->iGuildRival);

            if (lpRivalGuild)
            {
                if (lpRivalGuild->iGuildUnion)
                {
                    this->InitRivalMemberInfo(lpRivalGuild->iGuildUnion);
                }
                else
                {
                    this->InitRivalMemberInfo(lpRivalGuild->Number);
                }

                DGRelationShipListSend(lpRivalGuild, 2, 0, -1);
                this->NotifyList(lpRivalGuild, 1);
            }
        }
    }

    return 1;
}

//int TUnionEx::SetRelationUnion(int iReqGuildNumber,int iTargetGuildNumber,int iRelationOperation)
//{
//    if (!this->m_pGuildClass || !this->m_pGuildDBSet)
//        return -1;
//
//    if (iReqGuildNumber == iTargetGuildNumber)
//        return -1;
//
//    _GUILD_INFO_STRUCT* lpReqGuild =
//        this->m_pGuildClass->SearchGuild_Number(iReqGuildNumber);
//
//    _GUILD_INFO_STRUCT* lpTargetGuild =
//        this->m_pGuildClass->SearchGuild_Number(iTargetGuildNumber);
//
//    // =========================================================
//    // JOIN UNION
//    // =========================================================
//    if (iRelationOperation == 1)
//    {
//        if (!lpReqGuild || !lpTargetGuild)
//            return -1;
//
//        // target already belongs to another union
//        if (lpTargetGuild->iGuildUnion &&
//            lpTargetGuild->Number != lpTargetGuild->iGuildUnion)
//        {
//            return -1;
//        }
//
//        // Add requesting guild into target union
//        if (this->AddUnionMemberInfo(
//            iTargetGuildNumber,
//            lpReqGuild->Number,
//            1,
//            1) == -1)
//        {
//            return -1;
//        }
//
//        // Create self-union if needed
//        if (!lpTargetGuild->iGuildUnion)
//        {
//            lpTargetGuild->iGuildUnion = iTargetGuildNumber;
//
//            this->m_pGuildDBSet->UpdateGuildUnion(
//                iTargetGuildNumber,
//                iTargetGuildNumber);
//        }
//
//        // Refresh relationship lists
//        DGRelationShipListSend(lpTargetGuild, 1, 0, -1);
//        DGRelationShipListSend(lpTargetGuild, 2, 0, -1);
//
//        this->NotifyList(lpTargetGuild, 0);
//
//        m_Sync.Lock();
//
//        TUnionExInfo* pUnionExInfo =
//            this->SearchUnion(lpTargetGuild->iGuildUnion);
//
//        // Refresh all rival caches
//        for (auto it = pUnionExInfo->m_mpUnionMember.begin();
//            it != pUnionExInfo->m_mpUnionMember.end();
//            ++it)
//        {
//            _GUILD_INFO_STRUCT* lpUnionGuild = it->second;
//
//            if (!lpUnionGuild)
//                continue;
//
//            if (!lpUnionGuild->iGuildRival)
//                continue;
//
//            _GUILD_INFO_STRUCT* lpRivalGuild =
//                this->m_pGuildClass->SearchGuild_Number(
//                    lpUnionGuild->iGuildRival);
//
//            if (lpRivalGuild->iGuildUnion)
//                this->InitRivalMemberInfo(lpRivalGuild->iGuildUnion);
//            else
//                this->InitRivalMemberInfo(lpRivalGuild->Number);
//
//            DGRelationShipListSend(lpRivalGuild, 2, 0, -1);
//
//            this->NotifyList(lpRivalGuild, 1);
//        }
//
//        m_Sync.Unlock();
//    }
//
//    // =========================================================
//    // LEAVE UNION
//    // =========================================================
//    else if (iRelationOperation == 2)
//    {
//        if (!lpReqGuild || !lpTargetGuild)
//            return -1;
//
//        TUnionExInfo* pUnionExInfo = this->SearchUnion(iTargetGuildNumber);
//
//        if (!pUnionExInfo)
//            return -1;
//
//        // Remove member
//        if (this->DelUnionMemberInfo(iTargetGuildNumber,lpReqGuild->Number,1,1) == -1)
//        {
//            return -1;
//        }
//
//        // Union became invalid
//        if (!pUnionExInfo->IsValidUnion())
//        {
//            this->m_pGuildDBSet->UpdateGuildUnion(iTargetGuildNumber,0);
//            DGRelationShipNotificationSend(0x10,1,&lpTargetGuild->iGuildUnion,0);
//
//            lpTargetGuild->iGuildUnion = 0;
//        }
//
//        // Refresh lists
//        DGRelationShipListSend(lpTargetGuild, 1, 0, -1);
//        DGRelationShipListSend(lpTargetGuild, 2, 0, -1);
//
//        this->NotifyList(lpTargetGuild, 0);
//
//        DGRelationShipListSend(lpReqGuild, 1, 0, -1);
//        DGRelationShipListSend(lpReqGuild, 2, 0, -1);
//
//        this->NotifyList(lpReqGuild, 0);
//
//        m_Sync.Lock();
//
//        // Rebuild rival caches
//        for (auto it = pUnionExInfo->m_mpUnionMember.begin();
//            it != pUnionExInfo->m_mpUnionMember.end();
//            ++it)
//        {
//            _GUILD_INFO_STRUCT* second = it->second;
//
//            if (!second)
//                continue;
//
//            if (!second->iGuildRival)
//                continue;
//
//            _GUILD_INFO_STRUCT* lpGuildInfo = this->m_pGuildClass->SearchGuild_Number(second->iGuildRival);
//
//            if (lpGuildInfo->iGuildUnion)
//                this->InitRivalMemberInfo(lpGuildInfo->iGuildUnion);
//            else
//                this->InitRivalMemberInfo(lpGuildInfo->Number);
//
//            DGRelationShipListSend(lpGuildInfo, 2, 0, -1);
//
//            this->NotifyList(lpGuildInfo, 1);
//        }
//
//        m_Sync.Unlock();
//
//        // Additional rival refresh
//        if (lpReqGuild->iGuildRival)
//        {
//            if (lpReqGuild->iGuildUnion)
//                this->InitRivalMemberInfo(lpReqGuild->iGuildUnion);
//            else
//                this->InitRivalMemberInfo(lpReqGuild->Number);
//
//            DGRelationShipListSend(lpReqGuild, 2, 0, -1);
//
//            this->NotifyList(lpReqGuild, 1);
//
//            _GUILD_INFO_STRUCT* rivalGuild = this->m_pGuildClass->SearchGuild_Number( lpReqGuild->iGuildRival);
//
//            if (rivalGuild->iGuildUnion)
//                this->InitRivalMemberInfo(rivalGuild->iGuildUnion);
//            else
//                this->InitRivalMemberInfo(rivalGuild->Number);
//
//            DGRelationShipListSend(rivalGuild, 2, 0, -1);
//
//            this->NotifyList(rivalGuild, 1);
//        }
//    }
//
//    return 1;
//}