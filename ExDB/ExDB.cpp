
// ExDB.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "ExDB.h"
#include "MainFrm.h"

#include "CExdbDoc.h"
#include "CExdbView.h"
#include "Thread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CExdbApp

BEGIN_MESSAGE_MAP(CExdbApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CExdbApp::OnAppAbout)
	// 표준 파일을 기초로 하는 문서 명령입니다.
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CExdbApp 생성

CExdbApp::CExdbApp()
{
	m_bHiColorIcons = TRUE;
}

// 유일한 CExdbApp 개체입니다.

CGameServerMng gsm;
CDirPath gDirPath;
CLogProc cLog;
CMuNameCheck gMuName;
CGuildClass CGuildManager;
CFriendClass CFriendManager;
WZQueue FriendQueue;
WZQueue MailQueue;
CWZScriptEncode WzSEncode;
CExdbApp theApp;
CQuery m_DBQueryFriend;
CQuery m_DBQueryMail;
CQuery m_DBQueryMailReadBlob;

char    szVersion[];
char    szChatServerIp[]; 
DWORD   dwExDbPort;
char    szKorSkillTextFileName[256];
char    szKorItemTextFileName[256];
char    szSkillTextFileName[256];
char    szItemTextFileName[256];
char    szSystemMailAdmin[11]; // idb
char    szSystemMailMemo[1000]; // idb
char    szSystemMailSubject[32]; // idb
int     gLanguage; // idb
char    szDbConnectDsn[]; // idb
char    szDbConnectPass[]; // idb
char    szDbConnectId[]; // idb

int gMaxFriend = 50; // idb
int gMaxMail = 50; // idb

int gExpireDayForCleanGuildMatching; // idb

int Run; // idb

BOOL g_bEnableFriend = true;
BOOL g_bEnableFriendMail = true;

int CExdbApp::InitInstance()
{
    // Standard MFC initialization
    //Enable3dControlsStatic();
    SetRegistryKey("Local AppWizard-Generated Applications");
    LoadStdProfileSettings(4);  // Load standard INI file options

    // Register the application's document template
    CSingleDocTemplate* pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,                          // Resource ID
        RUNTIME_CLASS(CExdbDoc),                // Document class
        RUNTIME_CLASS(CMainFrame),              // Frame class (SDI)
        RUNTIME_CLASS(CExdbView)                // View class
    );

    if (!pDocTemplate)
        return FALSE;

    AddDocTemplate(pDocTemplate);

    // Parse command line for standard shell commands
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    // Force FileNew command
    cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;

    // Dispatch commands specified on the command line
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

    // Initialize chat server IP and port from command line
    memset(szChatServerIp, 0, sizeof(szChatServerIp));
    dwExDbPort = 55906;
    sscanf(m_lpCmdLine, "%s %d", szChatServerIp, &dwExDbPort);

    // Validate chat server IP
    if (strlen(szChatServerIp) == 0)
    {
        MsgBox("error : ChatServerIP = %s", szChatServerIp);
        return FALSE;
    }

    // Show the window
    m_pMainWnd->ShowWindow(SW_SHOWNORMAL);  // SW_SHOWMINIMIZED = 5 in original
    m_pMainWnd->UpdateWindow();

    // Initialize ExDB components
    m_hGuildLoad = 0;
    cLog.Create(true);
    cLog.AddTD( "ExDB Server DataLoad!!");
    if (!DataLoad())
    {
        cLog.AddTD( "ExDB Server DataLoad Failed!!");
        return FALSE;
    }

    cLog.AddTD( "ExDB Server DBConnect!!");
    if (!DbConnect())
    {
        cLog.AddTD( "ExDB Server DBConnect Failed!!");
        return FALSE;
    }

    cLog.AddTD( "ExDB Server GuildLoadThreadRun!!");
    GuildLoadThreadRun();

    // Load Gens system settings
    g_GensSystem_DBSet.LoadGensSetting();

    return TRUE;
}

int CExdbApp::DataLoad()
{
    // Open the encrypted config file
	if (!WzSEncode.Open("exdb.ini.dat"))
	{
	    MsgBox("Error : ExDB.ini.dat file not found!!");
	    return 0;
	}

    // Read database connection ID
    WzSEncode.GetToken();
    strcpy(szDbConnectId, WzSEncode.GetString());

    // Read database connection password
    WzSEncode.GetToken();
    strcpy(szDbConnectPass, WzSEncode.GetString());

    // Read optional max friend/mail settings
    WzSEncode.GetToken();
    if (WzSEncode.GetNumber() == 1)
    {
        WzSEncode.GetToken();
        gMaxFriend = WzSEncode.GetNumber();

        WzSEncode.GetToken();
        gMaxMail = WzSEncode.GetNumber();
    }

    // Read DSN (Data Source Name)
    if (WzSEncode.GetToken() == T_END)
    {
        strcpy(szDbConnectDsn, "MuOnline");
    }
    else
    {
        strcpy(szDbConnectDsn, WzSEncode.GetString());
    }

    // Set default DSN if empty
    if (strcmp(szDbConnectDsn, "") == 0)
    {
        strcpy(szDbConnectDsn, "MuOnline");
    }

    WzSEncode.Close();

    // Initialize paths and load configuration
    gDirPath.SetFirstPath("..\\data\\");

    // Load language setting
    const char* commonServerPath = gDirPath.GetNewPath( "commonserver.cfg");
    gLanguage = GetPrivateProfileIntA("GameServerInfo", "Language", 0, commonServerPath);

    // Load mail configuration
    GetPrivateProfileStringA("MAIL", "Account", "Webzen", szSystemMailAdmin, 10, ".\\ExDBSystemMail.cfg");
    GetPrivateProfileStringA("MAIL", "Subject", "None", szSystemMailSubject, 32, ".\\ExDBSystemMail.cfg");
    GetPrivateProfileStringA("MAIL", "Memo", "None", szSystemMailMemo, 1000, ".\\ExDBSystemMail.cfg");
    gExpireDayForCleanGuildMatching = GetPrivateProfileIntA("MAIL", "ExpireDay", 14, ".\\ExDBSystemMail.cfg");

    // Load Korean files (always needed)
    strcpy(szKorItemTextFileName, gDirPath.GetNewPath( "lang\\kor\\item(kor).txt"));
    strcpy(szKorSkillTextFileName, gDirPath.GetNewPath( "lang\\kor\\Skill(kor).txt"));

    // Set version string
    strcpy(szVersion, "MU ExDB Ver (0.50.R) (-WEBZEN-) ");
    strcat(szVersion, " (FOR C.S)");

    // Load language-specific item/skill files
    switch (gLanguage)
    {
    case 0: // Korean
        strcpy(szItemTextFileName, gDirPath.GetNewPath( "lang\\kor\\item(kor).txt"));
        strcpy(szSkillTextFileName, gDirPath.GetNewPath( "lang\\kor\\Skill(kor).txt"));
        strcat(szVersion, "-KOR-");
        break;

    case 1:  // English
    case 8:  // Another English variant
        strcpy(szItemTextFileName, gDirPath.GetNewPath( "lang\\eng\\item(eng).txt"));
        strcpy(szSkillTextFileName, gDirPath.GetNewPath( "lang\\eng\\Skill(eng).txt"));
        strcat(szVersion, "-ENG-");
        break;

    case 2: // Japanese
        strcpy(szItemTextFileName, gDirPath.GetNewPath( "lang\\jpn\\item(jpn).txt"));
        strcpy(szSkillTextFileName, gDirPath.GetNewPath( "lang\\jpn\\Skill(jpn).txt"));
        strcat(szVersion, "-JPN-");
        break;

    case 3: // Chinese Simplified
        strcpy(szItemTextFileName, gDirPath.GetNewPath( "lang\\chs\\item(chs).txt"));
        strcpy(szSkillTextFileName, gDirPath.GetNewPath( "lang\\chs\\Skill(chs).txt"));
        strcat(szVersion, "-CHA-");
        break;

    case 4: // Taiwanese
        strcpy(szItemTextFileName, gDirPath.GetNewPath( "lang\\tai\\item(tai).txt"));
        strcpy(szSkillTextFileName, gDirPath.GetNewPath( "lang\\tai\\Skill(tai).txt"));
        strcat(szVersion, "-THI-");
        break;

    case 5: // Thai
        strcpy(szItemTextFileName, gDirPath.GetNewPath( "lang\\tha\\item(Tha).txt"));
        strcpy(szSkillTextFileName, gDirPath.GetNewPath( "lang\\tha\\Skill(Tha).txt"));
        strcat(szVersion, "-THA-");
        break;

    case 6: // Filipino
        strcpy(szItemTextFileName, gDirPath.GetNewPath( "lang\\phi\\item(phi).txt"));
        strcpy(szSkillTextFileName, gDirPath.GetNewPath( "lang\\phi\\Skill(phi).txt"));
        strcat(szVersion, "-PHI-");
        break;

    case 7: // Vietnamese
        strcpy(szItemTextFileName, gDirPath.GetNewPath( "lang\\vtm\\item(vtm).txt"));
        strcpy(szSkillTextFileName, gDirPath.GetNewPath( "lang\\vtm\\Skill(vtm).txt"));
        strcat(szVersion, "-VTM-");
        break;
    }

    // Load item names
    if (gLanguage)
    {
        if (!gMuName.NameLoad( szItemTextFileName))
            MsgBox("Item File Not Found.");
    }
    else
    {
        if (!gMuName.NameLoad( szKorItemTextFileName))
            MsgBox("Item File Not Found.");
    }

    // Load monster names
    if (!gMuName.NameLoad( "..\\data\\Monster.txt"))
        MsgBox("Monster File Not Found.");

    // Load skill names
    if (gLanguage)
    {
        if (!gMuName.NameLoad( szSkillTextFileName))
            MsgBox("Skill File Not Found.");
    }
    else
    {
        if (!gMuName.NameLoad( szKorSkillTextFileName))
            MsgBox("Skill File Not Found.");
    }

    // Load bad syntax filter
    if (!cBadStrChk.Load( "..\\data\\badsyntax.txt"))
        MsgBox("badstring load error");

    return 1;
}
int CExdbApp::ServerCreate()
{
    // Check if server is already running
    if (::Run)
        return 1;

    // Create the IOCP (I/O Completion Port) engine
    CWZIocpEngine* pWzIoEngine = new CWZIocpEngine();

    if (!pWzIoEngine)
    {
        MsgBox("Failed to create IOCP Engine!");
        return 0;
    }

    WzIoEngine = pWzIoEngine;

    // Create the server on the configured port
    // dwExDbPort is typically 55906 (from InitInstance)
    // Parameter 5 likely means 5 worker threads
    if (!pWzIoEngine->Create(dwExDbPort, 5))
    {
        MsgBox("ExDB Server Create Fail!!");
        delete pWzIoEngine;
        WzIoEngine = 0;
        return 0;
    }

    // Log success and start the server
    cLog.AddTD("ExDB Server Start!!");

    // Start friend and mail queue processing threads
    FriendQueueThreadRun();
    MailQueueThreadRun();

    // Set the running flag
    ::Run = true;

    // Start the dummy server for status monitoring
    // Uses port dwExDbPort + 1 (typically 55907)
    gWhatsUpDummyServer.Start(m_pMainWnd->m_hWnd, dwExDbPort + 1);

    return 1;
}
// CExdbApp.cpp
LRESULT CExdbApp::OnGuildListDlgClose(WPARAM wParam, LPARAM lParam)
{
    if (glistDlg)
    {
        delete glistDlg;
        glistDlg = NULL;
    }
    return 0;
}

HANDLE gFriendQueueThreadHandle = NULL;
HANDLE gMailQueueThreadHandle = NULL;

int CExdbApp::GuildLoadThreadRun()
{
	DWORD ThreadID;

	this->m_hGuildLoad = ::CreateThread(NULL, 0, GuildLoadThread, NULL, 0, &ThreadID);

	return 1;
}


DWORD WINAPI GuildLoadThread(LPVOID lpParam)
{
    GuildDbSet->ReadAllGuild();
    return 1;
}

BOOL CExdbApp::DbConnect()
{
    // Friend DB
    FriendDbSet = new CFriendDBSet;

    if (!FriendDbSet)
    {
        MsgBox("FriendDbSet Memory allocation error");
        return FALSE;
    }

    if (!FriendDbSet->Connect())
    {
        return FALSE;
    }

    // Guild DB
    GuildDbSet = new CGuildDBSet;

    if (!GuildDbSet)
    {
        MsgBox("GuildDbSet Memory allocation error");
        return FALSE;
    }

    if (!GuildDbSet->Connect())
    {
        return FALSE;
    }

    // Gens DB
    if (!g_GensSystem_DBSet.Connect())
    {
        return FALSE;
    }

    // Castle DB
    CastleDbSet = new CCastleDBSet;

    if (!CastleDbSet)
    {
        MsgBox("CastleDBSet Memory allocation Error!");
        return FALSE;
    }

    if (!CastleDbSet->Connect())
    {
        return FALSE;
    }

    // Party Matching DB
    if (!g_PartyMatching_DBSet.Connect())
    {
        return FALSE;
    }

    return TRUE;
}
void CExdbApp::Destroy()
{
    // Signal friend queue thread to exit
    unsigned __int8 buf[12];
    buf[0] = 0xFF;
    buf[1] = 0xFF;
    buf[2] = 0xFF;

    FriendQueue.AddToQueue(buf, 3, 0xFF, 0);
    WaitForSingleObject(gFriendQueueThreadHandle, INFINITE);

    // Signal mail queue thread to exit
    MailQueue.AddToQueue(buf, 3, 0xFF, 0);
    WaitForSingleObject(gMailQueueThreadHandle, INFINITE);

    // Terminate guild loading thread
    TerminateThread(m_hGuildLoad, 0);
    if (m_hGuildLoad)
    {
        CloseHandle(m_hGuildLoad);
        m_hGuildLoad = NULL;
    }

    // Destroy database objects
    if (GuildDbSet)
    {
        delete GuildDbSet;
        GuildDbSet = NULL;
    }

    if (FriendDbSet)
    {
        delete FriendDbSet;
        FriendDbSet = NULL;
    }

    if (CastleDbSet)
    {
        delete CastleDbSet;
        CastleDbSet = NULL;
    }
}
// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// 대화 상자를 실행하기 위한 응용 프로그램 명령입니다.
void CExdbApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CExdbApp 사용자 지정 로드/저장 메서드

//
//void CExdbApp::LoadCustomState()
//{
//}
//
//void CExdbApp::SaveCustomState()
//{
//}

// CExdbApp 메시지 처리기



