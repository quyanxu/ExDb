// TUnionEx.h
#pragma once

#include <map>
#include <vector>
#include "TUnionExInfo.h"    // Contains TUnionExInfo class
#include "GuildClass.h"     // Contains _GUILD_INFO_STRUCT and related defines
#include "dbset/GuildDBSet.h"

class TUnionEx
{
public:
    TUnionEx();
    ~TUnionEx();

    // Initialization
    int Init();
    int SetGuildClass(CGuildClass* pGuildClass);
    int SetGuildDBSet(CGuildDBSet* pGuildDBSet);

    // Union Management
    int AddUnion(int iMasterGuildNumber, _GUILD_INFO_STRUCT* lpGuildInfo, int bWriteDB);
    int AddUnionMemberInfo(int iMasterGuildNumber, _GUILD_INFO_STRUCT* lpGuildInfo, int bWriteDB, int bInitRival);
    int AddUnionMemberInfo(int iMasterGuildNumber, int iGuildNumber, int bWriteDB, int bInitRival);
    int DelUnion(int iMasterGuildNumber);
    int DelUnionMemberInfo(int iMasterGuildNumber, int iGuildNumber, int bWriteDB, int bInitRival);
    int DelAllUnion();
    TUnionExInfo* SearchUnion(int iMasterGuildNumber);

    // Rival Management
    int InitAllRivalMemberInfo();
    int InitRivalMemberInfo(int iMasterGuildNumber);

    // Relationships
    int GetGuildRelationShip(int iMasterGuildNumber, int iTargetGuildNumber);
    int SetRelationUnion(int iReqGuildNumber, int iTargetGuildNumber, int iRelationOperation);
    int SetRelationRival(int iReqGuildNumber, int iTargetGuildNumber, int iRelationOperation);

    // Notifications
    int NotifyInitRelationShip(_GUILD_INFO_STRUCT* lpGuildInfo, int iRelationShipType);
    int NotifyInitRelationUnion(_GUILD_INFO_STRUCT* lpGuildInfo);
    int NotifyInitRelationRival(_GUILD_INFO_STRUCT* lpGuildInfo);
    void NotifyList(_GUILD_INFO_STRUCT* lpGuildInfo, int iRelationShipType);

    // Report
    //std::vector<TUnionExInfo*>* ReportUnionInfoList(std::vector<TUnionExInfo*>* result);
    std::vector<TUnionExInfo*> TUnionEx::ReportUnionInfoList();

    // Accessors
    CGuildClass* GetGuildClass();
    CGuildDBSet* GetGuildDBSet();
    CWZIocpEngine* GetWzIoEngine();

public:
    std::map<int, TUnionExInfo*> m_mpUnion;
    std::vector<TUnionExInfo*> m_vtUnion;
    CGuildClass* m_pGuildClass;
    CGuildDBSet* m_pGuildDBSet;
    CWZIocpEngine* m_pIoEngine;
    MSync<int> m_Sync;
};

// Global instance (declare extern in header, define in one cpp)
extern TUnionEx UnionExManager;