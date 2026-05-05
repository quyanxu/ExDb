// AutoSync.h

#pragma once

#include <Windows.h>

class CAutoSync
{
public:
    CAutoSync(CRITICAL_SECTION* pCS);
    virtual ~CAutoSync();

private:
    CRITICAL_SECTION* m_pCs;
};