#pragma once

#include <windows.h>
#include <map>
#include <string>
#pragma message("Name and Names sizes are 13 to match with the original")
struct _GUILD_INFO_STRUCT
{
    int Number;
    char Name[13];
    BYTE Mark[MAX_GUILDMARKBUFFER];
    BYTE Count;

    char Names[MAX_GUILD][13];
    BYTE Use[MAX_GUILD];
    short pServer[MAX_GUILD];

    BYTE PlayScore;
    int TotalScore;

    char Notice[MAX_GUILDNOTICE];

    int IsSendGuildInfo;

    int GuildStatus[MAX_GUILD];

    BYTE btGuildType;

    int iGuildUnion;
    int iGuildRival;

    _GUILD_INFO_STRUCT* back;
    _GUILD_INFO_STRUCT* next;
};

class CGuildClass
{
public:
    CGuildClass();
    virtual ~CGuildClass();

public:
    void Init();

    void GuildOnOff(int on_off);
    int GetGuildOnOff();
    BOOL IsCreateGuild();
    void ClearSendGuildInfo();
    _GUILD_INFO_STRUCT* AddGuild(int number,char* guildname,BYTE* mark,char* mastername,char* notice,int score,int SystemCheck);
    void AddTail(_GUILD_INFO_STRUCT* pNewNode);
    void Print();
    void PrintGuild(char* guildname);
    void AllDelete();
    int DeleteGuild(char* guildname, char* commander);
    _GUILD_INFO_STRUCT* SearchGuild(char* guildname);
    _GUILD_INFO_STRUCT* SearchGuild_Number(int number);
    _GUILD_INFO_STRUCT* SearchGuild_NumberAndId(int number, char* name);
    _GUILD_INFO_STRUCT* AddMember(char* guild_name,char* player_name,int iGuildStatus,short pServer);
    int DelMember(char* guild_name, char* player_name);
    int GetGuildMemberStatus(char* szGuildName, char* szMemberName);
    int SetGuildMemberStatus(char* szGuildName,char* szMemberName,int iGuildStatus);
    int GetGuildType(char* szGuildName);
    int SetGuildType(char* szGuildName, BYTE iGuildType);
    int ConnectMember(char* guild_name, char* player_name, short pServer);
    int DisconnectMember(char* guild_name, char* player_name);
    _GUILD_INFO_STRUCT* GetHead();
    void AllGuildSend(int aIndex);
    int GuildListAddDlg(class CListCtrl* guildLB, class CListBox* memberLB);

public:
    std::map<std::string, _GUILD_INFO_STRUCT*> m_GuildMap;
    std::map<int, _GUILD_INFO_STRUCT*> m_GuildNumberMap;

    _GUILD_INFO_STRUCT* head;
    _GUILD_INFO_STRUCT* tail;

    int m_count;

    int m_OnOff;
    int m_OnOffCount;

    CRITICAL_SECTION m_guild_cs;

    _GUILD_INFO_STRUCT* pGuildViewCursor;
};


extern int GuildSendThreadCount;

unsigned int __stdcall GuildSendThread(void* p);