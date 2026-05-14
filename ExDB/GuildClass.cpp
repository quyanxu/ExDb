#include "stdafx.h"
#include "GuildClass.h"
#include <string.h>

int GuildSendThreadCount = 0;

CGuildClass::CGuildClass()
{
    this->head = 0;
    this->tail = 0;

    this->m_count = 0;

    this->m_OnOff = 0;
    this->m_OnOffCount = 0;

    this->pGuildViewCursor = 0;

    InitializeCriticalSection(&this->m_guild_cs);
}

CGuildClass::~CGuildClass()
{
    this->AllDelete();

    DeleteCriticalSection(&this->m_guild_cs);
}

void CGuildClass::Init()
{
    this->m_count = 0;
    this->m_OnOffCount = 0;

    this->m_GuildMap.clear();
    this->m_GuildNumberMap.clear();
}

void CGuildClass::GuildOnOff(int on_off)
{
    EnterCriticalSection(&this->m_guild_cs);

    this->m_OnOff = on_off;

    if (this->m_OnOff)
        this->m_OnOffCount--;
    else
        this->m_OnOffCount++;

    cLog.Add(" GuildOnOff system : %d Count: %d", this->m_OnOff, this->m_OnOffCount);

    LeaveCriticalSection(&this->m_guild_cs);
}

int CGuildClass::GetGuildOnOff()
{
    return this->m_OnOff;
}

BOOL CGuildClass::IsCreateGuild()
{
    return (!this->m_OnOff && this->m_OnOffCount);
}

void CGuildClass::ClearSendGuildInfo()
{
    for (_GUILD_INFO_STRUCT* pNode = this->head; pNode; pNode = pNode->next)
    {
        pNode->IsSendGuildInfo = 0;
    }
}

_GUILD_INFO_STRUCT* CGuildClass::AddGuild(int number, char* guildname, BYTE* mark, char* mastername, char* notice, int score, int SystemCheck)
{
	if (SystemCheck == 1 && !this->m_OnOff && this->m_OnOffCount)
	{
		cLog.Add("Guild system Off %d ", this->m_OnOffCount);
		return 0;
	}

	// Guild already exists
	if (this->SearchGuild(guildname))
	{
		return 0;
	}

	_GUILD_INFO_STRUCT* pNewNode = (_GUILD_INFO_STRUCT*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(_GUILD_INFO_STRUCT));

	if (!pNewNode)
	{
		return 0;
	}

	strcpy(pNewNode->Name, guildname);
	strcpy(pNewNode->Names[0], mastername);

	if (mark)
	{
		memcpy(pNewNode->Mark, mark, sizeof(pNewNode->Mark));
	}

	pNewNode->Number = number;
	pNewNode->Count = 1;
	pNewNode->PlayScore = 0;
	pNewNode->TotalScore = score;
	pNewNode->next = 0;
	pNewNode->IsSendGuildInfo = 0;
	pNewNode->iGuildUnion = 0;
	pNewNode->iGuildRival = 0;

	if (notice && strlen(notice) > 0)
	{
		strcpy(pNewNode->Notice, notice);
	}
	else
	{
		memset(pNewNode->Notice, 0, sizeof(pNewNode->Notice));
	}

	for (int n = 1; n < 80; n++)
	{
		pNewNode->Use[n] = 0;
		pNewNode->pServer[n] = -1;
		pNewNode->GuildStatus[n] = -1;
	}

	// Guild master
	pNewNode->Use[0] = 1;
	pNewNode->pServer[0] = -1;
	pNewNode->GuildStatus[0] = 128;

	EnterCriticalSection(&this->m_guild_cs);

	this->AddTail(pNewNode);

	cLog.Add("[%s] Create Guild  Master = %s", guildname, mastername);

	++this->m_count;

	LeaveCriticalSection(&this->m_guild_cs);

	return pNewNode;
}

void CGuildClass::AddTail(_GUILD_INFO_STRUCT* pNewNode)
{
    EnterCriticalSection(&this->m_guild_cs);

    pNewNode->back = this->tail;
    pNewNode->next = 0;

    if (this->head)
        this->tail->next = pNewNode;
    else
        this->head = pNewNode;

    this->tail = pNewNode;

    this->m_GuildMap[pNewNode->Name] = pNewNode;
    this->m_GuildNumberMap[pNewNode->Number] = pNewNode;

    LeaveCriticalSection(&this->m_guild_cs);
}

void CGuildClass::Print()
{
    EnterCriticalSection(&this->m_guild_cs);

    for (_GUILD_INFO_STRUCT* lpGuild = this->head;
        lpGuild != 0;
        lpGuild = lpGuild->next)
    {
        cLog.Add("Guild print : %s  %s\n",lpGuild->Name,lpGuild->Names[0]);
    }

    LeaveCriticalSection(&this->m_guild_cs);
}

void CGuildClass::PrintGuild(char* guildname)
{
    SearchGuild(guildname);
}

void CGuildClass::AllDelete()
{
    _GUILD_INFO_STRUCT* pNode = this->head;

    while (pNode)
    {
        _GUILD_INFO_STRUCT* pTemp = pNode;

        pNode = pNode->next;

        HeapFree(GetProcessHeap(), 0, pTemp);
    }

    this->head = 0;
    this->tail = 0;

    this->m_count = 0;
    this->m_OnOff = 0;

    this->m_GuildMap.clear();
    this->m_GuildNumberMap.clear();
}

int CGuildClass::DeleteGuild(char* guildName, char* commander)
{
    if (!this->m_OnOff || this->m_OnOffCount)
    {
        cLog.Add("[Delete] Guild system Off");
        return 0;
    }

    _GUILD_INFO_STRUCT* lpGuild = this->SearchGuild(guildName);

    if (lpGuild == 0)
    {
        return 0;
    }

    EnterCriticalSection(&this->m_guild_cs);

    // Remove from guild name map
    {
        std::map<std::string, _GUILD_INFO_STRUCT*>::iterator it;
        it = this->m_GuildMap.find(guildName);

        if (it != this->m_GuildMap.end())
        {
            this->m_GuildMap.erase(it);
        }
    }

    // Remove from guild number map
    {
        std::map<int, _GUILD_INFO_STRUCT*>::iterator it;
        it = this->m_GuildNumberMap.find(lpGuild->Number);

        if (it != this->m_GuildNumberMap.end())
        {
            this->m_GuildNumberMap.erase(it);
        }
    }

    _GUILD_INFO_STRUCT* lpPrev = lpGuild->back;
    _GUILD_INFO_STRUCT* lpNext = lpGuild->next;

    // Unlink from linked list
    if (lpPrev != 0)
    {
        lpPrev->next = lpNext;
    }
    else
    {
        this->head = lpNext;
    }

    if (lpNext != 0)
    {
        lpNext->back = lpPrev;
    }
    else
    {
        this->tail = lpPrev;
    }

    // Free guild node
    HeapFree(GetProcessHeap(), 0, lpGuild);

    // Empty list
    if (this->m_count <= 1)
    {
        this->head = 0;
        this->tail = 0;
    }

    this->m_count--;

    LeaveCriticalSection(&this->m_guild_cs);

    return 1;
}

_GUILD_INFO_STRUCT* CGuildClass::SearchGuild(char* guildname)
{
    _GUILD_INFO_STRUCT* pNode = NULL;

    EnterCriticalSection(&this->m_guild_cs);

    std::map<std::string, _GUILD_INFO_STRUCT*>::iterator it;

    it = this->m_GuildMap.find(std::string(guildname));

    if (it == this->m_GuildMap.end())
    {
        LeaveCriticalSection(&this->m_guild_cs);
        return NULL;
    }

    pNode = it->second;

    LeaveCriticalSection(&this->m_guild_cs);

    return pNode;
}
//_GUILD_INFO_STRUCT* CGuildClass::SearchGuild(char* guildname)
//{
//    EnterCriticalSection(&this->m_guild_cs);
//
//    auto it = this->m_GuildMap.find(guildname);
//
//    if (it == this->m_GuildMap.end())
//    {
//        LeaveCriticalSection(&this->m_guild_cs);
//        return 0;
//    }
//
//    _GUILD_INFO_STRUCT* lpGuild = it->second;
//
//    LeaveCriticalSection(&this->m_guild_cs);
//
//    return lpGuild;
//}

_GUILD_INFO_STRUCT* CGuildClass::SearchGuild_Number(int number)
{
    EnterCriticalSection(&this->m_guild_cs);

    std::map<int, _GUILD_INFO_STRUCT*>::iterator it;

    it = this->m_GuildNumberMap.find(number);

    if (it == this->m_GuildNumberMap.end())
    {
        LeaveCriticalSection(&this->m_guild_cs);
        return NULL;
    }

    _GUILD_INFO_STRUCT* pNode = it->second;

    LeaveCriticalSection(&this->m_guild_cs);

    return pNode;
}

//_GUILD_INFO_STRUCT* CGuildClass::SearchGuild_Number(int number)
//{
//    _GUILD_INFO_STRUCT* pNode = NULL;
//    EnterCriticalSection(&this->m_guild_cs);
//
//    std::map<std::string, _GUILD_INFO_STRUCT*>::iterator it;
//
//    it = this->m_GuildNumberMap.find(number);
//
//    if (it == this->m_GuildNumberMap.end())
//    {
//        LeaveCriticalSection(&this->m_guild_cs);
//        return 0;
//    }
//
//    _GUILD_INFO_STRUCT* lpGuild = it->second;
//
//    LeaveCriticalSection(&this->m_guild_cs);
//
//    return lpGuild;
//}

_GUILD_INFO_STRUCT* CGuildClass::SearchGuild_NumberAndId(int number, char* name)
{
    _GUILD_INFO_STRUCT* pNode = this->SearchGuild_Number(number);

    if (!pNode)
    {
        return 0;
    }

    EnterCriticalSection(&this->m_guild_cs);

    for (int n = 0; n < 80; n++)
    {
        if (!strcmp(pNode->Names[n], name))
        {
            LeaveCriticalSection(&this->m_guild_cs);
            return pNode;
        }
    }

    LeaveCriticalSection(&this->m_guild_cs);

    return 0;
}

_GUILD_INFO_STRUCT* CGuildClass::AddMember(
    char* guild_name,
    char* player_name,
    int iGuildStatus,
    short pServer)
{
    _GUILD_INFO_STRUCT* pNode = this->SearchGuild(guild_name);

    if (!pNode)
    {
        return 0;
    }

    int blank = -1;

    EnterCriticalSection(&this->m_guild_cs);

    for (int n = 0; n < 80; n++)
    {
        if (pNode->Use[n] == 1)
        {
            if (!strcmp(pNode->Names[n], player_name))
            {
                LeaveCriticalSection(&this->m_guild_cs);
                return 0;
            }
        }
        else if (blank < 0)
        {
            blank = n;
        }
    }

    if (blank < 0)
    {
        LeaveCriticalSection(&this->m_guild_cs);
        return 0;
    }

    strcpy(pNode->Names[blank], player_name);

    pNode->Use[blank] = 1;
    pNode->pServer[blank] = pServer;
    pNode->GuildStatus[blank] = iGuildStatus;

    pNode->Count++;

    LeaveCriticalSection(&this->m_guild_cs);

    return pNode;
}

int CGuildClass::DelMember(char* guild_name, char* player_name)
{
    _GUILD_INFO_STRUCT* pNode = this->SearchGuild(guild_name);

    if (!pNode)
    {
        return 0;
    }

    EnterCriticalSection(&this->m_guild_cs);

    for (int n = 0; n < 80; n++)
    {
        if (pNode->Use[n] == 1)
        {
            if (!strcmp(pNode->Names[n], player_name))
            {
                pNode->Use[n] = 0;
                pNode->Count--;

                LeaveCriticalSection(&this->m_guild_cs);

                return 1;
            }
        }
    }

    LeaveCriticalSection(&this->m_guild_cs);

    return 0;
}

int CGuildClass::GetGuildMemberStatus(char* szGuildName, char* szMemberName)
{
    _GUILD_INFO_STRUCT* pNode = this->SearchGuild(szGuildName);

    if (!pNode)
    {
        return -1;
    }

    for (int n = 0; n < 80; n++)
    {
        if (pNode->Use[n] == 1)
        {
            if (!strcmp(pNode->Names[n], szMemberName))
            {
                return pNode->GuildStatus[n];
            }
        }
    }

    return -1;
}

int CGuildClass::SetGuildMemberStatus(
    char* szGuildName,
    char* szMemberName,
    int iGuildStatus)
{
    _GUILD_INFO_STRUCT* pNode = this->SearchGuild(szGuildName);

    if (!pNode)
    {
        return 0;
    }

    for (int n = 0; n < 80; n++)
    {
        if (pNode->Use[n] == 1)
        {
            if (!strcmp(pNode->Names[n], szMemberName))
            {
                pNode->GuildStatus[n] = iGuildStatus;
                return 1;
            }
        }
    }

    return 0;
}

int CGuildClass::GetGuildType(char* szGuildName)
{
    _GUILD_INFO_STRUCT* pNode = this->SearchGuild(szGuildName);

    if (!pNode)
    {
        return -1;
    }

    return pNode->btGuildType;
}

int CGuildClass::SetGuildType(char* szGuildName, BYTE iGuildType)
{
    _GUILD_INFO_STRUCT* pNode = this->SearchGuild(szGuildName);

    if (!pNode)
    {
        return 0;
    }

    pNode->btGuildType = iGuildType;

    return 1;
}

int CGuildClass::ConnectMember(char* guild_name, char* player_name, short pServer)
{
    _GUILD_INFO_STRUCT* pNode = this->SearchGuild(guild_name);

    if (!pNode)
    {
        return 0;
    }

    EnterCriticalSection(&this->m_guild_cs);

    for (int n = 0; n < 80; n++)
    {
        if (pNode->Use[n] == 1)
        {
            if (!strcmp(pNode->Names[n], player_name))
            {
                pNode->pServer[n] = pServer;

                LeaveCriticalSection(&this->m_guild_cs);

                return 1;
            }
        }
    }

    LeaveCriticalSection(&this->m_guild_cs);

    return 0;
}

int CGuildClass::DisconnectMember(char* guild_name, char* player_name)
{
    _GUILD_INFO_STRUCT* pNode = this->SearchGuild(guild_name);

    if (!pNode)
    {
        return 0;
    }

    EnterCriticalSection(&this->m_guild_cs);

    for (int n = 0; n < 80; n++)
    {
        if (pNode->Use[n] == 1)
        {
            if (!strcmp(pNode->Names[n], player_name))
            {
                pNode->pServer[n] = -1;

                LeaveCriticalSection(&this->m_guild_cs);

                return 1;
            }
        }
    }

    LeaveCriticalSection(&this->m_guild_cs);

    return 0;
}

_GUILD_INFO_STRUCT* CGuildClass::GetHead()
{
    return this->head;
}

void CGuildClass::AllGuildSend(int aIndex)
{
    _GUILD_INFO_STRUCT* pNode; // [esp+DCh] [ebp-14h]

    pNode = this->head;
    while (pNode)
    {
        EnterCriticalSection(&this->m_guild_cs);
        pNode = pNode->next;
        LeaveCriticalSection(&this->m_guild_cs);
    }
}

int CGuildClass::GuildListAddDlg(class CListCtrl* guildLB, class CListBox* memberLB)
{
    int count = 0;
    int viewCount = 0;

    LVITEMA li = {};
    li.mask = LVIF_TEXT;

    _GUILD_INFO_STRUCT* lpGuild = this->GetHead();

    if (this->pGuildViewCursor != 0)
    {
        lpGuild = this->pGuildViewCursor;
    }
    else
    {
        this->pGuildViewCursor = lpGuild;
    }

    this->GuildOnOff(0);

    while (lpGuild != 0)
    {
        EnterCriticalSection(&this->m_guild_cs);

        _GUILD_INFO_STRUCT* lpTemp = lpGuild;

        li.iItem = count;
        li.iSubItem = 0;
        li.pszText = lpTemp->Name;

        guildLB->InsertItem(&li);

        // Member count
        char szCount[32];

        _itoa(lpTemp->Count, szCount, 10);

        li.iSubItem = 1;
        li.pszText = szCount;

        guildLB->SetItem(&li);

        // Guild master
        li.iSubItem = 2;
        li.pszText = lpTemp->Names[0];
        guildLB->SetItem(&li);

        ++count;

        if (viewCount >= 100)
        {
            LeaveCriticalSection(&this->m_guild_cs);
            break;
        }

        ++viewCount;
        lpGuild = lpGuild->next;
        LeaveCriticalSection(&this->m_guild_cs);
    }

    this->pGuildViewCursor = lpGuild;
    this->GuildOnOff(1);

    return 0;
}

DWORD WINAPI GuildSendThread(LPVOID* p)
{
    int count = GuildSendThreadCount++;

    CGuildManager.GuildOnOff(0);

    EnterCriticalSection(&CGuildManager.m_guild_cs);

    DGGuildInfoSendState((int)p, 0, CGuildManager.m_count);

    LeaveCriticalSection(&CGuildManager.m_guild_cs);

    _GUILD_INFO_STRUCT* pNode = CGuildManager.GetHead();

    while (pNode)
    {
        EnterCriticalSection(&CGuildManager.m_guild_cs);

        if (!DGGuildInfoSend((int)p,pNode->Name,pNode->Names[0],pNode->Mark,pNode->TotalScore,pNode->Number))
        {
            LeaveCriticalSection(&CGuildManager.m_guild_cs);
            break;
        }

        pNode = pNode->next;

        LeaveCriticalSection(&CGuildManager.m_guild_cs);

        Sleep(1000);
    }

    CGuildManager.GuildOnOff(1);

    EnterCriticalSection(&CGuildManager.m_guild_cs);

    DGGuildInfoSendState((int)p, 1, 0);

    LeaveCriticalSection(&CGuildManager.m_guild_cs);

    return 0;
}