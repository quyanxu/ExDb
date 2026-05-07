
// MainFrm.cpp : CMainFrame 클래스의 구현
//

#include "stdafx.h"
#include "ExDB.h"

#include "MainFrm.h"

#include "GuildListDlg.h"
#include "ServerListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
CServerListDlg* SLDlg;
CGuildListDlg* glistDlg;

const char pszFormat[] = "%s ";
char g_szDBID_DESC[20];

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

//const int  iMaxUserToolbars = 10;
//const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
//const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_MESSAGE(0x7E8, OnGuildListDlgClose)
	ON_MESSAGE(0x7E9, OnServerListDlgClose)
	ON_COMMAND(ID_GUILD_MANAGER, &CMainFrame::OnGuildManager)
	ON_COMMAND(ID_SERVER_MANAGER, &CMainFrame::OnServermngOpen)
	//ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 상태 줄 표시기
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame 생성/소멸

CMainFrame::CMainFrame()
{
	glistDlg = 0;
	SLDlg = 0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	// Toolbar
	if (!m_wndToolBar.CreateEx(
			this,
			TBSTYLE_FLAT,
			WS_CHILD | WS_VISIBLE |
			CBRS_TOP |
			CBRS_GRIPPER |
			CBRS_TOOLTIPS |
			CBRS_FLYBY |
			CBRS_SIZE_DYNAMIC,
			CRect(0, 0, 0, 0),
			IDR_MAINFRAME) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}

	// Status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(
			indicators,
			sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;
	}

	// Docking
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// Timers
	SetTimer(1001, 1000, NULL);
	SetTimer(1002, 1000, NULL);

	// Server list dialog
	SLDlg = new CServerListDlg(this);

	if (SLDlg != NULL)
	{
		SLDlg->Create();
	}

	// Guild list dialog
	glistDlg = new CGuildListDlg(this);

	if (glistDlg != NULL)
	{
		glistDlg->Create();
	}

	return 0;
}

void CMainFrame::OnGuildmngOpen()
{
	if (!glistDlg)
	{
		glistDlg = new CGuildListDlg(this);
		if (glistDlg)
		{
			glistDlg->Create();
		}
	}

	if (glistDlg)
	{
		glistDlg->ShowWindow(SW_SHOW);  // 5 = SW_SHOW
		glistDlg->Reload();
	}
}

LRESULT CMainFrame::OnGuildListDlgClose(WPARAM wParam, LPARAM lParam)
{
	if (glistDlg)
	{
		delete glistDlg;
		glistDlg = NULL;
	}
	return 0;
}

LRESULT CMainFrame::OnServerListDlgClose(WPARAM wParam, LPARAM lParam)
{
	if (SLDlg)
	{
		delete SLDlg;
		SLDlg = NULL;
	}
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	//cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	//cs.lpszClass = AfxRegisterWndClass(0, 0, 0, 0);

	return TRUE;
}


void CMainFrame::OnTimer(UINT nIDEvent)
{
	strcpy(szVersion, "MU ExDB Ver (0.50.R) (-WEBZEN-) ");
	strcat(szVersion, " (FOR C.S)");
	if (nIDEvent == 1001)
	{
		// Invalidate the window to force repaint
		Invalidate(TRUE);

		// Format and set window title
		CString szTitle;
		szTitle.Format(pszFormat, szVersion, g_szDBID_DESC);
		SetWindowText(szTitle);
	}
	else if (nIDEvent == 1002)
	{
		// Get application instance and create server
		CExdbApp* pApp = (CExdbApp*)AfxGetApp();
		if (pApp)
		{
			pApp->ServerCreate();
		}

		// Kill the 1002 timer (0x3EA = 1002 in decimal)
		KillTimer(1002);
	}

	// Call base class OnTimer
	CWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnClose()
{
	// Ask for confirmation before closing
	if (AfxMessageBox("ExDB server close?", MB_YESNO | MB_ICONQUESTION, 0) == IDYES)
	{
		CExdbApp* pApp = (CExdbApp*)AfxGetApp();

		// Destroy the application (stops all services)
		pApp->Destroy();

		// Destroy guild list dialog
		if (glistDlg)
		{
			glistDlg->DestroyWindow();
			delete glistDlg;
			glistDlg = NULL;
		}

		// Destroy server list dialog
		if (SLDlg)
		{
			SLDlg->DestroyWindow();
			delete SLDlg;
			SLDlg = NULL;
		}

		// Destroy IOCP engine
		if (WzIoEngine)
		{
			delete WzIoEngine;
			WzIoEngine = NULL;
		}

		// Call base class OnClose (closes the window)
		CFrameWnd::OnClose();
	}
}
void CMainFrame::DisplayStatusBar(char* szString)
{
	this->m_wndStatusBar.SetWindowText(szString);
}

void CMainFrame::OnGuildManager()
{
	if (glistDlg == 0)
	{
		glistDlg = new CGuildListDlg(this);

		if (!glistDlg->Create())
		{
			delete glistDlg;
			glistDlg = 0;
			return;
		}
	}

	glistDlg->ShowWindow(SW_SHOW);
	glistDlg->SetForegroundWindow();
}

void CMainFrame::OnServermngOpen()
{
	if (SLDlg == 0)
	{
		SLDlg = new CServerListDlg(this);

		if (!SLDlg->Create())
		{
			delete SLDlg;
			SLDlg = 0;
			return;
		}
	}

	SLDlg->ShowWindow(SW_SHOW);
	SLDlg->SetForegroundWindow();
}

// CMainFrame 진단

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 메시지 처리기


//BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
//{
//	// 기본 클래스가 실제 작업을 수행합니다.
//
//	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
//	{
//		return FALSE;
//	}
//
//
//	// 모든 사용자 도구 모음에 사용자 지정 단추를 활성화합니다.
//	BOOL bNameValid;
//	CString strCustomize;
//	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
//	ASSERT(bNameValid);
//
//	for (int i = 0; i < iMaxUserToolbars; i ++)
//	{
//		CToolBar* pUserToolbar = GetUserToolBarByIndex(i);
//		if (pUserToolbar != NULL)
//		{
//			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
//		}
//	}
//
//	return TRUE;
//}
//
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// forward focus to the view window
	//m_wndView.SetFocus();
	//if ( this->m_pViewActive )
 //   CWnd::SetFocus(this->m_pViewActive);
	//	else
 //   CWnd::OnSetFocus(this, pOldWnd);
}