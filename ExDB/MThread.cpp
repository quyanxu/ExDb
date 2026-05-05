// MThread.cpp

#include "stdafx.h"
#include "MThread.h"

MThread::MThread()
{
    this->m_handle = (HANDLE)-1;
    this->m_id = 0;

    this->m_bIsRunning = FALSE;
    this->m_bIsAlive = FALSE;
}

MThread::~MThread()
{
    this->Stop();
}

int MThread::Start()
{
    if (this->m_handle != (HANDLE)-1)
    {
        this->Terminate();
        this->Stop();
    }

    this->m_bIsAlive = TRUE;

    this->m_handle = (HANDLE)_beginthreadex(NULL,0,ThreadProc,this,0,&this->m_id);

    if (this->m_handle == (HANDLE)-1)
    {
        this->m_bIsAlive = FALSE;
        return FALSE;
    }

    this->m_bIsRunning = TRUE;

    return TRUE;
}

unsigned int __stdcall ThreadProc(void* pThis)
{
    return ((MThread*)pThis)->Run();
}

void MThread::Stop()
{
    if (this->m_bIsRunning)
    {
        if (this->m_handle != (HANDLE)-1)
        {
            this->m_bIsRunning = FALSE;
            WaitForSingleObject(this->m_handle, INFINITE);
            CloseHandle(this->m_handle);
            this->m_handle = (HANDLE)-1;
        }
    }
}

int MThread::IsRunning()
{
    return this->m_bIsRunning;
}

int MThread::IsAlive()
{
    return this->m_bIsAlive;
}

void MThread::Terminate()
{
    this->m_bIsAlive = FALSE;
}