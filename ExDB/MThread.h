// MThread.h

#pragma once

#include <Windows.h>
#include <process.h>

class MThread
{
public:
    MThread();
    virtual ~MThread();

public:
    int Start();
    void Stop();

    int IsRunning();
    int IsAlive();

    void Terminate();

public:
    virtual unsigned int Run() = 0;

protected:
    HANDLE          m_handle;
    unsigned int    m_id;

    int             m_bIsRunning;
    volatile int    m_bIsAlive;
};

unsigned int __stdcall ThreadProc(void* pThis);