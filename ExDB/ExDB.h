
// ExDB.h : ExDB 응용 프로그램에 대한 주 헤더 파일
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif
#include "CQuery.h"
#include "GameServerMng.h"
#include "GensSystem.h"
#include "resource.h"       // 주 기호입니다.
#include "CWhatsUpDummyServer.h"
#include "GensSystem.h"
#include "WZQueue.h"
#include "dbset/CastleDBSet.h"
#include "dbset/FriendDBSet.h"
#include "dbset/GensSystemDBSet.h"
#include "dbset/GuildDBSet.h"
#include "dbset/PartyMatchingDBSet.h"


// CExdbApp:
// 이 클래스의 구현에 대해서는 ExDB.cpp을 참조하십시오.
//

extern CGameServerMng gsm;
extern CDirPath gDirPath;
extern CLogProc cLog;
extern CMuNameCheck gMuName;
extern CGuildClass CGuildManager;
extern CFriendClass CFriendManager;
extern WZQueue FriendQueue;
extern WZQueue MailQueue;
extern CWZScriptEncode WzSEncode;
//CExdbApp theApp;
extern CQuery m_DBQueryFriend;
extern CQuery m_DBQueryMail;
extern CQuery m_DBQueryMailReadBlob;

// Database configuration
extern char szDbConnectId[256];
extern char szDbConnectPass[256];
extern char szDbConnectDsn[256];

// Game configuration
extern int gLanguage;
extern int gMaxFriend;
extern int gMaxMail;
extern int gExpireDayForCleanGuildMatching;

// File paths
extern char szKorItemTextFileName[];
extern char szKorSkillTextFileName[];
extern char szItemTextFileName[];
extern char szSkillTextFileName[];

// Version string
extern char szVersion[256];

// Mail configuration
extern char szSystemMailAdmin[];
extern char szSystemMailSubject[];
extern char szSystemMailMemo[];

extern char szChatServerIp[16];
extern DWORD dwExDbPort;

extern BOOL g_bEnableFriend;
extern BOOL g_bEnableFriendMail;

extern HANDLE gMailQueueThreadHandle; // idb
extern HANDLE gFriendQueueThreadHandle; // idb

class CExdbApp : public CWinApp
{
public:
	CExdbApp();
	int ServerCreate();
	int GuildLoadThreadRun();
	BOOL DbConnect();
	void Destroy();
	LRESULT OnGuildListDlgClose(WPARAM wParam, LPARAM lParam);
// 재정의입니다.
public:
	virtual BOOL InitInstance();

// 구현입니다.
	BOOL  m_bHiColorIcons;
	HANDLE m_hGuildLoad;
	//virtual void PreLoadState();
	//virtual void LoadCustomState();
	//virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg int DataLoad();
	DECLARE_MESSAGE_MAP()
};

extern CExdbApp theApp;
DWORD WINAPI GuildLoadThread(LPVOID lpParam);