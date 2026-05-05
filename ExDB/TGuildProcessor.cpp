// TGuildProcessor.cpp
#include "stdafx.h"
#include "TGuildProcessor.h"


TGuildProcessor::TGuildProcessor()
    : MThread()
    , m_MsgQueue(0x1388)
{
    this->m_IsDBConnected = FALSE;
    this->m_DBQuery = NULL;
}

TGuildProcessor::~TGuildProcessor()
{
}

BOOL TGuildProcessor::ConnectDB(char* szDSN, char* szID, char* szPASS)
{
    this->m_DBQuery = new CQuery;

    if (this->m_DBQuery == NULL)
    {
        this->m_IsDBConnected = FALSE;
        return FALSE;
    }

    this->m_IsDBConnected =
        this->m_DBQuery->Connect(3, szDSN, szID, szPASS);

    if (!this->m_IsDBConnected)
    {
        AfxMessageBox("New Guild DB ODBC ", 0, 0);
    }

    return this->m_IsDBConnected;
}

int TGuildProcessor::Start()
{
    if (this->m_IsDBConnected)
    {
        return MThread::Start();
    }

    AfxMessageBox("New Guild DB ODBC ", 0, 0);
    return FALSE;
}

void TGuildProcessor::Stop()
{
    MThread::Stop();
}

unsigned int TGuildProcessor::Run()
{
    BYTE btHeadCode[9];
    int iIndex[3];
    DWORD iSize[3];

    BYTE buf[1032];

    while (MThread::IsAlive())
    {
        if (!this->m_MsgQueue.GetFromQueue(buf,iSize,btHeadCode,iIndex))
        {
            Sleep(5);
            continue;
        }

        if (btHeadCode[0] == 0xFF)
        {
            break;
        }

        Sleep(5);
    }

    return 0;
}