// ServerListDlg.h

#pragma once

#include <afxwin.h>
#include <afxcmn.h>

class CMainFrame;

class CServerListDlg : public CDialog
{
    DECLARE_DYNAMIC(CServerListDlg)

public:
    CServerListDlg(CWnd* pParent = NULL);
    CServerListDlg(CMainFrame* pWinApp);
    virtual ~CServerListDlg();

public:
    enum
    {
        IDD = IDD_GAMESERVER_LIST
    };


public:
    BOOL Create();

    void CreateServerList();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnBtnGuildlistsend();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnRefresh();
    afx_msg void On_Close();

    DECLARE_MESSAGE_MAP()

private:
    CMainFrame* m_pWinApp;

public:
    CListCtrl      m_ServerListView;
};
