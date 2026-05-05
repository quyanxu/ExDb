#pragma once

#include "afxcmn.h"
#include "afxwin.h"

class CMainFrame;

class CGuildListDlg : public CDialog
{
	DECLARE_DYNAMIC(CGuildListDlg)

public:

	enum
	{
		IDD = IDD_GUILDLIST
	};

public:

	CGuildListDlg(CWnd* pParent = NULL);
	CGuildListDlg(CMainFrame* pWinApp);
	virtual ~CGuildListDlg();

	int Create();

	void Reload();
	void GuildMemberAdd(char* guildname);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	afx_msg void OnDblclkGuildlist();
	afx_msg void On_Ok();
	afx_msg void OnDblclkGuildListv(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickGuildListv(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSearchBtn();
	afx_msg void OnBtnNext();

	DECLARE_MESSAGE_MAP()

public:

	CMainFrame* m_pWinApp;

	CListCtrl m_GuildListV;
	CListBox  m_GuildMember;

	CString m_Notice;
	CString m_GuildName;
};