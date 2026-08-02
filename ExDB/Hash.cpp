#include "stdafx.h"
#include "Hash.h"

CHash::CHash()
{
    // m_Bucket array elements are initialized by Bucket_Class default constructor
    // The original uses `vector constructor iterator` to call Bucket_Class::Bucket_Class
    // for each of the 4 elements (0x10 = 16 bytes stride).
    // In C++, this happens automatically for static arrays.
}

CHash::~CHash()
{
    // Empty destructor
}

int CHash::Search(char* Name)
{
    int n = (Name[1] + *Name) % MAX_HASH_BUCKETS;
    for (int count = 0; count < MAX_HASH_BUCKETS; ++count)
    {
        if (m_Bucket[n].number != -1)
        {
            if (strcmp(m_Bucket[n].Name, Name) == 0)
                return n;
        }
        if (++n >= MAX_HASH_BUCKETS)
            n = 0;
    }
    return -1;  // Not found
}

int CHash::Insert(char* Name, int number)
{
    int n = (Name[1] + *Name) % MAX_HASH_BUCKETS;
    for (int count = 0; count < MAX_HASH_BUCKETS; ++count)
    {
        // Empty slot - insert here
        if (m_Bucket[n].number == -1)
        {
            m_Bucket[n].number = number;
            strcpy(m_Bucket[n].Name, Name);
            return n;
        }

        // Already exists - return existing index
        if (strcmp(m_Bucket[n].Name, Name) == 0)
            return n;

        if (++n >= MAX_HASH_BUCKETS)
            n = 0;
    }
    return -1;  // Hash table full
}