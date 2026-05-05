#include "stdafx.h"
#include "PlusLogDlg.h"

CLogToFile UNION_SYSTEM_LOG("UNION_SYSTEM_LOG", ".\\UNION_SYSTEM_LOG", TRUE);

void TRACE_OUT(int iLevel, const char* format, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, format);

    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    UNION_SYSTEM_LOG.Output("%s", buffer);
}