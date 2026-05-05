// GensSystem.cpp
#include "stdafx.h"
#include "GensSystem.h"
#include "dbset/GensSystemDBSet.h"
#include "dbset/GuildDBSet.h"

// ----------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------

GensSystem g_GensSystem;

// ----------------------------------------------------------------------
// GensSystem
// ----------------------------------------------------------------------

GensSystem::GensSystem()
{
}

GensSystem::~GensSystem()
{
}

int GensSystem::ReqRegGensMember(_tagPMSG_REQ_REG_GENS_MEMBER_EXDB* lpMsg)
{
	if (!lpMsg)
		return -1;

	int iRet = 0;

	char szGuildMaster[20];
	char szMemberName[20];

	memset(szGuildMaster, 0, sizeof(szGuildMaster));
	memset(szMemberName, 0, sizeof(szMemberName));

	memcpy(szMemberName, lpMsg->szCharName, MAX_IDSTRING);

	int iGuildNum = (lpMsg->GuildNumH << 16) | lpMsg->GuildNumL;

	_tagGensMemberInfo stGensMemberInfo;

	if (!g_GensSystem_DBSet.DBGetGensMemberInfo(
		szMemberName,
		&stGensMemberInfo)
		&&
		stGensMemberInfo.btInfluence != 0xFF)
	{
		return 1;
	}

	if (stGensMemberInfo.btInfluence == 0xFF)
	{
		iRet = g_GensSystem_DBSet.DBChkSecedeDate(szMemberName);

		if (iRet == 2)
			return 2;
	}

	memset(&stGensMemberInfo, 0, sizeof(stGensMemberInfo));

	if (iGuildNum)
	{
		iRet = g_GensSystem_DBSet.DBGetGuildMasterName(
			szGuildMaster,
			iGuildNum);

		if (iRet == -1)
			return -1;

		if (strcmp(szGuildMaster, szMemberName))
		{
			if (g_GensSystem_DBSet.DBGetGensMemberInfo(
				szGuildMaster,
				&stGensMemberInfo))
			{
				return 5;
			}

			if (stGensMemberInfo.btInfluence != lpMsg->btInfluence)
			{
				return 4;
			}
		}
		else
		{
			if (!g_GensSystem_DBSet.DBGetGensMemberInfo(
				szGuildMaster,
				&stGensMemberInfo)
				&&
				stGensMemberInfo.btInfluence != 0xFF)
			{
				return 1;
			}
		}
	}

	if (g_GensSystem_DBSet.DBRegGensMember(
		lpMsg->szAccount,
		lpMsg->szCharName,
		lpMsg->btInfluence))
	{
		return iRet;
	}

	return -100;
}

int GensSystem::ReqSecedeGensMember(_tagPMSG_REQ_SECEDE_GENS_MEMBER_EXDB* lpMsg)
{
	if (!lpMsg)
		return -1;

	int iRet = 0;

	char szGuildMaster[20];
	char szMemberName[20];

	memset(szGuildMaster, 0, sizeof(szGuildMaster));
	memset(szMemberName, 0, sizeof(szMemberName));

	memcpy(szMemberName, lpMsg->szCharName, MAX_IDSTRING);

	int iGuildNum = (lpMsg->GuildNumH << 16) | lpMsg->GuildNumL;

	_tagGensMemberInfo stGensMemberInfo;

	iRet = g_GensSystem_DBSet.DBGetGensMemberInfo(szMemberName, &stGensMemberInfo);

	if (stGensMemberInfo.btInfluence == 0xFF || iRet == -1)
	{
		return 1;
	}

	if (iGuildNum)
	{
		iRet = g_GensSystem_DBSet.DBGetGuildMasterName(szGuildMaster, iGuildNum);

		if (iRet == -1)
			return -1;

		if (!strcmp(szGuildMaster, szMemberName))
		{
			return 2;
		}
	}

	if (!GuildDbSet->GetGuildMatchingWaitState(szMemberName, 0))
	{
		return 4;
	}

	if (g_GensSystem_DBSet.DBSecedeGensMember(lpMsg->szCharName))
	{
		return iRet;
	}

	return -100;
}

int GensSystem::ReqGensInfo(_tagPMSG_REQ_GENS_INFO_EXDB* lpMsg, _tagGensMemberInfo* pST_GensMemberInfo)
{
	return g_GensSystem_DBSet.DBGetGensMemberInfo(lpMsg->szCharName, pST_GensMemberInfo);
}

int GensSystem::ReqSaveContributePoint(_tagPMSG_REQ_SAVE_CONTRIBUTE_POINT_EXDB* lpMsg)
{
	return g_GensSystem_DBSet.DBSaveContributePoint(lpMsg->szCharName, lpMsg->iContributePoint);
}

int GensSystem::ReqSaveAbusingKillUserName(_tagPMSG_REQ_SAVE_ABUSING_KILLUSER_EXDB* lpMsg)
{
	return g_GensSystem_DBSet.DBSaveAbusingKillUserName(lpMsg);
}

int GensSystem::ReqAbusingInfo(_tagPMSG_REQ_ABUSING_INFO* lpMsg, _tagPMSG_ANS_ABUSING_INFO* pAnsAbusingInfo)
{
	return g_GensSystem_DBSet.DBGetAbusingInfo(lpMsg->szCharName, pAnsAbusingInfo);
}

void GensSystem::ReqGensSecedeDateModify(_tagPMSG_REQ_SECEDE_GENS_DATE_MODIFY* lpMsg)
{
	g_GensSystem_DBSet.DBReqGensSecedeDateModify(lpMsg);
}

int GensSystem::ReqGensReward(_tagPMSG_REQ_GENS_REWARD_EXDB* lpMsg, _tagPMSG_ANS_GENS_REWARD_EXDB* pAnsGensReward)
{
	return g_GensSystem_DBSet.DBGetGensReward(lpMsg, pAnsGensReward);
}

int GensSystem::ReqGensRewardComplete(_tagPMSG_REQ_GENS_REWARD_COMPLETE_EXDB* lpMsg)
{
	return g_GensSystem_DBSet.DBGetGensRewardComplete(lpMsg->szCharName);
}

int GensSystem::ReqGensMemberCount(_tagPMSG_REQ_GENS_MEMBER_COUNT_EXDB* lpMsg, _tagPMSG_ANS_GENS_MEMBER_COUNT_EXDB* pAnsGensMemberCount)
{
	return g_GensSystem_DBSet.DBGetGensMemberCount(lpMsg->szCharName, pAnsGensMemberCount);
}

int GensSystem::ReqSetGensRewardDay(_tagPMSG_REQ_SET_GENS_REWARD_DAY_EXDB* lpMsg)
{
	return g_GensSystem_DBSet.DBGetSetGensRewardDay();
}

int GensSystem::ReqSetGensRanking(_tagPMSG_REQ_SET_GENS_RANKING_EXDB* lpMsg)
{
	return g_GensSystem_DBSet.DBGetSetGensRanking();
}

int GensSystem::ReqGensRewardDayCheck(_tagPMSG_REQ_GENS_REWARD_DAY_CHECK_EXDB* lpMsg, _tagPMSG_ANS_GENS_REWARD_DAY_CHECK_EXDB* pAnsMsg)
{
	return g_GensSystem_DBSet.DBGensRewardDayCheck(pAnsMsg);
}
