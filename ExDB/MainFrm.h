
// MainFrm.h : CMainFrame 클래스의 인터페이스
//

#pragma once
#include "GuildListDlg.h"

extern const char pszFormat[];
extern char g_szDBID_DESC[];

class CMainFrame : public CFrameWnd
{
	
protected: // serialization에서만 만들어집니다.
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// 특성입니다.
public:

// 재정의입니다.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	void OnGuildmngOpen();
	//virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// 구현입니다.
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 컨트롤 모음이 포함된 멤버입니다.
	CMFCMenuBar		m_wndMenuBar;
	CToolBar		m_wndToolBar;
	CStatusBar		m_wndStatusBar;
	//CMFCToolBarImages m_UserImages;

// 생성된 메시지 맵 함수
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void DisplayStatusBar(char* szString);
	afx_msg void OnClose();
	afx_msg void OnGuildManager();
	afx_msg void OnServermngOpen();
	afx_msg LRESULT OnGuildListDlgClose(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnServerListDlgClose(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

};


extern CGuildListDlg* glistDlg;