// GensSystem_DBSet.h
#pragma once

// ----------------------------------------------------------------------
// GensSystem_DBSet
// ----------------------------------------------------------------------

class GensSystem_DBSet
{
public:
	GensSystem_DBSet();
	virtual ~GensSystem_DBSet();

public:
	int Connect();
	int LoadGensSetting();

	int DBRegGensMember(char* szAccountID, char* szCharName, BYTE btInfluence);
	int DBChkSecedeDate(char* szCharName);
	int DBGetGuildMasterName(char* szCharName, int iGuildNum);
	int DBGetGensMemberInfo(char* szCharName, _tagGensMemberInfo* pST_GensMemberInfo);
	int DBSecedeGensMember(char* szCharName);
	int DBSaveContributePoint(char* szCharName, int iContributePoint);
	int DBSaveAbusingKillUserName(_tagPMSG_REQ_SAVE_ABUSING_KILLUSER_EXDB* lpMsg);
	int DBGetAbusingInfo(char* szCharName, _tagPMSG_ANS_ABUSING_INFO* lpMsg);
	void DBReqGensSecedeDateModify(_tagPMSG_REQ_SECEDE_GENS_DATE_MODIFY* lpMsg);
	int DBGetGensReward(_tagPMSG_REQ_GENS_REWARD_EXDB* lpReqMsg, _tagPMSG_ANS_GENS_REWARD_EXDB* lpMsg);
	int DBGetGensRewardComplete(char* szCharName);
	int DBGetGensMemberCount(char* szCharName, _tagPMSG_ANS_GENS_MEMBER_COUNT_EXDB* lpMsg);
	int DBGetSetGensRewardDay();
	int DBGetSetGensRanking();
	int DBGensRewardDayCheck(_tagPMSG_ANS_GENS_REWARD_DAY_CHECK_EXDB* lpMsg);

public:
	CQuery             m_GensMemberQry;
	int                m_iSecedeChkDay;
	CRITICAL_SECTION   m_csGensMemberQry;
};

extern GensSystem_DBSet g_GensSystem_DBSet;