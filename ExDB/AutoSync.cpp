// AutoSync.cpp

#include "stdafx.h"
#include "AutoSync.h"

CAutoSync::CAutoSync(CRITICAL_SECTION* pCS)
{
    this->m_pCs = pCS;

    EnterCriticalSection(this->m_pCs);
}

CAutoSync::~CAutoSync()
{
    LeaveCriticalSection(this->m_pCs);
}