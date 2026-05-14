// CastleDBSet.h
#pragma once
#include "stdafx.h"
// ----------------------------------------------------------------------
// Castle DB Set
// ----------------------------------------------------------------------

class CCastleDBSet
{
public:
    CCastleDBSet();
    virtual ~CCastleDBSet();

public:
    int Connect();
    int CheckCastleSiegeGuildList(char* szGuildName);

    CRITICAL_SECTION m_csCastleDBQuery;
    CQuery m_CastleDBQuery;
};

// ----------------------------------------------------------------------
// Global instance
// ----------------------------------------------------------------------

extern CCastleDBSet* CastleDbSet;