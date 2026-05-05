// FriendDBSet.cpp
#include "stdafx.h"
#include "AutoSync.h"
#include "FriendDBSet.h"
#include "..\\common\\winutil.h"

CFriendDBSet* FriendDbSet;

CFriendDBSet::CFriendDBSet()
{
	InitializeCriticalSection(&this->m_csDBQueryFriend);
	InitializeCriticalSection(&this->m_csDBQueryMail);
}

CFriendDBSet::~CFriendDBSet()
{
	DeleteCriticalSection(&this->m_csDBQueryFriend);
	DeleteCriticalSection(&this->m_csDBQueryMail);
}

int CFriendDBSet::Connect()
{
	const char* TableName[] =
	{
		"T_CGuid",
		"T_FriendList",
		"T_FriendMail",
		"T_FriendMain",
		"T_WaitFriend"
	};

	const char* SPName[] =
	{
		"WZ_FriendAdd",
		"WZ_FriendDel",
		"WZ_WaitFriendAdd",
		"WZ_GuildCreate",
		"WZ_WaitFriendDel",
		"WZ_WriteMail",
		"WZ_DelMail"
	};

	if (!this->m_DBQueryFriend.Connect(3, szDbConnectDsn, szDbConnectId, szDbConnectPass))
	{
		MsgBox("Friend DB SET ODBC Connect Fail");
		return FALSE;
	}

	if (!this->m_DBQueryMail.Connect(3, szDbConnectDsn, szDbConnectId, szDbConnectPass))
	{
		MsgBox("Friend DB SET ODBC Connect Fail");
		return FALSE;
	}

	for (int n = 0; n < 5; n++)
	{
		if (!this->m_DBQueryFriend.TableCheck((char*)TableName[n]))
		{
			MsgBox("[Friend] %s not Table", TableName[n]);
		}
	}

	for (int n = 0; n < 7; n++)
	{
		if (!this->m_DBQueryFriend.SPCheck((char*)SPName[n]))
		{
			MsgBox("[Friend] %s not script", SPName[n]);
		}
	}

	return TRUE;
}

int CFriendDBSet::CreateGuid(char* szName)
{
	CString qSql;

	CAutoSync AutoSync(&this->m_csDBQueryFriend);

	qSql.Format("WZ_UserGuidCreate '%s'", szName);

	if (this->m_DBQueryFriend.Exec((LPSTR)(LPCSTR)qSql))
	{
		this->m_DBQueryFriend.Clear();
		return TRUE;
	}

	this->m_DBQueryFriend.Clear();
	return FALSE;
}

int CFriendDBSet::GetGuid(char* szName, int* memoCount)
{
	CString qSql;

	CAutoSync AutoSync(&this->m_csDBQueryFriend);

	qSql.Format(
		"SELECT GUID, MemoTotal FROM T_FriendMain WHERE Name='%s'",
		szName);

	if (!this->m_DBQueryFriend.Exec((LPSTR)(LPCSTR)qSql))
	{
		this->m_DBQueryFriend.Clear();
		return -1;
	}

	short sqlRet = this->m_DBQueryFriend.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
	{
		this->m_DBQueryFriend.Clear();
		return -1;
	}

	int guid = this->m_DBQueryFriend.GetInt(1);

	*memoCount = this->m_DBQueryFriend.GetInt(2);

	this->m_DBQueryFriend.Clear();

	return guid;
}

int CFriendDBSet::GetFriendList(char* szName, _FRIEND_INFO_STRUCT* lpNode)
{
	CString qSql;

	int memoCount = 0;

	int guid = this->GetGuid(szName, &memoCount);

	if (guid < 0)
	{
		if (!this->CreateGuid(szName))
		{
			return FALSE;
		}

		guid = this->GetGuid(szName, &memoCount);

		if (guid < 0)
		{
			return FALSE;
		}
	}

	//
	// ORIGINAL EXDB STARTS FROM INDEX 1
	//
	for (int i = 1; i < 50; i++)
	{
		lpNode->Use[i] = -1;
		lpNode->Names[i][0] = 0;
	}

	CAutoSync AutoSync(&this->m_csDBQueryFriend);

	lpNode->GUID = guid;

	qSql.Format(
		"SELECT FriendGuid, FriendName, Del FROM T_FriendList WHERE GUID=%d",
		guid);

	if (!this->m_DBQueryFriend.Exec(qSql))
	{
		this->m_DBQueryFriend.Clear();
		return FALSE;
	}

	//
	// ORIGINAL STARTS AT 1
	//
	int count = 1;

	while (true)
	{
		short sqlRet = this->m_DBQueryFriend.Fetch();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
		{
			break;
		}

		int FriendGuid = this->m_DBQueryFriend.GetInt(1);

		char szFriendName[20] = { 0 };

		this->m_DBQueryFriend.GetStr("FriendName", szFriendName);

		int del = this->m_DBQueryFriend.GetInt(3);

		lpNode->Use[count] = (del == 1) ? 30000 : 1;

		if (strlen(szFriendName) > 0)
		{
			strcpy(lpNode->Names[count], szFriendName);

			cLog.Add("Guid:[%d] Name:[%s]", FriendGuid, szFriendName);

			if (count >= 49)
			{
				cLog.AddTD("error-L1 : SQL Max friend (%d)", count);
				break;
			}

			count++;
		}
	}

	lpNode->Count = count;
	lpNode->MailCount = memoCount;

	this->m_DBQueryFriend.Clear();

	return TRUE;
}

int CFriendDBSet::FriendAdd(char* szName, char* szFriendName)
{
	CString qSql;
	CAutoSync AutoSync(&this->m_csDBQueryFriend);
	qSql.Format("WZ_FriendAdd '%s', '%s'", szName, szFriendName);

	if (!this->m_DBQueryFriend.Exec(qSql))
	{
		return 0;
	}

	short sqlRet = this->m_DBQueryFriend.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
	{
		cLog.AddTD("error-L3 : Friend Add fail %s %s", szName, szFriendName);
		this->m_DBQueryFriend.Clear();
		return 0;
	}

	int result = this->m_DBQueryFriend.GetInt(1);
	this->m_DBQueryFriend.Clear();
	cLog.AddTD("friend add result %s %s %d", szName, szFriendName, result);

	return result;
}

unsigned char CFriendDBSet::FriendWaitAdd(char* szName, char* szFriendName)
{
	CString qSql;
	CAutoSync AutoSync(&this->m_csDBQueryFriend);
	qSql.Format("WZ_WaitFriendAdd '%s', '%s'", szFriendName, szName);

	if (!this->m_DBQueryFriend.Exec((LPSTR)(LPCSTR)qSql))
	{
		cLog.AddTD("WZ_WaitFriendAdd run fail %s %s", szName, szFriendName);
		this->m_DBQueryFriend.Clear();
		return 0;
	}

	short sqlRet = this->m_DBQueryFriend.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
	{
		cLog.AddTD("error-L3 : friend wait add fail %s %s", szName, szFriendName);
		this->m_DBQueryFriend.Clear();
		return 0;
	}

	int result = this->m_DBQueryFriend.GetInt(1);

	this->m_DBQueryFriend.Clear();

	if (result == 0)
	{
		cLog.AddTD("WZ_WaitFriendAdd success %s %s", szName, szFriendName);
		return 1;
	}

	if (result == 5 || result == 6)
	{
		return (unsigned char)result;
	}

	cLog.AddTD("WZ_WaitFriendAdd fail %d", result);

	return 0;
}

int CFriendDBSet::FriendDel(char* szName, char* szFriendName)
{
	int result = 0;
	CString qSql;

	CAutoSync CAutoSync(&m_csDBQueryFriend);

	qSql.Format("WZ_FriendDel '%s', '%s'", szName, szFriendName);

	if (m_DBQueryFriend.Exec(qSql) == 1)
	{
		__int16 sqlRet = m_DBQueryFriend.Fetch();

		if (sqlRet == 100 || sqlRet == -1)
		{
			cLog.AddTD("error-L3 : WZ_FriendDel fail %s %s", szName, szFriendName);
			m_DBQueryFriend.Clear();
			return 0;
		}

		result = m_DBQueryFriend.GetInt(1);
		m_DBQueryFriend.Clear();

		if (result == 5)
		{
			cLog.AddTD("WZ_FriendDel fail unknown Friend Del %d ", 5);
		}
		else if (result != 1)
		{
			cLog.AddTD("WZ_FriendDel fail #2 %d ", result);
			return 0;
		}

		cLog.AddTD("WZ_FriendDel success %s %s ", szName, szFriendName);
		return 1;
	}
	else
	{
		// Note: Original code logs success even when Exec fails,
		// which might be a bug or intentional behavior for stored procedure
		m_DBQueryFriend.Clear();
		cLog.AddTD("WZ_FriendDel success %s %s ", szName, szFriendName);
		return 1;
	}
}

int CFriendDBSet::WaitFriendDel(char* szName, char* szFriendName)
{
	int result = 0;
	CString qSql;

	CAutoSync CAutoSync(&m_csDBQueryFriend);

	qSql.Format("WZ_WaitFriendDel '%s', '%s'", szName, szFriendName);

	if (m_DBQueryFriend.Exec(qSql) == 1)
	{
		__int16 sqlRet = m_DBQueryFriend.Fetch();

		if (sqlRet == 100 || sqlRet == -1)
		{
			cLog.AddTD("error-L3 : WZ_WaitFriendDel fail %s %s", szName, szFriendName);
			m_DBQueryFriend.Clear();
			return 0;
		}

		result = m_DBQueryFriend.GetInt(1);
		m_DBQueryFriend.Clear();

		if (result != 0)
		{
			cLog.AddTD("WZ_WaitFriendDel fail #2 %d ", result);
			return 0;
		}

		cLog.AddTD("WZ_WaitFriendDel success %s %s ", szName, szFriendName);
		return 1;
	}
	else
	{
		m_DBQueryFriend.Clear();
		cLog.AddTD("WZ_WaitFriendDel run fail %s %s ", szName, szFriendName);
		return 0;
	}
}

int CFriendDBSet::GetWaitFriend(int GUID, char* szFriendName)
{
	int result = 0;
	CString qSql;

	CAutoSync CAutoSync(&m_csDBQueryFriend);

	qSql.Format("SELECT FriendName FROM T_WaitFriend where GUID=%d", GUID);

	if (m_DBQueryFriend.Exec(qSql) != 1)
	{
		m_DBQueryFriend.Clear();
		return 0;
	}

	__int16 sqlRet = m_DBQueryFriend.Fetch();
	if (sqlRet == 100 || sqlRet == -1)
	{
		m_DBQueryFriend.Clear();
		return 0;
	}

	cLog.AddTD("wait friend : [%d] %s", GUID, szFriendName);
	m_DBQueryFriend.GetStr("FriendName", szFriendName);
	m_DBQueryFriend.Clear();
	return 1;
}
//
//int CFriendDBSet::ReadMemoSubject(int aIndex, char* UserName, unsigned __int16 UserIndex, int guid)
//{
//    int result = 0;
//    CString qSql;
//    char_ID szFriendName;
//
//    CAutoSync CAutoSync(&m_csDBQueryFriend);
//
//    qSql.Format("SELECT MemoIndex, FriendName, wDate, Subject, bRead FROM T_FriendMail where GUID=%d", guid);
//
//    if (m_DBQueryMail.Exec(qSql) == 1)
//    {
//        __int16 sqlRet;
//        while (true)
//        {
//            sqlRet = m_DBQueryMail.Fetch();
//            if (sqlRet == 100 || sqlRet == -1)
//                break;
//
//            int MemoIndex = m_DBQueryMail.GetInt(1);
//            m_DBQueryMail.GetStr(2, szFriendName.Name);
//            char szDate[40];
//            m_DBQueryMail.GetStr(3, szDate);
//            char szSubject[68];
//            m_DBQueryMail.GetStr(4, szSubject);
//            unsigned __int8 read = (unsigned __int8)m_DBQueryMail.GetInt(5);
//
//            DGFriendMemoList(aIndex, UserIndex, MemoIndex, szFriendName.Name, UserName, szDate, szSubject, read);
//            cLog.Add("%s %s %s %s", UserName, szFriendName.Name, szDate, szSubject);
//        }
//    }
//
//    m_DBQueryMail.Clear();
//    return 1;
//}

int CFriendDBSet::ReadMemoSubject(int aIndex, char* UserName, unsigned short UserIndex, int guid)
{
	CString qSql;

	char_ID szFriendName;

	char szDate[40] = { 0 };
	char szSubject[68] = { 0 };

	CAutoSync AutoSync(&this->m_csDBQueryMail);

	qSql.Format("SELECT MemoIndex, FriendName, wDate, Subject, bRead ""FROM T_FriendMail WHERE GUID=%d", guid);

	if (this->m_DBQueryMail.Exec(qSql) == TRUE)
	{
		while (true)
		{
			short sqlRet = this->m_DBQueryMail.Fetch();

			if (sqlRet == SQL_NO_DATA || sqlRet == SQL_ERROR)
			{
				break;
			}

			int MemoIndex = this->m_DBQueryMail.GetInt(1);

			this->m_DBQueryMail.GetStr(2, szFriendName.GetBuffer());
			this->m_DBQueryMail.GetStr(3, szDate);
			this->m_DBQueryMail.GetStr(4, szSubject);

			BYTE read = (BYTE)this->m_DBQueryMail.GetInt(5);

			DGFriendMemoList(aIndex, UserIndex, MemoIndex, szFriendName.GetBuffer(), UserName, szDate, szSubject, read);

			cLog.Add("%s %s %s %s", UserName, szFriendName.GetBuffer(), szDate, szSubject);
		}
	}

	this->m_DBQueryMail.Clear();

	return TRUE;
}

int CFriendDBSet::WriteMemo(char* SendName, char* RecvName, char* subject, char* memo, unsigned char* lpPhoto, unsigned char Dir, unsigned char Action)
{
	CString qSql;
	int userguid = -1;

	if (!SQLSyntexCheckConvert(subject) || !SQLSyntexCheckConvert(SendName) || !SQLSyntexCheckConvert(RecvName))
	{
		return -4;
	}

	CAutoSync CAutoSync(&m_csDBQueryFriend);

	qSql.Format("WZ_WriteMail '%s', '%s', '%s', %d, %d", SendName, RecvName, subject, Dir, Action);

	if (!m_DBQueryMail.Exec(qSql))
	{
		cLog.Add("error-L3 : %s %d ", __FILE__, __LINE__);
		m_DBQueryMail.Clear();
		return -1;
	}

	__int16 sqlRet = m_DBQueryMail.Fetch();
	if (sqlRet == 100 || sqlRet == -1)
	{
		cLog.Add("error-L3 : sql_no_data : %s %d ", __FILE__, __LINE__);
		m_DBQueryMail.Clear();
		return -1;
	}

	int memoindex = m_DBQueryMail.GetInt(1);

	if (memoindex <= 10)
	{
		m_DBQueryMail.Clear();
		switch (memoindex)
		{
		case 2:
		case 3:
			return -3;
		case 5:
			return -2;
		case 6:
			return -6;
		default:
			cLog.Add("error-L3 : memoindex : %d   %s %d ", memoindex, __FILE__, __LINE__);
			return -1;
		}
	}

	userguid = m_DBQueryMail.GetInt(2);
	if (userguid < 0)
	{
		cLog.Add("error-L3 : userguid : %d   %s %d ", userguid, __FILE__, __LINE__);
		m_DBQueryMail.Clear();
		return -1;
	}

	m_DBQueryMail.Clear();

	// Write Photo blob
	qSql.Format("UPDATE T_FriendMail SET Photo=? where MemoIndex=%d AND GUID=%d", memoindex, userguid);
	m_DBQueryMail.WriteBlob(qSql, lpPhoto, 0x12);

	// Write Memo blob
	SQLUINTEGER memosize = (SQLUINTEGER)strlen(memo);
	qSql.Format("UPDATE T_FriendMail SET Memo=? where MemoIndex=%d AND GUID=%d", memoindex, userguid);
	m_DBQueryMail.WriteBlob(qSql, (unsigned __int8*)memo, memosize);

	return memoindex;
}

int CFriendDBSet::ReadMemo(int aIndex, int memoIndex, int guid, char* Memo, BYTE* Photo, BYTE* Dir, BYTE* Act, int* strsize)
{
	CString qSql;

	int read = 0;

	CAutoSync AutoSync(&this->m_csDBQueryMail);

	qSql.Format("SELECT bRead, Dir, Act ""FROM T_FriendMail ""WHERE MemoIndex=%d AND GUID=%d", memoIndex, guid);

	if (this->m_DBQueryMail.Exec((LPSTR)(LPCSTR)qSql) != TRUE)
	{
		this->m_DBQueryMail.Clear();
		return FALSE;
	}

	short sqlRet = this->m_DBQueryMail.Fetch();

	if (sqlRet != SQL_NO_DATA && sqlRet != SQL_ERROR)
	{
		read = this->m_DBQueryMail.GetInt(1);

		int ret = this->m_DBQueryMail.GetInt(2);

		if (ret >= 0)
		{
			*Dir = (BYTE)ret;
		}

		ret = this->m_DBQueryMail.GetInt(3);

		if (ret >= 0)
		{
			*Act = (BYTE)ret;
		}
	}

	this->m_DBQueryMail.Clear();

	cLog.Add("ReadMemo MemoIndex:%d", memoIndex);

	qSql.Format("SELECT Photo FROM T_FriendMail ""WHERE MemoIndex='%d' AND GUID=%d", memoIndex, guid);

	memset(Photo, 0, 0x12);

	this->m_DBQueryMail.ReadBlob(qSql, Photo);

	qSql.Format("SELECT Memo FROM T_FriendMail ""WHERE MemoIndex='%d' AND GUID=%d", memoIndex, guid);

	*strsize = this->m_DBQueryMail.ReadBlob(qSql, (BYTE*)Memo);

	if (!read)
	{
		qSql.Format("UPDATE T_FriendMail ""SET bRead=1 ""WHERE MemoIndex=%d AND GUID=%d", memoIndex, guid);

		this->m_DBQueryMail.Exec((LPSTR)(LPCSTR)qSql);
		this->m_DBQueryMail.Clear();
	}

	return TRUE;
}

int CFriendDBSet::DelMemo(char* szName, int memoIndex)
{
	int result = 0;
	CString qSql;

	qSql.Format("exec WZ_DelMail '%s', %d", szName, memoIndex);

	CAutoSync CAutoSync(&m_csDBQueryFriend);

	if (!m_DBQueryMail.Exec(qSql))
	{
		cLog.Add("error-L3 : %s %d ", __FILE__, __LINE__);
		m_DBQueryMail.Clear();
		return 0;
	}

	__int16 sqlRet = m_DBQueryMail.Fetch();
	if (sqlRet == 100 || sqlRet == -1)
	{
		cLog.Add("error-L3 : sql_no_data : %s %d ", __FILE__, __LINE__);
		m_DBQueryMail.Clear();
		return 0;
	}

	result = m_DBQueryMail.GetInt(1);
	m_DBQueryMail.Clear();

	if (result == 1)
	{
		return 1;
	}
	else
	{
		cLog.Add("error-L3 : DelMemo %s %d Result:%d", szName, memoIndex, result);
		return 0;
	}
}

