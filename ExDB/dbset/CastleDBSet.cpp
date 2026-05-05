// CastleDBSet.cpp
#include "stdafx.h"
#include "AutoSync.h"
#include "CastleDBSet.h"

// ----------------------------------------------------------------------
// Global instance
// ----------------------------------------------------------------------

CCastleDBSet* CastleDbSet;

// ----------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------

CCastleDBSet::CCastleDBSet()
{
    InitializeCriticalSection(&this->m_csCastleDBQuery);
}

// ----------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------

CCastleDBSet::~CCastleDBSet()
{
    DeleteCriticalSection(&this->m_csCastleDBQuery);
}

// ----------------------------------------------------------------------
// Connect
// ----------------------------------------------------------------------

int CCastleDBSet::Connect()
{
    const char* TableName[] =
    {
        "MuCastle_SIEGE_GUILDLIST"
    };

    const char* SPName[] =
    {
        "WZ_CS_CheckSiegeGuildList"
    };

    if (!this->m_CastleDBQuery.Connect(3,szDbConnectDsn,szDbConnectId,szDbConnectPass))
    {
        MsgBox("Castle DB SET ODBC Connect Fail");
        return 0;
    }

    // ------------------------------------------------------------------
    // Table checks
    // ------------------------------------------------------------------

    for (int n = 0; n < (sizeof(TableName) / sizeof(TableName[0])); n++)
    {
        if (!this->m_CastleDBQuery.TableCheck((char*)TableName[n]))
        {
            MsgBox("[Castle] %s not Table", TableName[n]);
        }
    }

    // ------------------------------------------------------------------
    // Stored procedure checks
    // ------------------------------------------------------------------

    for (int n = 0; n < (sizeof(SPName) / sizeof(SPName[0])); n++)
    {
        if (!this->m_CastleDBQuery.SPCheck((char*)SPName[n]))
        {
            MsgBox("[Castle] %s not script", SPName[n]);
        }
    }

    return 1;
}

// ----------------------------------------------------------------------
// CheckCastleSiegeGuildList
// ----------------------------------------------------------------------

int CCastleDBSet::CheckCastleSiegeGuildList(char* szGuildName)
{
	int bRet = 1;
	int iResult = 1;

	CString qSql;

	qSql.Format("WZ_CS_CheckSiegeGuildList '%s'", szGuildName);
	CAutoSync AutoSync(&this->m_csCastleDBQuery);
	if (this->m_CastleDBQuery.Exec(qSql) == 1)
	{
		short sqlRet = this->m_CastleDBQuery.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_ERROR)
		{
			iResult = this->m_CastleDBQuery.GetInt(1);
			bRet = (iResult != 0);
		}
	}

	cLog.Add("[CastleDBSet] Check CastleSiege GuildList = %d", bRet);
	this->m_CastleDBQuery.Clear();

	return bRet;
}