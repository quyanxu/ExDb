// TUnionExInfo.h
#pragma once

#include <map>
#include "GuildClass.h"     // Contains _GUILD_INFO_STRUCT and related defines

class TUnionExInfo
{
public:
    TUnionExInfo();
    TUnionExInfo(int iGuildNumber, _GUILD_INFO_STRUCT* lpGuildInfo);
    ~TUnionExInfo();

    // Core operations
    int SetUnionMaster(int iGuildNumber, _GUILD_INFO_STRUCT* lpGuildInfo);
    void Init();
    void Clear(int iRelationShipType);

    // Member management
    int AddUnionMemberInfo(_GUILD_INFO_STRUCT* lpGuildInfo);
    int AddRivalMemberInfo(_GUILD_INFO_STRUCT* lpGuildInfo);
    int DelUnionMemberInfo(int iGuildNumber);
    int DelRivalMemberInfo(int iGuildNumber);

    // Searching
    _GUILD_INFO_STRUCT* SearchUnionMember(int iGuildNumber);
    _GUILD_INFO_STRUCT* SearchRivalMember(int iGuildNumber);

    // Status
    BOOL IsValidUnion();
    unsigned int GetUnionMemberCount();
    unsigned int GetRivalMemberCount();

public:
    int m_iMasterGuild;
    _GUILD_INFO_STRUCT* m_lpMasterGuildInfo;
    char m_szMasterGuild[MAX_GUILDNAMESTRING+1];      // 8 bytes + null terminator (as per memcpy 8u)
    std::map<int, _GUILD_INFO_STRUCT*> m_mpUnionMember;
    std::map<int, _GUILD_INFO_STRUCT*> m_mpRivalMember;
    MSync<int> m_Sync;
};