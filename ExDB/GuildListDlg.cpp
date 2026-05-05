#include "stdafx.h"
#include "GuildListDlg.h"
#include "MainFrm.h"
#include "GuildClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CGuildClass CGuildManager;

IMPLEMENT_DYNAMIC(CGuildListDlg, CDialog)

BEGIN_MESSAGE_MAP(CGuildListDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_OK, On_Ok)
	ON_NOTIFY(NM_DBLCLK, IDC_GUILDLISTV, OnDblclkGuildListv)
	ON_NOTIFY(NM_CLICK, IDC_GUILDLISTV, OnClickGuildListv)
	ON_BN_CLICKED(IDC_SEARCHBTN, OnSearchBtn)
	ON_BN_CLICKED(IDC_BTN_NEXT, OnBtnNext)
END_MESSAGE_MAP()

CGuildListDlg::CGuildListDlg(CWnd* pParent)
	: CDialog(CGuildListDlg::IDD, pParent)
{
	this->m_Notice = "";
	this->m_GuildName = "";
	this->m_pWinApp = NULL;
}

CGuildListDlg::CGuildListDlg(CMainFrame* pWinApp)
	: CDialog()
{
	this->m_GuildListV;
	this->m_GuildMember;
	this->m_pWinApp = pWinApp;
}

CGuildListDlg::~CGuildListDlg()
{
}

void CGuildListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_GUILDLISTV, this->m_GuildListV);
	DDX_Control(pDX, IDC_GUILDMEMBER, this->m_GuildMember);

	DDX_Text(pDX, IDC_NOTICE, this->m_Notice);
	DDX_Text(pDX, IDC_GUILDNAME, this->m_GuildName);

	DDV_MaxChars(pDX, this->m_GuildName, 8);
}

BOOL CGuildListDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	this->m_Notice = "";
	UpdateData(FALSE);

	LV_COLUMN col;

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;

	col.cx = 80;
	col.iSubItem = 0;
	col.pszText = "Name";

	this->m_GuildListV.InsertColumn(0, &col);

	col.cx = 60;
	col.iSubItem = 1;
	col.pszText = "Count";

	this->m_GuildListV.InsertColumn(1, &col);

	col.cx = 100;
	col.iSubItem = 2;
	col.pszText = "Master";

	this->m_GuildListV.InsertColumn(2, &col);

	this->m_GuildListV.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	return TRUE;
}

void CGuildListDlg::Reload()
{
	CGuildManager.GuildListAddDlg(
		&this->m_GuildListV,
		&this->m_GuildMember);
}

void CGuildListDlg::GuildMemberAdd(char* guildname)
{
	int count = 0;

	_GUILD_INFO_STRUCT* pSearchGuild = CGuildManager.SearchGuild(guildname);

	if (pSearchGuild == NULL)
	{
		return;
	}

	this->m_GuildMember.ResetContent();

	for (int n = 0; n < 80; n++)
	{
		if (pSearchGuild->Use[n])
		{
			this->m_GuildMember.AddString(
				pSearchGuild->Names[n]);

			count++;
		}
	}

	this->m_Notice.Format("%s Guild Member Count : %d",guildname,count);
	UpdateData(FALSE);
}

void CGuildListDlg::OnDblclkGuildlist()
{
}

int CGuildListDlg::Create()
{
	return CDialog::Create(IDD, NULL);
}

void CGuildListDlg::On_Ok()
{
	DestroyWindow();

	if (m_pWinApp)
	{
		m_pWinApp->SendMessage(0x7E8, 1, 0);
	}
}

void CGuildListDlg::OnDblclkGuildListv(NMHDR* pNMHDR, LRESULT* pResult)
{
	char szGuildName[20];
	memset(szGuildName, 0, sizeof(szGuildName));
	int item = this->m_GuildListV.GetNextItem(-1,LVNI_SELECTED);

	if (item >= 0)
	{
		this->m_GuildListV.GetItemText(item,0,szGuildName,sizeof(szGuildName));
		this->GuildMemberAdd(szGuildName);
	}

	*pResult = 0;
}

void CGuildListDlg::OnClickGuildListv(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
}

void CGuildListDlg::OnSearchBtn()
{
	UpdateData(TRUE);

	char szName[20];

	memset(szName, 0, sizeof(szName));

	strcpy(szName, this->m_GuildName);

	if (strlen(szName) <= 0)
	{
		return;
	}

	LVFINDINFO fi;

	memset(&fi, 0, sizeof(fi));

	fi.flags = LVFI_STRING;
	fi.psz = szName;
	fi.vkDirection = VK_DOWN;

	int idx = this->m_GuildListV.FindItem(
		&fi,
		-1);

	if (idx == -1)
	{
		this->m_Notice = "Guild not found";
		UpdateData(FALSE);
	}
	else
	{
		this->m_GuildListV.SetItemState(
			-1,
			0,
			LVIS_SELECTED);

		this->m_GuildListV.SetItemState(
			idx,
			LVIS_SELECTED | LVIS_FOCUSED,
			LVIS_SELECTED | LVIS_FOCUSED);

		this->m_GuildListV.EnsureVisible(
			idx,
			FALSE);
	}
}

BOOL CGuildListDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		this->OnSearchBtn();
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CGuildListDlg::OnBtnNext()
{
	this->m_GuildListV.DeleteAllItems();

	CGuildManager.GuildListAddDlg(
		&this->m_GuildListV,
		&this->m_GuildMember);
}

