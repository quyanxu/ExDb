// MSync.h
#pragma once

#include <windows.h>
#include <cassert>

template<typename T>
class MSync
{
public:
    MSync();
    ~MSync();

    void Lock();
    void Unlock();

private:
    CRITICAL_SECTION m_cs;
    volatile LONG m_nLock;  // Using LONG for InterlockedIncrement/Decrement
};

// Constructor
template<typename T>
MSync<T>::MSync()
    : m_nLock(0)
{
    InitializeCriticalSection(&m_cs);
}

// Destructor
template<typename T>
MSync<T>::~MSync()
{
    DeleteCriticalSection(&m_cs);
}

// Lock - Enter critical section and increment lock count
template<typename T>
void MSync<T>::Lock()
{
    EnterCriticalSection(&m_cs);
    InterlockedIncrement(&m_nLock);
}

// Unlock - Decrement lock count and leave critical section
template<typename T>
void MSync<T>::Unlock()
{
    LONG nResult = InterlockedDecrement(&m_nLock);
    // Assert that lock count didn't go below zero (debug check)
    assert(nResult >= 0 && "m_nLock Count Err");
    LeaveCriticalSection(&m_cs);
}