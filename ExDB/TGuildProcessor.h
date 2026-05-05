// TGuildProcessor.h
#pragma once
#include "MThread.h"
#include "WZQueue.h"

class TGuildProcessor : public MThread
{
public:
    TGuildProcessor();
    virtual ~TGuildProcessor();

    BOOL ConnectDB(char* szDSN, char* szID, char* szPASS);

    virtual int Start();
    virtual void Stop();

    virtual unsigned int Run();

private:
    WZQueue m_MsgQueue;
    BOOL    m_IsDBConnected;
    CQuery* m_DBQuery;
};