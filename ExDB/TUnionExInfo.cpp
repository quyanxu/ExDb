#include "stdafx.h"
// TUnionExInfo.cpp
#include "TUnionExInfo.h"
#include <cstring>

TUnionExInfo::TUnionExInfo()
{
    memset(m_szMasterGuild, 0, sizeof(m_szMasterGuild));
    // m_mpUnionMember, m_mpRivalMember, m_Sync are default-constructed
}

TUnionExInfo::TUnionExInfo(int iGuildNumber, _GUILD_INFO_STRUCT* lpGuildInfo)
{
    memset(m_szMasterGuild, 0, sizeof(m_szMasterGuild));
    // m_mpUnionMember, m_mpRivalMember, m_Sync are default-constructed
    SetUnionMaster(iGuildNumber, lpGuildInfo);
}

TUnionExInfo::~TUnionExInfo()
{
    Clear(0);
    // m_Sync, m_mpRivalMember, m_mpUnionMember destructors called automatically
}

int TUnionExInfo::SetUnionMaster(int iGuildNumber, _GUILD_INFO_STRUCT* lpGuildInfo)
{
    if (!lpGuildInfo)
        return -1;

    m_Sync.Lock();
    m_iMasterGuild = iGuildNumber;
    m_lpMasterGuildInfo = lpGuildInfo;
    memcpy(m_szMasterGuild, lpGuildInfo->Name, 8);
    m_szMasterGuild[8] = '\0'; // Ensure null termination
    m_mpUnionMember[lpGuildInfo->Number] = lpGuildInfo;
    m_Sync.Unlock();
    return 1;
}

_GUILD_INFO_STRUCT* TUnionExInfo::SearchUnionMember(int iGuildNumber)
{
    std::map<int, _GUILD_INFO_STRUCT*>::iterator it;

    it = this->m_mpUnionMember.find(iGuildNumber);

    if (it != this->m_mpUnionMember.end())
    {
        return it->second;
    }

    return NULL;
}

_GUILD_INFO_STRUCT* TUnionExInfo::SearchRivalMember(int iGuildNumber)
{
    std::map<int, _GUILD_INFO_STRUCT*>::iterator it;

    it = this->m_mpRivalMember.find(iGuildNumber);

    if (it != this->m_mpRivalMember.end())
    {
        return it->second;
    }

    return NULL;
}

//_GUILD_INFO_STRUCT* TUnionExInfo::SearchUnionMember(int iGuildNumber)
//{
//    auto it = m_mpUnionMember.find(iGuildNumber);
//    if (it != m_mpUnionMember.end())
//        return it->second;
//    return 0;
//}
//
//_GUILD_INFO_STRUCT* TUnionExInfo::SearchRivalMember(int iGuildNumber)
//{
//    auto it = m_mpRivalMember.find(iGuildNumber);
//    if (it != m_mpRivalMember.end())
//        return it->second;
//    return 0;
//}

BOOL TUnionExInfo::IsValidUnion()
{
    return m_mpUnionMember.size() >= 2;
}

void TUnionExInfo::Init()
{
    Clear(0);
    SetUnionMaster(m_iMasterGuild, m_lpMasterGuildInfo);
}

unsigned int TUnionExInfo::GetUnionMemberCount()
{
    return static_cast<unsigned int>(m_mpUnionMember.size());
}

unsigned int TUnionExInfo::GetRivalMemberCount()
{
    return static_cast<unsigned int>(m_mpRivalMember.size());
}

int TUnionExInfo::DelUnionMemberInfo(int iGuildNumber)
{
    _GUILD_INFO_STRUCT* lpGuildInfo = SearchUnionMember(iGuildNumber);
    if (!lpGuildInfo)
        return -1;

    m_Sync.Lock();
    m_mpUnionMember.erase(lpGuildInfo->Number);
    lpGuildInfo->iGuildUnion = 0;
    m_Sync.Unlock();
    return 1;
}

int TUnionExInfo::DelRivalMemberInfo(int iGuildNumber)
{
    _GUILD_INFO_STRUCT* lpGuildInfo = SearchRivalMember(iGuildNumber);
    if (!lpGuildInfo)
        return -1;

    m_Sync.Lock();
    m_mpRivalMember.erase(lpGuildInfo->Number);
    m_Sync.Unlock();
    return 1;
}

void TUnionExInfo::Clear(int iRelationShipType)
{
    m_Sync.Lock();
    if (iRelationShipType == 1)
    {
        m_mpUnionMember.clear();
    }
    else if (iRelationShipType == 2)
    {
        m_mpRivalMember.clear();
    }
    else // 0: Clear all
    {
        m_mpRivalMember.clear();
        m_mpUnionMember.clear();
    }
    m_Sync.Unlock();
}

int TUnionExInfo::AddUnionMemberInfo(_GUILD_INFO_STRUCT* lpGuildInfo)
{
    m_Sync.Lock();
    lpGuildInfo->iGuildUnion = m_iMasterGuild;
    m_mpUnionMember[lpGuildInfo->Number] = lpGuildInfo;
    m_Sync.Unlock();
    return 1;
}

int TUnionExInfo::AddRivalMemberInfo(_GUILD_INFO_STRUCT* lpGuildInfo)
{
    m_Sync.Lock();
    m_mpRivalMember[lpGuildInfo->Number] = lpGuildInfo;
    m_Sync.Unlock();
    return 1;
}