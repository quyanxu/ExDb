// GensSystem_DBSet.cpp
#include "stdafx.h"
#include "AutoSync.h"
#include "GensSystemDBSet.h"

GensSystem_DBSet g_GensSystem_DBSet;

// ----------------------------------------------------------------------
// Constructor / Destructor
// ----------------------------------------------------------------------

GensSystem_DBSet::GensSystem_DBSet()
{
	m_iSecedeChkDay = 0;
	InitializeCriticalSection(&m_csGensMemberQry);
}

GensSystem_DBSet::~GensSystem_DBSet()
{
	DeleteCriticalSection(&m_csGensMemberQry);
}

// ----------------------------------------------------------------------
// Connection
// ----------------------------------------------------------------------

int GensSystem_DBSet::Connect()
{
	if (m_GensMemberQry.Connect(3, szDbConnectDsn, szDbConnectId, szDbConnectPass))
	{
		return 1;
	}

	MsgBox("[DEFAULT] GensSystem_DBSet ODBC Connect Fail");
	return 0;
}

// ----------------------------------------------------------------------
// Settings
// ----------------------------------------------------------------------

int GensSystem_DBSet::LoadGensSetting()
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	CString qSql;
	qSql.Format("SELECT SecedeChkDay FROM T_GENS_SETTING");

	if (m_GensMemberQry.Exec(qSql))
	{
		short sqlRet = m_GensMemberQry.Fetch();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
		{
			MsgBox("[DEFAULT] GensSystem_DBSet Fail to Load GensSetting");
			m_GensMemberQry.Clear();
			return 0;
		}

		m_iSecedeChkDay = m_GensMemberQry.GetInt("SecedeChkDay");

		cLog.AddC(4, "[GensSystem] Secede check Day : %d", m_iSecedeChkDay);
	}

	m_GensMemberQry.Clear();
	return 1;
}

// ----------------------------------------------------------------------
// Register Member
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBRegGensMember(char* szAccountID, char* szCharName, BYTE btInfluence)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	CString qSql;

	qSql.Format("WZ_AddGensMember '%s', '%s', %d", szAccountID, szCharName, btInfluence);

	int iResult = m_GensMemberQry.Exec(qSql);

	m_GensMemberQry.Clear();

	return iResult ? 1 : 0;
}

// ----------------------------------------------------------------------
// Secede Date Check
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBChkSecedeDate(char* szCharName)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	char szMemberName[MAX_IDSTRING + 1] = { 0 };
	memcpy(szMemberName, szCharName, MAX_IDSTRING);

	CString qSql;
	qSql.Format("WZ_GetRegGensDayDiff '%s'", szMemberName);

	if (!m_GensMemberQry.Exec(qSql))
	{
		m_GensMemberQry.Clear();
		return 0;
	}

	short sqlRet = m_GensMemberQry.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
	{
		m_GensMemberQry.Clear();
		return 0;
	}

	int iDiffDay = m_GensMemberQry.GetInt("DiffDay");

	m_GensMemberQry.Clear();

	if (iDiffDay < m_iSecedeChkDay)
		return 2;

	return 0;
}

// ----------------------------------------------------------------------
// Guild Master Name
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBGetGuildMasterName(char* szCharName, int iGuildNum)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	CString qSql;
	qSql.Format("WZ_GetGuildMasterName %d", iGuildNum);

	if (m_GensMemberQry.Exec(qSql))
	{
		short sqlRet = m_GensMemberQry.Fetch();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
		{
			m_GensMemberQry.Clear();
			return -1;
		}

		char szGuildMasterName[MAX_IDSTRING + 1] = { 0 };

		m_GensMemberQry.GetStr("G_Master", szGuildMasterName);

		memcpy(szCharName, szGuildMasterName, MAX_IDSTRING);
	}

	m_GensMemberQry.Clear();
	return 0;
}

// ----------------------------------------------------------------------
// Get Member Info
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBGetGensMemberInfo(char* szCharName, _tagGensMemberInfo* pST_GensMemberInfo)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	CString qSql;

	qSql.Format("WZ_GetGensMemberInfo '%s'", szCharName);

	if (!m_GensMemberQry.Exec(qSql))
	{
		m_GensMemberQry.Clear();
		return -2;
	}

	short sqlRet = m_GensMemberQry.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
	{
		m_GensMemberQry.Clear();
		return -1;
	}

	m_GensMemberQry.GetStr("Char_Name", pST_GensMemberInfo->szCharName);

	pST_GensMemberInfo->iContributePoint = m_GensMemberQry.GetInt("Contribute_Point");
	pST_GensMemberInfo->btInfluence = (BYTE)m_GensMemberQry.GetInt("Influence");
	pST_GensMemberInfo->iGensClass = m_GensMemberQry.GetInt("Gens_Class");

	m_GensMemberQry.Clear();

	qSql.Format("WZ_GetGensRanking '%s'", szCharName);

	if (m_GensMemberQry.Exec(qSql))
	{
		if (m_GensMemberQry.Fetch() != SQL_NO_DATA)
		{
			pST_GensMemberInfo->iGensRanking = m_GensMemberQry.GetInt("Gens_Ranking");
		}
	}

	m_GensMemberQry.Clear();

	return 0;
}

// ----------------------------------------------------------------------
// Secede Member
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBSecedeGensMember(char* szCharName)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	CString qSql;

	qSql.Format("WZ_SecedeGensMember '%s'", szCharName);

	int iResult = m_GensMemberQry.Exec(qSql);

	m_GensMemberQry.Clear();

	return iResult ? 1 : 0;
}

// ----------------------------------------------------------------------
// Save Contribute Point
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBSaveContributePoint(char* szCharName, int iContributePoint)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	CString qSql;

	qSql.Format("WZ_SaveContributePoint '%s', %d", szCharName, iContributePoint);

	m_GensMemberQry.Exec(qSql);
	m_GensMemberQry.Clear();

	return 0;
}

// ----------------------------------------------------------------------
// Save Abusing Kill User
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBSaveAbusingKillUserName(_tagPMSG_REQ_SAVE_ABUSING_KILLUSER_EXDB* lpMsg)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	CString qSql;

	qSql.Format(
		"WZ_AbusingKillUser '%s', '%s', "
		"'%s', %d, "
		"'%s', %d, "
		"'%s', %d, "
		"'%s', %d, "
		"'%s', %d, "
		"'%s', %d, "
		"'%s', %d, "
		"'%s', %d, "
		"'%s', %d, "
		"'%s', %d",
		lpMsg->szAccount,
		lpMsg->szCharName,
		lpMsg->szKillUserName[0], lpMsg->iKillUserCnt[0],
		lpMsg->szKillUserName[1], lpMsg->iKillUserCnt[1],
		lpMsg->szKillUserName[2], lpMsg->iKillUserCnt[2],
		lpMsg->szKillUserName[3], lpMsg->iKillUserCnt[3],
		lpMsg->szKillUserName[4], lpMsg->iKillUserCnt[4],
		lpMsg->szKillUserName[5], lpMsg->iKillUserCnt[5],
		lpMsg->szKillUserName[6], lpMsg->iKillUserCnt[6],
		lpMsg->szKillUserName[7], lpMsg->iKillUserCnt[7],
		lpMsg->szKillUserName[8], lpMsg->iKillUserCnt[8],
		lpMsg->szKillUserName[9], lpMsg->iKillUserCnt[9]);

	m_GensMemberQry.Exec(qSql);
	m_GensMemberQry.Clear();

	return 0;
}

// ----------------------------------------------------------------------
// Get Abusing Info
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBGetAbusingInfo(char* szCharName, _tagPMSG_ANS_ABUSING_INFO* lpMsg)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	CString qSql;
	qSql.Format("WZ_GetAbusingInfo '%s'", szCharName);

	if (!m_GensMemberQry.Exec(qSql))
	{
		m_GensMemberQry.Clear();
		return -1;
	}

	short sqlRet = m_GensMemberQry.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
	{
		m_GensMemberQry.Clear();
		return -1;
	}

	char szKillCharName[64];
	char szKillCount[64];

	for (int i = 0; i < 10; i++)
	{
		wsprintf(szKillCharName, "Kill_Char_Name_%d", i + 1);
		wsprintf(szKillCount, "Kill_Count_%d", i + 1);

		m_GensMemberQry.GetStr(szKillCharName, lpMsg->szKillUserName[i]);

		lpMsg->iKillUserCnt[i] = m_GensMemberQry.GetInt(szKillCount);
	}

	m_GensMemberQry.Clear();

	return 0;
}

// ----------------------------------------------------------------------
// Secede Date Modify
// ----------------------------------------------------------------------

void GensSystem_DBSet::DBReqGensSecedeDateModify(_tagPMSG_REQ_SECEDE_GENS_DATE_MODIFY* lpMsg)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	CString qSql;

	qSql.Format(
		"UPDATE T_GENS_MEMBER "
		"SET SecedeDate = GetDate() - %d "
		"WHERE Char_Name = '%s'",
		lpMsg->iDateModify,
		lpMsg->szCharName);

	m_GensMemberQry.Exec(qSql);
	m_GensMemberQry.Clear();
}

// ----------------------------------------------------------------------
// Reward
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBGetGensReward(_tagPMSG_REQ_GENS_REWARD_EXDB* lpReqMsg, _tagPMSG_ANS_GENS_REWARD_EXDB* lpMsg)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	CString qSql;

	qSql.Format("WZ_GetGensReward '%s', '%d'", lpReqMsg->szCharName, lpReqMsg->iInfluence);

	if (m_GensMemberQry.Exec(qSql))
	{
		short sqlRet = m_GensMemberQry.Fetch();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
		{
			m_GensMemberQry.Clear();
			return -1;
		}

		lpMsg->btResult = (BYTE)m_GensMemberQry.GetInt(1);
		lpMsg->iGensClass = m_GensMemberQry.GetInt("Gens_Class");
	}

	m_GensMemberQry.Clear();

	return 0;
}

// ----------------------------------------------------------------------
// Reward Complete
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBGetGensRewardComplete(char* szCharName)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	CString qSql;

	qSql.Format("WZ_SetGensRewardComplete '%s'", szCharName);

	int iResult = m_GensMemberQry.Exec((LPCTSTR)qSql);

	m_GensMemberQry.Clear();

	return iResult ? 0 : -1;
}

// ----------------------------------------------------------------------
// Member Count
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBGetGensMemberCount(char* szCharName, _tagPMSG_ANS_GENS_MEMBER_COUNT_EXDB* lpMsg)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	if (!m_GensMemberQry.Exec("WZ_GetGensMemberCount"))
	{
		m_GensMemberQry.Clear();
		return -1;
	}

	short sqlRet = m_GensMemberQry.Fetch();

	if (sqlRet != SQL_NO_DATA && sqlRet != SQL_ERROR)
	{
		lpMsg->iGensMemberCount1 = m_GensMemberQry.GetInt("GensMemberCount_1");
		lpMsg->iGensMemberCount2 = m_GensMemberQry.GetInt("GensMemberCount_2");
	}

	m_GensMemberQry.Clear();

	return 0;
}

// ----------------------------------------------------------------------
// Set Reward Day
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBGetSetGensRewardDay()
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	if (m_GensMemberQry.Exec("WZ_SetGensRanking"))
	{
		m_GensMemberQry.Clear();

		if (m_GensMemberQry.Exec("WZ_SetGensRewardDay"))
		{
			m_GensMemberQry.Clear();
			return 0;
		}
	}

	m_GensMemberQry.Clear();

	return -1;
}

// ----------------------------------------------------------------------
// Set Ranking
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBGetSetGensRanking()
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	if (m_GensMemberQry.Exec("WZ_SetGensRanking"))
	{
		m_GensMemberQry.Clear();
		return 0;
	}

	m_GensMemberQry.Clear();

	return -1;
}

// ----------------------------------------------------------------------
// Reward Day Check
// ----------------------------------------------------------------------

int GensSystem_DBSet::DBGensRewardDayCheck(_tagPMSG_ANS_GENS_REWARD_DAY_CHECK_EXDB* lpMsg)
{
	CAutoSync AutoSync(&m_csGensMemberQry);

	if (!m_GensMemberQry.Exec("WZ_GetGensRewardDayCheck"))
	{
		m_GensMemberQry.Clear();
		return -1;
	}

	short sqlRet = m_GensMemberQry.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
	{
		m_GensMemberQry.Clear();
		return -1;
	}

	lpMsg->iResult = m_GensMemberQry.GetInt(1);

	m_GensMemberQry.Clear();

	return 0;
}

