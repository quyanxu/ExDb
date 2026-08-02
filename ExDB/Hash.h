#pragma once

#define MAX_HASH_BUCKETS    4
#define MAX_IDSTRING        10

struct Bucket_Class
{
    Bucket_Class()
    {
        number = -1;
        memset(Name, 0, sizeof(Name));
    }

    int number;                     // Object index (-1 = empty)
    char Name[MAX_IDSTRING + 1];    // Character/Account name (11 bytes)
};

class CHash
{
public:
    CHash();
    ~CHash();

    int Search(char* Name);
    int Insert(char* Name, int number);

private:
    Bucket_Class m_Bucket[MAX_HASH_BUCKETS];
};