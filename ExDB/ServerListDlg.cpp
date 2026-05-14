// ServerListDlg.cpp

#include "stdafx.h"
#include "ServerListDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CServerListDlg, CDialog)

//BEGIN_MESSAGE_MAP(CServerListDlg, CDialog)
//    ON_BN_CLICKED(IDC_GSLISTSEND, OnBtnGuildlistsend)
//    ON_WM_SHOWWINDOW()
//    ON_COMMAND(IDREFRESH, OnRefresh)
//    ON_BN_CLICKED(IDCLOSE, On_Close)
//END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CServerListDlg, CDialog)
    // WM_COMMAND, ID 0x3F2 (1010) - OnBtnGuildlistsend()
    ON_COMMAND(IDC_GSLISTSEND, OnBtnGuildlistsend)
    // WM_SHOWWINDOW - OnShowWindow(BOOL, UINT)
    ON_WM_SHOWWINDOW()
    // WM_COMMAND, ID 2 - OnRefresh()
    ON_COMMAND(IDREFRESH, OnRefresh)
    // IDCLOSE (IDOK = 1) is NOT in the original message map!
    // The Close button uses IDOK and is handled by CDialog's default OnOK/OnCancel
    //ON_BN_CLICKED(IDCLOSE, On_Close)
END_MESSAGE_MAP()

CServerListDlg::CServerListDlg(CWnd* pParent)
    : CDialog(IDD, pParent)
{
    this->m_ServerListView;
}

CServerListDlg::CServerListDlg(CMainFrame* pWinApp)
    : CDialog()
{
    this->m_pWinApp = pWinApp;
}

CServerListDlg::~CServerListDlg()
{
}

void CServerListDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_GSLIST, this->m_ServerListView);
}

BOOL CServerListDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    LVCOLUMN COL;

    COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    COL.fmt = LVCFMT_LEFT;

    COL.cx = 50;
    COL.iSubItem = 0;
    COL.pszText = (LPSTR)"Index";

    this->m_ServerListView.InsertColumn(0, &COL);

    COL.iSubItem = 1;
    COL.pszText = (LPSTR)"Code";

    this->m_ServerListView.InsertColumn(1, &COL);

    COL.cx = 150;
    COL.iSubItem = 2;
    COL.pszText = (LPSTR)"Name";

    this->m_ServerListView.InsertColumn(2, &COL);

    return TRUE;
}

BOOL CServerListDlg::Create()
{
    return CDialog::Create(IDD, NULL);
}

void CServerListDlg::CreateServerList()
{
    this->m_ServerListView.DeleteAllItems();

    LVITEM LI;
    char Buffer[28];

    LI.mask = LVIF_TEXT;

    for (int n = 0; n < 100; n++)
    {
        LI.iItem = n;
        LI.iSubItem = 0;

        itoa(n, Buffer, 10);

        LI.pszText = Buffer;

        this->m_ServerListView.InsertItem(&LI);

        LI.iSubItem = 1;

        itoa(gsm.m_Obj[n].pServer, Buffer, 10);

        LI.pszText = Buffer;

        this->m_ServerListView.SetItem(&LI);

        LI.iSubItem = 2;

        if (gsm.m_Obj[n].m_Used)
        {
            LI.pszText = gsm.m_Obj[n].m_ServerName;
        }
        else
        {
            LI.pszText = (LPSTR)"-none-";
        }

        this->m_ServerListView.SetItem(&LI);
    }
}

void CServerListDlg::OnBtnGuildlistsend()
{
    for (int n = 0; n < 100; n++)
    {
        if (gsm.m_Obj[n].m_Used)
        {
            GDGuildListRequest(); //empty
        }
    }
}

void CServerListDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);

    this->CreateServerList();
}

void CServerListDlg::OnRefresh()
{
    this->CreateServerList();
}
void CServerListDlg::On_Close()
{
    ShowWindow(SW_HIDE);
}