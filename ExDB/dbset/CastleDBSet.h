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

    CQuery m_CastleDBQuery;
    CRITICAL_SECTION m_csCastleDBQuery;
};

// ----------------------------------------------------------------------
// Global instance
// ----------------------------------------------------------------------

extern CCastleDBSet* CastleDbSet;