// PartyMatchingDBSet.cpp
#include "stdafx.h"
#include "AutoSync.h"
#include "PartyMatchingDBSet.h"

CPartyMatchingDBSet g_PartyMatching_DBSet;

CPartyMatchingDBSet::CPartyMatchingDBSet()
{
    InitializeCriticalSection(&this->m_csDBQueryPartyMatching);
}

CPartyMatchingDBSet::~CPartyMatchingDBSet()
{
    DeleteCriticalSection(&this->m_csDBQueryPartyMatching);
}

int CPartyMatchingDBSet::Connect()
{
    if (this->m_PartyMatchingQry.Connect(3, szDbConnectDsn, szDbConnectId, szDbConnectPass))
    {
        this->DeleteAllPartyMatching();
        return 1;
    }

    MsgBox("[DEFAULT] PartyMatching_DBSet ODBC Connect Fail");
    return 0;
}

int CPartyMatchingDBSet::GetRandomMatching(int nUserLevel,BYTE btClass,BYTE btGensType,char* szLeaderNameOutput)
{
    CAutoSync AutoSync(&this->m_csDBQueryPartyMatching);

    CString qSql;
    qSql.Format("EXEC WZ_GetPartyMatchingRandom %d, %d, %d",nUserLevel,btClass,btGensType);

    if (!this->m_PartyMatchingQry.Exec((LPCTSTR)qSql))
    {
        this->m_PartyMatchingQry.Clear();
        return -1;
    }

    short sqlRet = this->m_PartyMatchingQry.Fetch();

    if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
    {
        this->m_PartyMatchingQry.Clear();
        return -1;
    }

    this->m_PartyMatchingQry.GetStr("PartyLeaderName",szLeaderNameOutput);

    this->m_PartyMatchingQry.Clear();

    return 0;
}

int CPartyMatchingDBSet::GetPartyMatchingList(int nPage,_PARTY_INFO_LIST* lpOutPutList)
{
    CAutoSync AutoSync(&this->m_csDBQueryPartyMatching);

    CString qSql;
    qSql.Format("EXEC WZ_GetPartyMatchingList %d, %d",nPage,PARTY_MATCHING_MAX_LIST);

    int n = 0;

    if (this->m_PartyMatchingQry.Exec((LPCTSTR)qSql))
    {
        short sqlRet;

        for (sqlRet = this->m_PartyMatchingQry.Fetch();
            sqlRet != SQL_NO_DATA && sqlRet != SQL_ERROR;
            sqlRet = this->m_PartyMatchingQry.Fetch())
        {
            this->m_PartyMatchingQry.GetStr("PartyLeaderName", lpOutPutList[n].szLeaderName);
            this->m_PartyMatchingQry.GetStr("Title", lpOutPutList[n].szTitle);

            lpOutPutList[n].nMinLevel = this->m_PartyMatchingQry.GetInt("MinLevel");
            lpOutPutList[n].nMaxLevel = this->m_PartyMatchingQry.GetInt("MaxLevel");
            lpOutPutList[n].nHuntingGround = this->m_PartyMatchingQry.GetInt("HuntingGround");

            lpOutPutList[n].btWantedClass = this->m_PartyMatchingQry.GetInt("WantedClass");
            lpOutPutList[n].btCurPartyMemberCnt = this->m_PartyMatchingQry.GetInt("CurMemberCount");
            lpOutPutList[n].btUsePassword = this->m_PartyMatchingQry.GetInt("UsePassWord");

            lpOutPutList[n].btWantedClassDetailInfo[0] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo1");
            lpOutPutList[n].btWantedClassDetailInfo[1] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo2");
            lpOutPutList[n].btWantedClassDetailInfo[2] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo3");
            lpOutPutList[n].btWantedClassDetailInfo[3] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo4");
            lpOutPutList[n].btWantedClassDetailInfo[4] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo5");
            lpOutPutList[n].btWantedClassDetailInfo[5] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo6");
            lpOutPutList[n].btWantedClassDetailInfo[6] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo7");

            lpOutPutList[n].btGensType = this->m_PartyMatchingQry.GetInt("GensType");

            lpOutPutList[n].nServerChennel = this->m_PartyMatchingQry.GetInt("LeaderChannel");
            lpOutPutList[n].nLeaderLevel = this->m_PartyMatchingQry.GetInt("LeaderLevel");

            lpOutPutList[n].btLeaderClass = this->m_PartyMatchingQry.GetInt("LeaderClass");

            n++;
        }
    }

    this->m_PartyMatchingQry.Clear();

    return n;
}

int CPartyMatchingDBSet::GetPartyMatchingListSearchWord(int nPage,char* szSearchWord,_PARTY_INFO_LIST* lpOutPutList)
{
    CAutoSync AutoSync(&this->m_csDBQueryPartyMatching);

    CString qSql;
    qSql.Format("EXEC WZ_GetPartyMatchingListKeyword %d, %d, '%s'",nPage,PARTY_MATCHING_MAX_LIST,szSearchWord);

    int n = 0;

    if (this->m_PartyMatchingQry.Exec((LPCTSTR)qSql))
    {
        short sqlRet;

        for (sqlRet = this->m_PartyMatchingQry.Fetch();
            sqlRet != SQL_NO_DATA && sqlRet != SQL_ERROR;
            sqlRet = this->m_PartyMatchingQry.Fetch())
        {
            this->m_PartyMatchingQry.GetStr("PartyLeaderName", lpOutPutList[n].szLeaderName);
            this->m_PartyMatchingQry.GetStr("Title", lpOutPutList[n].szTitle);

            lpOutPutList[n].nMinLevel = this->m_PartyMatchingQry.GetInt("MinLevel");
            lpOutPutList[n].nMaxLevel = this->m_PartyMatchingQry.GetInt("MaxLevel");
            lpOutPutList[n].nHuntingGround = this->m_PartyMatchingQry.GetInt("HuntingGround");

            lpOutPutList[n].btWantedClass = this->m_PartyMatchingQry.GetInt("WantedClass");
            lpOutPutList[n].btCurPartyMemberCnt = this->m_PartyMatchingQry.GetInt("CurMemberCount");
            lpOutPutList[n].btUsePassword = this->m_PartyMatchingQry.GetInt("UsePassWord");

            lpOutPutList[n].btWantedClassDetailInfo[0] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo1");
            lpOutPutList[n].btWantedClassDetailInfo[1] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo2");
            lpOutPutList[n].btWantedClassDetailInfo[2] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo3");
            lpOutPutList[n].btWantedClassDetailInfo[3] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo4");
            lpOutPutList[n].btWantedClassDetailInfo[4] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo5");
            lpOutPutList[n].btWantedClassDetailInfo[5] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo6");
            lpOutPutList[n].btWantedClassDetailInfo[6] = this->m_PartyMatchingQry.GetInt("WantedClassDetailInfo7");

            lpOutPutList[n].btGensType = this->m_PartyMatchingQry.GetInt("GensType");

            lpOutPutList[n].nServerChennel = this->m_PartyMatchingQry.GetInt("LeaderChannel");
            lpOutPutList[n].nLeaderLevel = this->m_PartyMatchingQry.GetInt("LeaderLevel");

            lpOutPutList[n].btLeaderClass = this->m_PartyMatchingQry.GetInt("LeaderClass");

            n++;
        }
    }

    this->m_PartyMatchingQry.Clear();

    return n;
}

int CPartyMatchingDBSet::GetPartyMatchingListJoinAble(int nPage, BYTE btClass, int nLevel, BYTE btGens, _PARTY_INFO_LIST* lpOutPutList)
{
    CAutoSync AutoSync(&m_csDBQueryPartyMatching);
    CString qSql;
    int n = 0;

    qSql.Format("EXEC WZ_GetPartyMatchingListJoinAble %d, %d,%d,%d, %d", nLevel, btClass, btGens, nPage, 6);

    if (m_PartyMatchingQry.Exec(qSql))
    {
        __int16 sqlRet = m_PartyMatchingQry.Fetch();
        while (sqlRet != 100 && sqlRet != -1)
        {
            m_PartyMatchingQry.GetStr("PartyLeaderName", lpOutPutList[n].szLeaderName);
            m_PartyMatchingQry.GetStr("Title", lpOutPutList[n].szTitle);
            lpOutPutList[n].nMinLevel = m_PartyMatchingQry.GetInt("MinLevel");
            lpOutPutList[n].nMaxLevel = m_PartyMatchingQry.GetInt("MaxLevel");
            lpOutPutList[n].nHuntingGround = m_PartyMatchingQry.GetInt("HuntingGround");
            lpOutPutList[n].btWantedClass = m_PartyMatchingQry.GetInt("WantedClass");
            lpOutPutList[n].btCurPartyMemberCnt = m_PartyMatchingQry.GetInt("CurMemberCount");
            lpOutPutList[n].btUsePassword = m_PartyMatchingQry.GetInt("UsePassWord");
            lpOutPutList[n].btWantedClassDetailInfo[0] = m_PartyMatchingQry.GetInt("WantedClassDetailInfo1");
            lpOutPutList[n].btWantedClassDetailInfo[1] = m_PartyMatchingQry.GetInt("WantedClassDetailInfo2");
            lpOutPutList[n].btWantedClassDetailInfo[2] = m_PartyMatchingQry.GetInt("WantedClassDetailInfo3");
            lpOutPutList[n].btWantedClassDetailInfo[3] = m_PartyMatchingQry.GetInt("WantedClassDetailInfo4");
            lpOutPutList[n].btWantedClassDetailInfo[4] = m_PartyMatchingQry.GetInt("WantedClassDetailInfo5");
            lpOutPutList[n].btWantedClassDetailInfo[5] = m_PartyMatchingQry.GetInt("WantedClassDetailInfo6");
            lpOutPutList[n].btWantedClassDetailInfo[6] = m_PartyMatchingQry.GetInt("WantedClassDetailInfo7");
            lpOutPutList[n].btGensType = m_PartyMatchingQry.GetInt("GensType");
            lpOutPutList[n].nServerChennel = m_PartyMatchingQry.GetInt("LeaderChannel");
            lpOutPutList[n].nLeaderLevel = m_PartyMatchingQry.GetInt("LeaderLevel");
            lpOutPutList[n].btLeaderClass = m_PartyMatchingQry.GetInt("LeaderClass");

            n++;
            sqlRet = m_PartyMatchingQry.Fetch();
        }
    }

    m_PartyMatchingQry.Clear();
    return n;
}

int CPartyMatchingDBSet::GetPartyMatchingListJoinAbleTotalCount(BYTE btClass, int nLevel, BYTE btGens)
{
    CAutoSync autosync(&m_csDBQueryPartyMatching);
    CString qSql;
    int nCount = 0;

    qSql.Format("EXEC WZ_GetPartyMatchingListJoinAbleTotalCount %d, %d,%d", nLevel, btClass, btGens);

    if (m_PartyMatchingQry.Exec(qSql))
    {
        __int16 sqlRet = m_PartyMatchingQry.Fetch();
        if (sqlRet != 100 && sqlRet != -1)
        {
            nCount = m_PartyMatchingQry.GetInt("nCount");
        }
    }

    m_PartyMatchingQry.Clear();
    return nCount;
}
int CPartyMatchingDBSet::GetPartyMatchingListCount()
{
    CString qSql;
    int nCount = 0;

    qSql.Format("EXEC WZ_GetPartyMatchingListCount");

    if (m_PartyMatchingQry.Exec(qSql))
    {
        __int16 sqlRet = m_PartyMatchingQry.Fetch();
        if (sqlRet != 100 && sqlRet != -1)
        {
            nCount = m_PartyMatchingQry.GetInt("Count");
        }
    }

    m_PartyMatchingQry.Clear();
    return nCount;
}

int CPartyMatchingDBSet::InsertPartyMatchingList(char* szLeaderName, char* szTitle, char* szPassWord, short nMinLevel, short nMaxLevel, short nHuntingGround, BYTE btWantedClass, BYTE btCurMemberCount, BYTE btUsePassWord, BYTE btAcceptType, BYTE* lpbtWantedClassDetailInfo, int nServerChannel, BYTE btGensType, short nLeaderLevel, BYTE btLeaderClass)
{
    CAutoSync AutoSync(&m_csDBQueryPartyMatching);
    CString qSql;

    char szTitleDB[52];
    memset(szTitleDB, 0, sizeof(szTitleDB));
    memcpy(szTitleDB, szTitle, 40); // 0x28 = 40
    szTitleDB[40] = '\0'; // Ensure null termination for safety

    qSql.Format(
        "EXEC WZ_InsertPartyMatchingList '%s', '%s','%s', %d, %d, %d, %d,%d, %d,%d , %d,%d,%d,%d,%d,%d,%d, %d,%d,%d,%d",
        szLeaderName,
        szTitleDB,
        szPassWord,
        nMinLevel,
        nMaxLevel,
        nHuntingGround,
        btWantedClass,
        btCurMemberCount,
        btUsePassWord,
        btAcceptType,
        lpbtWantedClassDetailInfo[0],
        lpbtWantedClassDetailInfo[1],
        lpbtWantedClassDetailInfo[2],
        lpbtWantedClassDetailInfo[3],
        lpbtWantedClassDetailInfo[4],
        lpbtWantedClassDetailInfo[5],
        lpbtWantedClassDetailInfo[6],
        nServerChannel,
        btGensType,
        nLeaderLevel,
        btLeaderClass
    );

    if (!m_PartyMatchingQry.Exec(qSql))
    {
        m_PartyMatchingQry.Clear();
        return -1;
    }

    __int16 sqlRet = m_PartyMatchingQry.Fetch();
    if (sqlRet == 100 || sqlRet == -1)
    {
        m_PartyMatchingQry.Clear();
        return -1;
    }

    int nResult = m_PartyMatchingQry.GetInt("result");
    m_PartyMatchingQry.Clear();
    return nResult;
}

int CPartyMatchingDBSet::GetPartyMatchingPasswordInfo(char* szLeaderName, char* szPassWordOut, BYTE* btUsePassWordOut, BYTE* btAcceptType)
{
    CAutoSync AutoSync(&m_csDBQueryPartyMatching);
    CString qSql;

    qSql.Format("EXEC WZ_GetPartyMatchingPassWordInfo '%s'", szLeaderName);

    if (!m_PartyMatchingQry.Exec(qSql))
    {
        m_PartyMatchingQry.Clear();
        return -1;
    }

    __int16 sqlRet = m_PartyMatchingQry.Fetch();
    if (sqlRet == 100 || sqlRet == -1)
    {
        m_PartyMatchingQry.Clear();
        return -1;
    }

    m_PartyMatchingQry.GetStr("PassWord", szPassWordOut);
    *btUsePassWordOut = m_PartyMatchingQry.GetInt("UsePassWord");
    *btAcceptType = m_PartyMatchingQry.GetInt("AcceptType");
    m_PartyMatchingQry.Clear();
    return 0;
}

int CPartyMatchingDBSet::InsertWaitPartyMatching(char* szLeaderName, char* szMemberName, BYTE btClass, int nLevel, int nUserDBNumber)
{
    CAutoSync AutoSync(&m_csDBQueryPartyMatching);
    CString qSql;

    qSql.Format("EXEC WZ_InsertWaitPartyMatching '%s','%s',%d,%d,%d", szLeaderName, szMemberName, btClass, nLevel, nUserDBNumber);

    if (!m_PartyMatchingQry.Exec(qSql))
    {
        m_PartyMatchingQry.Clear();
        return -1;
    }

    __int16 sqlRet = m_PartyMatchingQry.Fetch();
    if (sqlRet == 100 || sqlRet == -1)
    {
        m_PartyMatchingQry.Clear();
        return -1;
    }

    int nResult = m_PartyMatchingQry.GetInt("result");
    m_PartyMatchingQry.Clear();
    return nResult;
}

int CPartyMatchingDBSet::GetPartyMatchingWaitList(char* szMemberName, char* szLeaderName)
{
    CAutoSync AutoSync(&m_csDBQueryPartyMatching);
    CString qSql;

    qSql.Format("EXEC WZ_GetPartyMatchingWaitList '%s'", szMemberName);

    if (!m_PartyMatchingQry.Exec(qSql))
    {
        m_PartyMatchingQry.Clear();
        return -1;
    }

    __int16 sqlRet = m_PartyMatchingQry.Fetch();
    if (sqlRet == 100 || sqlRet == -1)
    {
        m_PartyMatchingQry.Clear();
        return -1;
    }

    m_PartyMatchingQry.GetStr("LeaderName", szLeaderName);
    m_PartyMatchingQry.Clear();
    return 0;
}

int CPartyMatchingDBSet::GetPartyMatchingWaitListForLeader(char* szLeaderName, _PARTY_MEMBER_WAIT_LIST* stList)
{
    CAutoSync AutoSync(&m_csDBQueryPartyMatching);
    CString qSql;
    int n = 0;

    qSql.Format("EXEC WZ_GetPartyMatchingWaitListForLeader '%s'", szLeaderName);

    if (m_PartyMatchingQry.Exec(qSql))
    {
        __int16 sqlRet = m_PartyMatchingQry.Fetch();
        while (sqlRet != 100 && sqlRet != -1)
        {
            m_PartyMatchingQry.GetStr("MemberName", stList[n].szName);
            stList[n].btClass = m_PartyMatchingQry.GetInt("Class");
            stList[n].nLevel = m_PartyMatchingQry.GetInt("MemberLevel");
            n++;
            sqlRet = m_PartyMatchingQry.Fetch();
        }
    }

    m_PartyMatchingQry.Clear();
    return n;
}

void CPartyMatchingDBSet::DeleteAllPartyMatching()
{
    CAutoSync AutoSync(&m_csDBQueryPartyMatching);
    CString qSql;

    qSql.Format("EXEC WZ_DeleteAllPartyMatchingList");
    m_PartyMatchingQry.Exec(qSql);
    m_PartyMatchingQry.Clear();
}

int CPartyMatchingDBSet::UpdatePartyMatchingList(char* szLeaderName, char* szNewLeaderName, int nServerChannel, short nLeaderLevel, BYTE btLeaderClass)
{
    CAutoSync AutoSync(&this->m_csDBQueryPartyMatching);

    CString qSql;

    qSql.Format(
        "EXEC WZ_UpdatePartyMatchingList '%s', '%s', %d, %d, %d",
        szLeaderName,
        szNewLeaderName,
        nServerChannel + 1,
        nLeaderLevel,
        btLeaderClass);

    if (this->m_PartyMatchingQry.Exec((LPCTSTR)qSql))
    {
        this->m_PartyMatchingQry.Clear();
        return -1;
    }

    this->m_PartyMatchingQry.Clear();

    return 0;
}

int CPartyMatchingDBSet::DeletePartyMatching(char* szLeaderName)
{
    CAutoSync AutoSync(&m_csDBQueryPartyMatching);
    CString qSql;

    qSql.Format("EXEC WZ_DeletePartyMatchingList '%s'", szLeaderName);

    if (m_PartyMatchingQry.Exec(qSql))
    {
        m_PartyMatchingQry.Clear();
        return 1;
    }
    else
    {
        m_PartyMatchingQry.Clear();
        return 0;
    }
}

int CPartyMatchingDBSet::DeleteWaitList(char* szMemberName)
{
    CAutoSync AutoSync(&m_csDBQueryPartyMatching);
    CString qSql;

    qSql.Format(" EXEC WZ_DeleteWaitPartyMatchingList '%s' ", szMemberName);

    if (m_PartyMatchingQry.Exec(qSql))
    {
        m_PartyMatchingQry.Clear();
        return 1;
    }
    else
    {
        m_PartyMatchingQry.Clear();
        return 0;
    }
}

int CPartyMatchingDBSet::UpdatePartyMemberCount(char* szLeaderName, int nMemberCount)
{
    CAutoSync AutoSync(&m_csDBQueryPartyMatching);
    CString qSql;

    qSql.Format(" EXEC WZ_UpdatePartyMatchingMemberCount '%s', %d ", szLeaderName, nMemberCount);

    if (m_PartyMatchingQry.Exec(qSql))
    {
        m_PartyMatchingQry.Clear();
        return 1;
    }
    else
    {
        m_PartyMatchingQry.Clear();
        return 0;
    }
}

int CPartyMatchingDBSet::GetPartyMatchingWaitMemberInfo(char* szMemberName, int* nMemberLevel, unsigned __int8* btMemberClass)
{
	CString qSql;
	CAutoSync AutoSync(&m_csDBQueryPartyMatching);

	qSql.Format("EXEC WZ_GetPartyMatchingWaitMemberInfo '%s'", szMemberName);

	if (m_PartyMatchingQry.Exec(qSql))
	{
		__int16 sqlRet = m_PartyMatchingQry.Fetch();
		if (sqlRet != 100 && sqlRet != -1)
		{
			*btMemberClass = (unsigned __int8)m_PartyMatchingQry.GetInt("Class");
			*nMemberLevel = m_PartyMatchingQry.GetInt("MemberLevel");
		}

		m_PartyMatchingQry.Clear();
		return 0;
	}
	else
	{
		m_PartyMatchingQry.Clear();
		return -1;
	}
}
