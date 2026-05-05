#include "stdafx.h"
#include "AutoSync.h"
#include "GuildDBSet.h"
#include "MainFrm.h"
#include "TUnionEx.h"

CGuildDBSet* GuildDbSet;
extern TUnionEx UnionExManager;


CGuildDBSet::CGuildDBSet()
{
	InitializeCriticalSection(&this->m_csUnionQry);
	InitializeCriticalSection(&this->m_csGuildMemberQry);
	InitializeCriticalSection(&this->m_csGuildMarkQry);
	InitializeCriticalSection(&this->m_csDBQuery);

	this->m_TotalCount = 0;
}

CGuildDBSet::~CGuildDBSet()
{
	DeleteCriticalSection(&this->m_csUnionQry);
	DeleteCriticalSection(&this->m_csGuildMemberQry);
	DeleteCriticalSection(&this->m_csGuildMarkQry);
	DeleteCriticalSection(&this->m_csDBQuery);
}

int CGuildDBSet::Connect()
{
	char* TableName[] =
	{
		"Guild",
		"GuildMember"
	};

	char* SPName[] =
	{
		"WZ_GuildCreate"
	};

	if (!this->m_DBQuery.Connect(3, szDbConnectDsn, szDbConnectId, szDbConnectPass))
	{
		MsgBox("[DEFAULT] GUILDDBSET ODBC Connect Fail");
		return 0;
	}

	if (!this->m_GuildMemberQry.Connect(3, szDbConnectDsn, szDbConnectId, szDbConnectPass))
	{
		MsgBox("[MEMBER] GUILDMEMBERSET ODBC Connect Fail");
		return 0;
	}

	if (!this->m_GuildMarkQry.Connect(3, szDbConnectDsn, szDbConnectId, szDbConnectPass))
	{
		MsgBox("[MARK] GUILDMARKDBSET ODBC Connect Fail");
		return 0;
	}

	if (!this->m_UnionQry.Connect(3, szDbConnectDsn, szDbConnectId, szDbConnectPass))
	{
		MsgBox("[UNION] GUILDUNIONDBSET ODBC Connect Fail");
		return 0;
	}

	if (!this->CheckDBID())
	{
		MsgBox("Unknown Mu DB Identification");
	}

	for (int n = 0; n < 2; n++)
	{
		if (!this->m_DBQuery.TableCheck(TableName[n]))
		{
			MsgBox("[Guild] %s not Table", TableName[n]);
		}
	}

	for (int n = 0; n < 1; n++)
	{
		if (!this->m_DBQuery.SPCheck(SPName[n]))
		{
			MsgBox("[Guild] %s not script", SPName[n]);
		}
	}

	this->GetGuildCount();

	return 1;
}

int CGuildDBSet::CheckDBID()
{
	CAutoSync AutoSync(&this->m_csDBQuery);

	CString qSql;
	qSql.Format("exec WZ_Get_DBID");

	if (!this->m_DBQuery.Exec(qSql))
	{
		this->m_DBQuery.Clear();
		return 0;
	}

	short sqlRet = this->m_DBQuery.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
	{
		this->m_DBQuery.Clear();
		return 0;
	}

	this->m_DBQuery.GetStr("DESC", g_szDBID_DESC);
	this->m_DBQuery.Clear();

	return 1;
}

int CGuildDBSet::DeleteGuild(char* Name)
{
	CAutoSync AutoSync(&this->m_csDBQuery);

	CString QuerySql;
	QuerySql.Format(
		"DELETE FROM Guild WHERE G_Name='%s'",
		Name
	);

	int ret = this->m_DBQuery.Exec(QuerySql);

	this->m_DBQuery.Clear();

	return ret;
}

int CGuildDBSet::CreateGuild(char* GuildName, char* Master, unsigned char* Mark)
{
	CString QuerySql;
	short sqlRet;
	int ErrCode = 0;

	{
		CAutoSync AutoSync(&this->m_csGuildMemberQry);

		QuerySql.Format(
			"SELECT G_Name FROM Guild where G_Name='%s'",
			GuildName
		);

		this->m_GuildMemberQry.Exec(QuerySql);
		sqlRet = this->m_GuildMemberQry.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_ERROR)
		{
			this->m_GuildMemberQry.Clear();
			return 4;
		}

		this->m_GuildMemberQry.Clear();

		QuerySql.Format(
			"SELECT Name FROM GuildMember where Name='%s'",
			Master
		);

		this->m_GuildMemberQry.Exec(QuerySql);
		sqlRet = this->m_GuildMemberQry.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_ERROR)
		{
			this->m_GuildMemberQry.Clear();
			return 2;
		}

		this->m_GuildMemberQry.Clear();
	}

	{
		CAutoSync AutoSync(&this->m_csDBQuery);

		QuerySql.Format(
			"WZ_GuildCreate '%s','%s'",
			GuildName,
			Master
		);

		if (!this->m_DBQuery.Exec(QuerySql))
		{
			cLog.AddTD(
				"error-L3 : WZ_GuildCreate Guild:[%s] Master:[%s]",
				GuildName,
				Master
			);

			this->m_DBQuery.Clear();
			return 1;
		}

		sqlRet = this->m_DBQuery.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_ERROR)
		{
			ErrCode = this->m_DBQuery.GetInt(1);
		}

		this->m_DBQuery.Clear();

		if (ErrCode == 0)
		{
			QuerySql.Format("UPDATE Guild SET G_Mark=? where G_Name='%s'", GuildName);
			this->m_DBQuery.WriteBlob(QuerySql, Mark, MAX_GUILDMARKBUFFER);
		}
	}

	return ErrCode;
}

int CGuildDBSet::GetNumber(char* GuildName)
{
	CAutoSync AutoSync(&this->m_csDBQuery);

	CString qSql;

	qSql.Format(
		"SELECT Number FROM Guild where G_Name='%s'",
		GuildName
	);

	return this->m_DBQuery.ExecGetInt(qSql);
}

int CGuildDBSet::GetGuildCount()
{
	CAutoSync AutoSync(&this->m_csDBQuery);

	CString qSql;
	qSql.Format("select count(*) from Guild");

	this->m_TotalCount = this->m_DBQuery.ExecGetInt(qSql);

	return 1;
}

int CGuildDBSet::ReadAllUnion()
{
	UnionExManager.SetGuildDBSet(this);
	UnionExManager.SetGuildClass(&CGuildManager);
	UnionExManager.Init();

	return 1;
}
int CGuildDBSet::ReadAllGuild()
{
	CString qSql;
	int iGuildRival = 0;
	int iGuildUnion = 0;

	DWORD starttime = GetTickCount();

	// Get main window for status bar updates
	CWinApp* p = AfxGetApp();
	CMainFrame* pWnd = (CMainFrame*)p->GetMainWnd();

	// Turn off guild processing during load
	CGuildManager.GuildOnOff(0);

	// Query all guilds
	qSql.Format("SELECT * FROM Guild");

	if (!m_DBQuery.Exec(qSql))
	{
		cLog.Add("Guild not found");
		CGuildManager.GuildOnOff(1);
		m_DBQuery.Clear();
		return 0;
	}

	char GuildNotice[72];
	GuildNotice[60] = 0;

	__int16 sqlRet = m_DBQuery.Fetch();
	while (sqlRet != 100 && sqlRet != -1)
	{
		int Number = m_DBQuery.GetInt("Number");
		char GuildName[20];
		m_DBQuery.GetStr("G_Name", GuildName);
		char Master[20];
		m_DBQuery.GetStr("G_Master", Master);
		m_DBQuery.GetStr("G_Notice", GuildNotice);
		int score = m_DBQuery.GetInt("G_Score");

		// Read guild notice blob
		qSql.Format("SELECT G_Notice FROM Guild where G_Name='%s'", GuildName);
		m_GuildMarkQry.ReadBlob(qSql, (unsigned __int8*)GuildNotice);

		// Read guild mark blob
		unsigned __int8 Mark[40];
		qSql.Format("SELECT G_Mark FROM Guild where G_Name='%s'", GuildName);
		m_GuildMarkQry.ReadBlob(qSql, Mark);

		// Add guild to in-memory manager
		_GUILD_INFO_STRUCT* pNewGuild = CGuildManager.AddGuild(
			Number, GuildName, Mark, Master, GuildNotice, score, 0);

		if (pNewGuild)
		{
			iGuildRival = m_DBQuery.GetInt("G_Rival");
			iGuildUnion = m_DBQuery.GetInt("G_Union");
			pNewGuild->iGuildUnion = iGuildUnion;
			pNewGuild->iGuildRival = iGuildRival;

			TRACE_OUT(2, "Guild: %s Master: %s Union: %d Rival: %d",
				GuildName, Master, pNewGuild->iGuildUnion, pNewGuild->iGuildRival);

			// Set guild type
			unsigned __int8 iGuildType = (unsigned __int8)m_DBQuery.GetInt("G_Type");
			CGuildManager.SetGuildType(GuildName, iGuildType);

			// Load guild members
			qSql.Format("SELECT * FROM GuildMember where G_Name = '%s'", GuildName);
			m_GuildMemberQry.Exec(qSql);

			__int16 sqlRet2 = m_GuildMemberQry.Fetch();
			while (sqlRet2 != 100 && sqlRet2 != -1)
			{
				unsigned __int8 btGuildStatus = (unsigned __int8)m_GuildMemberQry.GetInt("G_Status");
				m_GuildMemberQry.GetStr("G_Name", GuildName);
				char MemberName[20];
				m_GuildMemberQry.GetStr("Name", MemberName);

				CGuildManager.AddMember(GuildName, MemberName, btGuildStatus, -1);

				sqlRet2 = m_GuildMemberQry.Fetch();
			}
			m_GuildMemberQry.Clear();

			// Update status bar
			CHAR szTemp[264];
			wsprintfA(szTemp, "%d / %d Guild now loading...", CGuildManager.m_count, m_TotalCount);
			//pWnd->DisplayStatusBar(szTemp);

			sqlRet = m_DBQuery.Fetch();
		}
		else
		{
			TRACE_OUT(2, "Can't add Guild [%s][%s][%d][%d]",
				GuildName, Master, iGuildRival, iGuildUnion);
		}
	}

	m_GuildMarkQry.Clear();
	m_DBQuery.Clear();

	// Log completion
	DWORD TickCount = GetTickCount();
	CHAR szTemp[264];
	wsprintfA(szTemp, "Total %d / Guild data load complete. %d(ms)",CGuildManager.m_count, TickCount - starttime);
	cLog.AddC(4, szTemp);
	//pWnd->DisplayStatusBar(szTemp);

	// Load unions, re-enable guild processing, start timer
	ReadAllUnion();
	CGuildManager.GuildOnOff(1);
	pWnd->SetTimer(0x3EA, 0x3E8, NULL);  // Timer ID 1002, interval 1000ms

	return 1;
}
// NOTE:
// ReadAllGuild() was extremely large in the decompilation.
// This is the cleaned reconstruction preserving original behavior.

//int CGuildDBSet::ReadAllGuild()
//{
//	DWORD starttime = GetTickCount();
//
//	CWinApp* pApp = AfxGetApp();
//	CMainFrame* pWnd = (CMainFrame*)pApp->GetMainWnd();
//
//	CGuildManager.GuildOnOff(FALSE);
//
//	CString qSql;
//	qSql.Format("SELECT * FROM Guild");
//
//	if (!this->m_DBQuery.Exec((LPCTSTR)qSql))
//	{
//		cLog.Add("Guild not found");
//		this->m_DBQuery.Clear();
//		CGuildManager.GuildOnOff(TRUE);
//		return 0;
//	}
//
//	short sqlRet = this->m_DBQuery.Fetch();
//
//	while (sqlRet != SQL_NO_DATA && sqlRet != SQL_ERROR)
//	{
//		int Number = this->m_DBQuery.GetInt("Number");
//
//		char GuildName[20] = { 0 };
//		char Master[20] = { 0 };
//		char GuildNotice[72] = { 0 };
//		unsigned char Mark[40] = { 0 };
//
//		this->m_DBQuery.GetStr("G_Name", GuildName);
//		this->m_DBQuery.GetStr("G_Master", Master);
//		this->m_DBQuery.GetStr("G_Notice", GuildNotice);
//
//		int score = this->m_DBQuery.GetInt("G_Score");
//
//		qSql.Format("SELECT G_Mark FROM Guild where G_Name='%s'", GuildName);
//
//		this->m_GuildMarkQry.ReadBlob(qSql, Mark);
//
//		_GUILD_INFO_STRUCT* pNewGuild = CGuildManager.AddGuild(
//			Number,
//			GuildName,
//			Mark,
//			Master,
//			GuildNotice,
//			score,
//			0
//		);
//
//		if (pNewGuild)
//		{
//			pNewGuild->iGuildUnion = this->m_DBQuery.GetInt("G_Union");
//			pNewGuild->iGuildRival = this->m_DBQuery.GetInt("G_Rival");
//
//			BYTE iGuildType = this->m_DBQuery.GetInt("G_Type");
//
//			CGuildManager.SetGuildType(
//				GuildName,
//				iGuildType
//			);
//
//			CString qSqlMember;
//
//			qSqlMember.Format(
//				"SELECT * FROM GuildMember where G_Name = '%s'",
//				GuildName
//			);
//
//			this->m_GuildMemberQry.Exec((LPCTSTR)qSqlMember);
//
//			short sqlRet2 = this->m_GuildMemberQry.Fetch();
//
//			while (sqlRet2 != SQL_NO_DATA && sqlRet2 != SQL_ERROR)
//			{
//				BYTE btGuildStatus = this->m_GuildMemberQry.GetInt("G_Status");
//
//				char MemberName[20] = { 0 };
//
//				this->m_GuildMemberQry.GetStr("Name", MemberName);
//
//				CGuildManager.AddMember(
//					GuildName,
//					MemberName,
//					btGuildStatus,
//					-1
//				);
//
//				sqlRet2 = this->m_GuildMemberQry.Fetch();
//			}
//
//			this->m_GuildMemberQry.Clear();
//		}
//
//		sqlRet = this->m_DBQuery.Fetch();
//	}
//
//	this->m_GuildMarkQry.Clear();
//	this->m_DBQuery.Clear();
//
//	char szTemp[256];
//
//	wsprintf(szTemp, "Total %d / Guild data load complete. %d(ms)", CGuildManager.m_count, GetTickCount() - starttime);
//
//	cLog.AddC(4, szTemp);
//
//	//pWnd->DisplayStatusBar(szTemp);
//
//	this->ReadAllUnion();
//
//	CGuildManager.GuildOnOff(TRUE);
//
//	pWnd->SetTimer(1002, 1000, NULL);
//
//	return 1;
//}

int CGuildDBSet::ScoreUpdate(char* guildName, int score)
{
	if (guildName == 0)
	{
		return 0;
	}

	CAutoSync AutoSync(&this->m_csDBQuery);

	CString qSql;
	qSql.Format("UPDATE Guild SET G_Score=%d WHERE G_Name='%s'", score, guildName);

	if (!this->m_DBQuery.Exec((LPCTSTR)qSql))
	{
		this->m_DBQuery.Clear();
		return 0;
	}

	_GUILD_INFO_STRUCT* lpGuild = CGuildManager.SearchGuild(guildName);

	if (lpGuild == 0)
	{
		cLog.Add("error-L2 : %s Guild not found", guildName);

		this->m_DBQuery.Clear();
		return 0;
	}

	lpGuild->TotalScore = score;

	cLog.Add("[%s] Guild Score Update %d", lpGuild->Name, lpGuild->TotalScore);

	this->m_DBQuery.Clear();

	return 1;
}

int CGuildDBSet::NoticeUpdate(char* guildName, char* guild_notice)
{
	CAutoSync AutoSync(&this->m_csDBQuery);

	CString qSql;
	CString sGuildNotice;

	sGuildNotice = guild_notice;

	// basic SQL escaping
	sGuildNotice.Replace("'", "''");
	sGuildNotice.Replace(";", "''");
	sGuildNotice.Replace("--", "''");

	qSql.Format("UPDATE Guild SET G_Notice='%s' WHERE G_Name='%s'", (LPCSTR)sGuildNotice, guildName);

	if (m_DBQuery.Exec(qSql))
	{
		cLog.Add("Guild notice : %s", guild_notice);

		_GUILD_INFO_STRUCT* lpGuild = CGuildManager.SearchGuild(guildName);

		if (lpGuild)
		{
			strcpy(lpGuild->Notice, guild_notice);
			cLog.Add("[%s] guild notice update %s", lpGuild->Name, lpGuild->Notice);
			m_DBQuery.Clear();
			return TRUE;
		}
		else
		{
			cLog.Add("error : %s Guild not found", guildName);
			m_DBQuery.Clear();
			return FALSE;
		}
	}

	m_DBQuery.Clear();
	return FALSE;
}

BOOL CGuildDBSet::UpdateGuildMemberStatus(char* szGuildName, char* szMemberName, int iGuildStatus)
{
	CString qSql;

	qSql.Format("UPDATE GuildMember SET G_Status=%d WHERE Name='%s'", iGuildStatus, szMemberName);

	CAutoSync AutoSync(&this->m_csUnionQry);

	if (m_UnionQry.Exec(qSql))
	{
		cLog.AddTD("[UPDATE] Guild Member Status : %d", iGuildStatus);

		if (CGuildManager.SetGuildMemberStatus(szGuildName, szMemberName, iGuildStatus))
		{
			m_UnionQry.Clear();

			return TRUE;
		}
		else
		{
			cLog.AddTD("[ERROR] Guild Member Status : %s Guild not found", szGuildName);

			m_UnionQry.Clear();

			return FALSE;
		}
	}

	m_UnionQry.Clear();

	return FALSE;
}

BOOL CGuildDBSet::UpdateGuildType(char* szGuildName, int iGuildType)
{
	CString qSql;

	qSql.Format("UPDATE Guild SET G_Type=%d WHERE G_Name='%s'", iGuildType, szGuildName);

	CAutoSync AutoSync(&this->m_csUnionQry);

	if (m_UnionQry.Exec(qSql))
	{
		cLog.AddTD("[UPDATE] Guild Additional Info #GuildType: %d", iGuildType);

		if (CGuildManager.SetGuildType(szGuildName, iGuildType))
		{
			m_UnionQry.Clear();

			return TRUE;
		}
		else
		{
			cLog.AddTD("[ERROR] Guild Additional Info #GuildType: %s Guild not found", szGuildName);
			m_UnionQry.Clear();
			return FALSE;
		}
	}

	m_UnionQry.Clear();
	return FALSE;
}

BOOL CGuildDBSet::UpdateGuildUnion(int iGuildNumber, int iUnion)
{
	CString qSql;

	qSql.Format("UPDATE Guild SET G_Union=%d WHERE Number=%d", iUnion, iGuildNumber);

	CAutoSync AutoSync(&this->m_csUnionQry);

	if (m_UnionQry.Exec(qSql))
	{
		cLog.AddTD(

			"[UPDATE] Guild Union [ %d ] Join to [ %d ]",
			iGuildNumber,
			iUnion);

		m_UnionQry.Clear();

		return TRUE;
	}

	m_UnionQry.Clear();

	return FALSE;
}

BOOL CGuildDBSet::UpdateGuildRival(int iGuildNumber, int iRival)
{
	CString qSql;

	qSql.Format("UPDATE Guild SET G_Rival=%d WHERE Number=%d", iRival, iGuildNumber);

	CAutoSync AutoSync(&this->m_csUnionQry);

	if (m_UnionQry.Exec((LPCSTR)qSql))
	{
		cLog.AddTD("[UPDATE] Guild Rival [ %d ] Join to [ %d ]", iGuildNumber, iRival);
		m_UnionQry.Clear();
		return TRUE;
	}

	m_UnionQry.Clear();
	return FALSE;
}

int CGuildDBSet::GetDBGuildMemberInfo(char* szGuildMemberName,ST_GUILD_COMMON_DBINFO* stGuildCommonDBInfo)
{
	CString qSql;
	char szMemberName[20];
	CAutoSync AutoSync(&m_csGuildMemberQry);

	// Sanitize member name (10 bytes + null)
	memset(szMemberName, 0, MAX_IDSTRING+1);
	memcpy(szMemberName, szGuildMemberName, MAX_IDSTRING);  // 10 bytes

	qSql.Format("SELECT G_Name, G_Level, G_Status FROM GuildMember where Name='%s'", szMemberName);

	if (!m_GuildMemberQry.Exec(qSql))
	{
		m_GuildMemberQry.Clear();
		return 0;
	}

	__int16 sqlRet = m_GuildMemberQry.Fetch();
	if (sqlRet == 100 || sqlRet == -1)
	{
		m_GuildMemberQry.Clear();
		return 0;
	}

	// Populate output structure
	m_GuildMemberQry.GetStr("G_Name", stGuildCommonDBInfo->m_szGuildName);
	stGuildCommonDBInfo->m_iGuildLevel = m_GuildMemberQry.GetInt("G_Level");
	stGuildCommonDBInfo->m_iGuildMemberStatus = m_GuildMemberQry.GetInt("G_Status");

	m_GuildMemberQry.Clear();
	return 1;
}

//BOOL CGuildDBSet::GetDBGuildMemberInfo(char* szGuildMemberName, ST_GUILD_COMMON_DBINFO* stGuildCommonDBInfo)
//{
//	CAutoSync AutoSync(&this->m_csGuildMemberQry);
//
//	CString qSql;
//
//	char szMemberName[MAX_IDSTRING+1] = { 0 };
//
//	memcpy(szMemberName, szGuildMemberName, MAX_IDSTRING);
//
//	qSql.Format("SELECT G_Name, G_Level, G_Status ""FROM GuildMember WHERE Name='%s'", szMemberName);
//
//	if (!m_GuildMemberQry.Exec(qSql))
//	{
//		m_GuildMemberQry.Clear();
//		return FALSE;
//	}
//
//	SQLRETURN sqlRet = m_GuildMemberQry.Fetch();
//
//	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
//	{
//		m_GuildMemberQry.Clear();
//		return FALSE;
//	}
//
//	m_GuildMemberQry.GetStr("G_Name", stGuildCommonDBInfo->m_szGuildName);
//	stGuildCommonDBInfo->m_iGuildLevel = m_GuildMemberQry.GetInt("G_Level");
//	stGuildCommonDBInfo->m_iGuildMemberStatus = m_GuildMemberQry.GetInt("G_Status");
//	m_GuildMemberQry.Clear();
//
//	return TRUE;
//}

BOOL CGuildDBSet::DelDBGuild(char* szGuildName)
{
	CAutoSync AutoSync(&this->m_csGuildMemberQry);

	CString qSql;

	qSql.Format("DELETE FROM Guild WHERE G_Name='%s'", szGuildName);

	if (m_GuildMemberQry.Exec(qSql))
	{
		m_GuildMemberQry.Clear();

		return TRUE;
	}

	m_GuildMemberQry.Clear();

	return FALSE;
}

int CGuildDBSet::CheckDBGuildMember(char* szGuildMemberName)
{
	CAutoSync AutoSync(&this->m_csGuildMemberQry);

	CString qSql;
	qSql.Format("SELECT Name FROM GuildMember where Name='%s'", szGuildMemberName);

	if (!this->m_GuildMemberQry.Exec(qSql))
	{
		this->m_GuildMemberQry.Clear();
		return 0;
	}

	short sqlRet = this->m_GuildMemberQry.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
	{
		this->m_GuildMemberQry.Clear();
		return 0;
	}

	this->m_GuildMemberQry.Clear();
	return 1;
}

int CGuildDBSet::CheckDBGuildMaster(char* szGuildMasterName)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("SELECT G_Master FROM Guild where G_Master='%s'", szGuildMasterName);

	if (!m_GuildMemberQry.Exec(qSql))
	{
		m_GuildMemberQry.Clear();
		return 0;
	}

	__int16 sqlRet = m_GuildMemberQry.Fetch();
	if (sqlRet == 100 || sqlRet == -1)
	{
		m_GuildMemberQry.Clear();
		return 0;
	}

	// Query succeeded and returned a row - this name IS a guild master
	m_GuildMemberQry.Clear();
	return 1;
}

int CGuildDBSet::AddDBGuildMember(char* szGuildMemberName, char* szGuildName)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("INSERT GuildMember (Name, G_Name) VALUES ('%s','%s')", szGuildMemberName, szGuildName);

	if (m_GuildMemberQry.Exec(qSql))
	{
		m_GuildMemberQry.Clear();
		return 1;
	}
	else
	{
		m_GuildMemberQry.Clear();
		return 0;
	}
}

int CGuildDBSet::DelDBGuildMember(char* szGuildMemberName)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("DELETE FROM GuildMember where Name='%s'", szGuildMemberName);

	if (m_GuildMemberQry.Exec(qSql))
	{
		m_GuildMemberQry.Clear();
		return 1;
	}
	else
	{
		m_GuildMemberQry.Clear();
		return 0;
	}
}

int CGuildDBSet::DelAllDBGuildMember(char* szGuildName)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("DELETE FROM GuildMember where G_Name='%s'", szGuildName);

	if (m_GuildMemberQry.Exec(qSql))
	{
		m_GuildMemberQry.Clear();
		return 1;
	}
	else
	{
		m_GuildMemberQry.Clear();
		return 0;
	}
}

int CGuildDBSet::InsertGuildMemberBuff(char* szGuildName, unsigned short wBuffIndex, unsigned char btEffectType1, unsigned char btEffectType2, unsigned int dwDuration, int lExpireDate)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	_GUILD_INFO_STRUCT* lpGuild = CGuildManager.SearchGuild(szGuildName);
	if (!lpGuild)
	{
		cLog.Add("[PeriodBuff] error : %s Guild not found", szGuildName);
		return 0;
	}

	int iSleepCnt = 0;
	for (int i = 0; i < MAX_GUILD_MEMBER; ++i)
	{
		// Only process slots that are in use and have a name
		if (strlen(lpGuild->Names[i]) == 0 || !lpGuild->Use[i])
			continue;

		// Throttle: sleep 1ms every 10 members to reduce DB load
		if (iSleepCnt == 10)
		{
			iSleepCnt = 0;
			Sleep(1);
		}
		++iSleepCnt;

		qSql.Format("EXEC WZ_PeriodBuffInsert '%s', %d, %d, %d, %d, %d", lpGuild->Names[i], wBuffIndex, btEffectType1, btEffectType2, dwDuration, lExpireDate);

		if (!m_GuildMemberQry.Exec(qSql))
		{
			m_GuildMemberQry.Clear();
			cLog.AddTD(
				"[ERROR] Period Buff Insert Name [%s] GuildName [%s]",
				lpGuild->Names[0],  // Note: original logs Names[0], possibly a bug
				lpGuild->Name);
			return 0;
		}
		m_GuildMemberQry.Clear();
	}

	return 1;
}

int CGuildDBSet::DeleteGuildMemberBuff(unsigned short* wBuffIndex, unsigned char btGuildCnt)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	for (int i = 0; i < btGuildCnt; ++i)
	{
		qSql.Format("EXEC WZ_GuildMemberBuffDelete %d", wBuffIndex[i]);

		if (!m_GuildMemberQry.Exec(qSql))
		{
			m_GuildMemberQry.Clear();
			return 0;
		}
		m_GuildMemberQry.Clear();
	}

	return 1;
}

int CGuildDBSet::GetDBGuildMatchingList(int nPage, _stGuildMatchingList* lpstMatchingList)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("EXEC WZ_GetGuildMatchingList %d, %d", nPage, 9);

	int n = 0;
	if (m_GuildMemberQry.Exec(qSql))
	{
		__int16 sqlRet;
		do
		{
			sqlRet = m_GuildMemberQry.Fetch();
			if (sqlRet == 100 || sqlRet == -1)
				break;

			lpstMatchingList[n].nBoardNumber = m_GuildMemberQry.GetInt("identNo");
			m_GuildMemberQry.GetStr("GuildName", lpstMatchingList[n].szGuildName);
			m_GuildMemberQry.GetStr("GuildMasterName", lpstMatchingList[n].szRegistrant);
			lpstMatchingList[n].nGuildNumber = m_GuildMemberQry.GetInt("GuildNum");
			lpstMatchingList[n].nGuildMasterLevel = m_GuildMemberQry.GetInt("GuildMasterLevel");
			lpstMatchingList[n].btGuildMasterClass = (unsigned __int8)m_GuildMemberQry.GetInt("GuildMasterClass");
			lpstMatchingList[n].btGuildMemberCnt = (unsigned __int8)m_GuildMemberQry.GetInt("GuildMemberCnt");
			m_GuildMemberQry.GetStr("Memo", lpstMatchingList[n].szMemo);
			lpstMatchingList[n].btInterestType = (unsigned __int8)m_GuildMemberQry.GetInt("InterestType");
			lpstMatchingList[n].btLevelRange = (unsigned __int8)m_GuildMemberQry.GetInt("LevelRange");
			lpstMatchingList[n].btClassType = (unsigned __int8)m_GuildMemberQry.GetInt("ClassType");
			lpstMatchingList[n].btGensType = (unsigned __int8)m_GuildMemberQry.GetInt("GensType");

			n++;
		} while (n < 9);
	}

	m_GuildMemberQry.Clear();
	return n;
}

int CGuildDBSet::GetDBGuildMatchingListSearchWord(int nPage, char* szSearchWord, _stGuildMatchingList* lpstMatchingList)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	if (!szSearchWord)
		return 0;

	qSql.Format("EXEC WZ_GetGuildMatchingListKeyword %d, %d, '%s'", nPage, 9, szSearchWord);

	int n = 0;
	if (m_GuildMemberQry.Exec(qSql))
	{
		__int16 sqlRet;
		do
		{
			sqlRet = m_GuildMemberQry.Fetch();
			if (sqlRet == 100 || sqlRet == -1)
				break;

			lpstMatchingList[n].nBoardNumber = m_GuildMemberQry.GetInt("identNo");
			m_GuildMemberQry.GetStr("GuildName", lpstMatchingList[n].szGuildName);
			m_GuildMemberQry.GetStr("GuildMasterName", lpstMatchingList[n].szRegistrant);
			lpstMatchingList[n].nGuildNumber = m_GuildMemberQry.GetInt("GuildNum");
			lpstMatchingList[n].nGuildMasterLevel = m_GuildMemberQry.GetInt("GuildMasterLevel");
			lpstMatchingList[n].btGuildMasterClass = (unsigned __int8)m_GuildMemberQry.GetInt("GuildMasterClass");
			lpstMatchingList[n].btGuildMemberCnt = (unsigned __int8)m_GuildMemberQry.GetInt("GuildMemberCnt");
			m_GuildMemberQry.GetStr("Memo", lpstMatchingList[n].szMemo);
			lpstMatchingList[n].btInterestType = (unsigned __int8)m_GuildMemberQry.GetInt("InterestType");
			lpstMatchingList[n].btLevelRange = (unsigned __int8)m_GuildMemberQry.GetInt("LevelRange");
			lpstMatchingList[n].btClassType = (unsigned __int8)m_GuildMemberQry.GetInt("ClassType");
			lpstMatchingList[n].btGensType = (unsigned __int8)m_GuildMemberQry.GetInt("GensType");

			n++;
		} while (n < 9);
	}

	m_GuildMemberQry.Clear();
	return n;
}

int CGuildDBSet::GetDBGuildMatchingListCount()
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("EXEC WZ_GetGuildMatchingListCount");

	int nCount = 0;
	if (m_GuildMemberQry.Exec(qSql))
	{
		__int16 sqlRet = m_GuildMemberQry.Fetch();
		if (sqlRet != 100 && sqlRet != -1)
		{
			nCount = m_GuildMemberQry.GetInt("Count");
		}
	}

	m_GuildMemberQry.Clear();
	return nCount;
}

int CGuildDBSet::RegGuildMatchingData(_stGuildMatchingList stMatchingList)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	// Prepare sanitized string buffers
	char szGuildName[20];
	char szMemo[52];
	char szRegistrant[20];

	// Copy with explicit size limits (matching the original memcpy sizes)
	memcpy(szGuildName, stMatchingList.szGuildName, 8);
	szGuildName[8] = 0;

	memcpy(szMemo, &stMatchingList, 40);  // 0x28 = 40 bytes
	szMemo[40] = 0;

	memcpy(szRegistrant, stMatchingList.szRegistrant, 0x0A);  // 0x0A = 10 bytes
	szRegistrant[10] = 0;

	qSql.Format("EXEC WZ_InsertGuildMatchingList '%s','%s',%d,%d,%d,%d,'%s',%d,%d,%d,%d",
		szGuildName,
		szRegistrant,
		stMatchingList.nGuildNumber,
		stMatchingList.nGuildMasterLevel,
		stMatchingList.btGuildMasterClass,
		stMatchingList.btGuildMemberCnt,
		szMemo,
		stMatchingList.btInterestType,
		stMatchingList.btLevelRange,
		stMatchingList.btClassType,
		stMatchingList.btGensType);

	if (m_GuildMemberQry.Exec(qSql))
	{
		m_GuildMemberQry.Clear();
		return 1;
	}
	else
	{
		m_GuildMemberQry.Clear();
		return 0;
	}
}

int CGuildDBSet::GuildMatchingRegCheck(int nGuildNum)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("EXEC WZ_GetGuildMatchingRegCheck %d", nGuildNum);

	int nCount = 0;
	if (m_GuildMemberQry.Exec(qSql))
	{
		__int16 sqlRet = m_GuildMemberQry.Fetch();
		if (sqlRet != 100 && sqlRet != -1)
		{
			nCount = m_GuildMemberQry.GetInt("Count");
		}
	}

	m_GuildMemberQry.Clear();
	return nCount;
}

int CGuildDBSet::DeleteGuildMatching(int nGuildNum)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("EXEC WZ_DeleteGuildMatchingData %d", nGuildNum);

	int nResult = 0;
	if (m_GuildMemberQry.Exec(qSql))
	{
		__int16 sqlRet = m_GuildMemberQry.Fetch();
		if (sqlRet != 100 && sqlRet != -1)
		{
			nResult = m_GuildMemberQry.GetInt("Result");
		}
	}

	m_GuildMemberQry.Clear();
	return nResult;
}

int CGuildDBSet::InsertWaitGuildMatching(_stGuildMatchingAllowListDB stAllowList)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	char szApplicantName[20];
	memcpy(szApplicantName, stAllowList.szApplicantName, 0x0A);  // 10 bytes
	szApplicantName[10] = 0;

	qSql.Format(
		"EXEC WZ_InsertGuildMatchingWaitList %d,'%s',%d,%d,%d",
		stAllowList.nGuildNumber,
		szApplicantName,
		stAllowList.btApplicantClass,
		stAllowList.nApplicantLevel,
		stAllowList.btState);

	if (m_GuildMemberQry.Exec(qSql))
	{
		m_GuildMemberQry.Clear();
		return 1;
	}
	else
	{
		m_GuildMemberQry.Clear();
		return 0;
	}
}

int CGuildDBSet::GetGuildMatchingWaitState(char* szName, int nState)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	char szSelectName[20];
	memset(szSelectName, 0, 11);
	memcpy(szSelectName, szName, 0x0A);  // 10 bytes

	qSql.Format("EXEC WZ_GetGuildMatchingWaitState '%s', %d", szSelectName, nState);

	int nResult = -1;
	if (m_GuildMemberQry.Exec(qSql))
	{
		__int16 sqlRet = m_GuildMemberQry.Fetch();
		if (sqlRet != 100 && sqlRet != -1)
		{
			nResult = m_GuildMemberQry.GetInt("State");
		}
	}

	m_GuildMemberQry.Clear();
	return nResult;
}

int CGuildDBSet::DeleteWaitGuildMatching(char* szName)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	char szDeleteName[20];
	memset(szDeleteName, 0, 11);
	memcpy(szDeleteName, szName, 0x0A);  // 10 bytes

	qSql.Format("EXEC WZ_DelGuildMatchingWaitState '%s'", szDeleteName);

	if (m_GuildMemberQry.Exec(qSql))
	{
		m_GuildMemberQry.Clear();
		return 1;
	}
	else
	{
		m_GuildMemberQry.Clear();
		return 0;
	}
}

int CGuildDBSet::GetWaitGuildMatching(int nGuildNumber, int nState, _stGuildMatchingAllowList* lpAllowList)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("EXEC WZ_GetWaitGuildMatching %d, %d", nGuildNumber, nState);

	int n = 0;
	if (m_GuildMemberQry.Exec(qSql))
	{
		__int16 sqlRet;
		for (sqlRet = m_GuildMemberQry.Fetch();
			sqlRet != 100 && sqlRet != -1;
			sqlRet = m_GuildMemberQry.Fetch())
		{
			lpAllowList[n].btClass = (unsigned __int8)m_GuildMemberQry.GetInt("ApplicantClass");
			lpAllowList[n].nLevel = m_GuildMemberQry.GetInt("ApplicantLevel");
			m_GuildMemberQry.GetStr("ApplicantName", lpAllowList[n].szName);
			n++;
		}
	}

	m_GuildMemberQry.Clear();
	return n;
}

int CGuildDBSet::SetWaitGuildMatching(char* szName, int nState)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("EXEC WZ_SetGuildMatchingWaitState '%s', %d", szName, nState);

	int nResult = 0;
	if (m_GuildMemberQry.Exec(qSql))
	{
		__int16 sqlRet = m_GuildMemberQry.Fetch();
		if (sqlRet != 100 && sqlRet != -1)
		{
			nResult = m_GuildMemberQry.GetInt("result");
		}
	}

	m_GuildMemberQry.Clear();
	return nResult;
}

int CGuildDBSet::GetWaitStateListGuildMatching(char* szApplicantName, char* szGuildName, char* szGuildMasterName)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("EXEC WZ_GetGuildMatchingWaitStateList '%s'", szApplicantName);

	int bReturn = 0;
	if (m_GuildMemberQry.Exec(qSql))
	{
		__int16 sqlRet = m_GuildMemberQry.Fetch();
		if (sqlRet != 100 && sqlRet != -1)
		{
			m_GuildMemberQry.GetStr("GuildName", szGuildName);
			m_GuildMemberQry.GetStr("GuildMasterName", szGuildMasterName);
			bReturn = 1;
		}
	}

	m_GuildMemberQry.Clear();
	return bReturn;
}

int CGuildDBSet::GetGuildMatchingAcceptNRejectInfo(char* szName)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("EXEC WZ_GetGuildMatchingAcceptNRejectInfo '%s'", szName);

	int nResult = -1;
	if (m_GuildMemberQry.Exec(qSql))
	{
		__int16 sqlRet = m_GuildMemberQry.Fetch();
		if (sqlRet != 100 && sqlRet != -1)
		{
			nResult = m_GuildMemberQry.GetInt("State");
		}
	}

	m_GuildMemberQry.Clear();
	return nResult;
}

int CGuildDBSet::GetIsApplicantListGuildMatching(int nGuildNumber)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("EXEC WZ_IsApplicantListGuildMatching %d", nGuildNumber);

	int nResult = 0;
	if (m_GuildMemberQry.Exec(qSql))
	{
		__int16 sqlRet = m_GuildMemberQry.Fetch();
		if (sqlRet != 100 && sqlRet != -1)
		{
			nResult = m_GuildMemberQry.GetInt("result");
		}
	}

	m_GuildMemberQry.Clear();
	return nResult;
}

int CGuildDBSet::SetGuildMatchingMemberCount(char* szGuildName, int nMemberCount)
{
	CString qSql;
	CAutoSync AutoSync(&m_csGuildMemberQry);

	qSql.Format("EXEC WZ_UpdateGuildMatchingMemberCount '%s', %d", szGuildName, nMemberCount);

	if (m_GuildMemberQry.Exec(qSql))
	{
		m_GuildMemberQry.Clear();
		return 1;
	}
	else
	{
		m_GuildMemberQry.Clear();
		return 0;
	}
}

int CGuildDBSet::CleanGuildMatchingData()
{
	CString qSql;

	qSql.Format("EXEC WZ_GuildMatching_Clear '%s','%s',?, %d", szSystemMailAdmin, szSystemMailSubject, gExpireDayForCleanGuildMatching);
	m_DBQuery.WriteBlob(qSql, (unsigned __int8*)szSystemMailMemo, 0x3E7);

	return 1;
}