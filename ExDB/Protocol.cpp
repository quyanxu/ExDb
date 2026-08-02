// Protocol.cpp
#include "stdafx.h"
#include "Protocol.h"

#include "FriendClass.h"
#include "GameServerMng.h"
#include "GensSystem.h"
#include "PartyMatching.h"
#include "TUnionEx.h"
#include "WZIocpEngine.h"
#include "WZQueue.h"
#include "..\common\winutil.h"
#include "dbset/CastleDbSet.h"
#include "dbset/FriendDbSet.h"
#include "dbset/GensSystemDBSet.h"
#include "dbset/GuildDbSet.h"
#include "dbset/PartyMatchingDBSet.h"


// ----------------------------------------------------------------------
// External Globals
// ----------------------------------------------------------------------

MSync<int> g_Sync; // idb

// ----------------------------------------------------------------------
// Protocol Dispatcher
// ----------------------------------------------------------------------

int ProtocolCore(BYTE ProtocolNumber, LPBYTE RecvBuffer, SIZE_T RecvLength, int Index)
{
#ifdef DEBUG_PROTOCOL
	cLog.Add("Protocol:0x%x", ProtocolNumber);
#endif
	switch (ProtocolNumber)
	{
		// =========================================================
		// Server Connection / Character
		// =========================================================
	case 0x00:
		GetJoinInfo((SDHP_SERVERINFO*)RecvBuffer, Index);
		break;

	case 0x02:
		GDCharacterClose((SDHP_USERCLOSE*)RecvBuffer, Index);
		break;
		// =========================================================
		// Guild Management (0x30 - 0x42)
		// =========================================================
	case 0x30:
		GSGuildCreate((SDHP_GUILDCREATE*)RecvBuffer, Index);
		break;

	case 0x31:
		GDGuildDestroy((SDHP_GUILDDESTROY*)RecvBuffer, Index);
		break;

	case 0x32:
		GDGuildMemberAdd((SDHP_GUILDMEMBERADD*)RecvBuffer, Index);
		break;

	case 0x33:
		GDGuildMemberDel((SDHP_GUILDMEMBERDEL*)RecvBuffer, Index);
		break;

	case 0x35:
		DGGuildMemberInfoRequest((SDHP_GUILDMEMBER_INFO_REQUEST*)RecvBuffer, Index);
		break;

	case 0x37:
		GDGuildScoreUpdateRecv((SDHP_GUILDSCOREUPDATE*)RecvBuffer);
		break;

	case 0x38:
		GDGuildNoticeRecv((SDHP_GUILDNOTICE*)RecvBuffer);
		break;

	case 0x39:
		GDGuildMemberAddWithoutUserIndex((SDHP_GUILDMEMBERADD_WITHOUT_USERINDEX*)RecvBuffer, Index);
		break;

	case 0x40:
		GDGuildListRequest();
		break;

	case 0x42:
		GDGuildAllSend(Index, (SDHP_GUILDMEMBER_INFO_GUILDNAME_REQUEST*)RecvBuffer);
		break;

		// =========================================================
		// Server Group Chatting (0x50 - 0x53)
		// =========================================================
	case 0x50:
		GDGuildServerGroupChatting((EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND*)RecvBuffer, Index);
		break;

	case 0x51:
		GDUnionServerGroupChatting((EXSDHP_SERVERGROUP_UNION_CHATTING_SEND*)RecvBuffer, Index);
		break;

	case 0x52:
		GDGensServerGroupChatting((EXSDHP_SERVERGROUP_GENS_CHATTING_SEND*)RecvBuffer, Index);
		break;

	case 0x53:
	{
		PBMSG_HEAD2* lpDef = (PBMSG_HEAD2*)RecvBuffer;
		switch (lpDef->subcode)
		{
		case 0x01:
			GDRewardGuildPeriodBuffInsert((PMSG_REQ_GUILD_PERIODBUFF_INSERT*)RecvBuffer);
			break;
		case 0x02:
			GDRewardGuildPeriodBuffDelete((PMSG_REQ_GUILD_PERIODBUFF_DELETE*)RecvBuffer);
			break;
		}
		break;
	}
	/*Type = RecvBuffer->Type;
	if (Type == 1)
		GDRewardGuildPeriodBuffInsert((PMSG_REQ_GUILD_PERIODBUFF_INSERT*)RecvBuffer);
	else if (Type == 2)
		GDRewardGuildPeriodBuffDelete((PMSG_REQ_GUILD_PERIODBUFF_DELETE*)RecvBuffer);
	break;*/

	// =========================================================
	// Friend System (0x60 - 0x66) - Queue-based
	// =========================================================
	case 0x60:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	{
		if (g_bEnableFriend)
		{
			//PBMSG_HEAD* lpHead = (PBMSG_HEAD*)RecvBuffer;
			FriendQueue.AddToQueue(RecvBuffer, RecvLength, ProtocolNumber, Index);
		}
	}
	/*if (g_bEnableFriend)
		FriendQueue.AddToQueue(&RecvBuffer->h.c, RecvLength, ProtocolNumber, Index);*/
	break;

	case 0x66:
		GDFriendChatRoomCreateRecv((FHP_FRIEND_CHATROOM_CREATE_REQ*)RecvBuffer, Index);
		break;

		// =========================================================
		// Friend Mail System (0x70 - 0x74) - Queue-based
		// =========================================================
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	{
		if (g_bEnableFriendMail)
		{
			MailQueue.AddToQueue(RecvBuffer, RecvLength, ProtocolNumber, Index);
		}
	}
	//if (g_bEnableFriendMail)
	//	MailQueue.AddToQueue(&RecvBuffer->h.c, RecvLength, ProtocolNumber, Index);
	break;

	case 0x74:
		GDFriendInvitationRecv((FHP_FRIEND_INVITATION_REQ*)RecvBuffer, Index);
		break;

		// =========================================================
		// Chat Room (0xA0 - 0xA2)
		// =========================================================
	case 0xA0:
		CDCreateChatRoomResult((FHP_CHAT_ROOMCREATE_RESULT*)RecvBuffer);
		break;

	case 0xA1:
		DGRoomInvitationRecv();
		break;

	case 0xA2:
		DCChatRoomInvitation((FHP_CHAT_ROOMINVITATION*)RecvBuffer);
		break;

		// =========================================================
		// Guild Matching (0xA3)
		// =========================================================
	case 0xA3:
	{
		//switch (RecvBuffer->Type)
		PBMSG_HEAD2* lpDef = (PBMSG_HEAD2*)RecvBuffer;
		switch (lpDef->subcode)
		{
		case 0:
			GDReqGuildMatchingList((_stReqGuildMatchingList*)RecvBuffer, Index);
			break;
		case 1:
			GDReqGuildMatchingListSearchWord((_stReqGuildMatchingListSearchWord*)RecvBuffer, Index);
			break;
		case 2:
			GDInsertGuildMatchingList((_stReqGuildMatchingData*)RecvBuffer, Index);
			break;
		case 3:
			GDDeleteGuildMatching((_stReqDelGuildMatchingList*)RecvBuffer, Index);
			break;
		case 4:
			GDReqJoinGuildMatching((_stRegWaitGuildMatching*)RecvBuffer, Index);
			break;
		case 5:
			GDReqCancelJoinGuildMatching((_stReqDelWaitGuildMatchingList*)RecvBuffer, Index);
			break;
		case 6:
			GDReqAllowJoinGuildMatching((_stReqAllowJoinGuildMatching*)RecvBuffer, Index);
			break;
		case 7:
			GDReqWaitGuildMatchingList((_stReqWaitGuildMatchingList*)RecvBuffer, Index);
			break;
		case 8:
			GDReqGetWaitStateListGuildMatching((_stReqWaitStateListGuildMatching*)RecvBuffer, Index);
			break;
		default:
			return 1;
		}
		break;
	}
	// =========================================================
	// Party Matching (0xA4)
	// =========================================================
	case 0xA4:
		//switch (RecvBuffer->Type)
	{
		PBMSG_HEAD2* lpDef = (PBMSG_HEAD2*)RecvBuffer;
		switch (lpDef->subcode)
		{
		case 0x00:
			GDReqRegPartyMatchingList((_stReqRegWantedPartyMember*)RecvBuffer, Index);
			break;
		case 0x01:
			GDReqGetPartyMatchingList((_stReqGetPartyMatchingList*)RecvBuffer, Index);
			break;
		case 0x02:
			GDReqMemeberJoinPartyMatching((_stReqJoinMemberPartyMatching*)RecvBuffer, Index);
			break;
		case 0x03:
			GDReqMemberJoinStateList((_stReqJoinMemberStateListPartyMatching*)RecvBuffer, Index);
			break;
		case 0x04:
			GDReqMemberJoinStateListLeader((_stReqWaitListPartyMatching*)RecvBuffer, Index);
			break;
		case 0x05:
			GDReqAddPartyMember((__stReqAddPartyMember*)RecvBuffer, Index);
			break;
		case 0x06:
			GDReqCancelPartyMatching((_stReqCancePartyMatching*)RecvBuffer, Index);
			break;
		case 0x07:
			GDDeletePartyUser((_stReqDelPartyUserPartyMatching*)RecvBuffer, Index);
			break;
		case 0x11:
			GDReqSendPartyMemberList((__stReqSendPartyMemberList*)RecvBuffer);
			break;
		case 0x12:
			GDSendChattingMsgPartyMatching((_stReqChattingPartyMatching*)RecvBuffer);
			break;
		case 0x13:
			GDReqSwapPartyLeader((_stReqChangePartyLeader*)RecvBuffer, Index);
			break;
		default:
			return 1;
		}
		break;
	}
	// =========================================================
	// Guild Status & Type (0xE1 - 0xE2)
	// =========================================================
	case 0xE1:
		GDGuildReqAssignStatus((EXSDHP_GUILD_ASSIGN_STATUS_REQ*)RecvBuffer, Index);
		break;

	case 0xE2:
		GDGuildReqAssignType((EXSDHP_GUILD_ASSIGN_TYPE_REQ*)RecvBuffer, Index);
		break;

		// =========================================================
		// Guild Relationships (0xE5 - 0xEB)
		// =========================================================
	case 0xE5:
		GDRelationShipReqJoin((EXSDHP_RELATIONSHIP_JOIN_REQ*)RecvBuffer, Index);
		break;

	case 0xE6:
		GDRelationShipReqBreakOff((EXSDHP_RELATIONSHIP_BREAKOFF_REQ*)RecvBuffer, Index);
		break;

	case 0xE9:
		GDUnionListReq((EXSDHP_UNION_LIST_REQ*)RecvBuffer, Index);
		break;

	case 0xEB:
	{
		//if (RecvBuffer->Type == 1)
		////GDRelationShipReqKickOutUnionMember((EXSDHP_KICKOUT_UNIONMEMBER_REQ*)RecvBuffer, Index);
		PBMSG_HEAD2* lpDef = (PBMSG_HEAD2*)RecvBuffer;
		if (lpDef->subcode)
		{
			if (lpDef->subcode == 1)
				GDRelationShipReqKickOutUnionMember((EXSDHP_KICKOUT_UNIONMEMBER_REQ*)RecvBuffer, Index);
		}
		break;
	}
	// =========================================================
	// Gens System (0xF8)
	// =========================================================
	case 0xF8:
		//if (RecvBuffer->Type)
	{
		PBMSG_HEAD2* lpDef = (PBMSG_HEAD2*)RecvBuffer;
#ifdef DEBUG_PROTOCOL
		cLog.Add("GENSProtocol:0x%x", RecvBuffer->Type);
#endif
		//switch (RecvBuffer->Type | 0xF800)
		WORD wPacketType = MAKEWORD(lpDef->subcode, 0xF8);
		switch (wPacketType)
		{
		case 0xF801:
			GDReqGensInfo((_tagPMSG_REQ_GENS_INFO_EXDB*)RecvBuffer, Index);
			break;
		case 0xF803:
			GDReqRegGensMember((_tagPMSG_REQ_REG_GENS_MEMBER_EXDB*)RecvBuffer, Index);
			break;
		case 0xF805:
			GDReqSecedeGensMember((_tagPMSG_REQ_SECEDE_GENS_MEMBER_EXDB*)RecvBuffer, Index);
			break;
		case 0xF807:
			GDReqSaveContributePoint((_tagPMSG_REQ_SAVE_CONTRIBUTE_POINT_EXDB*)RecvBuffer);
			break;
		case 0xF808:
			GDReqSaveContributePoint((_tagPMSG_REQ_SAVE_ABUSING_KILLUSER_EXDB*)RecvBuffer);
			break;
		case 0xF809:
			GDReqAbusingInfo((_tagPMSG_REQ_ABUSING_INFO*)RecvBuffer, Index);
			break;
		case 0xF80B:
			GDReqGensSecedeDateModify((_tagPMSG_REQ_SECEDE_GENS_DATE_MODIFY*)RecvBuffer);
			break;
		case 0xF80C:
			GDReqGensReward((_tagPMSG_REQ_GENS_REWARD_EXDB*)RecvBuffer, Index);
			break;
		case 0xF80E:
			GDReqGensRewardComplete((_tagPMSG_REQ_GENS_REWARD_COMPLETE_EXDB*)RecvBuffer);
			break;
		case 0xF80F:
			GDReqGensMemberCount((_tagPMSG_REQ_GENS_MEMBER_COUNT_EXDB*)RecvBuffer, Index);
			break;
		case 0xF811:
			GDReqGensSetRewardDay((_tagPMSG_REQ_SET_GENS_REWARD_DAY_EXDB*)RecvBuffer);
			break;
		case 0xF812:
			GDReqGensSetRanking((_tagPMSG_REQ_SET_GENS_RANKING_EXDB*)RecvBuffer);
			break;
		case 0xF813:
			GDReqGensRewardDayCheck((_tagPMSG_REQ_GENS_REWARD_DAY_CHECK_EXDB*)RecvBuffer, Index);
			break;
		default:
			return 1;
		}
	}
	break;

	// =========================================================
	// Unknown Protocol
	// =========================================================
	default:
		return 1;
	}

	return 1;
}

int cSend(BYTE* Buffer, int size)
{
	if (gsm.m_CSIndex >= 0)
	{
		WzIoEngine->send(gsm.m_Obj[gsm.m_CSIndex].lpPHD, Buffer, size);
		return 1;
	}
	else
	{
		cLog.Add("error-L3 : Chat Server Index %d", gsm.m_CSIndex);
		return 0;
	}
}

void GetJoinInfo(SDHP_SERVERINFO* lpMsg, short aIndex)
{
	SDHP_RESULT pResult;

	pResult.h.c = 0xC1;
	pResult.h.headcode = 0x00;
	pResult.h.size = sizeof(pResult);
	pResult.Result = 1;

	if (!gsm.Set(aIndex, lpMsg->Port, lpMsg->Type, lpMsg->ServerName, lpMsg->ServerCode, lpMsg->btGameServerType))
	{
		pResult.Result = 0;
		cLog.Add("Client Join Fail");
	}

	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pResult.h.c, sizeof(pResult));
}

void GDCharacterClose(SDHP_USERCLOSE* lpMsg, short aIndex)
{
	char szGuildName[20];
	char szChar[16];

	memset(szChar, 0, sizeof(szChar));
	memset(szGuildName, 0, sizeof(szGuildName));

	memcpy(szChar, lpMsg->CharName, MAX_IDSTRING);
	memcpy(szGuildName, lpMsg->GuildName, MAX_GUILDNAMESTRING);

	// Party Matching
	if (g_PartyMatching.DisConnectMember(szChar) == TRUE)
	{
		_PARTY_INFO_STRUCT* lpPartyInfo = g_PartyMatching.SearchPartyMemberName(szChar);
		if (lpPartyInfo != NULL)
		{
			DGSendPartyMember(lpPartyInfo->stPartyMember[0].Name, 0);
		}
	}

	g_PartyMatching_DBSet.DeleteWaitList(szChar);

	// Friend Offline Queue
	if (strlen(szChar) >= 4)
	{
		FriendQueue.AddToQueue((BYTE*)szChar, 0x0B, 0x02, aIndex);
	}

	// Guild Disconnect
	if (lpMsg->Type != 1)
	{
		CGuildManager.DisconnectMember(szGuildName, szChar);
		cLog.AddTD("guild member close : %s %s", szGuildName, szChar);

		for (int n = 0; n < MAX_SERVEROBJECT; n++)
		{
			if (gsm.m_Obj[n].m_Used)
			{
				if (gsm.m_Obj[n].m_Type == 1)
				{
					WzIoEngine->send(gsm.m_Obj[n].lpPHD, &lpMsg->h.c, 22);
				}
			}
		}
	}
}

void GSGuildCreate(SDHP_GUILDCREATE* lpMsg, int aIndex)
{
	SDHP_GUILDCREATE_RESULT pMsg;

	PHeadSetB((LPBYTE)&pMsg, 0x30, sizeof(SDHP_GUILDCREATE_RESULT));

	pMsg.Result = 0;
	pMsg.Flag = 0;

	pMsg.NumberH = lpMsg->NumberH;
	pMsg.NumberL = lpMsg->NumberL;

	char guildname[MAX_GUILDNAMESTRING + 1] = { 0 };
	char guildmaster[MAX_IDSTRING + 1] = { 0 };

	memcpy(guildname, lpMsg->GuildName, MAX_GUILDNAMESTRING);
	memcpy(guildmaster, lpMsg->Master, MAX_IDSTRING);

	memset(pMsg.Master, 0, sizeof(pMsg.Master));
	memcpy(pMsg.Master, guildmaster, MAX_IDSTRING);

	memset(pMsg.GuildName, 0, sizeof(pMsg.GuildName));
	memcpy(pMsg.GuildName, guildname, MAX_GUILDNAMESTRING);

	if (strlen(guildname) < 4)
	{
		pMsg.Result = 2;
		pMsg.Flag = 1;

		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg, sizeof(SDHP_GUILDCREATE_RESULT));

		return;
	}

	if (strlen(guildmaster) < 3)
	{
		pMsg.Result = 2;
		pMsg.Flag = 1;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg, sizeof(SDHP_GUILDCREATE_RESULT));
		cLog.Add("error-L2 : Guild Master Name [%s]", guildmaster);

		return;
	}

	memcpy(pMsg.Mark, lpMsg->Mark, sizeof(pMsg.Mark));

	if (!SpaceSyntexCheck(guildname))
	{
		pMsg.Result = 4;
		pMsg.Flag = 1;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg, sizeof(SDHP_GUILDCREATE_RESULT));

		return;
	}

	if (!SQLSyntexCheck(guildname))
	{
		pMsg.Result = 5;
		pMsg.Flag = 1;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg, sizeof(SDHP_GUILDCREATE_RESULT));

		return;
	}

	if (cBadStrChk.CmpString(guildname) == TRUE || gMuName.CmpString(guildname) == TRUE)
	{
		pMsg.Result = 6;
		pMsg.Flag = 1;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg, sizeof(SDHP_GUILDCREATE_RESULT));

		return;
	}

	BOOL guildcreate_option = CGuildManager.IsCreateGuild();

	if (guildcreate_option)
	{
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg, sizeof(SDHP_GUILDCREATE_RESULT));
		cLog.Add("error-L3 : Guild not created. Guild:[%s] Master:[%s] ThreadCount:[%d]", guildname, guildmaster, guildcreate_option);
		return;
	}

	int Result = GuildDbSet->CreateGuild(guildname, guildmaster, lpMsg->Mark);

	switch (Result)
	{
	case 0:
		break;
	case 1:
		cLog.Add("error-L3 : Guild create failed Guild:[%s] Master:[%s]", guildname, guildmaster);
		break;
	case 2:
		pMsg.Result = 3;
		cLog.Add("error-L3 : he is member Guild:[%s] Master:[%s]", guildname, guildmaster);
		break;
	case 3:
		pMsg.Result = 3;
		cLog.Add("error-L3 : (DB) he is member Guild:[%s] Master:[%s]", guildname, guildmaster);
		break;
	case 4:
		pMsg.Result = 0;
		cLog.Add("error-L3 : (DB) existing guild :[%s] Master:[%s]", guildname, guildmaster);
		break;
	default:
		cLog.Add("error-L3 : Unknown error Guild:[%s] Master:[%s]", guildname, guildmaster);
		break;
	}

	if (Result != 0)
	{
		pMsg.Flag = 1;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg, sizeof(SDHP_GUILDCREATE_RESULT));
		return;
	}

	int GuildNumber = GuildDbSet->GetNumber(guildname);

	pMsg.Result = 1;
	pMsg.GuildNumber = GuildNumber;
	pMsg.btGuildType = lpMsg->btGuildType;

	_GUILD_INFO_STRUCT* lpGuildInfo = CGuildManager.AddGuild(GuildNumber, guildname, lpMsg->Mark, guildmaster, 0, -1, TRUE);

	if (lpGuildInfo == NULL)
	{
		return;
	}

	if (GuildDbSet->UpdateGuildMemberStatus(guildname, guildmaster, 128) == FALSE)
	{
		pMsg.Flag = 1;
		pMsg.Result = 0;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg, sizeof(SDHP_GUILDCREATE_RESULT));
		cLog.Add("error-L3 : UpdateGuildMemberStatus :[%s] Master:[%s]", guildname, guildmaster);
		return;
	}

	if (GuildDbSet->UpdateGuildType(guildname, lpMsg->btGuildType) == FALSE)
	{
		pMsg.Flag = 1;
		pMsg.Result = 0;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg, sizeof(SDHP_GUILDCREATE_RESULT));
		cLog.Add("error-L3 : UpdateGuildType :[%s] Master:[%s]", guildname, guildmaster);
		return;
	}

	if (UnionExManager.AddUnion(lpGuildInfo->Number, lpGuildInfo, 0) == 1)
	{
		TRACE_OUT(3, "Union [ %d ] : [ %s ] [ %d ] ", lpGuildInfo->iGuildUnion, lpGuildInfo->Name, lpGuildInfo->Number);
	}
	else
	{
		TRACE_OUT(2, "Union [ %d ] : [ %s ] [ %d ] ", lpGuildInfo->iGuildUnion, lpGuildInfo->Name, lpGuildInfo->Number);
	}

	for (int n = 0; n < MAX_SERVEROBJECT; n++)
	{
		if (!gsm.m_Obj[n].m_Used)
		{
			continue;
		}

		if (gsm.m_Obj[n].m_Type != 1)
		{
			continue;
		}

		pMsg.Flag = (aIndex == n);

		WzIoEngine->send(gsm.m_Obj[n].lpPHD, (BYTE*)&pMsg, sizeof(SDHP_GUILDCREATE_RESULT));
	}

	DGGuildMemberInfoSend(aIndex, guildmaster, gsm.m_Obj[aIndex].pServer);

	cLog.AddTD("Guild Created [%s][%s]", guildmaster, guildname);
}

void DGGuildMemberInfoRequest(SDHP_GUILDMEMBER_INFO_REQUEST* lpMsg, int aIndex)
{
	char Master[16];
	memset(Master, 0, MAX_IDSTRING + 1);           // 0x0B = 11 bytes (MAX_IDSTRING + 1)
	memcpy(Master, lpMsg->MemberID, MAX_IDSTRING);  // 0x0A = 10 bytes (MAX_IDSTRING)
	DGGuildMemberInfoSend(aIndex, Master, gsm.m_Obj[aIndex].pServer);
}

void DGGuildMemberInfoSend(int aIndex, char* name, short pServer)
{
	if (name == NULL || strlen(name) < 3)
	{
		cLog.Add("error-L2 : Guild Info send %s %s %d",
			name,
			__FILE__,
			__LINE__);
		return;
	}

	char GuildName[MAX_GUILDNAMESTRING + 1];
	memset(GuildName, 0, sizeof(GuildName));

	ST_GUILD_COMMON_DBINFO stGuildCommonDBInfo;
	memset(&stGuildCommonDBInfo, 0, sizeof(stGuildCommonDBInfo));

	int iGuildStatus = -1;

	// Get guild info from DB
	if (GuildDbSet->GetDBGuildMemberInfo(name, &stGuildCommonDBInfo) != 1)
	{
		return;
	}

	memcpy(GuildName, stGuildCommonDBInfo.m_szGuildName, MAX_GUILDNAMESTRING);
	iGuildStatus = stGuildCommonDBInfo.m_iGuildMemberStatus;
	int level = stGuildCommonDBInfo.m_iGuildLevel;

	// Send full guild info
	GDGuildAllSend(aIndex, GuildName);

	_GUILD_INFO_STRUCT* lpGuild = CGuildManager.SearchGuild(GuildName);

	if (lpGuild == NULL || lpGuild->Count <= 0)
	{
		return;
	}

	// Send union/rival relationship lists
	DGRelationShipListSend(lpGuild, 1, TRUE, aIndex);
	DGRelationShipListSend(lpGuild, 2, TRUE, aIndex);

	SDHP_GUILDMEMBER_INFO pMsg;
	//PHeadSubSetB
	pMsg.h.set(0x35, sizeof(SDHP_GUILDMEMBER_INFO));

	memset(pMsg.GuildName, 0, sizeof(pMsg.GuildName));
	memset(pMsg.MemberID, 0, sizeof(pMsg.MemberID));

	memcpy(pMsg.MemberID, name, MAX_IDSTRING);
	memcpy(pMsg.GuildName, GuildName, MAX_GUILDNAMESTRING);

	pMsg.btGuildStatus = iGuildStatus;
	pMsg.pServer = pServer;

	cLog.Add("Guild information send : name:%s guildname:%s", name, GuildName);

	// Broadcast to all GameServers
	for (int n = 0; n < MAX_SERVEROBJECT; n++)
	{
		if (gsm.m_Obj[n].m_Used == FALSE)
			continue;

		if (gsm.m_Obj[n].m_Type != 1)
			continue;

		WzIoEngine->send(gsm.m_Obj[n].lpPHD, (BYTE*)&pMsg, sizeof(SDHP_GUILDMEMBER_INFO));
	}

	// Mark member connected
	CGuildManager.ConnectMember(GuildName, name, pServer);
}

void GDGuildAllSend(int aIndex, char* GuildName)
{
	_GUILD_INFO_STRUCT* lpGuild = CGuildManager.SearchGuild(GuildName);

	if (lpGuild == NULL)
		return;

	if (lpGuild->Count <= 0)
		return;

	if (lpGuild->IsSendGuildInfo)
		return;

	BYTE sendbuf[2056];
	memset(sendbuf, 0, sizeof(sendbuf));

	SDHP_GUILDALL_COUNT pCount;
	memset(&pCount, 0, sizeof(pCount));

	int  lOfs = sizeof(SDHP_GUILDALL_COUNT);
	BYTE count = 0;

	// =========================================
	// Build member list
	// =========================================
	for (int n = 0; n < MAX_GUILD_MEMBER; n++)
	{
		if (lpGuild->Use[n] == FALSE)
			continue;

		SDHP_GUILDALL pList;
		memset(&pList, 0, sizeof(pList));

		memcpy(pList.MemberID, lpGuild->Names[n], MAX_IDSTRING);

		pList.pServer = lpGuild->pServer[n];
		pList.btGuildStatus = lpGuild->GuildStatus[n];

		memcpy(&sendbuf[lOfs], &pList, sizeof(SDHP_GUILDALL));

		lOfs += sizeof(SDHP_GUILDALL);

		count++;

		cLog.Add("guild member send %s", lpGuild->Names[n]);
	}

	// =========================================
	// Header
	// =========================================
	pCount.h.c = 0xC2;
	pCount.h.headcode = 0x36;

	pCount.h.sizeH = HIBYTE(lOfs);
	pCount.h.sizeL = LOBYTE(lOfs);

	// =========================================
	// Guild info
	// =========================================
	pCount.Count = count;
	pCount.Number = lpGuild->Number;
	pCount.score = lpGuild->TotalScore;

	memcpy(pCount.Mark, lpGuild->Mark, sizeof(pCount.Mark));

	// Guild master = first guild member
	memcpy(pCount.Master, lpGuild->Names[0], MAX_IDSTRING);

	pCount.btGuildType = lpGuild->btGuildType;

	pCount.iGuildUnion = lpGuild->iGuildUnion;
	pCount.iGuildRival = lpGuild->iGuildRival;

	// Rival guild name
	if (lpGuild->iGuildRival)
	{
		_GUILD_INFO_STRUCT* lpGuildInfo = CGuildManager.SearchGuild_Number(lpGuild->iGuildRival);

		if (lpGuildInfo)
		{
			memcpy(pCount.szGuildRivalName, lpGuildInfo->Name, MAX_GUILDNAMESTRING);
		}
		else
		{
			pCount.szGuildRivalName[0] = 0;
		}
	}

	memcpy(pCount.GuildName, lpGuild->Name, MAX_GUILDNAMESTRING);

	// Copy header structure to packet
	memcpy(sendbuf, &pCount, sizeof(SDHP_GUILDALL_COUNT));

	// =========================================
	// Broadcast to GameServers
	// =========================================
	for (int i = 0; i < MAX_SERVEROBJECT; i++)
	{
		if (gsm.m_Obj[i].m_Used == FALSE)
			continue;

		if (gsm.m_Obj[i].m_Type != 1)
			continue;

		WzIoEngine->send(gsm.m_Obj[i].lpPHD, sendbuf, lOfs);
	}

	lpGuild->IsSendGuildInfo = TRUE;

	cLog.AddTD("望萄 薑爾 爾鹵 [%s]", GuildName);

	// =========================================
	// Guild Matching
	// =========================================
	if (GuildDbSet->GuildMatchingRegCheck(lpGuild->Number) > 0)
	{
		DGAnsUseGuildMatchingGuild(lpGuild->Number);
		if (GuildDbSet->GetIsApplicantListGuildMatching(lpGuild->Number) == 1)
		{
			DGAnsSendNotiGuildMatchingForGuildMaster(lpGuild->Number);
		}
	}
}

void GDGuildAllSend(int aIndex, SDHP_GUILDMEMBER_INFO_GUILDNAME_REQUEST* lpRecv)
{
	_GUILD_INFO_STRUCT* lpGuild = 0;
	_GUILD_INFO_STRUCT* lpGuildInfo = 0;
#pragma message("GuildName 20?")
	char GuildName[20] = { 0 };
	memcpy(GuildName, lpRecv->szGuildName, MAX_GUILDNAMESTRING);
	GuildName[MAX_GUILDNAMESTRING] = '\0';

	lpGuild = CGuildManager.SearchGuild(GuildName);

	if (!lpGuild || lpGuild->Count == 0 || lpGuild->IsSendGuildInfo)
		return;

	SDHP_GUILDALL_COUNT pCount;
	SDHP_GUILDALL pList;
	unsigned char sendbuf[2056] = { 0 };

	int lOfs = sizeof(SDHP_GUILDALL_COUNT);
	int count = 0;

	for (int n = 0; n < MAX_GUILD_MEMBER; n++)
	{
		if (!lpGuild->Use[n])
			continue;

		memset(&pList, 0, sizeof(pList));

		memcpy(pList.MemberID, lpGuild->Names[n], MAX_IDSTRING);
		pList.pServer = lpGuild->pServer[n];
		pList.btGuildStatus = lpGuild->GuildStatus[n];

		memcpy(&sendbuf[lOfs], &pList, sizeof(SDHP_GUILDALL));
		lOfs += sizeof(SDHP_GUILDALL);
		count++;

		cLog.Add("guild member send %s", lpGuild->Names[n]);
	}

	pCount.h.c = 0xC2;
	pCount.h.headcode = 0x36;

	pCount.h.sizeH = HIBYTE(lOfs);
	pCount.h.sizeL = LOBYTE(lOfs);

	pCount.Number = lpGuild->Number;
	pCount.score = lpGuild->TotalScore;

	memcpy(pCount.Mark, lpGuild->Mark, sizeof(pCount.Mark));
	memcpy(pCount.Master, lpGuild->Names[0], MAX_IDSTRING);

	pCount.btGuildType = lpGuild->btGuildType;
	pCount.iGuildUnion = lpGuild->iGuildUnion;
	pCount.iGuildRival = lpGuild->iGuildRival;

	pCount.Count = count;

	if (lpGuild->iGuildRival)
	{
		lpGuildInfo = CGuildManager.SearchGuild_Number(lpGuild->iGuildRival);

		if (lpGuildInfo)
			memcpy(pCount.szGuildRivalName, lpGuildInfo->Name, MAX_GUILDNAMESTRING);
		else
			memset(pCount.szGuildRivalName, 0, sizeof(pCount.szGuildRivalName));
	}
	else
	{
		memset(pCount.szGuildRivalName, 0, sizeof(pCount.szGuildRivalName));
	}

	memcpy(sendbuf, &pCount, sizeof(SDHP_GUILDALL_COUNT));

	for (int i = 0; i < MAX_SERVEROBJECT; i++)
	{
		if (gsm.m_Obj[i].m_Used && gsm.m_Obj[i].m_Type == 1)
		{
			WzIoEngine->send(gsm.m_Obj[i].lpPHD, sendbuf, lOfs);
		}
	}

	lpGuild->IsSendGuildInfo = 1;

	cLog.AddTD("Guild info sent [%s]", GuildName);
}

int DGGuildInfoSend(int aIndex, char* guildName, char* guildMaster, BYTE* mark, int score, WORD number)
{
	SDHP_GUILDCREATED pMsg;

	pMsg.h.c = 0xC1;
	pMsg.h.headcode = 0x40;
	pMsg.h.size = sizeof(SDHP_GUILDCREATED);

	pMsg.score = score;
	pMsg.Number = number;

	memcpy(pMsg.Mark, mark, sizeof(pMsg.Mark));

	memset(pMsg.GuildName, 0, sizeof(pMsg.GuildName));

	memcpy(pMsg.GuildName, guildName, MAX_GUILDNAMESTRING);
	memcpy(pMsg.Master, guildMaster, MAX_IDSTRING);

	return WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (LPBYTE)&pMsg, sizeof(SDHP_GUILDCREATED));
}

void DGGuildInfoSendState(int aIndex, BYTE state, int count)
{
	SDHP_GUILDLISTSTATE pMsg;

	PHeadSetB((LPBYTE)&pMsg, 0x41, sizeof(pMsg));

	pMsg.State = state;
	pMsg.Count = count;

	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (LPBYTE)&pMsg, sizeof(pMsg));
}

void GDGuildListRequest()
{
	;
}

void DGGuildInfoAllSend(char* guildname, char* guildmaster, unsigned char* mark, int score, unsigned short number)
{
	SDHP_GUILDCREATED pMsg;

	pMsg.h.c = PMHC_BYTE;              // -63 signed ⊥ 0xC1
	pMsg.h.headcode = 0x40;
	pMsg.h.size = sizeof(pMsg);

	pMsg.score = score;
	pMsg.Number = number;

	memcpy(pMsg.Mark, mark, sizeof(pMsg.Mark));

	memset(pMsg.GuildName, 0, sizeof(pMsg.GuildName));
	memcpy(pMsg.GuildName, guildname, MAX_GUILDNAMESTRING);

	memset(pMsg.Master, 0, sizeof(pMsg.Master));
	memcpy(pMsg.Master, guildmaster, MAX_IDSTRING);

	for (int n = 0; n < MAX_SERVEROBJECT; n++)
	{
		if (!gsm.m_Obj[n].m_Used)
			continue;

		if (gsm.m_Obj[n].m_Type == MAX_GUILDNAMESTRING)
		{
			WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(pMsg));
		}
	}
}

void GDGuildDestroy(SDHP_GUILDDESTROY* lpMsg, int aIndex)
{
#pragma message("sizes changed here")
	SDHP_GUILDDESTROY_RESULT pMsg;

	char szGuildName[MAX_GUILDNAMESTRING + 1] = {};
	char szMaster[MAX_IDSTRING + 1] = {};

	memcpy(pMsg.Master, lpMsg->Master, MAX_IDSTRING);
	memcpy(pMsg.GuildName, lpMsg->GuildName, MAX_GUILDNAMESTRING);

	memcpy(szMaster, lpMsg->Master, MAX_IDSTRING);
	memcpy(szGuildName, lpMsg->GuildName, MAX_GUILDNAMESTRING);

	cLog.AddTD("GuildDelete %s %s", szGuildName, szMaster);

	pMsg.h.c = PMHC_BYTE;        // -63
	pMsg.h.headcode = 0x31;
	pMsg.h.size = sizeof(SDHP_GUILDDESTROY_RESULT);

	pMsg.NumberH = lpMsg->NumberH;
	pMsg.NumberL = lpMsg->NumberL;
	pMsg.Result = 0;

	// -------------------------
	// Validation: Master name
	// -------------------------
	if (strlen(szMaster) < 3)
	{
		pMsg.Result = 3;
		cLog.Add("error-L3 : ID Length %s %s %d", szMaster, __FILE__, __LINE__);
	}

	// -------------------------
	// Validation: Guild name
	// -------------------------
	if (strlen(szGuildName) < 2)
	{
		pMsg.Result = 3;
		cLog.Add("error-L3 : GuildName Length %s %s %d", szGuildName, __FILE__, __LINE__);
	}

	// -------------------------
	// Server restrictions
	// -------------------------
	if (CGuildManager.IsCreateGuild())
	{
		cLog.Add("error-L3 : Guild not delete. guildcreate_option");
		pMsg.Result = 3;
	}

	// -------------------------
	// Castle Siege protection
	// -------------------------
	if (pMsg.Result != 3 && CastleDbSet->CheckCastleSiegeGuildList(szGuildName))
	{
		cLog.Add("≠error-L3 : [Castle] Already exist CastleGuildList");
		pMsg.Result = 3;
	}

	// -------------------------
	// Relationship cleanup
	// -------------------------
	_GUILD_INFO_STRUCT* lpGuildInfo = 0;

	if (pMsg.Result != 3)
	{
		lpGuildInfo = CGuildManager.SearchGuild(szGuildName);

		if (lpGuildInfo)
		{
			if (lpGuildInfo->iGuildUnion &&
				UnionExManager.NotifyInitRelationShip(lpGuildInfo, 1) == -1)
			{
				pMsg.Result = 3;
			}

			if (lpGuildInfo->iGuildRival &&
				UnionExManager.NotifyInitRelationShip(lpGuildInfo, 2) == -1)
			{
				pMsg.Result = 3;
			}
		}
	}

	// -------------------------
	// DB deletion flow
	// -------------------------
	if (pMsg.Result != 3)
	{
		pMsg.Result = 2;

		if (GuildDbSet->CheckDBGuildMaster(szMaster))
		{
			if (GuildDbSet->DelAllDBGuildMember(szGuildName))
			{
				if (GuildDbSet->DelDBGuild(szGuildName))
				{
					pMsg.Result = 4;
					CGuildManager.DeleteGuild(szGuildName, szMaster);
				}
				else
				{
					cLog.AddTD("[DelDBGuild] Failed to delete Guild in DB : %s", szGuildName);
				}
			}
			else
			{
				cLog.AddTD("[DelAllDBGuildMember] Failed to delete All GuildMember in DB : %s", szGuildName);
			}
		}
	}

	// -------------------------
	// Broadcast result
	// -------------------------
	for (int n = 0; n < MAX_SERVEROBJECT; n++)
	{
		if (gsm.m_Obj[n].m_Used && gsm.m_Obj[n].m_Type == ST_GAMESERVER)
		{
			pMsg.Flag = (aIndex == n);
			WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(SDHP_GUILDDESTROY_RESULT));
			cLog.Add("%d [%s] guild delete send result %d", n, szGuildName, pMsg.Result);
		}
	}
}

void GDGuildMemberAdd(SDHP_GUILDMEMBERADD* lpMsg, int aIndex)
{
	SDHP_GUILDMEMBERADD_RESULT pMsg;

	char memberID[MAX_IDSTRING + 1] = {};
	char guildName[MAX_GUILDNAMESTRING + 1] = {};

	memcpy(memberID, lpMsg->MemberID, MAX_IDSTRING);
	memcpy(guildName, lpMsg->GuildName, MAX_GUILDNAMESTRING);

	memcpy(pMsg.MemberID, memberID, MAX_IDSTRING);
	memcpy(pMsg.GuildName, guildName, MAX_GUILDNAMESTRING);

	pMsg.h.c = 0xC1;     // -63
	pMsg.h.headcode = 0x32;
	pMsg.h.size = sizeof(pMsg);

	pMsg.NumberH = lpMsg->NumberH;
	pMsg.NumberL = lpMsg->NumberL;

	pMsg.Result = 0;

	// --------------------------------------------------
	// Basic validation
	// --------------------------------------------------
	if (strlen(memberID) < 3 || strlen(guildName) < 4)
	{
		pMsg.Result = 0;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(SDHP_GUILDMEMBERADD_RESULT));
		return;
	}

	// --------------------------------------------------
	// Already exists in DB
	// --------------------------------------------------
	if (GuildDbSet->CheckDBGuildMember(memberID) == 1)
	{
		pMsg.Result = 4;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(SDHP_GUILDMEMBERADD_RESULT));
		return;
	}

	cLog.AddTD("[CheckDBGuildMember] %s already exists in GuildMember.", memberID);

	// --------------------------------------------------
	// Insert into DB
	// --------------------------------------------------
	if (!GuildDbSet->AddDBGuildMember(memberID, guildName))
	{
		pMsg.Result = 4;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(SDHP_GUILDMEMBERADD_RESULT));
		cLog.AddTD("[AddDBGuildMember] Failed to join guild : %s %s", guildName, memberID);
		return;
	}

	// --------------------------------------------------
	// Add to runtime guild structure
	// --------------------------------------------------
	_GUILD_INFO_STRUCT* lpGuild = CGuildManager.AddMember(guildName, memberID, gsm.m_Obj[aIndex].pServer, -1);

	if (!lpGuild)
	{
		pMsg.Result = 0;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(SDHP_GUILDMEMBERADD_RESULT));
		cLog.Add("error-L3 : Guild add member failed. %s", memberID);
		return;
	}

	// --------------------------------------------------
	// Broadcast update
	// --------------------------------------------------
	pMsg.Result = 1;
	pMsg.pServer = gsm.m_Obj[aIndex].pServer;

	for (int n = 0; n < MAX_SERVEROBJECT; n++)
	{
		if (!gsm.m_Obj[n].m_Used || gsm.m_Obj[n].m_Type != ST_GAMESERVER)
			continue;

		pMsg.Flag = (aIndex == n);
		WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(SDHP_GUILDMEMBERADD_RESULT));
		cLog.Add("%d [%s] Guild member add send", n, memberID);
	}

	GuildDbSet->SetGuildMatchingMemberCount(guildName, lpGuild->Count);
}

void GDGuildMemberAddWithoutUserIndex(SDHP_GUILDMEMBERADD_WITHOUT_USERINDEX* lpMsg, int aIndex)
{
	CStringA qSql;

	SDHP_GUILDMEMBERADD_RESULT_WITHOUT_USERINDEX pMsg;
	pMsg.h.c = 0xC1;
	pMsg.h.headcode = 0x39;
	pMsg.h.size = sizeof(pMsg);
	pMsg.Result = 0;

	char memberID[MAX_IDSTRING + 1] = {};
	char guildName[MAX_GUILDNAMESTRING + 1] = {};

	memcpy(memberID, lpMsg->MemberID, MAX_IDSTRING);
	memcpy(guildName, lpMsg->GuildName, MAX_GUILDNAMESTRING);

	// basic validation (original uses strlen but is unsafe/decompiler-inferred)
	if (strlen(memberID) < 3 || strlen(guildName) < 4)
	{
		pMsg.Result = 0;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
		return;
	}

	// already exists in DB
	if (GuildDbSet->CheckDBGuildMember(memberID) == 1)
	{
		pMsg.Result = 4;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
		return;
	}

	cLog.AddTD("[CheckDBGuildMember] %s already exists in GuildMember.", memberID);

	// insert into DB
	if (!GuildDbSet->AddDBGuildMember(memberID, guildName))
	{
		pMsg.Result = 4;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
		cLog.AddTD("[AddDBGuildMember] Failed to participate in %s Guild : %s", guildName, memberID);
		return;
	}

	pMsg.Result = 1;

	// check if player is online
	_FRIEND_INFO_STRUCT* lpFindMember = CFriendManager.Search(memberID);
	bool connected = false;

	if (lpFindMember)
	{
		aIndex = lpFindMember->sIndex;
		pMsg.Number = lpFindMember->Number;
		connected = true;
	}

	// add to guild memory structure
	_GUILD_INFO_STRUCT* lpNode = CGuildManager.AddMember(guildName, memberID, gsm.m_Obj[aIndex].pServer, -1);

	if (!lpNode)
	{
		cLog.Add("error-L3 : Guild add member failed. %s %d", __FILE__, __LINE__);
		pMsg.Result = 0;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
		return;
	}

	// broadcast to all servers
	for (int n = 0; n < MAX_SERVEROBJECT; n++)
	{
		if (gsm.m_Obj[n].m_Used && gsm.m_Obj[n].m_Type == ST_GAMESERVER)
		{
			pMsg.Flag = (aIndex == n && connected);
			pMsg.pServer = gsm.m_Obj[aIndex].pServer;

			WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(pMsg));
			cLog.Add("%d [%s] Guild member add send", n, memberID);
		}
	}

	GuildDbSet->SetGuildMatchingMemberCount(guildName, lpNode->Count);
}

void GDGuildMemberDel(SDHP_GUILDMEMBERDEL* lpMsg, int aIndex)
{
	CStringA qSql;

	SDHP_GUILDMEMBERDEL_RESULT pMsg;
	pMsg.h.c = 0xC1;
	pMsg.h.headcode = 0x33;
	pMsg.h.size = sizeof(pMsg);
	pMsg.Result = 0;
	pMsg.Flag = 0;
	pMsg.NumberH = lpMsg->NumberH;
	pMsg.NumberL = lpMsg->NumberL;

	char memberID[MAX_IDSTRING + 1] = {};
	char guildName[MAX_GUILDNAMESTRING + 1] = {};

	memcpy(memberID, lpMsg->MemberID, MAX_IDSTRING);
	memcpy(guildName, lpMsg->GuildName, MAX_GUILDNAMESTRING);

	// decompiler noise ⊥ real intent is strlen checks
	if (strlen(memberID) < 3 || strlen(guildName) < 2)
	{
		pMsg.Result = 3;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
		return;
	}

	// DB delete first
	if (!GuildDbSet->DelDBGuildMember(memberID))
	{
		pMsg.Result = 3;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
		return;
	}

	// remove from in-memory guild structure
	if (CGuildManager.DelMember(guildName, memberID))
	{
		pMsg.Result = 1;
	}
	else
	{
		cLog.Add("error-L3 : guild member delete fail. %s %s", guildName, memberID);
		pMsg.Result = 3;
	}

	// broadcast to all servers
	for (int n = 0; n < MAX_SERVEROBJECT; n++)
	{
		if (gsm.m_Obj[n].m_Used && gsm.m_Obj[n].m_Type == 1)
		{
			pMsg.Flag = (aIndex == n);
			WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(pMsg));
			cLog.Add("%d [%s][%s] guild delete send", n, guildName, memberID);
		}
	}

	// update cached member count
	_GUILD_INFO_STRUCT* lpNode = CGuildManager.SearchGuild(guildName);
	if (lpNode)
	{
		GuildDbSet->SetGuildMatchingMemberCount(pMsg.GuildName, lpNode->Count);
	}
}

void GDGuildScoreUpdateRecv(SDHP_GUILDSCOREUPDATE* lpMsg)
{
	char szGuildName[MAX_GUILDNAMESTRING + 1] = {};
	memcpy(szGuildName, lpMsg->GuildName, MAX_GUILDNAMESTRING);

	// decompiler artifact: v1 is strlen(szGuildName)
	if (strlen(szGuildName) >= 2)
	{
		if (GuildDbSet->ScoreUpdate(szGuildName, lpMsg->Score))
		{
			DGGuildScoreUpdate(szGuildName, lpMsg->Score);
		}
	}
	else
	{
		cLog.Add("error-L2 : %s %d Guild Name Not found",
			__FILE__,
			__LINE__);
	}
}

void DGGuildScoreUpdate(char* guildname, int score)
{
	SDHP_GUILDSCOREUPDATE pMsg;

	pMsg.h.c = 0xC1;
	pMsg.h.headcode = 0x37;
	pMsg.h.size = sizeof(pMsg);
	pMsg.Score = score;

	memcpy(pMsg.GuildName, guildname, MAX_GUILDNAMESTRING);

	for (int n = 0; n < MAX_SERVEROBJECT; n++)
	{
		if (gsm.m_Obj[n].m_Used && gsm.m_Obj[n].m_Type == 1)
		{
			WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(pMsg));
			cLog.Add("Guild score update send : %s %d", guildname, score);
		}
	}
}

void GDGuildNoticeRecv(SDHP_GUILDNOTICE* lpMsg)
{
	SDHP_GUILDNOTICE pMsg;
	char szTemp[64] = {};

	// copy and sanitize incoming notice (for logging)
	memcpy(szTemp, lpMsg->szGuildNotice, sizeof(lpMsg->szGuildNotice));

	// update DB/cache
	GuildDbSet->NoticeUpdate(lpMsg->GuildName, lpMsg->szGuildNotice);

	// build broadcast packet
	pMsg.h.c = 0xC1;
	pMsg.h.headcode = 0x38;
	pMsg.h.size = sizeof(pMsg);

	memcpy(pMsg.GuildName, lpMsg->GuildName, MAX_GUILDNAMESTRING);
	memcpy(pMsg.szGuildNotice, lpMsg->szGuildNotice, sizeof(pMsg.szGuildNotice));

	cLog.Add("Guild notice update [%s] : %s", pMsg.GuildName, szTemp);

	// broadcast to all GS instances
	for (int n = 0; n < MAX_SERVEROBJECT; n++)
	{
		if (gsm.m_Obj[n].m_Used && gsm.m_Obj[n].m_Type == ST_GAMESERVER)
		{
			WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(pMsg));
		}
	}
}

void DGFriendList(_FRIEND_INFO_STRUCT* lpNode, int aIndex)
{
	unsigned char sendBuf[2008] = {};
	int offset = 20;
	unsigned char count = 0;

	FHP_FRIENDLIST pMsg;
	FHP_FRIENDLIST_COUNT pCount;
	cLog.AddTD("sizeof(FHP_FRIENDLIST) = %d", sizeof(FHP_FRIENDLIST));
	if (lpNode->Count <= 0)
		return;

	for (int n = 1; n < lpNode->Count; n++)
	{
		_FRIEND_INFO_STRUCT* lpFriendNode = CFriendManager.Search(lpNode->Names[n]);
		// default offline
		pMsg.Server = -1;

		if (lpFriendNode)
		{
			// basic validity checks (online / allowed / not blocked etc.)
			if (lpNode->Use[n] != 30000 &&
				CFriendManager.GetChatveto(lpFriendNode) &&
				CFriendManager.GetState(lpFriendNode) != 253 &&
				CFriendManager.IsSearchMember(lpFriendNode, lpNode->Name))
			{
				pMsg.Server = lpFriendNode->pServer[0];

				lpNode->Use[n] = lpFriendNode->Number;
				lpNode->pServer[n] = lpFriendNode->pServer[0];
				lpNode->ServerIndex[n] = lpFriendNode->sIndex;

				CFriendManager.SetConnectMember(lpFriendNode, lpNode->Name, lpNode->Number, lpNode->pServer[0], aIndex);
			}
		}
		else
		{
			lpNode->pServer[n] = -1;
		}

		// serialize friend entry
		memcpy(&pMsg, lpNode->Names[n], MAX_IDSTRING);
		memcpy(&sendBuf[offset], &pMsg, MAX_IDSTRING + 1);

		offset += MAX_IDSTRING + 1;
		count++;
	}

	// build packet header
	pCount.h.set((unsigned char*)&pCount.h, 0x60, offset);

	pCount.MailCount = lpNode->MailCount;
	pCount.Number = lpNode->Number;
	pCount.Count = count;

	memcpy(pCount.Name, lpNode->Name, sizeof(pCount.Name));

	// server/channel mapping
	unsigned char serverChannel = gsm.m_Obj[aIndex].pServer;

	switch (gsm.m_Obj[aIndex].btGameServerType)
	{
	case 1: serverChannel = 0xC7; break;
	case 2: serverChannel = 0xC8; break;
	case 3: serverChannel = 0xC9; break;
	}

	pCount.Server = serverChannel;

	// final packet
	memcpy(sendBuf, &pCount.h, sizeof(pCount));

	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, sendBuf, offset);
}

int DGWaitFriendList(int GUID, char* Name, __int16 Number, int aIndex)
{
	char_ID friendName;
	FHP_WAITFRIENDLIST_COUNT pCount;

	// initialize helper object
	memset(friendName.GetBuffer(), 0, MAX_IDSTRING);

	// build packet header
	pCount.h.set(&pCount.h.c, 0x61, sizeof(pCount));

	// copy request info
	memcpy(pCount.Name, Name, sizeof(pCount.Name));
	pCount.Number = Number;

	// check DB for pending friend entry
	if (FriendDbSet->GetWaitFriend(GUID, friendName.GetBuffer()) == 1)
	{
		// copy result into packet
		memcpy(pCount.FriendName, friendName.GetBuffer(), sizeof(pCount.FriendName));
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pCount.h.c, sizeof(pCount));
		return 1;
	}

	return 0;
}

void GDFrinedListReq(FHP_FRIENDLIST_REQ* lpMsg, int aIndex)
{
	char_ID name(lpMsg->Name);

	unsigned char server = gsm.m_Obj[aIndex].pServer;
	int number = lpMsg->Number;

	cLog.AddTD("Friend Login request : %s %d server:%d", name.GetBuffer(), number, server);

	// Register / update friend in runtime manager
	_FRIEND_INFO_STRUCT* lpFriend = CFriendManager.Add(number, name.GetBuffer(), server, aIndex);

	if (!lpFriend)
		return;

	// Load DB friend data into runtime struct
	FriendDbSet->GetFriendList(name.GetBuffer(), lpFriend);

	// Send friend list & pending requests
	DGFriendList(lpFriend, aIndex);
	DGWaitFriendList(lpFriend->GUID, lpFriend->Name, lpFriend->Number, aIndex);

	lpFriend->WaitFriendCommand = 0;
	lpFriend->sIndex = aIndex;

	// Broadcast friend state
	DGFriendStateAllSend(lpFriend);

	// Notify mail system
	FHP_FRIEND_MEMO_LIST_REQ mailMsg;
	mailMsg.h.set(&mailMsg.h.c, 0x71, sizeof(mailMsg));

	memcpy(mailMsg.Name, lpFriend->Name, sizeof(mailMsg.Name));
	mailMsg.Number = lpFriend->Number;

	MailQueue.AddToQueue(&mailMsg.h.c, mailMsg.h.size, 0x71, aIndex);

	// Guild matching notification
	DGAnsSendNotiGuildMatching(lpFriend->Name, lpFriend->Number, aIndex);

	// Party matching handling
	if (g_PartyMatching.ConnectMember(lpFriend->Name, server, aIndex, lpFriend->Number))
	{
		_PARTY_INFO_STRUCT* lpPartyInfo = g_PartyMatching.SearchPartyMemberName(lpFriend->Name);

		if (lpPartyInfo)
		{
			// Member is not leader
			if (strcmp(lpPartyInfo->stPartyMember[0].Name, lpFriend->Name))
			{
				if (g_PartyMatching.IsSameServerLeader(lpFriend->Name, lpPartyInfo->stPartyMember[0].Name))
				{
					DGFixPartyMember(lpPartyInfo->stPartyMember[0].nUserIndex, lpFriend->Number, aIndex);
				}
			}
			else
			{
				// Leader login
				DGFixPartyMember(lpFriend->Number, lpPartyInfo->stPartyMember[0].nUserIndex, aIndex);

				for (int i = 1; i < 5; i++)
				{
					if (lpPartyInfo->stPartyMember[i].bLogOn &&
						lpPartyInfo->stPartyMember[i].bUse &&
						lpPartyInfo->stPartyMember[i].nServerIndex == aIndex)
					{
						DGFixPartyMember(lpFriend->Number, lpPartyInfo->stPartyMember[i].nUserIndex, aIndex);
					}
				}
			}

			DGSendPartyMember(lpPartyInfo->stPartyMember[0].Name, 0);
		}
	}
}

void DGFriendStateSend(char* Name, char* FriendName, unsigned char State, short Number, int aIndex)
{
	FHP_FRIEND_STATE pMsg;

	pMsg.h.set(&pMsg.h.c, 0x62, sizeof(pMsg));
	pMsg.Number = Number;
	pMsg.State = State;
	memcpy(pMsg.Name, Name, sizeof(pMsg.Name));
	memcpy(pMsg.FriendName, FriendName, sizeof(pMsg.FriendName));

	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
}

void DGFriendOfflineSend(char* szName)
{
	_FRIEND_INFO_STRUCT* lpNode = CFriendManager.Search(szName);

	if (!lpNode)
		return;

	// Notify all valid friend entries that this player went offline
	for (int n = 1; n < 50; n++)
	{
		if (lpNode->Use[n] > 1 &&
			lpNode->Use[n] != 30000 &&
			lpNode->ServerIndex[n] >= 0)
		{
			DGFriendStateSend(lpNode->Names[n],
				lpNode->Name,
				0xFF,   // offline state marker
				lpNode->Use[n],
				lpNode->ServerIndex[n]);
		}
	}

	// Remove from friend runtime manager
	CFriendManager.Delete(szName, 0);
}

void DGFriendStateAllSend(_FRIEND_INFO_STRUCT* lpNode)
{
	for (int n = 1; n < 50; ++n)
	{
		if (lpNode->Use[n] >= 0 && lpNode->Use[n] != 30000)
		{
			_FRIEND_INFO_STRUCT* lpFriendNode = CFriendManager.Search(lpNode->Names[n]);
			if (lpFriendNode)
				DGFriendStateSend(
					lpFriendNode->Name,
					lpNode->Name,
					lpNode->pServer[0],
					lpFriendNode->Number,
					lpFriendNode->sIndex);
		}
	}
}


void DGFriendStateAllSend(_FRIEND_INFO_STRUCT* lpNode, __int16 number, int aIndex, unsigned __int8 state)
{
	for (int n = 1; n < 50; ++n)
	{
		if (lpNode->Use[n] >= 0 && lpNode->Use[n] != 30000)
		{
			_FRIEND_INFO_STRUCT* lpFriendNode = CFriendManager.Search(lpNode->Names[n]);
			if (lpFriendNode)
				DGFriendStateSend(lpFriendNode->Name, lpNode->Name, state, lpFriendNode->Number, lpFriendNode->sIndex);
		}
	}
}

void GDFriendStateChange(FHP_FRIEND_STATE_C* lpMsg, int aIndex)
{
	char_ID szName(lpMsg->Name);
	_FRIEND_INFO_STRUCT* lpNode;

	char* name = szName.GetBuffer();
	lpNode = CFriendManager.Search(name);

	if (lpNode)
	{
		// State handling
		if (lpMsg->State)
		{
			if (lpMsg->State == 2)
			{
				// Offline / forced state (0xFD)
				CFriendManager.SetState(lpNode, 0xFD);
				DGFriendStateAllSend(lpNode, lpMsg->Number, aIndex, 0xFD);
			}
			else
			{
				// Chat allowed / normal update
				BYTE state = lpNode->pServer[0];
				CFriendManager.SetChatVeto(lpNode, 1);
				DGFriendStateAllSend(lpNode, lpMsg->Number, aIndex, state);
			}
		}
		else
		{
			// Chat veto disabled / offline-like state
			CFriendManager.SetChatVeto(lpNode, 0);
			DGFriendStateAllSend(lpNode, lpMsg->Number, aIndex, 0xFE);
		}
	}
}

void GDFriendAddReq(FHP_FRIEND_ADD_REQ* lpMsg, int aIndex)
{
	char_ID szName(lpMsg->Name);
	char_ID szFriendName(lpMsg->FriendName);

	FHP_FRIEND_ADD_RESULT pMsg;
	pMsg.h.set(&pMsg.h.c, 0x63, sizeof(pMsg));

	memcpy(pMsg.Name, szName.GetBuffer(), sizeof(pMsg.Name));
	memcpy(pMsg.FriendName, szFriendName.GetBuffer(), sizeof(pMsg.FriendName));
	pMsg.Number = lpMsg->Number;

	cLog.Add("Friend Add req : %s %s", szName.GetBuffer(), szFriendName.GetBuffer());

	_FRIEND_INFO_STRUCT* lpNode = CFriendManager.Search(szName.GetBuffer());

	if (!lpNode)
	{
		pMsg.Result = 0;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
		return;
	}

	if (lpNode->Count > gMaxFriend)
	{
		pMsg.Result = 3;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
		return;
	}

	// already friend check
	if (CFriendManager.IsSearchMember(lpNode, szFriendName.GetBuffer()))
	{
		pMsg.Result = 4;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
		return;
	}

	// DB wait insert
	pMsg.Result = FriendDbSet->FriendWaitAdd(szFriendName.GetBuffer(), szName.GetBuffer());

	// retry / GUID creation fallback
	if (pMsg.Result == 5)
	{
		if (FriendDbSet->CreateGuid(szFriendName.GetBuffer()))
		{
			pMsg.Result = FriendDbSet->FriendWaitAdd(szFriendName.GetBuffer(), szName.GetBuffer());
		}
		else
		{
			pMsg.Result = 0;
		}
	}

	pMsg.Server = -1;

	if (pMsg.Result == 1)
	{
		_FRIEND_INFO_STRUCT* lpFriend = CFriendManager.Search(szFriendName.GetBuffer());

		if (lpFriend)
		{
			// already mutual friend?
			if (CFriendManager.IsSearchMember(szFriendName.GetBuffer(), szName.GetBuffer()))
			{
				pMsg.Server = lpFriend->pServer[0];
			}
			else
			{
				if (!lpFriend->WaitFriendCommand)
				{
					if (DGWaitFriendList(
						lpFriend->GUID,
						lpFriend->Name,
						lpFriend->Number,
						lpFriend->sIndex) == 1)
					{
						lpFriend->WaitFriendCommand++;
					}
				}
				pMsg.Server = -1;
			}

			pMsg.Result = 1;

			DGFriendStateSend(szFriendName.GetBuffer(), szName.GetBuffer(), gsm.m_Obj[aIndex].pServer, lpFriend->Number, lpFriend->sIndex);
			CFriendManager.AddMember(szName.GetBuffer(), szFriendName.GetBuffer(), lpFriend->pServer[0], lpFriend->Number, lpFriend->sIndex);
		}
		else
		{
			CFriendManager.AddMember(szName.GetBuffer(), szFriendName.GetBuffer(), 0xFF, 1, -1);
		}
	}

	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
}

int FriendAddFunc(char* Name, char* friendName, unsigned __int8 server, int index, unsigned __int8* FriendConectServer, __int16 serverindex)
{
	_FRIEND_INFO_STRUCT* lpNode = CFriendManager.Search(Name);

	if (!lpNode)
		return 0;

	// max friend check
	if (lpNode->Count >= gMaxFriend - 1)
	{
		cLog.AddTD("error-L3 : [%s] Friend MAX :%d", Name, lpNode->Count);
		FriendDbSet->WaitFriendDel(Name, friendName);
		return 3;
	}

	*FriendConectServer = (unsigned __int8)-1;

	int result = FriendDbSet->FriendAdd(Name, friendName);

	if (result != 1)
		return result; // 0 or 2

	_FRIEND_INFO_STRUCT* lpFriendNode = CFriendManager.Search(friendName);

	__int16 friend_GS_Index = 0;

	if (lpFriendNode)
		friend_GS_Index = lpFriendNode->Number;

	_FRIEND_INFO_STRUCT* lpNewNode = CFriendManager.AddMember(lpNode, friendName, server, friend_GS_Index, serverindex);

	if (!lpNewNode)
		return 0;

	// if friend is online, propagate state immediately
	if (lpFriendNode)
	{
		*FriendConectServer = lpFriendNode->pServer[0];

		DGFriendStateSend(friendName, Name, lpNewNode->pServer[0], lpFriendNode->Number, lpFriendNode->sIndex);
	}

	return 1;
}

void GDWaitFriendAddReq(FHP_WAITFRIEND_ADD_REQ* lpMsg, int aIndex)
{
	char_ID szFriendName(lpMsg->FriendName);
	char_ID szName(lpMsg->Name);

	FHP_WAITFRIEND_ADD_RESULT pMsg;
	pMsg.h.set(&pMsg.h.c, 0x64, sizeof(pMsg));

	pMsg.Number = lpMsg->Number;
	memcpy(pMsg.Name, lpMsg->Name, sizeof(pMsg.Name));
	memcpy(pMsg.FriendName, lpMsg->FriendName, sizeof(pMsg.FriendName));
	pMsg.Result = 0;

	_FRIEND_INFO_STRUCT* lpNode = CFriendManager.Search(szName.GetBuffer());

	if (lpNode)
		--lpNode->WaitFriendCommand;

	if (lpMsg->Result)
	{
		cLog.Add("wait friend add req : %s %s", szName.GetBuffer(), szFriendName.GetBuffer());

		unsigned __int8 pServer = gsm.m_Obj[aIndex].pServer;
		int Number = lpMsg->Number;

		pMsg.Result = FriendAddFunc(szName.GetBuffer(), szFriendName.GetBuffer(), pServer, Number, &pMsg.pServer, aIndex);

		// DB cleanup on duplicate / invalid state
		if (pMsg.Result == 2)
		{
			FriendDbSet->WaitFriendDel(szName.GetBuffer(), szFriendName.GetBuffer());
		}

		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));

		// re-trigger friend list sync if needed
		if (pMsg.Result != 2 &&
			lpNode &&
			lpNode->WaitFriendCommand < 0)
		{
			if (DGWaitFriendList(
				lpNode->GUID,
				lpNode->Name,
				lpNode->Number,
				aIndex) == 1)
			{
				++lpNode->WaitFriendCommand;
			}
		}
	}
	else
	{
		cLog.Add("wait friend add req ret : %s %s", szName.GetBuffer(), szFriendName.GetBuffer());
		FriendDbSet->WaitFriendDel(szName.GetBuffer(), szFriendName.GetBuffer());
		pMsg.Result = 0;
		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
	}
}

void GDFriendDelReq(FHP_FRIEND_ADD_REQ* lpMsg, int aIndex)
{
	char_ID Name(lpMsg->Name);
	char_ID FriendName(lpMsg->FriendName);

	FHP_FRIEND_DEL_RESULT pMsg;
	pMsg.h.set(&pMsg.h.c, 0x65, sizeof(pMsg));

	pMsg.Number = lpMsg->Number;
	memcpy(pMsg.Name, lpMsg->Name, sizeof(pMsg.Name));
	memcpy(pMsg.FriendName, lpMsg->FriendName, sizeof(pMsg.FriendName));

	cLog.Add("friend delete req result : %s %s", Name.GetBuffer(), FriendName.GetBuffer());

	// 1) remove from runtime friend list
	_FRIEND_INFO_STRUCT* lpNode = CFriendManager.DelMember(Name.GetBuffer(), FriendName.GetBuffer());

	// 2) remove from DB only if it existed in memory
	if (lpNode)
	{
		pMsg.Result = FriendDbSet->FriendDel(Name.GetBuffer(), FriendName.GetBuffer());
	}

	// send result to requester
	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));

	// 3) notify opposite side (if still online)
	lpNode = CFriendManager.Search(FriendName.GetBuffer());

	if (lpNode)
	{
		DGFriendStateSend(FriendName.GetBuffer(), Name.GetBuffer(), 0xFF, lpNode->Number, lpNode->sIndex);
	}
}

void GDFriendMemoSend(FHP_FRIEND_MEMO_SEND* lpMsg, int aIndex)
{
	int result = -1;
	char Subject[44];
	char Memo[1012];
	char szSQLSubject[76];
	char szDate[264];

	// Initialize name wrappers from packet
	char_ID szSendName(lpMsg->Name);
	char_ID szRecvName(lpMsg->ToName);

	// Clear and copy subject (0x20 = 32 bytes)
	memset(Subject, 0, MAX_MEMO_SUBJECT+1);  // 0x21
	memcpy(Subject, lpMsg->Subject, MAX_MEMO_SUBJECT);  // 0x20

	// Copy memo if size is valid (max 1000 bytes)
	memset(Memo, 0, MAX_MEMO +1);  // 0x3E9
	if (lpMsg->MemoSize <= MAX_MEMO)
	{
		memcpy(Memo, lpMsg->Memo, lpMsg->MemoSize);
		result = 1;
	}

	// Build response packet
	FHP_FRIEND_MEMO_SEND_RESULT pMsg;
	pMsg.h.set(0x70, sizeof(pMsg));  // Headcode 0x70, size 24
	pMsg.Number = lpMsg->Number;
	pMsg.WindowGuid = lpMsg->WindowGuid;
	memcpy(pMsg.Name, lpMsg->Name, sizeof(pMsg.Name));

	if (result == 1)
	{
		// Prepare SQL-safe subject
		memset(szSQLSubject, 0, 65);  // 0x41
		memcpy(szSQLSubject, Subject, MAX_MEMO_SUBJECT);  // 0x20

		BYTE Action = lpMsg->Action;
		BYTE Dir = lpMsg->Dir;

		// Write memo to database
		result = FriendDbSet->WriteMemo(szSendName.GetBuffer(), szRecvName.GetBuffer(), szSQLSubject, Memo, lpMsg->Photo, Dir, Action);
	}

	// Map result codes
	if (result >= 0)
	{
		pMsg.Result = 1;  // Success
	}
	else
	{
		switch (result)
		{
		case -2:
			pMsg.Result = 2;
			break;
		case -3:
			pMsg.Result = 3;
			break;
		case -6:
			pMsg.Result = 6;
			break;
		default:
			pMsg.Result = 0;  // Unknown error
			break;
		}
	}

	// Log the result
	cLog.Add("%s -> %s mail save Result: %d memoindex:%d", szSendName.GetBuffer(), szRecvName.GetBuffer(), pMsg.Result, result);

	// Send result back to sender
	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(FHP_FRIEND_MEMO_SEND_RESULT));

	// If success, notify the recipient if they're online
	if (pMsg.Result == 1)
	{
		_FRIEND_INFO_STRUCT* lpNode = CFriendManager.Search(szRecvName.GetBuffer());
		if (lpNode)
		{
			// Build timestamp
			time_t ltime;
			time(&ltime);
			tm* today = localtime(&ltime);
			today->tm_year += 1900;

			wsprintfA(szDate,
				"%04d-%02d-%02d %02d:%02d:%02d",
				today->tm_year,
				today->tm_mon + 1,
				today->tm_mday,
				today->tm_hour,
				today->tm_min,
				today->tm_sec);

			// Send memo list update to recipient
			DGFriendMemoList(lpNode->sIndex,
				lpNode->Number,
				result,
				szSendName.GetBuffer(),
				szRecvName.GetBuffer(),
				szDate,
				Subject,
				2);  // read = 2 (new mail notification)
		}
	}
}

void GDFriendMemoListReq(FHP_FRIEND_MEMO_LIST_REQ* lpMsg, int aIndex)
{
	char_ID name(lpMsg->Name);
	int number = lpMsg->Number;

	_FRIEND_INFO_STRUCT* lpNode = CFriendManager.Search(name.GetBuffer());

	if (!lpNode)
		return;

	FriendDbSet->ReadMemoSubject(aIndex, name.GetBuffer(), number, lpNode->GUID);
}

void DGFriendMemoList(int aIndex, unsigned __int16 Number, unsigned __int16 MemoNumber, char* SendName, char* RecvName, char* Date, char* Subject, unsigned __int8 read)
{
	FHP_FRIEND_MEMO_LIST pMsg;

	pMsg.h.set(0x71, sizeof(FHP_FRIEND_MEMO_LIST));

	pMsg.Number = Number;
	pMsg.MemoIndex = MemoNumber;
	memcpy(pMsg.SendName, SendName, sizeof(pMsg.SendName));
	memcpy(pMsg.RecvName, RecvName, sizeof(pMsg.RecvName));
	memcpy(pMsg.Date, Date, sizeof(pMsg.Date));
	memcpy(pMsg.Subject, Subject, sizeof(pMsg.Subject));
	pMsg.read = read;

	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(FHP_FRIEND_MEMO_LIST));
}

void GDFriendMemoRead(FHP_FRIEND_MEMO_RECV_REQ* lpMsg, int aIndex)
{
	char_ID name(lpMsg->Name);

	_FRIEND_INFO_STRUCT* lpNode = CFriendManager.Search(name.GetBuffer());

	if (!lpNode)
		return;

	FHP_FRIEND_MEMO_RECV pMsg;
	int sizeOut[3] = { 0 };

	if (!FriendDbSet->ReadMemo(aIndex, lpMsg->MemoIndex, lpNode->GUID, pMsg.Memo, pMsg.Photo, &pMsg.Dir, &pMsg.Action, sizeOut))
	{
		return;
	}

	memcpy(pMsg.Name, lpMsg->Name, sizeof(pMsg.Name));
	pMsg.MemoSize = sizeOut[0];

	if (pMsg.MemoSize > MAX_MEMO)
	{
		pMsg.MemoSize = MAX_MEMO;
		cLog.AddTD("error-L2 memo size clamp");
	}

	int packetSize = pMsg.MemoSize + 40;

	pMsg.h.set((unsigned __int8*)&pMsg.h, 0x72, packetSize);
	pMsg.Number = lpMsg->Number;
	pMsg.MemoIndex = lpMsg->MemoIndex;

	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (unsigned __int8*)&pMsg, packetSize);
}

void GDFriendMemoDel(FHP_FRIEND_MEMO_DEL_REQ* lpMsg, int aIndex)
{
	char_ID name(lpMsg->Name);

	_FRIEND_INFO_STRUCT* lpNode = CFriendManager.Search(name.GetBuffer());

	if (!lpNode)
		return;

	FHP_FRIEND_MEMO_DEL_RESULT pMsg;
	pMsg.h.set(&pMsg.h.c, 0x73u, sizeof(pMsg));

	pMsg.Number = lpMsg->Number;
	memcpy(pMsg.Name, lpMsg->Name, sizeof(pMsg.Name));

	pMsg.Result = FriendDbSet->DelMemo(name.GetBuffer(), lpMsg->MemoIndex);

	pMsg.MemoIndex = lpMsg->MemoIndex;
	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));
}

void GDFriendChatRoomCreateRecv(FHP_FRIEND_CHATROOM_CREATE_REQ* lpMsg, int Index)
{
	// Stack-allocated string wrappers (RAII - destructors auto-called)
	char_ID szName(lpMsg->Name);
	char_ID szFriendName(lpMsg->fName);

	// Working arrays
	int friendNumber[3] = { 0 };
	__int16 fserver[6] = { 0 };
	int room[3] = { -1 };

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// 1. Verify chat server connection
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	if (gsm.m_CSIndex < 0)
	{
		FHP_FRIEND_CHATROOM_CREATE_RESULT response;
		response.h.set(&response.h.c, 0x66, sizeof(response));
		response.Result = 2;
		response.Number = lpMsg->Number;
		memcpy(response.Name, lpMsg->Name, sizeof(response.Name));
		memcpy(response.FriendName, lpMsg->fName, sizeof(response.FriendName));

		WzIoEngine->send(gsm.m_Obj[Index].lpPHD, &response.h.c, sizeof(response));
		cLog.Add("error-L1 : ChatServer not found");
		return;
	}

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// 2. Locate requester in friend manager
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	_FRIEND_INFO_STRUCT* lpNode = CFriendManager.Search(szName.GetBuffer());
	if (!lpNode)
	{
		return; // Silent fail: player not registered in friend system
	}

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// 3. Search for friend's room membership
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	int searchResult = CFriendManager.SearchMemberRoom(lpNode, szFriendName.GetBuffer(), friendNumber, fserver, room);

	if (searchResult < 0)
	{
		cLog.Add("Friend not found. %s", szFriendName.GetBuffer());

		FHP_CHAT_ROOMCREATE_RESULT result = {};
		result.ServerNumber = Index;
		result.Result = 0;
		result.UserNumber = lpMsg->Number;
		memcpy(result.Name, lpMsg->Name, sizeof(result.Name));
		CDCreateChatRoomResult(&result);
		return;
	}

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// 4. Chat veto permission check
	//    Note: GetChatveto() returning FALSE means "do not allow chat"
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	if (!CFriendManager.GetChatveto(lpNode))
	{
		return; // Veto disabled for requester ⊥ silent reject
	}

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// 5. Check friend's veto status and create room if both allow
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	_FRIEND_INFO_STRUCT* lpFriendNode = CFriendManager.Search(szFriendName.GetBuffer());

	if (lpFriendNode && CFriendManager.GetChatveto(lpFriendNode))
	{
		// Both parties permit chat ⊥ delegate to DC layer
		DCCreateChatRoom(
			szName.GetBuffer(),      // Requester name
			lpMsg->Number,                  // Requester connection ID
			szFriendName.GetBuffer(), // Friend name
			lpFriendNode->Number,           // Friend connection ID
			Index,                          // Requester server index
			lpFriendNode->sIndex            // Friend server index
		);
		return;
	}

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// 6. Friend not found or veto mismatch ⊥ send failure
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	FHP_CHAT_ROOMCREATE_RESULT result = {};
	result.Result = 0;
	result.ServerNumber = Index;
	result.UserNumber = lpMsg->Number;
	memcpy(result.Name, lpMsg->Name, sizeof(result.Name));
	CDCreateChatRoomResult(&result);
}

void GDFriendInvitationRecv(FHP_FRIEND_INVITATION_REQ* lpMsg, int aIndex)
{
	char_ID szName(lpMsg->Name);
	char_ID szFriendName(lpMsg->FriendName);

	FHP_FRIEND_INVITATION_RET pMsg;
	pMsg.h.set(&pMsg.h.c, 0x74, sizeof(pMsg));

	pMsg.Number = lpMsg->Number;
	pMsg.WindowGuid = lpMsg->WindowGuid;

	memcpy(pMsg.Name, lpMsg->Name, sizeof(pMsg.Name));
	pMsg.Result = 0;

	_FRIEND_INFO_STRUCT* lpFriendNode = CFriendManager.Search(szFriendName.GetBuffer());

	if (lpFriendNode)
	{
		if (CFriendManager.GetChatveto(lpFriendNode))
		{
			DGRoomInvitationReq(szFriendName.GetBuffer(), lpMsg->RoomNumber, lpFriendNode->sIndex, lpFriendNode->Number, 1);

			cLog.Add("Msg-L3 : friend Invitation req : %s->%s ROOM:%d Server:%d User:%d",
				szName.GetBuffer(),
				szFriendName.GetBuffer(),
				lpMsg->RoomNumber,
				lpFriendNode->sIndex,
				lpFriendNode->Number);
			pMsg.Result = 1;
		}
		else
		{
			cLog.Add("Msg-L3 : friend Invitation rejection: %s ROOM:%d Server:%d User:%d",
				szFriendName.GetBuffer(),
				lpMsg->RoomNumber,
				lpFriendNode->sIndex,
				lpFriendNode->Number);
			pMsg.Result = 0;
		}
	}
	else
	{
		cLog.Add("Msg-L3: friend Invitation req fail: user not connected %s", szFriendName.GetBuffer());
		pMsg.Result = 0;
	}

	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, &pMsg.h.c, 20);
}

void DCCreateChatRoom(char* szName, int usernumber, char* szFriendName, int friendnumber, __int16 servernumber, __int16 friendservernumber)
{
	FHP_CHAT_ROOMCREATE pMsg;

	pMsg.h.set(&pMsg.h.c, 0xA0u, sizeof(pMsg));
	memcpy(pMsg.Name, szName, sizeof(pMsg.Name));
	memcpy(pMsg.FriendName, szFriendName, sizeof(pMsg.FriendName));
	pMsg.UserNumber = usernumber;
	pMsg.ServerNumber = servernumber;
	pMsg.FriendNumber = friendnumber;
	pMsg.FriendServerNumber = friendservernumber;

	cLog.Add("chat room create req : %s %d %d", szName, usernumber, friendnumber);

	cSend(&pMsg.h.c, sizeof(pMsg));
}

void DCChatRoomInvitation(FHP_CHAT_ROOMINVITATION* lpMsg)
{
	char_ID szName(lpMsg->Name);

	_FRIEND_INFO_STRUCT* lpNode = CFriendManager.Search(szName.GetBuffer());

	if (!lpNode)
		return;

	if (!CFriendManager.GetChatveto(lpNode))
	{
		cLog.AddTD("Msg-L3 : [%s] chat veto state ", lpNode->Name);
		return;
	}

	FHP_FRIEND_CHATROOM_CREATE_RESULT pMsg;
	pMsg.h.set(&pMsg.h.c, 0x66, sizeof(pMsg));

	pMsg.Result = 1;
	pMsg.Number = lpNode->Number;
	pMsg.Type = 2;

	memcpy(pMsg.Name, lpNode->Name, sizeof(pMsg.Name));
	memcpy(pMsg.FriendName, lpMsg->Name, sizeof(pMsg.FriendName));
	memcpy(pMsg.ServerIp, szChatServerIp, sizeof(pMsg.ServerIp));

	pMsg.RoomNumber = lpMsg->RoomNumber;
	pMsg.Ticket = lpMsg->Ticket;

	WzIoEngine->send(gsm.m_Obj[lpNode->sIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));

	cLog.Add("chat room #2 create req result (%s / %s)", lpNode->Name, szChatServerIp);
}

void CDCreateChatRoomResult(FHP_CHAT_ROOMCREATE_RESULT* lpMsg)
{
	char_ID szName(lpMsg->Name);
	char_ID szFriendName(lpMsg->FriendName);

	int serverIndex = lpMsg->ServerNumber;

	FHP_FRIEND_CHATROOM_CREATE_RESULT pMsg;
	pMsg.h.set(&pMsg.h.c, 0x66, sizeof(pMsg));
	pMsg.Result = lpMsg->Result;
	pMsg.Number = lpMsg->UserNumber;
	pMsg.Type = lpMsg->Type;

	memcpy(pMsg.Name, lpMsg->Name, sizeof(pMsg.Name));
	memcpy(pMsg.FriendName, lpMsg->FriendName, sizeof(pMsg.FriendName));

	if (lpMsg->Result == 1)
	{
		memcpy(pMsg.ServerIp, szChatServerIp, sizeof(pMsg.ServerIp));
		pMsg.RoomNumber = lpMsg->RoomNumber;
		pMsg.Ticket = lpMsg->Ticket;
	}

	WzIoEngine->send(gsm.m_Obj[serverIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));

	CFriendManager.SetMemberRoom(szName.GetBuffer(), szFriendName.GetBuffer(), lpMsg->RoomNumber);

	cLog.Add("chat room create req ret (%s / %s) %d", szName.GetBuffer(), szChatServerIp, lpMsg->Result);
}

void DGRoomInvitationReq(char* Name, signed __int16 room, __int16 server, __int16 usernumber, unsigned __int8 Type)
{
	FHP_CHAT_JOINUSER_REQ pMsg;

	pMsg.h.set(&pMsg.h.c, 0xA1, sizeof(pMsg));
	memcpy(pMsg.Name, Name, sizeof(pMsg.Name));
	pMsg.RoomNumber = room;
	pMsg.ServerNumber = server;
	pMsg.UserNumber = usernumber;
	pMsg.Type = Type;
	cSend(&pMsg.h.c, sizeof(pMsg));
	cLog.Add("chat server friend Invitation req (%d / %d) %d", room, server, usernumber);
}

void DGRoomInvitationRecv()
{
	;
}

void GDGuildReqAssignStatus(EXSDHP_GUILD_ASSIGN_STATUS_REQ* lpMsg, int iServerIndex)
{
	// Ensure safe C-string termination (defensive)
	lpMsg->szGuildName[MAX_GUILDNAMESTRING] = 0;
	lpMsg->szTargetName[MAX_IDSTRING] = 0;

	EXSDHP_GUILD_ASSIGN_STATUS_RESULT pMsg;//30

	const bool success = GuildDbSet->UpdateGuildMemberStatus(lpMsg->szGuildName, lpMsg->szTargetName, lpMsg->btGuildStatus) != 0;

	pMsg.h.c = 0xC1;        // -63
	pMsg.h.headcode = 0xE1; // -31
	pMsg.h.size = sizeof(pMsg);

	pMsg.btResult = success;
	pMsg.btType = lpMsg->btType;
	pMsg.wUserIndex = lpMsg->wUserIndex;
	pMsg.btGuildStatus = lpMsg->btGuildStatus;

	memcpy(pMsg.szGuildName, lpMsg->szGuildName, sizeof(pMsg.szGuildName));
	memcpy(pMsg.szTargetName, lpMsg->szTargetName, sizeof(pMsg.szTargetName));

	pMsg.btFlag = 1;

	// Send result back to requesting server
	WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));

	// Broadcast to other game servers if update succeeded
	if (success)
	{
		pMsg.btFlag = 0;

		for (int n = 0; n < MAX_SERVEROBJECT; ++n)
		{
			if (gsm.m_Obj[n].m_Used &&
				gsm.m_Obj[n].m_Type == ST_GAMESERVER &&
				n != iServerIndex)
			{
				WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(pMsg));
			}
		}
	}
}

void GDGuildReqAssignType(EXSDHP_GUILD_ASSIGN_TYPE_REQ* lpMsg, int iServerIndex)
{
	lpMsg->szGuildName[MAX_GUILDNAMESTRING] = 0;

	EXSDHP_GUILD_ASSIGN_TYPE_RESULT pMsg;//18

	const bool success = GuildDbSet->UpdateGuildType(lpMsg->szGuildName, lpMsg->btGuildType) != 0;
	//sizeof(EXSDHP_GUILD_ASSIGN_TYPE_REQ);//16
	pMsg.h.c = 0xC1;        // -63
	pMsg.h.headcode = 0xE2; // -30
	pMsg.h.size = sizeof(pMsg);

	pMsg.btResult = success;
	pMsg.wUserIndex = lpMsg->wUserIndex;
	pMsg.btGuildType = lpMsg->btGuildType;

	memcpy(pMsg.szGuildName, lpMsg->szGuildName, sizeof(pMsg.szGuildName));

	pMsg.btFlag = 1;

	WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));

	if (success)
	{
		pMsg.btFlag = 0;

		for (int n = 0; n < MAX_SERVEROBJECT; ++n)
		{
			if (gsm.m_Obj[n].m_Used && gsm.m_Obj[n].m_Type == 1 && n != iServerIndex)
			{
				WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(pMsg));
			}
		}
	}
}

void GDRelationShipReqJoin(EXSDHP_RELATIONSHIP_JOIN_REQ* lpMsg, int iServerIndex)
{
	sizeof(EXSDHP_RELATIONSHIP_JOIN_REQ);
	// Initialize response packet
	EXSDHP_RELATIONSHIP_JOIN_RESULT pMsg;//40

	// Lookup both guilds by their numeric IDs
	_GUILD_INFO_STRUCT* lpReqGuild = CGuildManager.SearchGuild_Number(lpMsg->iRequestGuildNum);
	_GUILD_INFO_STRUCT* lpTargetGuild = CGuildManager.SearchGuild_Number(lpMsg->iTargetGuildNum);

	unsigned char btResult = 0;
	bool bIsCastleSiegeGuild = false;

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// 1. Castle Siege Restriction Check
	//    Guilds participating in Castle Siege cannot form relationships
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	if (lpReqGuild && lpTargetGuild)
	{
		if (CastleDbSet->CheckCastleSiegeGuildList(lpReqGuild->Name) ||
			CastleDbSet->CheckCastleSiegeGuildList(lpTargetGuild->Name))
		{
			bIsCastleSiegeGuild = true;
			btResult = 16;  // Error code: Castle Siege guild restriction
		}
	}

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// 2. Process Relationship Type
	//    btRelationShipType: 1 = Union, 2 = Rival
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	if (lpMsg->btRelationShipType != 1 || bIsCastleSiegeGuild)
	{
		// Handle RIVAL relationship (type 2)
		if (lpMsg->btRelationShipType == 2 && !bIsCastleSiegeGuild)
		{
			// SetRelationRival returns 1 on success
			if (UnionExManager.SetRelationRival(lpMsg->iRequestGuildNum, lpMsg->iTargetGuildNum, 1) == 1)
			{
				btResult = 1;  // Success
			}
		}
		// Union requests with castle siege guilds or invalid types fall through to failure
	}
	else
	{
		// Handle UNION relationship (type 1)
		if (UnionExManager.SetRelationUnion(lpMsg->iRequestGuildNum, lpMsg->iTargetGuildNum, 1) == 1)
		{
			btResult = 1;  // Success
		}
	}

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// 3. Build Response Packet
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Note: Mu Online uses signed byte opcodes: -63 = 0xC1, -27 = 0xE5
	pMsg.h.c = 0xC1;                    // Packet type marker
	pMsg.h.headcode = 0xE5;             // Sub-opcode for relationship result
	pMsg.h.size = sizeof(EXSDHP_RELATIONSHIP_JOIN_RESULT);

	pMsg.wRequestUserIndex = lpMsg->wRequestUserIndex;
	pMsg.wTargetUserIndex = lpMsg->wTargetUserIndex;
	pMsg.btResult = btResult;
	pMsg.btRelationShipType = lpMsg->btRelationShipType;
	pMsg.iRequestGuildNum = lpMsg->iRequestGuildNum;
	pMsg.iTargetGuildNum = lpMsg->iTargetGuildNum;
	pMsg.szRequestGuildName[0] = '\0';
	pMsg.szTargetGuildName[0] = '\0';

	// Include guild names only on successful Rival creation
	if (btResult == 1 && lpMsg->btRelationShipType == 2)
	{
		// Re-fetch to ensure we have latest data (defensive coding)
		_GUILD_INFO_STRUCT* pReq = CGuildManager.SearchGuild_Number(lpMsg->iRequestGuildNum);
		_GUILD_INFO_STRUCT* pTarget = CGuildManager.SearchGuild_Number(lpMsg->iTargetGuildNum);

		if (pReq && pTarget)
		{
			strncpy(pMsg.szRequestGuildName, pReq->Name, MAX_GUILDNAMESTRING);
			pMsg.szRequestGuildName[MAX_GUILDNAMESTRING] = '\0';
			strncpy(pMsg.szTargetGuildName, pTarget->Name, MAX_GUILDNAMESTRING);
			pMsg.szTargetGuildName[MAX_GUILDNAMESTRING] = '\0';
		}
	}

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// 4. Send to Requesting Server
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	pMsg.btFlag = 1;  // Flag = 1: direct response to requester
	WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, &pMsg.h.c, sizeof(EXSDHP_RELATIONSHIP_JOIN_RESULT));

	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// 5. Broadcast to Other Servers (if relationship created)
	//    Skip broadcast for error code 16 (castle siege restriction)
	// 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	if (btResult != 0 && btResult != 16)
	{
		pMsg.btFlag = 0;  // Flag = 0: broadcast to other servers

		for (int n = 0; n < MAX_SERVEROBJECT; ++n)
		{
			// Send to all active game server connections except the requester
			if (gsm.m_Obj[n].m_Used &&
				gsm.m_Obj[n].m_Type == ST_GAMESERVER &&  // Type 1 = GameServer
				n != iServerIndex)
			{
				WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(EXSDHP_RELATIONSHIP_JOIN_RESULT));
			}
		}
	}
}

void GDRelationShipReqBreakOff(EXSDHP_RELATIONSHIP_BREAKOFF_REQ* lpMsg, int iServerIndex)
{
	EXSDHP_RELATIONSHIP_BREAKOFF_RESULT pMsg;//20

	unsigned char btResult = 0;
	bool bIsCastleSiegeGuild = false;

	_GUILD_INFO_STRUCT* lpReqGuild = CGuildManager.SearchGuild_Number(lpMsg->iRequestGuildNum);

	_GUILD_INFO_STRUCT* lpTargetGuild = CGuildManager.SearchGuild_Number(lpMsg->iTargetGuildNum);

	// Castle siege guilds cannot modify relationships
	if (lpReqGuild && lpTargetGuild && (CastleDbSet->CheckCastleSiegeGuildList(lpReqGuild->Name) || CastleDbSet->CheckCastleSiegeGuildList(lpTargetGuild->Name)))
	{
		bIsCastleSiegeGuild = true;
		btResult = 16;
	}

	// Break relationship
	if (!bIsCastleSiegeGuild)
	{
		if (lpMsg->btRelationShipType == 1)
		{
			// Union break
			if (UnionExManager.SetRelationUnion(lpMsg->iRequestGuildNum, lpMsg->iTargetGuildNum, 2) == 1)
			{
				btResult = 1;
			}
		}
		else if (lpMsg->btRelationShipType == 2)
		{
			// Rival break
			if (UnionExManager.SetRelationRival(lpMsg->iRequestGuildNum, lpMsg->iTargetGuildNum, 2) == 1)
			{
				btResult = 1;
			}
		}
	}

	pMsg.h.c = 0xC1;
	pMsg.h.headcode = 0xE6;
	pMsg.h.size = sizeof(pMsg);

	pMsg.wRequestUserIndex = lpMsg->wRequestUserIndex;
	pMsg.wTargetUserIndex = lpMsg->wTargetUserIndex;

	pMsg.btResult = btResult;
	pMsg.btRelationShipType = lpMsg->btRelationShipType;

	pMsg.iRequestGuildNum = lpMsg->iRequestGuildNum;
	pMsg.iTargetGuildNum = lpMsg->iTargetGuildNum;

	pMsg.btFlag = 1;

	// Send response to requesting GS
	WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, &pMsg.h.c, sizeof(pMsg));

	// Broadcast successful changes
	if (btResult && btResult != 16)
	{
		pMsg.btFlag = 0;

		for (int n = 0; n < MAX_SERVEROBJECT; ++n)
		{
			if (gsm.m_Obj[n].m_Used &&
				gsm.m_Obj[n].m_Type == ST_GAMESERVER &&
				n != iServerIndex)
			{
				WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(pMsg));
			}
		}
	}
}

void GDUnionListReq(EXSDHP_UNION_LIST_REQ* lpRecvMsg, int iServerIndex)
{
	BYTE Buffer[8196];

	EXSDHP_UNION_LIST_COUNT* lpMsg = reinterpret_cast<EXSDHP_UNION_LIST_COUNT*>(Buffer);
	EXSDHP_UNION_LIST* lpList = reinterpret_cast<EXSDHP_UNION_LIST*>(lpMsg + 1);
	DWORD dwMemberCount = 0;

	lpMsg->btCount = 0;
	memset(Buffer, 0, 0x2000);
	lpMsg->btResult = 1;
	lpMsg->wRequestUserIndex = lpRecvMsg->wRequestUserIndex;
	lpMsg->iTimeStamp = GetTickCount();
	lpMsg->btRivalMemberNum = 0;
	lpMsg->btUnionMemberNum = 0;

	_GUILD_INFO_STRUCT* lpGuild = CGuildManager.SearchGuild_Number(lpRecvMsg->iUnionMasterGuildNumber);

	if (lpGuild)
	{
		if (lpGuild->iGuildUnion)
		{
			g_Sync.Lock();

			TUnionExInfo* lpUnion = UnionExManager.SearchUnion(lpGuild->iGuildUnion);

			if (lpUnion == NULL)
			{
				lpMsg->btResult = 0;
			}

			if (lpMsg->btResult == 1)
			{
				lpMsg->btRivalMemberNum = static_cast<BYTE>(lpUnion->m_mpRivalMember.size());
				lpMsg->btUnionMemberNum = static_cast<BYTE>(lpUnion->m_mpUnionMember.size());

				std::map<int, _GUILD_INFO_STRUCT*>::iterator it = lpUnion->m_mpUnionMember.begin();
				std::map<int, _GUILD_INFO_STRUCT*>::iterator end = lpUnion->m_mpUnionMember.end();

				while (it != end)
				{
					_GUILD_INFO_STRUCT* lpMember = it->second;

					if (lpMember)
					{
						lpList[dwMemberCount].btMemberNum = lpMember->Count;

						memcpy(lpList[dwMemberCount].szGuildName, lpMember->Name, sizeof(lpList[dwMemberCount].szGuildName));
						memcpy(lpList[dwMemberCount].Mark, lpMember->Mark, sizeof(lpList[dwMemberCount].Mark));

						++dwMemberCount;
					}
					++it;
				}
			}

			g_Sync.Unlock();
		}
		else
		{
			lpMsg->btResult = 0;
		}
	}
	else
	{
		lpMsg->btResult = 0;
	}
	
	if (dwMemberCount <= 100)
	{
		WORD PacketSize = sizeof(EXSDHP_UNION_LIST_COUNT) + sizeof(EXSDHP_UNION_LIST) * dwMemberCount;
		PHeadSetW(reinterpret_cast<LPBYTE>(lpMsg), 0xE9, PacketSize);
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, reinterpret_cast<LPBYTE>(lpMsg), _byteswap_ushort(*reinterpret_cast<WORD*>(&lpMsg->h.sizeH)));
	}
}

void DGRelationShipJoin(int iRequestGuildNum, int iTargetGuildNum, BYTE btRelationShipType, BYTE btTranFlag)
{
	EXSDHP_RELATIONSHIP_JOIN_RESULT pMsg;

	// Clear packet body (matches original memset(&pMsg.h.size, 0, 0x27u))
	memset(&pMsg.h.size, 0, 0x27);

	// Packet header
	pMsg.h.c = 0xC1;
	pMsg.h.headcode = 0xE5;
	pMsg.h.size = sizeof(EXSDHP_RELATIONSHIP_JOIN_RESULT);

	// Result
	pMsg.btResult = 1;

	// No requesting users (broadcast packet)
	pMsg.wRequestUserIndex = -1;
	pMsg.wTargetUserIndex = -1;

	// Relationship info
	pMsg.btRelationShipType = btRelationShipType;
	pMsg.iRequestGuildNum = iRequestGuildNum;
	pMsg.iTargetGuildNum = iTargetGuildNum;

	// Original code only broadcasts when btTranFlag == 0
	if (!btTranFlag)
	{
		for (int n = 0; n < MAX_SERVEROBJECT; ++n)
		{
			if (gsm.m_Obj[n].m_Used &&
				gsm.m_Obj[n].m_Type == ST_GAMESERVER)
			{
				WzIoEngine->send(gsm.m_Obj[n].lpPHD, &pMsg.h.c, sizeof(EXSDHP_RELATIONSHIP_JOIN_RESULT));
			}
		}
	}
}

void DGRelationShipBreakOff(int iRequestGuildNum, int iTargetGuildNum, BYTE iRelationShipType, BYTE btTranFlag)
{
	EXSDHP_RELATIONSHIP_BREAKOFF_RESULT pMsg;

	memset(&pMsg, 0, sizeof(pMsg));

	// C1:F6
	pMsg.h.c = 0xC1;
	pMsg.h.headcode = 0xF6;
	pMsg.h.size = sizeof(EXSDHP_RELATIONSHIP_BREAKOFF_RESULT);

	pMsg.btFlag = 0;

	pMsg.wRequestUserIndex = -1;
	pMsg.wTargetUserIndex = -1;

	pMsg.btResult = 1;
	pMsg.btRelationShipType = iRelationShipType;

	pMsg.iRequestGuildNum = iRequestGuildNum;
	pMsg.iTargetGuildNum = iTargetGuildNum;

	// Broadcast to all GameServers
	if (btTranFlag == 0)
	{
		for (int n = 0; n < MAX_SERVEROBJECT; n++)
		{
			if (gsm.m_Obj[n].m_Used == FALSE)
				continue;

			if (gsm.m_Obj[n].m_Type != ST_GAMESERVER)
				continue;

			WzIoEngine->send(gsm.m_Obj[n].lpPHD, (BYTE*)&pMsg, sizeof(EXSDHP_RELATIONSHIP_BREAKOFF_RESULT));
		}
	}
}

void DGRelationShipNotificationSend(BYTE iNotificationFlag, int iGuildListCount, int* iGuildList, BYTE btTranFlag)
{
	EXSDHP_NOTIFICATION_RELATIONSHIP pMsg;

	memset(&pMsg, 0, sizeof(pMsg));

	// C2:F8
	pMsg.h.c = 0xC2;
	pMsg.h.headcode = 0xF8;
	pMsg.h.sizeL = 0x98;
	pMsg.h.sizeH = 1;

	pMsg.btFlag = btTranFlag;
	pMsg.btUpdateFlag = iNotificationFlag;
	pMsg.btGuildListCount = iGuildListCount;

	if (iGuildList != NULL && iGuildListCount > 0)
	{
		memcpy(pMsg.iGuildList, iGuildList, sizeof(int) * iGuildListCount);
	}

	// Broadcast
	if (btTranFlag == 0)
	{
		for (int n = 0; n < MAX_SERVEROBJECT; n++)
		{
			if (gsm.m_Obj[n].m_Used == FALSE)
				continue;

			if (gsm.m_Obj[n].m_Type != ST_GAMESERVER)
				continue;

			WzIoEngine->send(gsm.m_Obj[n].lpPHD, (BYTE*)&pMsg, sizeof(EXSDHP_NOTIFICATION_RELATIONSHIP));
		}
	}
}

void DGRelationShipListSend(_GUILD_INFO_STRUCT* lpGuildInfo, int iRelationShipType, BYTE btTranFlag, int iServerIndex)
{
	if (lpGuildInfo == NULL)
	{
		TRACE_OUT(3, "RelationShip List");
		return;
	}

	int iUnion = (lpGuildInfo->iGuildUnion != 0) ? lpGuildInfo->iGuildUnion : lpGuildInfo->Number;
	EXSDHP_UNION_RELATIONSHIP_LIST pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	// C2:F7
	pMsg.h.c = 0xC2;
	pMsg.h.headcode = 0xF7;
	pMsg.h.sizeH = HIBYTE(sizeof(EXSDHP_UNION_RELATIONSHIP_LIST));
	pMsg.h.sizeL = LOBYTE(sizeof(EXSDHP_UNION_RELATIONSHIP_LIST));
	pMsg.btRelationShipType = iRelationShipType;
	pMsg.btRelationShipMemberCount = 0;

	// =========================================
	// UNION LIST
	// =========================================
	if (iRelationShipType == 1)
	{
		TUnionExInfo* pUnionExInfo = UnionExManager.SearchUnion(iUnion);

		if (pUnionExInfo == NULL)
			return;

		g_Sync.Lock();

		pMsg.iUnionMasterGuildNumber = pUnionExInfo->m_iMasterGuild;

		if (lpGuildInfo->iGuildUnion != 0)
		{
			memcpy(pMsg.szUnionMasterGuildName, pUnionExInfo->m_szMasterGuild, MAX_GUILDNAMESTRING);
		}
		else
		{
			pMsg.szUnionMasterGuildName[0] = 0;
		}

		for (std::map<int, _GUILD_INFO_STRUCT*>::iterator it = pUnionExInfo->m_mpUnionMember.begin(); it != pUnionExInfo->m_mpUnionMember.end(); ++it)
		{
			_GUILD_INFO_STRUCT* lpUnionGuild = it->second;

			if (lpUnionGuild == NULL)
				continue;

			pMsg.iRelationShipMember[pMsg.btRelationShipMemberCount++] = lpUnionGuild->Number;
		}

		g_Sync.Unlock();
	}

	// =========================================
	// RIVAL LIST
	// =========================================
	else if (iRelationShipType == 2)
	{
		TUnionExInfo* pUnionExInfo = UnionExManager.SearchUnion(iUnion);

		if (pUnionExInfo == NULL)
			return;

		g_Sync.Lock();

		pMsg.iUnionMasterGuildNumber = pUnionExInfo->m_iMasterGuild;

		if (lpGuildInfo->iGuildUnion != 0)
		{
			memcpy(pMsg.szUnionMasterGuildName, pUnionExInfo->m_szMasterGuild, MAX_GUILDNAMESTRING);
		}
		else
		{
			pMsg.szUnionMasterGuildName[0] = 0;
		}

		for (std::map<int, _GUILD_INFO_STRUCT*>::iterator it = pUnionExInfo->m_mpRivalMember.begin(); it != pUnionExInfo->m_mpRivalMember.end(); ++it)
		{
			_GUILD_INFO_STRUCT* lpGuild = it->second;

			if (lpGuild == NULL)
				continue;

			pMsg.iRelationShipMember[pMsg.btRelationShipMemberCount++] =
				lpGuild->Number;
		}

		g_Sync.Unlock();
	}

	// =========================================
	// SEND
	// =========================================
	if (btTranFlag == 1)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg, sizeof(EXSDHP_UNION_RELATIONSHIP_LIST));
	}
	else if (btTranFlag == 0)
	{
		for (int n = 0; n < MAX_SERVEROBJECT; n++)
		{
			if (gsm.m_Obj[n].m_Used == FALSE)
				continue;

			if (gsm.m_Obj[n].m_Type != ST_GAMESERVER)
				continue;

			WzIoEngine->send(gsm.m_Obj[n].lpPHD, (BYTE*)&pMsg, sizeof(EXSDHP_UNION_RELATIONSHIP_LIST));
		}
	}
}

void GDRelationShipReqKickOutUnionMember(EXSDHP_KICKOUT_UNIONMEMBER_REQ* lpMsg, int iServerIndex)
{
	char szUnionMasterGuildName[23];
	char szUnionMemberGuildName[20];
	unsigned __int8 btResult = 0;

	// Extract guild names from request (max 8 chars each)
	memset(szUnionMasterGuildName, 0, MAX_GUILDNAMESTRING + 1);
	memset(szUnionMemberGuildName, 0, MAX_GUILDNAMESTRING + 1);
	memcpy(szUnionMasterGuildName, lpMsg->szUnionMasterGuildName, MAX_GUILDNAMESTRING);
	memcpy(szUnionMemberGuildName, lpMsg->szUnionMemberGuildName, MAX_GUILDNAMESTRING);

	// Look up both guilds
	_GUILD_INFO_STRUCT* lpUnionMasterGuild = CGuildManager.SearchGuild(szUnionMasterGuildName);
	_GUILD_INFO_STRUCT* lpUnionMemberGuild = CGuildManager.SearchGuild(szUnionMemberGuildName);

	if (!lpUnionMasterGuild || !lpUnionMemberGuild)
	{
		if (!lpUnionMasterGuild)
		{
			cLog.Add("Error!! lpUnionMasterGuild is NULL MasterGuild : %s, MemberGuild : %s",
				lpMsg->szUnionMasterGuildName,
				lpMsg->szUnionMemberGuildName);
		}
		if (!lpUnionMemberGuild)
		{
			cLog.Add("Error!! lpUnionMemberGuild is NULL MasterGuild : %s, MemberGuild : %s",
				lpMsg->szUnionMasterGuildName,
				lpMsg->szUnionMemberGuildName);
		}
		return;
	}

	// Check castle siege guild list
	int bIsCastleSiegeGuild = 0;
	if (CastleDbSet->CheckCastleSiegeGuildList(lpUnionMasterGuild->Name) ||
		CastleDbSet->CheckCastleSiegeGuildList(lpUnionMemberGuild->Name))
	{
		bIsCastleSiegeGuild = 1;
		btResult = 16;  // Castle siege guild - cannot kick
	}

	// Process kick for union members
	if (lpMsg->btRelationShipType == 1 &&
		!bIsCastleSiegeGuild &&
		UnionExManager.SetRelationUnion(
			lpUnionMemberGuild->Number,
			lpUnionMasterGuild->Number,
			2) != -1)  // Operation 2 = remove from union
	{
		btResult = 1;  // Success
	}

	// Build response packet
	EXSDHP_KICKOUT_UNIONMEMBER_RESULT pMsg;
	memset(&pMsg.btFlag, 0, sizeof(EXSDHP_KICKOUT_UNIONMEMBER_REQ));  // Zero from btFlag to end of struct

	pMsg.h.c = 0xC1;              // -63 = 0xC1 (standard packet)
	pMsg.h.headcode = 0xEB;       // -21 = 0xEB
	pMsg.h.subcode = 1;
	pMsg.h.size = sizeof(EXSDHP_KICKOUT_UNIONMEMBER_RESULT);  // 28

	pMsg.wRequestUserIndex = lpMsg->wRequestUserIndex;
	pMsg.btResult = btResult;
	pMsg.btRelationShipType = lpMsg->btRelationShipType;
	memcpy(pMsg.szUnionMasterGuildName, lpUnionMasterGuild->Name, MAX_GUILDNAMESTRING);
	memcpy(pMsg.szUnionMemberGuildName, lpUnionMemberGuild->Name, MAX_GUILDNAMESTRING);
	pMsg.btFlag = 1;  // Override with 1 (direct response flag)

	// Send response to requesting server
	WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(EXSDHP_KICKOUT_UNIONMEMBER_RESULT));

	// If successful (and not castle siege blocked), broadcast to other servers
	if (btResult && btResult != 16)
	{
		pMsg.btFlag = 0;  // Broadcast flag
		for (int n = 0; n < MAX_SERVEROBJECT; ++n)
		{
			if (gsm.m_Obj[n].m_Used &&
				gsm.m_Obj[n].m_Type == ST_GAMESERVER &&
				n != iServerIndex)
			{
				WzIoEngine->send(gsm.m_Obj[n].lpPHD, (BYTE*)&pMsg.h, sizeof(EXSDHP_KICKOUT_UNIONMEMBER_RESULT));
			}
		}
	}
}

void GDGuildServerGroupChatting(EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND* lpMsg, int iServerIndex)
{
	if (CGuildManager.SearchGuild_Number(lpMsg->iGuildNum))
	{
		for (int n = 0; n < MAX_SERVEROBJECT; ++n)
		{
			if (gsm.m_Obj[n].m_Used && gsm.m_Obj[n].m_Type == 1 && n != iServerIndex)
				WzIoEngine->send(gsm.m_Obj[n].lpPHD, &lpMsg->h.c, lpMsg->h.size);
		}
	}
}

void GDUnionServerGroupChatting(EXSDHP_SERVERGROUP_UNION_CHATTING_SEND* lpMsg, int iServerIndex)
{
	if (UnionExManager.SearchUnion(lpMsg->iUnionNum))
	{
		for (int n = 0; n < MAX_SERVEROBJECT; ++n)
		{
			if (gsm.m_Obj[n].m_Used && gsm.m_Obj[n].m_Type == 1 && n != iServerIndex)
				WzIoEngine->send(gsm.m_Obj[n].lpPHD, &lpMsg->h.c, lpMsg->h.size);
		}
	}
}

void GDGensServerGroupChatting(EXSDHP_SERVERGROUP_GENS_CHATTING_SEND* lpMsg, int iServerIndex)
{
	for (int n = 0; n < MAX_SERVEROBJECT; ++n)
	{
		if (gsm.m_Obj[n].m_Used && gsm.m_Obj[n].m_Type == 1 && n != iServerIndex)
			WzIoEngine->send(gsm.m_Obj[n].lpPHD, &lpMsg->h.c, lpMsg->h.size);
	}
}

void GDRewardGuildPeriodBuffInsert(PMSG_REQ_GUILD_PERIODBUFF_INSERT* lpMsg)
{
	GuildDbSet->InsertGuildMemberBuff(lpMsg->szGuildName, lpMsg->wBuffIndex, lpMsg->btEffectType1, lpMsg->btEffectType2, lpMsg->dwDuration, lpMsg->lExpireDate);
}

void GDRewardGuildPeriodBuffDelete(PMSG_REQ_GUILD_PERIODBUFF_DELETE* lpMsg)
{
	if (lpMsg->btGuildCnt <= 5)
		GuildDbSet->DeleteGuildMemberBuff(lpMsg->wBuffIndex, lpMsg->btGuildCnt);
	else
		cLog.Add("Error!! GuildCnt %d", lpMsg->btGuildCnt);
}

void GDReqRegGensMember(_tagPMSG_REQ_REG_GENS_MEMBER_EXDB* lpMsg, int aIndex)
{
	_tagPMSG_ANS_REG_GENS_MEMBER_EXDB pMsg;  // Constructor zero-initializes members

	pMsg.btResult = g_GensSystem.ReqRegGensMember(lpMsg);
	pMsg.NumberH = lpMsg->NumberH;
	pMsg.NumberL = lpMsg->NumberL;
	pMsg.btInfluence = lpMsg->btInfluence;

	pMsg.h.c = 0xC1;
	pMsg.h.size = sizeof(pMsg);  // 8
	pMsg.h.headcode = 0xF8;
	pMsg.h.subcode = 4;

	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(pMsg));
}

void GDReqSecedeGensMember(_tagPMSG_REQ_SECEDE_GENS_MEMBER_EXDB* lpMsg, int aIndex)
{
	_tagPMSG_ANS_SECEDE_GENS_MEMBER_EXDB pMsg;  // Constructor zero-initializes members

	int iRet = g_GensSystem.ReqSecedeGensMember(lpMsg);

	if (iRet != -100)  // -100 likely means "already processing" or "ignore"
	{
		pMsg.btResult = (BYTE)iRet;
		pMsg.NumberH = lpMsg->NumberH;
		pMsg.NumberL = lpMsg->NumberL;

		pMsg.h.c = 0xC1;              // -63 = 0xC1
		pMsg.h.size = sizeof(pMsg);  // 7
		pMsg.h.headcode = 0xF8;       // -8 = 0xF8
		pMsg.h.subcode = 6;           // Secede response

		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(pMsg));
	}
	// If iRet == -100, no response is sent (silent ignore)
}

void GDReqGensInfo(_tagPMSG_REQ_GENS_INFO_EXDB* lpMsg, int aIndex)
{
	_tagPMSG_ANS_GENS_INFO_EXDB pMsg;        // Constructor zero-initializes
	_tagGensMemberInfo ST_GensMemberInfo;    // Constructor zero-initializes

	if (g_GensSystem.ReqGensInfo(lpMsg, &ST_GensMemberInfo) == -1)
		pMsg.btResult = 1;  // Error

	pMsg.btInfluence = ST_GensMemberInfo.btInfluence;
	pMsg.iContributePoint = ST_GensMemberInfo.iContributePoint;
	pMsg.NumberH = lpMsg->NumberH;
	pMsg.NumberL = lpMsg->NumberL;

	pMsg.h.c = 0xC1;              // -63 = 0xC1
	pMsg.h.size = sizeof(pMsg);  // 28
	pMsg.h.headcode = 0xF8;       // -8 = 0xF8
	pMsg.h.subcode = 2;

	pMsg.iGensRanking = ST_GensMemberInfo.iGensRanking;
	pMsg.iGensClass = ST_GensMemberInfo.iGensClass;

	WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(pMsg));
}

void GDReqSaveContributePoint(_tagPMSG_REQ_SAVE_CONTRIBUTE_POINT_EXDB* lpMsg)
{
	g_GensSystem.ReqSaveContributePoint(lpMsg);
}

void GDReqSaveContributePoint(_tagPMSG_REQ_SAVE_ABUSING_KILLUSER_EXDB* lpMsg)
{
	g_GensSystem.ReqSaveAbusingKillUserName(lpMsg);
}

void GDReqAbusingInfo(_tagPMSG_REQ_ABUSING_INFO* lpMsg, int aIndex)
{
	_tagPMSG_ANS_ABUSING_INFO pMsg;  // Constructor zero-initializes

	if (g_GensSystem.ReqAbusingInfo(lpMsg, &pMsg) != -1)
	{
		pMsg.NumberH = lpMsg->NumberH;
		pMsg.NumberL = lpMsg->NumberL;

		pMsg.h.c = 0xC1;              // -63 = 0xC1
		pMsg.h.size = sizeof(_tagPMSG_ANS_ABUSING_INFO);  // -100 = 156 (0x9C)
		pMsg.h.headcode = 0xF8;       // -8 = 0xF8
		pMsg.h.subcode = 10;          // 0x0A

		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_tagPMSG_ANS_ABUSING_INFO));
	}

	// If ReqAbusingInfo returns -1, no response is sent
}

void GDReqGensSecedeDateModify(_tagPMSG_REQ_SECEDE_GENS_DATE_MODIFY* lpMsg)
{
	g_GensSystem.ReqGensSecedeDateModify(lpMsg);
}

void GDReqGensReward(_tagPMSG_REQ_GENS_REWARD_EXDB* lpMsg, int aIndex)
{
	_tagPMSG_ANS_GENS_REWARD_EXDB pMsg;  // Constructor zero-initializes

	if (g_GensSystem.ReqGensReward(lpMsg, &pMsg) != -1)
	{
		pMsg.NumberH = lpMsg->NumberH;
		pMsg.NumberL = lpMsg->NumberL;

		pMsg.h.c = 0xC1;              // -63 = 0xC1
		pMsg.h.size = sizeof(_tagPMSG_ANS_GENS_REWARD_EXDB);  // 16
		pMsg.h.headcode = 0xF8;       // -8 = 0xF8
		pMsg.h.subcode = 13;          // 0x0D

		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_tagPMSG_ANS_GENS_REWARD_EXDB));
	}
	// If ReqGensReward returns -1, no response is sent
}

void GDReqGensRewardComplete(_tagPMSG_REQ_GENS_REWARD_COMPLETE_EXDB* lpMsg)
{
	g_GensSystem.ReqGensRewardComplete(lpMsg);
}

void GDReqGensMemberCount(_tagPMSG_REQ_GENS_MEMBER_COUNT_EXDB* lpMsg, int aIndex)
{
	_tagPMSG_ANS_GENS_MEMBER_COUNT_EXDB pMsg;  // Constructor zero-initializes

	if (g_GensSystem.ReqGensMemberCount(lpMsg, &pMsg) != -1)
	{
		pMsg.NumberH = lpMsg->NumberH;
		pMsg.NumberL = lpMsg->NumberL;

		pMsg.h.c = 0xC1;              // -63 = 0xC1
		pMsg.h.size = sizeof(_tagPMSG_ANS_GENS_MEMBER_COUNT_EXDB);  // 16
		pMsg.h.headcode = 0xF8;       // -8 = 0xF8
		pMsg.h.subcode = 16;          // 0x10

		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_tagPMSG_ANS_GENS_MEMBER_COUNT_EXDB));
	}
	// If ReqGensMemberCount returns -1, no response is sent
}

void GDReqGensSetRewardDay(_tagPMSG_REQ_SET_GENS_REWARD_DAY_EXDB* lpMsg)
{
	g_GensSystem.ReqSetGensRewardDay(lpMsg);
}

void GDReqGensSetRanking(_tagPMSG_REQ_SET_GENS_RANKING_EXDB* lpMsg)
{
	g_GensSystem.ReqSetGensRanking(lpMsg);
}

void GDReqGensRewardDayCheck(_tagPMSG_REQ_GENS_REWARD_DAY_CHECK_EXDB* lpMsg, int aIndex)
{
	_tagPMSG_ANS_GENS_REWARD_DAY_CHECK_EXDB pMsg;  // Constructor zero-initializes

	if (g_GensSystem.ReqGensRewardDayCheck(lpMsg, &pMsg) != -1)
	{
		pMsg.NumberH = lpMsg->NumberH;
		pMsg.NumberL = lpMsg->NumberL;

		pMsg.h.c = 0xC1;              // -63 = 0xC1
		pMsg.h.size = sizeof(_tagPMSG_ANS_GENS_REWARD_DAY_CHECK_EXDB);  // 12
		pMsg.h.headcode = 0xF8;       // -8 = 0xF8
		pMsg.h.subcode = 0x14;          // 0x14

		WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_tagPMSG_ANS_GENS_REWARD_DAY_CHECK_EXDB));
	}
	// If ReqGensRewardDayCheck returns -1, no response is sent
}

void GDReqGuildMatchingList(_stReqGuildMatchingList* lpMsg, int iServerIndex)
{
	_stGuildMatchingList stGuildMatchingList[9];
	memset(&stGuildMatchingList, 0, sizeof(stGuildMatchingList));

	int nPage = lpMsg->nPage;
	int nTotalCount = 0;
	int nCurPageListCnt = 0;

	// Get current page data (only if page > 0)
	if (nPage > 0)
		nCurPageListCnt = GuildDbSet->GetDBGuildMatchingList(nPage, stGuildMatchingList);

	// Get total count for pagination
	nTotalCount = GuildDbSet->GetDBGuildMatchingListCount();

	// Build response packet
	_stAnsGuildMatchingList pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	PHeadSubSetW((LPBYTE)&pMsg.h.c, 0xA3, 0, sizeof(_stAnsGuildMatchingList));  // 784

	pMsg.nUserIndex = lpMsg->nUserIndex;
	pMsg.nPage = lpMsg->nPage;

	// Calculate total pages (9 entries per page)
	if (nTotalCount > 0)
	{
		pMsg.nTotalPage = nTotalCount / 9;
		if (nTotalCount % 9 > 0)
			++pMsg.nTotalPage;
		if (nTotalCount < 9)
			pMsg.nTotalPage = 1;
	}
	else
	{
		pMsg.nTotalPage = 0;
	}

	// Set result
	if (nCurPageListCnt <= 0)
		pMsg.nResult = -1;  // No entries found
	else
		pMsg.nResult = 0;   // Success

	pMsg.nListCount = nCurPageListCnt;

	// Copy guild matching list into packet
	memcpy(pMsg.stGuildMatchingList, stGuildMatchingList, sizeof(pMsg.stGuildMatchingList));

	// Send response
	if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h.c, sizeof(_stAnsGuildMatchingList));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}
}

void GDReqGuildMatchingListSearchWord(_stReqGuildMatchingListSearchWord* lpMsg, int iServerIndex)
{
	_stGuildMatchingList stGuildMatchingList[9];
	memset(&stGuildMatchingList, 0, sizeof(stGuildMatchingList));

	int nPage = lpMsg->nPage;
	int nTotalCount = 0;
	int nCurPageListCnt = 0;

	// Prepare search string with SQL escaping
	CString strString;
	strString = lpMsg->szSearchWord;
	strString.Replace("'", "''");  // Escape single quotes for SQL

	int nStrLen = strString.GetLength();

	// Only query if page > 0 and search word is not empty
	if (nPage > 0 && nStrLen > 0)
	{
		char* Buffer = strString.GetBuffer(MAX_IDSTRING + 1);  // MAX_IDSTRING + 1
		nCurPageListCnt = GuildDbSet->GetDBGuildMatchingListSearchWord(nPage, Buffer, stGuildMatchingList);
		nTotalCount = GuildDbSet->GetDBGuildMatchingListCount();
	}

	// Build response packet
	_stAnsGuildMatchingList pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	PHeadSubSetW((LPBYTE)&pMsg.h.c, 0xA3, 0, sizeof(_stAnsGuildMatchingList));  // 784

	pMsg.nUserIndex = lpMsg->nUserIndex;
	pMsg.nPage = lpMsg->nPage;

	// Calculate total pages (9 entries per page)
	if (nTotalCount > 0)
	{
		pMsg.nTotalPage = nTotalCount / 9;
		if (nTotalCount % 9 > 0)
			++pMsg.nTotalPage;
		if (nTotalCount < 9)
			pMsg.nTotalPage = 1;
	}
	else
	{
		pMsg.nTotalPage = 0;
	}

	// Set result
	if (nCurPageListCnt <= 0)
		pMsg.nResult = -1;  // No entries found
	else
		pMsg.nResult = 0;   // Success

	pMsg.nListCount = nCurPageListCnt;

	// Copy guild matching list into packet
	memcpy(pMsg.stGuildMatchingList, stGuildMatchingList, sizeof(pMsg.stGuildMatchingList));

	// Send response
	if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h.c, sizeof(_stAnsGuildMatchingList));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}
}

void GDInsertGuildMatchingList(_stReqGuildMatchingData* lpMsg, int iServerIndex)
{
	_stGuildMatchingList stGuildMatchingList;
	memset(&stGuildMatchingList, 0, sizeof(stGuildMatchingList));

	int nResult = 0;

	// Copy numeric fields directly
	stGuildMatchingList.btClassType = lpMsg->stGuildMatchingList.btClassType;
	stGuildMatchingList.btGuildMasterClass = lpMsg->stGuildMatchingList.btGuildMasterClass;
	stGuildMatchingList.btGuildMemberCnt = lpMsg->stGuildMatchingList.btGuildMemberCnt;
	stGuildMatchingList.btInterestType = lpMsg->stGuildMatchingList.btInterestType;
	stGuildMatchingList.btLevelRange = lpMsg->stGuildMatchingList.btLevelRange;
	stGuildMatchingList.nBoardNumber = lpMsg->stGuildMatchingList.nBoardNumber;
	stGuildMatchingList.nGuildMasterLevel = lpMsg->stGuildMatchingList.nGuildMasterLevel;
	stGuildMatchingList.nGuildNumber = lpMsg->stGuildMatchingList.nGuildNumber;

	// Escape and copy memo field (SQL escaping)
	CString strString;
	strString = lpMsg->stGuildMatchingList.szMemo;
	strString.Replace("'", "''");  // Escape single quotes

	char* Buffer = strString.GetBuffer(41);  // Max 40 chars + null
	memcpy(&stGuildMatchingList, Buffer, 0x28);  // 0x28 = 40 bytes (szMemo)

	// Copy string fields with fixed sizes
	memcpy(stGuildMatchingList.szGuildName, lpMsg->stGuildMatchingList.szGuildName, 8);
	memcpy(stGuildMatchingList.szRegistrant, lpMsg->stGuildMatchingList.szRegistrant, 0x0A);  // 10 bytes

	stGuildMatchingList.btGensType = lpMsg->stGuildMatchingList.btGensType;

	// Insert into database
	if (!GuildDbSet->RegGuildMatchingData(stGuildMatchingList))
		nResult = -1;  // Failed

	// Build response packet
	_stAnsGuildMatchingData pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	pMsg.h.set(0xA3, 2, sizeof(pMsg));  // headcode=0xA3, subcode=2, size=12

	pMsg.nResult = nResult;
	pMsg.nUserIndex = lpMsg->nUserIndex;

	// Send response
	if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_stAnsGuildMatchingData));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}
}

void GDDeleteGuildMatching(_stReqDelGuildMatchingList* lpMsg, int iServerIndex)
{
	int nResult = 0;
	int nGuildNum = lpMsg->nGuildNumber;
	_stAnsDelGuildMatchingList pMsg;

	// Delete guild matching from database
	nResult = GuildDbSet->DeleteGuildMatching(nGuildNum);

	// Get wait list for this guild (state = 0 = pending)
	_stGuildMatchingAllowList pstWaitList[80];
	memset(pstWaitList, 0, sizeof(pstWaitList));
	int nListCount = GuildDbSet->GetWaitGuildMatching(nGuildNum, 0, pstWaitList);

	// Notify all waiting members that the guild matching was cancelled
	for (int i = 0; i < nListCount && i < 80; ++i)
	{
		_FRIEND_INFO_STRUCT* lpFindMember = CFriendManager.Search(pstWaitList[i].szName);
		if (lpFindMember)
		{
			int aIndex = lpFindMember->sIndex;
			if (gsm.m_Obj[aIndex].m_Used)
			{
				if (gsm.m_Obj[aIndex].m_Type == ST_GAMESERVER)  // Game server
				{
					_stAnsNotiGuildMatching pNotiMsg;
					memset(&pNotiMsg, 0, sizeof(pNotiMsg));

					pNotiMsg.h.set(0xA3, 9, sizeof(pMsg));  // subcode 9, size 12
					pNotiMsg.nResult = 3;  // Cancelled
					pNotiMsg.nUserIndex = lpFindMember->Number;

					if (aIndex >= 0 && aIndex <= 99)
					{
						WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pNotiMsg.h, sizeof(_stAnsNotiGuildMatching));
					}
					else
					{
						cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
					}
				}
			}
		}
	}

	// Send response to the requesting server
	memset(&pMsg, 0, sizeof(pMsg));
	pMsg.h.set(0xA3, 3, sizeof(pMsg));  // subcode 3, size 12
	pMsg.nResult = nResult;
	pMsg.nUserIndex = lpMsg->nUserIndex;

	if (iServerIndex >= 0 && iServerIndex <= 99)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(pMsg));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}
}

void GDReqJoinGuildMatching(_stRegWaitGuildMatching* lpMsg, int iServerIndex)
{
	int nResult = 0;
	int nGuildNumber = -1;
	char szName[20];
	_stAnsWaitGuildMatching pMsg;

	// Extract applicant name (10 bytes)
	memset(szName, 0, 11);
	memcpy(szName, lpMsg->stAllowList.szApplicantName, 0x0A);  // 10 bytes

	// Check if already waiting for another guild
	if (GuildDbSet->GetGuildMatchingWaitState(szName, 0) >= 0)
	{
		// Already in a wait list
		nResult = -1;
	}
	// Try to insert into wait list
	else if (!GuildDbSet->InsertWaitGuildMatching(lpMsg->stAllowList))
	{
		// Insert failed
		nResult = -4;
	}

	// Build response packet

	memset(&pMsg, 0, sizeof(pMsg));

	pMsg.h.set(0xA3, 4, sizeof(pMsg));  // subcode 4, size 12

	pMsg.nResult = nResult;
	pMsg.nUserIndex = lpMsg->nUserIndex;

	// Send response to requesting server
	if (iServerIndex >= 0 && iServerIndex <= 99)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_stAnsWaitGuildMatching));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}

	// Save guild number for notification
	nGuildNumber = lpMsg->stAllowList.nGuildNumber;

	// If successful, notify guild master
	if (nResult == 0)
		DGAnsSendNotiGuildMatchingForGuildMaster(nGuildNumber);
}

void GDReqCancelJoinGuildMatching(_stReqDelWaitGuildMatchingList* lpMsg, int iServerIndex)
{
	int nResult = 0;

	// Delete applicant from wait list
	if (!GuildDbSet->DeleteWaitGuildMatching(lpMsg->szName))
		nResult = -1;  // Delete failed

	// Build response packet
	_stAnsDelWaitGuildMatchingList pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	pMsg.h.set(0xA3, 5, sizeof(_stAnsDelWaitGuildMatchingList));  // subcode 5

	pMsg.nResult = nResult;
	pMsg.nUserIndex = lpMsg->nUserIndex;
	pMsg.nType = lpMsg->nType;

	// Send response
	if (iServerIndex >= 0 && iServerIndex <= 99)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_stAnsDelWaitGuildMatchingList));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}
}

void GDReqWaitGuildMatchingList(_stReqWaitGuildMatchingList* lpMsg, int iServerIndex)
{
	_stGuildMatchingAllowList stAllowList[80];
	memset(stAllowList, 0, sizeof(stAllowList));

	int nListCount = 0;

	// Get wait list for this guild (state = 0 = pending)
	nListCount = GuildDbSet->GetWaitGuildMatching(lpMsg->nGuildNumber, 0, stAllowList);

	// Build response packet
	_stAnsWaitGuildMatchingList pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	PHeadSubSetW((LPBYTE)&pMsg.h.c, 0xA3, 7, sizeof(_stAnsWaitGuildMatchingList));  // 1300

	pMsg.nUserIndex = lpMsg->nUserIndex;
	pMsg.nListCount = nListCount;
	pMsg.nResult = 0;

	if (nListCount == 0)
		pMsg.nResult = -3;  // No wait list entries

	// Copy wait list entries into packet
	memcpy(pMsg.stAllowList, stAllowList, sizeof(pMsg.stAllowList));

	// Send response
	if (iServerIndex >= 0 && iServerIndex <= 99)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h.c, sizeof(_stAnsWaitGuildMatchingList));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}
}

void GDReqGetWaitStateListGuildMatching(_stReqWaitStateListGuildMatching* lpMsg, int iServerIndex)
{
	int nResult = 0;

	// Initialize buffers
	char szName[20];
	char szGuildName[20];
	char szGuildMasterName[20];

	memset(szName, 0, MAX_IDSTRING + 1);
	memset(szGuildMasterName, 0, MAX_IDSTRING + 1);
	memset(szGuildName, 0, MAX_GUILDNAMESTRING + 1);

	// Copy applicant name (10 bytes)
	memcpy(szName, lpMsg->szName, MAX_IDSTRING);

	// Look up which guild this applicant is waiting for
	if (!GuildDbSet->GetWaitStateListGuildMatching(szName, szGuildName, szGuildMasterName))
		nResult = -2;  // Not found in any wait list

	// Build response packet
	_stAnsWaitStateListGuildMatching pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	pMsg.h.set(0xA3, 8, sizeof(pMsg));  // subcode 8, size 32

	pMsg.nResult = nResult;
	pMsg.nUserIndex = lpMsg->nUserIndex;

	// Copy guild name (8 bytes) and guild master name (10 bytes)
	memcpy(pMsg.szGuildName, szGuildName, MAX_GUILDNAMESTRING);
	memcpy(pMsg.szGuildMasterName, szGuildMasterName, MAX_IDSTRING);

	// Send response
	if (iServerIndex >= 0 && iServerIndex <= 99)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_stAnsWaitStateListGuildMatching));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}
}

void GDReqAllowJoinGuildMatching(_stReqAllowJoinGuildMatching* lpMsg, int iServerIndex)
{
	char szName[16];
	char szGuildName[20];
	char szGuildMasterName[20];
	int nState = 0;
	int nResult = 0;
	int nNotiResult = 1;
	int bCheckGuild = 0;

	// Initialize buffers
	memset(szName, 0, MAX_IDSTRING + 1);
	memset(szGuildName, 0, MAX_GUILDNAMESTRING + 1);

	// Copy applicant name and guild name
	memcpy(szName, lpMsg->szName, MAX_IDSTRING);      // 10 bytes
	memcpy(szGuildName, lpMsg->szGuildName, MAX_GUILDNAMESTRING); // 8 bytes

	// Check if already in a guild
	if (GuildDbSet->CheckDBGuildMember(szName))
	{
		bCheckGuild = 1;
		nResult = -4;  // Already in a guild
	}

	if (!bCheckGuild)
	{
		// Check if still in wait list with state 0 (pending)
		if (GuildDbSet->GetGuildMatchingWaitState(szName, 0))
		{
			nResult = -3;  // Not in wait list anymore
		}
		else
		{
			// Process accept/reject
			if (lpMsg->nAllowType == 1)  // Accepted
			{
				nState = 1;

				_tagGensMemberInfo stGensMemberInfo;
				_tagGensMemberInfo stGensMemberInfoGuildMaster;

				// Look up guild
				_GUILD_INFO_STRUCT* SearchGuild = CGuildManager.SearchGuild(szGuildName);
				if (SearchGuild)
				{
					// Get guild master name
					memset(szGuildMasterName, 0, MAX_IDSTRING + 1);
					memcpy(szGuildMasterName, SearchGuild->Names[0], MAX_IDSTRING);

					// Get Gens info for both applicant and guild master
					g_GensSystem_DBSet.DBGetGensMemberInfo(szName, &stGensMemberInfo);
					g_GensSystem_DBSet.DBGetGensMemberInfo(szGuildMasterName, &stGensMemberInfoGuildMaster);

					// Check Gens influence compatibility
					unsigned __int8 btGuildMasterInfluence = stGensMemberInfoGuildMaster.btInfluence;
					unsigned __int8 btMemberInfluence = stGensMemberInfo.btInfluence;

					if (stGensMemberInfoGuildMaster.btInfluence == 255)
						btGuildMasterInfluence = 0;
					if (btMemberInfluence == 255)
						btMemberInfluence = 0;

					// Different Gens factions cannot join
					if (btGuildMasterInfluence != btMemberInfluence)
					{
						nResult = -5;       // Gens conflict
						nState = 2;          // Rejected
						nNotiResult = 2;
					}
				}
				else
				{
					// Guild not found
					nResult = -6;
					nNotiResult = 2;
					nState = 2;
				}
			}
			else  // Rejected
			{
				nState = 2;
				nNotiResult = 2;
			}

			// Update wait state in database
			if (GuildDbSet->SetWaitGuildMatching(szName, nState))
				nResult = -2;  // Failed to update state
		}
	}

	// Build response packet to guild master
	_stAnsAllowJoinGuildMatching pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	pMsg.h.set(0xA3, 6, sizeof(pMsg));  // subcode 6, size 36

	pMsg.nResult = nResult;
	pMsg.nAllowType = lpMsg->nAllowType;
	pMsg.nUserIndex = lpMsg->nUserIndex;
	memcpy(pMsg.szMemberName, szName, MAX_IDSTRING);
	memcpy(pMsg.szGuildName, szGuildName, MAX_GUILDNAMESTRING);

	// Send response to guild master's server
	if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_stAnsAllowJoinGuildMatching));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}

	// Notify the applicant of the result
	_stAnsNotiGuildMatching pMsg2;
	memset(&pMsg2, 0, sizeof(pMsg2));

	pMsg2.h.set(0xA3, 9, sizeof(pMsg2));  // subcode 9, size 12

	_FRIEND_INFO_STRUCT* lpFindMember = CFriendManager.Search(szName);
	int bMemberConnected = 0;
	int aIndex = -1;

	if (lpFindMember)
	{
		aIndex = lpFindMember->sIndex;
		pMsg2.nUserIndex = lpFindMember->Number;
		pMsg2.nResult = nNotiResult;
		bMemberConnected = 1;
	}

	// Send notification to applicant if online (and not already rejected by state)
	for (int n = 0; n < MAX_SERVEROBJECT; ++n)
	{
		if (gsm.m_Obj[n].m_Used &&
			gsm.m_Obj[n].m_Type == 1 &&
			aIndex == n &&
			bMemberConnected &&
			nResult != -3)  // Don't notify if not in wait list
		{
			WzIoEngine->send(gsm.m_Obj[n].lpPHD, (BYTE*)&pMsg2.h, sizeof(_stAnsNotiGuildMatching));

			cLog.Add("%d [%s] Guild member add send to GuildMember - RESULT:[%d]",
				n, szName, lpMsg->nAllowType);
		}
	}
}

void DGAnsSendNotiGuildMatching(char* szName, int nUserIndex, int nServerIndex)
{
	if (szName == NULL)
	{
		return;
	}

	_stAnsNotiGuildMatching pMsg;

	char szFindName[MAX_IDSTRING + 1];
	int nState;

	memset(szFindName, 0, sizeof(szFindName));
	memcpy(szFindName, szName, MAX_IDSTRING);

	nState = GuildDbSet->GetGuildMatchingAcceptNRejectInfo(szFindName);

	if (nState == -1)
	{
		return;
	}

	pMsg.h.set((LPBYTE)&pMsg, 0xA3, 0x09, sizeof(_stAnsNotiGuildMatching));

	pMsg.nUserIndex = nUserIndex;
	pMsg.nResult = nState;

	if (nServerIndex >= 0 && nServerIndex <= MAX_SERVEROBJECT + 1)
	{
		WzIoEngine->send(gsm.m_Obj[nServerIndex].lpPHD, (BYTE*)&pMsg, sizeof(_stAnsNotiGuildMatching));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
	}
}

void DGAnsUseGuildMatchingGuild(int nGuildNumber)
{
	_GUILD_INFO_STRUCT* lpGuildInfo = CGuildManager.SearchGuild_Number(nGuildNumber);
	if (!lpGuildInfo)
		return;

	// Get guild master name (first entry in Names array)
	char szGuildMasterName[20];
	memset(szGuildMasterName, 0, 11);
	memcpy(szGuildMasterName, lpGuildInfo->Names[0], 0x0A);  // 10 bytes

	// Build the packet
	_stAnsUseGuildMatchingGuild pMsg;
	pMsg.h.set(0xA3, 0x11, sizeof(pMsg));

	_FRIEND_INFO_STRUCT* lpFindMember = CFriendManager.Search(szGuildMasterName);
	int bMemberConnected = 0;
	int aIndex = -1;

	if (lpFindMember)
	{
		aIndex = lpFindMember->sIndex;
		pMsg.nUserIndex = lpFindMember->Number;
		bMemberConnected = 1;
	}

	// Send to the guild master's server if connected
	for (int n = 0; n < MAX_SERVEROBJECT; ++n)
	{
		if (gsm.m_Obj[n].m_Used && gsm.m_Obj[n].m_Type == 1 && aIndex == n)
		{
			if (bMemberConnected)
				WzIoEngine->send(gsm.m_Obj[n].lpPHD, (BYTE*)&pMsg.h, sizeof(pMsg));
		}
	}
}

void DGAnsSendNotiGuildMatchingForGuildMaster(int nGuildNumber)
{
	_GUILD_INFO_STRUCT* lpGuildInfo = CGuildManager.SearchGuild_Number(nGuildNumber);
	if (!lpGuildInfo)
		return;

	// Get guild master name (first entry in Names array)
	char szGuildMasterName[20];
	memset(szGuildMasterName, 0, 11);
	memcpy(szGuildMasterName, lpGuildInfo->Names[0], 0x0A);  // 10 bytes

	// Build the packet
	_stAnsNotiGuildMatchingForGuildMaster pMsg;
	pMsg.h.set(0xA3, 0x10, sizeof(pMsg));  // sizeH=0x10, sizeL=0x0C => total size 12

	_FRIEND_INFO_STRUCT* lpFindMember = CFriendManager.Search(szGuildMasterName);
	int bMemberConnected = 0;
	int aIndex = -1;

	if (lpFindMember)
	{
		aIndex = lpFindMember->sIndex;
		pMsg.nUserIndex = lpFindMember->Number;
		pMsg.nResult = 0;
		bMemberConnected = 1;
	}

	// Send to the guild master's server if connected
	for (int n = 0; n < MAX_SERVEROBJECT; ++n)
	{
		if (gsm.m_Obj[n].m_Used && gsm.m_Obj[n].m_Type == 1 && aIndex == n)
		{
			if (bMemberConnected)
				WzIoEngine->send(gsm.m_Obj[n].lpPHD, (BYTE*)&pMsg.h, sizeof(pMsg));
		}
	}
}

void GDReqRegPartyMatchingList(_stReqRegWantedPartyMember* lpMsg, int iServerIndex)
{
	int nResult = 0;
	int bAbleJoin = 1;
	int nServerChannel;
	char szLeaderName[20];
	char szPassWord[16];
	char szTempName[20];
	CString strTitle;

	// Initialize name buffers
	memset(szLeaderName, 0, 11);
	memset(szPassWord, 0, sizeof(szPassWord));
	memset(szTempName, 0, 11);

	// Copy and escape title
	strTitle = lpMsg->szTitle;
	strTitle.Replace("'", "''");  // SQL escaping

	// Copy leader name (10 bytes)
	memcpy(szLeaderName, lpMsg->szPartyLeaderName, 0x0A);

	// Copy password (4 bytes)
	memcpy(szPassWord, lpMsg->szPassWord, 4);

	// Determine server channel
	nServerChannel = gsm.m_Obj[iServerIndex].pServer;
	switch (gsm.m_Obj[iServerIndex].btGameServerType)
	{
	case 1:
		nServerChannel = 199;
		break;
	case 2:
		nServerChannel = 200;
		break;
	case 3:
		nServerChannel = 201;
		break;
	}

	// Check if leader is already waiting in another party's wait list
	if (!g_PartyMatching_DBSet.GetPartyMatchingWaitList(szLeaderName, szTempName))
		bAbleJoin = 0;

	// Check if leader is already in an existing party
	_PARTY_INFO_STRUCT* lpPartyInfo = g_PartyMatching.SearchPartyMemberName(szLeaderName);
	if (lpPartyInfo)
	{
		if (strcmp(lpPartyInfo->stPartyMember[0].Name, szLeaderName) == 0)
			bAbleJoin = 0;
	}

	if (bAbleJoin)
	{
		// Insert into database
		char* Buffer = strTitle.GetBuffer(40);
		int nDBRet = g_PartyMatching_DBSet.InsertPartyMatchingList(
			szLeaderName,
			Buffer,
			szPassWord,
			lpMsg->nMinLevel,
			lpMsg->nMaxLevel,
			lpMsg->nHuntingGround,
			lpMsg->btWantedClass,
			lpMsg->btCurrentPartyCnt,
			lpMsg->btUsePassWord,
			lpMsg->btApprovalType,
			lpMsg->btWantedClassDetailInfo,
			nServerChannel + 1,
			lpMsg->btGensType,
			lpMsg->nLeaderLevel,
			lpMsg->btLeaderClass);

		if (nDBRet > 1)
		{
			// Insert failed (returned error code > 1)
			nResult = -1;
		}
		else
		{
			// Create party in memory
			g_PartyMatching.CreateParty(
				nServerChannel,
				iServerIndex,
				lpMsg->nUserIndex,
				szLeaderName,
				1,                      // Party member count
				lpMsg->nLeaderLevel,
				lpMsg->btLeaderClass);

			// Fix party member info
			DGFixPartyMember(lpMsg->nUserIndex, lpMsg->nUserIndex, iServerIndex);

			// Send party member list to leader
			DGSendPartyMember(szLeaderName, 0);

			nResult = 0;
		}
	}
	else
	{
		// Cannot register (already in wait list or existing party)
		nResult = -2;
	}

	// Build response packet
	_stAnsRegWantedPartyMember pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	pMsg.h.set(0xA4, 0, 0x0C);  // headcode=0xA4, subcode=0, size=12

	pMsg.nResult = nResult;
	pMsg.nUserIndex = lpMsg->nUserIndex;

	// Send response
	if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_stAnsRegWantedPartyMember));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}
}

void GDReqGetPartyMatchingList(_stReqGetPartyMatchingList* lpMsg, int iServerIndex)
{
	int nPage = lpMsg->nPage;
	int nListCount = 0;
	int nTotalCount = 0;
	char szSearchWord[20];
	_PARTY_INFO_LIST stPartyInfoList[6];

	// Initialize
	memset(stPartyInfoList, 0, sizeof(stPartyInfoList));
	memset(szSearchWord, 0, 11);
	memcpy(szSearchWord, lpMsg->szSearchWord, 0x0A);  // 10 bytes

	// Get party list based on type
	if (lpMsg->nType == 0)
	{
		// Type 0: Get all party matching list
		nListCount = g_PartyMatching_DBSet.GetPartyMatchingList(nPage, stPartyInfoList);
	}
	else if (lpMsg->nType == 1)
	{
		// Type 1: Search by keyword
		nListCount = g_PartyMatching_DBSet.GetPartyMatchingListSearchWord(
			nPage,
			szSearchWord,
			stPartyInfoList);
	}
	else
	{
		// Type 2+: Get joinable parties (filtered by class/level/gens)
		nListCount = g_PartyMatching_DBSet.GetPartyMatchingListJoinAble(
			nPage,
			lpMsg->btClass,
			lpMsg->nLevel,
			lpMsg->btGens,
			stPartyInfoList);
	}

	// Get total count for pagination
	if (lpMsg->nType > 1)
	{
		// Joinable list: filtered total count
		nTotalCount = g_PartyMatching_DBSet.GetPartyMatchingListJoinAbleTotalCount(
			lpMsg->btClass,
			lpMsg->nLevel,
			lpMsg->btGens);
	}
	else
	{
		// All list or search: unfiltered total count
		nTotalCount = g_PartyMatching_DBSet.GetPartyMatchingListCount();
	}

	// Build response packet
	_stAnsGetPartyMatchingList pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	PHeadSubSetW((LPBYTE)&pMsg.h.c, 0xA4, 1, sizeof(_stAnsGetPartyMatchingList));  // 532

	pMsg.nUserIndex = lpMsg->nUserIndex;
	pMsg.nListCount = nListCount;
	pMsg.nPage = lpMsg->nPage;
	pMsg.nResult = 0;

	// Calculate total pages (6 entries per page)
	if (nTotalCount > 0)
	{
		pMsg.nTotalPage = nTotalCount / 6;
		if (nTotalCount % 6 > 0)
			++pMsg.nTotalPage;
		if (nTotalCount < 6)
			pMsg.nTotalPage = 1;
	}
	else
	{
		pMsg.nTotalPage = 0;
	}

	// Copy party info list into packet
	memcpy(pMsg.stPartyInfoList, stPartyInfoList, sizeof(pMsg.stPartyInfoList));

	// Send response
	if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT + 1)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h.c, sizeof(_stAnsGetPartyMatchingList));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}
}

void GDReqMemeberJoinPartyMatching(_stReqJoinMemberPartyMatching* lpMsg, int iServerIndex)
{
	BYTE btRandom = lpMsg->btRandomParty;

	char szPassWord[16] = { 0 };
	BYTE szRecvPassWord[16] = { 0 };

	char szLeaderName[20] = { 0 };
	char szMemberName[20] = { 0 };
	char szTempName[20] = { 0 };

	BYTE btUsePassword[9] = { 0 };
	BYTE btAcceptType[12] = { 0 };

	int nLeaderServerIndex = -1;
	int nLeaderUserIndex = -1;

	int nResult = 0;
	int nRet = 0;
	int nListCount = 0;

	int bAbleJoin = 0;

	_PARTY_INFO_STRUCT* lpPartyInfo = NULL;
	_PARTY_INFO_STRUCT* lpTargetParty = NULL;

	_PARTY_MEMBER_WAIT_LIST stList[10];
	_PARTY_MEMBER_WAIT_LIST stRandomList[10];

	_stAnsJoinMemberPartyMatching pMsg;
	_stAnsRequestJoinPartyMatchingNoti pNotiMsg;

	memset(stList, 0, sizeof(stList));
	memset(stRandomList, 0, sizeof(stRandomList));

	memcpy(szLeaderName, lpMsg->szLeaderName, MAX_IDSTRING);
	memcpy(szMemberName, lpMsg->szMemberName, MAX_IDSTRING);

	AddFriendListForPartyMatching(lpMsg->nUserIndex, szMemberName, iServerIndex);

	// --------------------------------------------------------------------
	// Check member wait state
	// --------------------------------------------------------------------

	bAbleJoin = (g_PartyMatching_DBSet.GetPartyMatchingWaitList(szMemberName, szTempName) != 0);
	lpPartyInfo = g_PartyMatching.SearchPartyMemberName(szMemberName);
	lpTargetParty = g_PartyMatching.SearchParty(szLeaderName);

	if (lpPartyInfo)
	{
		bAbleJoin = 0;
	}

	// --------------------------------------------------------------------
	// Join logic
	// --------------------------------------------------------------------

	if (bAbleJoin)
	{
		// ----------------------------------------------------------------
		// Random party matching
		// ----------------------------------------------------------------

		if (btRandom)
		{
			if (g_PartyMatching_DBSet.GetRandomMatching(lpMsg->nLevel, lpMsg->btClass, lpMsg->btGensType, szLeaderName))
			{
				nResult = -3;
			}
			else
			{
				lpTargetParty = g_PartyMatching.SearchParty(szLeaderName);
				g_PartyMatching_DBSet.GetPartyMatchingPasswordInfo(szLeaderName, szPassWord, btUsePassword, btAcceptType);
				nListCount = g_PartyMatching_DBSet.GetPartyMatchingWaitListForLeader(szLeaderName, stRandomList);

				if (nListCount >= 10)
				{
					nResult = -7;
				}
				else
				{
					// Auto accept
					if (btAcceptType[0] == 1)
					{
						GDAutoMemberJoin(szLeaderName, szMemberName, lpMsg->nUserIndex, iServerIndex, lpMsg->nLevel, lpMsg->btClass);
						return;
					}
					g_PartyMatching_DBSet.InsertWaitPartyMatching(szLeaderName, szMemberName, lpMsg->btChangeUpClass, lpMsg->nLevel, lpMsg->nUserDBNumber);
					nResult = 0;
				}
			}
		}

		// ----------------------------------------------------------------
		// Normal matching
		// ----------------------------------------------------------------

		else
		{
			nRet = g_PartyMatching_DBSet.GetPartyMatchingPasswordInfo(szLeaderName, szPassWord, btUsePassword, btAcceptType);

			// Can't join own party
			if (strcmp(szLeaderName, szMemberName) == 0)
			{
				nResult = -5;
			}
			else
			{
				if (nRet || lpTargetParty == NULL)
				{
					nResult = -2;
				}
				else
				{
					nListCount = g_PartyMatching_DBSet.GetPartyMatchingWaitListForLeader(szLeaderName, stList);

					if (nListCount >= 10)
					{
						nResult = -7;
					}
					else
					{
						// Password not required
						if (btUsePassword[0] != 1)
						{
							if (btAcceptType[0] == 1)
							{
								GDAutoMemberJoin(szLeaderName, szMemberName, lpMsg->nUserIndex, iServerIndex, lpMsg->nLevel, lpMsg->btClass);
								return;
							}

							g_PartyMatching_DBSet.InsertWaitPartyMatching(szLeaderName, szMemberName, lpMsg->btChangeUpClass, lpMsg->nLevel, lpMsg->nUserDBNumber);
							nResult = 0;
						}
						else
						{
							memcpy(szRecvPassWord, lpMsg->szPassWord, 4);

							// Wrong password
							if (strcmp((char*)szRecvPassWord, szPassWord) != 0)
							{
								nResult = -1;
							}
							else
							{
								// Auto accept
								if (btAcceptType[0] == 1)
								{
									GDAutoMemberJoin(szLeaderName, szMemberName, lpMsg->nUserIndex, iServerIndex, lpMsg->nLevel, lpMsg->btClass);

									return;
								}

								g_PartyMatching_DBSet.InsertWaitPartyMatching(szLeaderName, szMemberName, lpMsg->btChangeUpClass, lpMsg->nLevel, lpMsg->nUserDBNumber);
								nResult = 0;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		nResult = -4;
	}

	// --------------------------------------------------------------------
	// Notify leader
	// --------------------------------------------------------------------

	if (nResult == 0 && lpTargetParty && lpTargetParty->stPartyMember[0].bLogOn && lpTargetParty->stPartyMember[0].bUse)
	{
		pNotiMsg.h.set((LPBYTE)&pNotiMsg, 0xA4, 0x08, sizeof(pNotiMsg));
		nLeaderServerIndex = lpTargetParty->stPartyMember[0].nServerIndex;
		nLeaderUserIndex = lpTargetParty->stPartyMember[0].nUserIndex;
		pNotiMsg.nUserIndex = nLeaderUserIndex;

		if (nLeaderServerIndex >= 0 && nLeaderServerIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[nLeaderServerIndex].lpPHD, (LPBYTE)&pNotiMsg, sizeof(pNotiMsg));
		}
		else
		{
			cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
		}
	}

	// --------------------------------------------------------------------
	// Send result
	// --------------------------------------------------------------------

	pMsg.nResult = nResult;
	pMsg.nUserIndex = lpMsg->nUserIndex;

	pMsg.h.set((LPBYTE)&pMsg, 0xA4, 0x02, sizeof(pMsg));

	if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (LPBYTE)&pMsg, sizeof(pMsg));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
	}
}

void GDReqMemberJoinStateList(_stReqJoinMemberStateListPartyMatching* lpMsg, int iServerIndex)
{
	_stAnsJoinMemberStateListPartyMatching pMsg;

	_FRIEND_INFO_STRUCT* lpFindMember = NULL;

	int nUserIndex = lpMsg->nUserIndex;

	char szMemberName[20] = { 0 };
	char szLeaderName[20] = { 0 };

	memcpy(szMemberName, lpMsg->szMemberName, 10);

	pMsg.h.set((LPBYTE)&pMsg, 0xA4, 0x03, sizeof(_stAnsJoinMemberStateListPartyMatching));

	// --------------------------------------------------------------------
	// Check wait list
	// --------------------------------------------------------------------

	if (g_PartyMatching_DBSet.GetPartyMatchingWaitList(szMemberName, szLeaderName))
	{
		// Not waiting / failed
		pMsg.nResult = -1;
		pMsg.nUserIndex = nUserIndex;

		memset(pMsg.szLeaderName, 0, sizeof(pMsg.szLeaderName));
		pMsg.btLeaderChannel = (BYTE)-1;
	}
	else
	{
		// Waiting state exists
		pMsg.nResult = 0;
		pMsg.nUserIndex = nUserIndex;

		memcpy(pMsg.szLeaderName, szLeaderName, 10);

		lpFindMember = CFriendManager.Search(szLeaderName);

		if (lpFindMember)
		{
			pMsg.btLeaderChannel = lpFindMember->pServer[0];
		}
		else
		{
			pMsg.btLeaderChannel = (BYTE)-1;
		}
	}

	// --------------------------------------------------------------------
	// Send result
	// --------------------------------------------------------------------

	if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (LPBYTE)&pMsg, sizeof(_stAnsJoinMemberStateListPartyMatching));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
	}
}

void GDReqMemberJoinStateListLeader(_stReqWaitListPartyMatching* lpMsg, int iServerIndex)
{
	_stAnsWaitListPartyMatching pMsg;
	_PARTY_MEMBER_WAIT_LIST stList[10];

	char szLeaderName[MAX_IDSTRING + 1];
	int nUserIndex;

	nUserIndex = lpMsg->nUserIndex;

	memset(szLeaderName, 0, sizeof(szLeaderName));
	memcpy(szLeaderName, lpMsg->szLeaderName, MAX_IDSTRING);

	memset(stList, 0, sizeof(stList));

	PHeadSubSetW((LPBYTE)&pMsg.h, 0xA4, 0x04, sizeof(_stAnsWaitListPartyMatching));

	pMsg.nListCount = g_PartyMatching_DBSet.GetPartyMatchingWaitListForLeader(szLeaderName, stList);

	pMsg.nUserIndex = nUserIndex;
	pMsg.nResult = 0;

	if (pMsg.nListCount > 0)
	{
		memcpy(pMsg.stList, stList, sizeof(pMsg.stList));
	}

	if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg, sizeof(_stAnsWaitListPartyMatching));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
	}
}

void GDReqCancelPartyMatching(_stReqCancePartyMatching* lpMsg, int iServerIndex)
{
	_stAnsCancePartyMatching pMsg;

	_PARTY_INFO_STRUCT* lpParty;
	_PARTY_INFO_STRUCT* lpMemberParty;

	_FRIEND_INFO_STRUCT* lpFindMember;

	_PARTY_MEMBER_WAIT_LIST stList[10];

	char szName[MAX_IDSTRING + 1];

	int nUserIndex;
	BYTE btType;

	int nListCount;
	int nMemberServerIndex;
	int aIndex;

	nUserIndex = lpMsg->nUserIndex;
	btType = lpMsg->btType;

	memset(szName, 0, sizeof(szName));
	memcpy(szName, lpMsg->szName, MAX_IDSTRING);

	pMsg.h.set((LPBYTE)&pMsg, 0xA4, 0x06, sizeof(_stAnsCancePartyMatching));

	pMsg.nResult = 0;
	pMsg.btType = btType;

	if (btType) // cancel wait list
	{
		g_PartyMatching_DBSet.DeleteWaitList(szName);
		lpMemberParty = g_PartyMatching.SearchPartyMemberName(szName);

		if (lpMemberParty)
		{
			g_PartyMatching.DelPartyMember(szName, lpMemberParty->stPartyMember[0].Name);
			g_PartyMatching_DBSet.UpdatePartyMemberCount(lpMemberParty->stPartyMember[0].Name, lpMemberParty->nCount);
			DGSendPartyMember(lpMemberParty->stPartyMember[0].Name, 0);
		}
	}
	else // cancel party matching
	{
		g_PartyMatching_DBSet.DeletePartyMatching(szName);
		lpParty = g_PartyMatching.SearchParty(szName);

		if (lpParty)
		{
			if (strcmp(lpParty->stPartyMember[0].Name, szName) != 0)
			{
				g_PartyMatching.DelPartyMember(szName, lpParty->stPartyMember[0].Name);
				g_PartyMatching_DBSet.UpdatePartyMemberCount(lpParty->stPartyMember[0].Name, lpParty->nCount);
			}
			else
			{
				// notify current members
				if (lpParty->nCount > 1)
				{
					pMsg.nResult = 2;

					for (int i = 1; i < 5; i++)
					{
						if (lpParty->stPartyMember[i].bUse && lpParty->stPartyMember[i].bLogOn)
						{
							nMemberServerIndex = lpParty->stPartyMember[i].nServerIndex;
							pMsg.nUserIndex = lpParty->stPartyMember[i].nUserIndex;

							if (nMemberServerIndex >= 0 && nMemberServerIndex <= MAX_SERVEROBJECT - 1)
							{
								WzIoEngine->send(gsm.m_Obj[nMemberServerIndex].lpPHD, (BYTE*)&pMsg, sizeof(_stAnsCancePartyMatching));
							}
							else
							{
								cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
							}
						}
					}
				}

				memset(stList, 0, sizeof(stList));
				nListCount = g_PartyMatching_DBSet.GetPartyMatchingWaitListForLeader(szName, stList);

				if (nListCount > 0)
				{
					for (int j = 0; j < nListCount && j < 10; j++)
					{
						lpFindMember = CFriendManager.Search(stList[j].szName);

						if (lpFindMember)
						{
							aIndex = lpFindMember->sIndex;

							pMsg.nUserIndex = lpFindMember->Number;
							pMsg.nResult = 1;
							pMsg.btType = btType;

							g_PartyMatching_DBSet.DeleteWaitList(stList[j].szName);

							if (aIndex >= 0 && aIndex <= MAX_SERVEROBJECT - 1)
							{
								WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (BYTE*)&pMsg, sizeof(_stAnsCancePartyMatching));
							}
							else
							{
								cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
							}
						}
					}
				}
			}
		}
		else
		{
			pMsg.nResult = -1;
		}
	}

	pMsg.btType = btType;
	pMsg.nUserIndex = nUserIndex;

	if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
	{
		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg, sizeof(_stAnsCancePartyMatching));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
	}
}

void GDAutoMemberJoin(char* szLeaderName, char* szMemberName, int nMemberIndex, int iServerIndex, int nLevel, BYTE btClass)
{
	if (strlen(szLeaderName) <= 0)
	{
		return;
	}

	if (strlen(szMemberName) <= 0)
	{
		return;
	}

	AddFriendListForPartyMatching(nMemberIndex, szMemberName, iServerIndex);

	_FRIEND_INFO_STRUCT* lpFindMember = CFriendManager.Search(szLeaderName);

	_FRIEND_INFO_STRUCT* lpFindTargetMember = CFriendManager.Search(szMemberName);

	int nLeaderIndex = -1;
	int nLeaderServerChannel = -1;
	int nLeaderServerIndex = -1;
	int nMemberServerIndex = -1;
	int nResult = -1;

	__stAnsAddPartyMember pMsg;

	pMsg.h.set((LPBYTE)&pMsg, 0xA4, 0x05, sizeof(__stAnsAddPartyMember));

	pMsg.nUserIndex = nLeaderIndex;
	pMsg.nMemberIndex = nMemberIndex;

	pMsg.btType = 2;
	pMsg.btSendType = 0;
	pMsg.btManualJoin = 0;

	memset(pMsg.szMemberName, 0, sizeof(pMsg.szMemberName));
	memcpy(pMsg.szMemberName, szMemberName, MAX_IDSTRING);

	_PARTY_INFO_STRUCT* lpPartyInfo =
		g_PartyMatching.SearchParty(szLeaderName);

	if (lpFindTargetMember)
	{
		nMemberServerIndex = lpFindTargetMember->ServerIndex[0];
	}

	if (lpPartyInfo)
	{
		nLeaderIndex = lpPartyInfo->stPartyMember[0].nUserIndex;
		nLeaderServerChannel = lpPartyInfo->stPartyMember[0].nServerChannel;
		nLeaderServerIndex = lpPartyInfo->stPartyMember[0].nServerIndex;
		pMsg.nUserIndex = nLeaderIndex;
		int nServerChannel = gsm.m_Obj[iServerIndex].pServer;

		nResult = g_PartyMatching.AddPartyMember(
			nServerChannel,
			iServerIndex,
			nMemberIndex,
			szMemberName,
			szLeaderName,
			nLevel,
			btClass);

		pMsg.nResult = nResult;

		if (nResult != -2)
		{
			if (nResult == -1)
			{
				nResult = -3;
			}
			else
			{
				_PARTY_INFO_STRUCT* lpParty = g_PartyMatching.SearchParty(szLeaderName);

				if (lpParty)
				{
					g_PartyMatching_DBSet.UpdatePartyMemberCount(lpParty->stPartyMember[0].Name, lpParty->nCount);
				}
			}
		}

		// send to leader
		pMsg.btSendType = 0;

		if (nLeaderServerIndex >= 0 && nLeaderServerIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[nLeaderServerIndex].lpPHD, (BYTE*)&pMsg, sizeof(__stAnsAddPartyMember));
		}
		else
		{
			cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
		}

		// send to member
		pMsg.btSendType = 1;

		if (nMemberServerIndex >= 0 &&
			nMemberServerIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[nMemberServerIndex].lpPHD, (BYTE*)&pMsg, sizeof(__stAnsAddPartyMember));
		}
		else
		{
			cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
		}

		if (nLeaderServerIndex == iServerIndex)
		{
			DGFixPartyMember(nLeaderIndex, nMemberIndex, nLeaderServerIndex);
		}

		DGSendPartyMember(szLeaderName, 0);
	}
	else
	{
		pMsg.nResult = -3;
		pMsg.btSendType = 1;

		if (nMemberServerIndex >= 0 && nMemberServerIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[nMemberServerIndex].lpPHD, (BYTE*)&pMsg, sizeof(__stAnsAddPartyMember));
		}
		else
		{
			cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
		}
	}
}

void GDReqAddPartyMember(__stReqAddPartyMember* lpMsg, int iServerIndex)
{
	int nUserIndex = lpMsg->nUserIndex;

	char szLeaderName[MAX_IDSTRING + 1] = { 0 };
	char szMemberName[MAX_IDSTRING + 1] = { 0 };

	memcpy(szLeaderName, lpMsg->szLeaderName, MAX_IDSTRING);
	memcpy(szMemberName, lpMsg->szMemberName, MAX_IDSTRING);

	if (strlen(szLeaderName) <= 0)
	{
		return;
	}

	if (strlen(szMemberName) <= 0)
	{
		return;
	}

	AddFriendListForPartyMatching(nUserIndex, szLeaderName, iServerIndex);

	_FRIEND_INFO_STRUCT* lpFindMember = CFriendManager.Search(szMemberName);

	int bMemberConnected = FALSE;
	int aIndex = -1;
	int nMemberIndex = -1;
	int nServerChannel = -1;

	if (lpFindMember)
	{
		aIndex = lpFindMember->sIndex;
		nMemberIndex = lpFindMember->Number;
		nServerChannel = lpFindMember->pServer[0];
		bMemberConnected = TRUE;
	}
	else if (lpMsg->btAlradyParty == 1)
	{
		nMemberIndex = lpMsg->nMemberIndex;
		nServerChannel = gsm.m_Obj[iServerIndex].pServer;
		bMemberConnected = TRUE;

		AddFriendListForPartyMatching(nMemberIndex, szMemberName, iServerIndex);
	}

	__stAnsAddPartyMember pMsg;

	pMsg.h.set((LPBYTE)&pMsg.h, 0xA4, 0x05, sizeof(__stAnsAddPartyMember));

	pMsg.nUserIndex = nUserIndex;
	pMsg.nMemberIndex = nMemberIndex;
	pMsg.btManualJoin = lpMsg->btManualJoin;

	memcpy(pMsg.szMemberName, szMemberName, MAX_IDSTRING);

	int nResult = -1;

	// Already in party
	if (lpMsg->btAlradyParty)
	{
		nResult = g_PartyMatching.AddPartyMember(nServerChannel, aIndex, nMemberIndex, szMemberName, szLeaderName, lpMsg->nMemberLevel, lpMsg->btMemeberClass);
		_PARTY_INFO_STRUCT* lpParty = g_PartyMatching.SearchParty(szLeaderName);

		if (lpParty)
		{
			g_PartyMatching_DBSet.UpdatePartyMemberCount(lpParty->stPartyMember[0].Name, lpParty->nCount);
		}
	}
	else
	{
		if (bMemberConnected)
		{
			// Real join request
			if (lpMsg->btType == 1)
			{
				_PARTY_INFO_STRUCT* pNode = g_PartyMatching.SearchParty(szLeaderName);

				// Create party if manual join
				if (lpMsg->btManualJoin == 1 && pNode == NULL)
				{
					int pServer = gsm.m_Obj[iServerIndex].pServer;

					switch (gsm.m_Obj[iServerIndex].btGameServerType)
					{
					case 1:
						pServer = 199;
						break;
					case 2:
						pServer = 200;
						break;
					case 3:
						pServer = 201;
						break;
					}

					g_PartyMatching.CreateParty(pServer, iServerIndex, nUserIndex, szLeaderName, 1, lpMsg->nLeaderLevel, lpMsg->btLeaderClass);
				}
				else if (pNode == NULL)
				{
					pMsg.btSendType = 0;
					pMsg.btType = lpMsg->btType;
					pMsg.nResult = -1;

					if (iServerIndex >= 0 && iServerIndex <= 99)
					{
						WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (LPBYTE)&pMsg.h, sizeof(__stAnsAddPartyMember));
					}
					else
					{
						cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
					}
					return;
				}

				int  nMemberLevel[1];
				BYTE btMemberClass[1];

				nMemberLevel[0] = lpMsg->nMemberLevel;
				btMemberClass[0] = lpMsg->btMemeberClass;

				if (nMemberLevel[0] == 0)
				{
					g_PartyMatching_DBSet.GetPartyMatchingWaitMemberInfo(szMemberName, nMemberLevel, btMemberClass);
				}

				nResult = g_PartyMatching.AddPartyMember(nServerChannel, aIndex, nMemberIndex, szMemberName, szLeaderName, nMemberLevel[0], btMemberClass[0]);

				pMsg.nResult = nResult;
				pMsg.btSendType = 0;
				pMsg.btType = lpMsg->btType;

				if (nResult != -2)
				{
					if (nResult == -1)
					{
						nResult = -3;
					}
					else
					{
						_PARTY_INFO_STRUCT* lpParty = g_PartyMatching.SearchParty(szLeaderName);

						if (lpParty)
						{
							g_PartyMatching_DBSet.UpdatePartyMemberCount(lpParty->stPartyMember[0].Name, lpParty->nCount);
						}
					}
				}

				// Send to leader
				if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
				{
					WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (LPBYTE)&pMsg.h, sizeof(__stAnsAddPartyMember));
				}
				else
				{
					cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
				}

				// Send to member
				pMsg.btSendType = 1;

				if (aIndex >= 0 && aIndex <= MAX_SERVEROBJECT - 1)
				{
					WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (LPBYTE)&pMsg.h, sizeof(__stAnsAddPartyMember));
				}
				else
				{
					cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
				}

				if (aIndex == iServerIndex && nResult == 0)
				{
					DGFixPartyMember(nUserIndex, nMemberIndex, iServerIndex);
				}

				DGSendPartyMember(szLeaderName, 0);
			}
			else
			{
				// Invitation only
				pMsg.nMemberIndex = nMemberIndex;
				pMsg.nUserIndex = nUserIndex;
				pMsg.nResult = 0;
				pMsg.btSendType = 0;
				pMsg.btType = lpMsg->btType;

				if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
				{
					WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (LPBYTE)&pMsg.h, sizeof(__stAnsAddPartyMember));
				}
				else
				{
					cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
				}

				if (g_PartyMatching.SearchParty(szLeaderName))
				{
					pMsg.btSendType = 1;

					if (aIndex >= 0 && aIndex <= MAX_SERVEROBJECT - 1)
					{
						WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (LPBYTE)&pMsg.h, sizeof(__stAnsAddPartyMember));
					}
					else
					{
						cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
					}
				}
			}
		}
		else
		{
			pMsg.nResult = -1;
			pMsg.btSendType = 0;

			if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
			{
				WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (LPBYTE)&pMsg.h, sizeof(__stAnsAddPartyMember));
			}
			else
			{
				cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
			}
		}
		g_PartyMatching_DBSet.DeleteWaitList(szMemberName);
	}
}

void DGFixPartyMember(int nLeaderIndex, int nMemberIndex, int nServerIndex)
{
	__stAnsAddRealPartyMember pMsg;

	pMsg.nLeaderUserIndex = nLeaderIndex;
	pMsg.nMemberUserIndex = nMemberIndex;

	pMsg.h.set((LPBYTE)&pMsg.h, 0xA4, 0x10, sizeof(__stAnsAddRealPartyMember));

	if (nServerIndex >= 0 && nServerIndex <= MAX_SERVEROBJECT - 1)
	{
		WzIoEngine->send(gsm.m_Obj[nServerIndex].lpPHD, (LPBYTE)&pMsg.h, sizeof(__stAnsAddRealPartyMember));
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
	}
}

void DGSendPartyMember(char* szLeaderName, int bEmptyList)
{
	_PARTY_INFO_STRUCT* lpPartyInfo =
		g_PartyMatching.SearchParty(szLeaderName);

	if (!lpPartyInfo)
		return;

	if (lpPartyInfo->nCount < 1)
		bEmptyList = TRUE;

	for (int i = 0; i < 5; i++)
	{
		_PARTY_MEMBER_INFO& member = lpPartyInfo->stPartyMember[i];

		if (!member.bLogOn || !member.bUse)
			continue;

		__stAnsSendPartyMemberList pMsg;

		PHeadSubSetB((LPBYTE)&pMsg, 0xA4, 0x11, sizeof(pMsg));

		pMsg.nMemberCount = lpPartyInfo->nCount;
		pMsg.nUserIndex = member.nUserIndex;
		pMsg.nPartyMatchingIndex = lpPartyInfo->nPartyNumber;

		int nSendServerIndex = member.nServerIndex;

		if (bEmptyList)
		{
			pMsg.nMemberCount = 0;
		}
		else
		{
			for (int k = 0; k < 5; k++)
			{
				_PARTY_MEMBER_INFO& src = lpPartyInfo->stPartyMember[k];

				if (!src.bUse)
					continue;

				memcpy(pMsg.stList[k].Name, src.Name, 10);

				pMsg.stList[k].bUse = src.bUse;
				pMsg.stList[k].nServerChannel = src.nServerChannel + 1;
				pMsg.stList[k].nUserIndex = src.nUserIndex;

				// same game server?
				pMsg.stList[k].bFlag = (src.nServerIndex == nSendServerIndex);
			}
		}

		if (nSendServerIndex >= 0 && nSendServerIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[nSendServerIndex].lpPHD, (BYTE*)&pMsg, sizeof(pMsg));
		}
		else
		{
			cLog.Add("error-L2 : Invalid index number");
		}
	}
}

void DGSendEmptyPartyMemberList(char* szMemberName)
{
	_PARTY_INFO_STRUCT* lpPartyInfo = g_PartyMatching.SearchPartyMemberName(szMemberName);

	if (lpPartyInfo == NULL)
	{
		return;
	}

	for (int i = 0; i < 5; i++)
	{
		if (lpPartyInfo->stPartyMember[i].bLogOn == FALSE)
		{
			continue;
		}

		if (lpPartyInfo->stPartyMember[i].bUse == FALSE)
		{
			continue;
		}

		if (strcmp(lpPartyInfo->stPartyMember[i].Name, szMemberName) != 0)
		{
			continue;
		}

		__stAnsSendPartyMemberList pMsgSendList;

		memset(&pMsgSendList, 0, sizeof(pMsgSendList));

		pMsgSendList.h.set((LPBYTE)&pMsgSendList.h, 0xA4, 0x11, sizeof(__stAnsSendPartyMemberList));

		pMsgSendList.nMemberCount = 0;
		pMsgSendList.nUserIndex = lpPartyInfo->stPartyMember[i].nUserIndex;
		pMsgSendList.nPartyMatchingIndex = lpPartyInfo->nPartyNumber;

		int nSendServerIndex = lpPartyInfo->stPartyMember[i].nServerIndex;

		if (nSendServerIndex >= 0 && nSendServerIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[nSendServerIndex].lpPHD, (LPBYTE)&pMsgSendList.h, sizeof(__stAnsSendPartyMemberList));
		}
		else
		{
			cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
		}
	}
}

void GDDeletePartyUser(_stReqDelPartyUserPartyMatching* lpMsg, int iServerIndex)
{
	_stAnsDelPartyUserPartyMatching pMsg;
	_stAnsCancePartyMatching pCancelMsg;

	int bChangeLeader = 0;

	char szNewLeader[MAX_IDSTRING + 1];
	char szLeaderName[MAX_IDSTRING + 1];
	char szTargetName[MAX_IDSTRING + 1];

	int nLeaderServerIndex;
	int nLeaderUserIndex;

	int nTargetServerIndex;
	int nTargetIndex;

	int nResult = 0;

	_PARTY_INFO_STRUCT* lpPartyInfo;

	memset(szTargetName, 0, sizeof(szTargetName));
	memcpy(szTargetName, lpMsg->szTargetName, MAX_IDSTRING);

	lpPartyInfo = g_PartyMatching.SearchPartyMemberName(szTargetName);

	if (lpPartyInfo == NULL)
	{
		cLog.Add("error-L2 : value is null  %s %d", __FILE__, __LINE__);
		return;
	}

	nTargetIndex = -1;
	nTargetServerIndex = -1;

	nLeaderUserIndex = lpPartyInfo->stPartyMember[0].nUserIndex;
	nLeaderServerIndex = lpPartyInfo->stPartyMember[0].nServerIndex;

	memset(szLeaderName, 0, sizeof(szLeaderName));
	memcpy(szLeaderName, lpPartyInfo->stPartyMember[0].Name, MAX_IDSTRING);

	for (int i = 0; i < 5; i++)
	{
		if (strcmp(lpPartyInfo->stPartyMember[i].Name, szTargetName) == 0)
		{
			nTargetIndex = lpPartyInfo->stPartyMember[i].nUserIndex;
			nTargetServerIndex = lpPartyInfo->stPartyMember[i].nServerIndex;
			break;
		}
	}

	if (nTargetIndex >= 0)
	{
		// not leader
		if (strcmp(szTargetName, szLeaderName) != 0)
		{
			DGSendEmptyPartyMemberList(szTargetName);
			g_PartyMatching.DelPartyMember(szTargetName, szLeaderName);
			g_PartyMatching_DBSet.UpdatePartyMemberCount(szLeaderName, lpPartyInfo->nCount);
			DGSendPartyMember(szLeaderName, 0);
		}
		else
		{
			memset(szNewLeader, 0, sizeof(szNewLeader));
			bChangeLeader = 0;

			if (g_PartyMatching.ChangeLeader(szLeaderName, szNewLeader, 0) == 1 && lpPartyInfo->nCount >= 2)
			{
				bChangeLeader = 1;
				DGSendEmptyPartyMemberList(szTargetName);
				g_PartyMatching.DelPartyMember(szTargetName, szNewLeader);
				DGSendPartyMember(szNewLeader, 0);
			}
			else
			{
				g_PartyMatching_DBSet.DeletePartyMatching(szLeaderName);
				DGSendPartyMember(szLeaderName, 1);
				g_PartyMatching.DelParty(szLeaderName);

				pCancelMsg.h.set((LPBYTE)&pCancelMsg, 0xA4, 0x06, sizeof(_stAnsCancePartyMatching));
				pCancelMsg.nResult = 0;
				pCancelMsg.btType = 0;
				pCancelMsg.nUserIndex = nLeaderUserIndex;

				if (nLeaderServerIndex >= 0 && nLeaderServerIndex <= MAX_SERVEROBJECT - 1)
				{
					WzIoEngine->send(gsm.m_Obj[nLeaderServerIndex].lpPHD, (LPBYTE)&pCancelMsg, sizeof(_stAnsCancePartyMatching));
				}
				else
				{
					cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
				}
			}

			if (lpPartyInfo->nCount <= 1 || bChangeLeader)
			{
				DGClearPartyWaitListForOldLeader(szLeaderName);
			}
		}
	}
	else
	{
		nResult = -1;
	}

	DGSendDelTypePartyMember(szLeaderName, szTargetName, lpMsg->btDelType);

	pMsg.nUserIndex = lpMsg->nUserIndex;
	pMsg.nTargetIndex = nTargetIndex;
	pMsg.nResult = nResult;

	pMsg.h.set((LPBYTE)&pMsg, 0xA4, 0x07, sizeof(_stAnsDelPartyUserPartyMatching));

	// self delete
	if (pMsg.nUserIndex == pMsg.nTargetIndex)
	{
		if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (LPBYTE)&pMsg, sizeof(_stAnsDelPartyUserPartyMatching));
		}
		else
		{
			cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
		}
	}
	else
	{
		// sender notify
		pMsg.btType = 0;

		if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (LPBYTE)&pMsg, sizeof(_stAnsDelPartyUserPartyMatching));
		}
		else
		{
			cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
		}

		// target notify
		pMsg.btType = 1;

		if (nTargetServerIndex >= 0 && nTargetServerIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[nTargetServerIndex].lpPHD, (LPBYTE)&pMsg, sizeof(_stAnsDelPartyUserPartyMatching));
		}
		else
		{
			cLog.Add("[PMATCHING]not connected [%s]  %s %d", szTargetName, __FILE__, __LINE__);
		}
	}
}

void GDReqSendPartyMember(__stReqSendPartyMemberList* lpMsg)
{
	DGSendPartyMember(lpMsg->szLeaderName, 0);
}

void AddFriendListForPartyMatching(int nNumber, char* szName, __int16 nServerindex)
{
	_FRIEND_INFO_STRUCT* lpFindMember;

	lpFindMember = CFriendManager.Search(szName);
	if (nServerindex >= 0 && nServerindex <= MAX_SERVEROBJECT - 1)
	{
		if (!lpFindMember)
			CFriendManager.Add(nNumber, szName, gsm.m_Obj[nServerindex].pServer, nServerindex);
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}
}

void GServerDownDelParty(int nServer)
{
	_stAnsCancePartyMatching pMsg;
	pMsg.h.set(0xA4, 6, sizeof(pMsg));

	// Process all parties whose leader is on the server that went down
	_PARTY_INFO_STRUCT* lpSerchParty;
	while ((lpSerchParty = g_PartyMatching.SearchPartyForLeaderServerIndex(nServer)) != NULL)
	{
		// Delete the party matching from the database
		g_PartyMatching_DBSet.DeletePartyMatching(lpSerchParty->stPartyMember[0].Name);

		// If party has more than just the leader, notify all members
		if (lpSerchParty->nCount > 1)
		{
			for (int nPartyCnt = 0; nPartyCnt < 5; ++nPartyCnt)
			{
				if (lpSerchParty->stPartyMember[nPartyCnt].bUse &&
					lpSerchParty->stPartyMember[nPartyCnt].bLogOn)
				{
					int nMemberServerIndex = lpSerchParty->stPartyMember[nPartyCnt].nServerIndex;
					pMsg.btType = 0;

					if (nMemberServerIndex >= 0 && nMemberServerIndex <= MAX_SERVEROBJECT - 1)
					{
						WzIoEngine->send(gsm.m_Obj[nMemberServerIndex].lpPHD, (unsigned __int8*)&pMsg.h, sizeof(pMsg));
					}
					else
					{
						cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
					}
				}
			}
		}

		// Get the wait list for this party leader
		_PARTY_MEMBER_WAIT_LIST stList[10];
		memset(stList, 0, sizeof(stList));

		int nListCount = g_PartyMatching_DBSet.GetPartyMatchingWaitListForLeader(lpSerchParty->stPartyMember[0].Name, stList);

		// Notify waiting members that the party was cancelled
		if (nListCount > 0)
		{
			for (int i = 0; i < nListCount && i < 10; ++i)
			{
				_FRIEND_INFO_STRUCT* lpFindMember = CFriendManager.Search(stList[i].szName);
				if (lpFindMember)
				{
					int aIndex = lpFindMember->sIndex;

					pMsg.nUserIndex = lpFindMember->Number;
					pMsg.nResult = 1;
					pMsg.btType = 0;

					// Remove from wait list
					g_PartyMatching_DBSet.DeleteWaitList(stList[i].szName);

					if (aIndex >= 0 && aIndex <= MAX_SERVEROBJECT - 1)
					{
						WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (unsigned __int8*)&pMsg.h, sizeof(_PARTY_MEMBER_WAIT_LIST));
					}
					else
					{
						cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
					}
				}
			}
		}

		// Remove party from in-memory manager
		g_PartyMatching.DelParty(lpSerchParty->stPartyMember[0].Name);
	}
}

void GDSendChattingMsgPartyMatching(_stReqChattingPartyMatching* lpMsg)
{
	_PARTY_INFO_STRUCT* lpParty = g_PartyMatching.SearchParty(lpMsg->nPartyIndex);

	if (lpParty == NULL)
	{
		return;
	}

	_stAnsChattingPartyMatching pMsg;

	pMsg.h.set((LPBYTE)&pMsg.h, 0xA4, 0x12, sizeof(_stAnsChattingPartyMatching));

	pMsg.nPartyIndex = lpMsg->nPartyIndex;
	memcpy(pMsg.szChat, lpMsg->szChat, MAX_CHAT);
	memcpy(pMsg.szSendCharName, lpMsg->szSendCharName, MAX_IDSTRING);

	int nCount = lpParty->nCount;

	for (int i = 0; i < nCount; i++)
	{
		if (lpParty->stPartyMember[i].bLogOn == FALSE)
		{
			continue;
		}

		if (lpParty->stPartyMember[i].bUse == FALSE)
		{
			continue;
		}

		pMsg.nRecvUserIndex = lpParty->stPartyMember[i].nUserIndex;
		pMsg.nPartyIndex = lpParty->nPartyNumber;
		memcpy(pMsg.szChat, lpMsg->szChat, MAX_CHAT);
		memcpy(pMsg.szSendCharName, lpMsg->szSendCharName, MAX_IDSTRING);

		int nDstServerIndex = lpParty->stPartyMember[i].nServerIndex;

		if (nDstServerIndex >= 0 && nDstServerIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[nDstServerIndex].lpPHD, (LPBYTE)&pMsg.h, sizeof(_stAnsChattingPartyMatching));
		}
		else
		{
			cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
		}
	}
}

void GDReqSwapPartyLeader(_stReqChangePartyLeader* lpMsg, int iServerIndex)
{
	_stAnsChangePartyLeader pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	// Extract new leader name
	char szNewLeaderName[16];
	memset(szNewLeaderName, 0, MAX_IDSTRING + 1);
	memcpy(szNewLeaderName, lpMsg->szNewLeaderName, MAX_IDSTRING);  // 10 bytes

	// Find party by searching for the new leader's name
	_PARTY_INFO_STRUCT* lpPartyInfo = g_PartyMatching.SearchPartyMemberName(szNewLeaderName);

	if (!lpPartyInfo)
	{
		cLog.Add("error-L2 : value is null  %s %d", __FILE__, __LINE__);
		return;
	}

	unsigned __int8 nResult = 1;
	int bChangeLeader = 0;
	int nNewLeaderIndex = -1;
	int nNewLeaderServerIndex = -1;
	int nNewLeaderChannelIndex = -1;

	// Get old leader info from party member[0]
	char szLeaderName[20];
	memset(szLeaderName, 0, MAX_IDSTRING + 1);

	int nOldLeaderUserIndex = lpPartyInfo->stPartyMember[0].nUserIndex;
	int nOldLeaderServerIndex = lpPartyInfo->stPartyMember[0].nServerIndex;
	int nOldLeaderChannelIndex = lpPartyInfo->stPartyMember[0].nServerChannel;
	memcpy(szLeaderName, lpPartyInfo->stPartyMember[0].Name, MAX_IDSTRING);

	// Find new leader in party member list
	int i;
	for (i = 0; i < 5; ++i)
	{
		if (strcmp(lpPartyInfo->stPartyMember[i].Name, szNewLeaderName) == 0)
		{
			nNewLeaderIndex = lpPartyInfo->stPartyMember[i].nUserIndex;
			nNewLeaderServerIndex = lpPartyInfo->stPartyMember[i].nServerIndex;
			nNewLeaderChannelIndex = lpPartyInfo->stPartyMember[i].nServerChannel;
			break;
		}
	}

	// Verify old leader name matches
	if (strcmp(szLeaderName, lpMsg->szOldLeaderName) != 0)
	{
		nResult = 0;  // Old leader name doesn't match
	}
	else if (nNewLeaderIndex >= 0)
	{
		// Attempt to change leader
		if (g_PartyMatching.ChangeLeader(szLeaderName, szNewLeaderName, 1) == 1)
			bChangeLeader = 1;
		else
			nResult = 0;

		// Clear wait list for old leader if change succeeded
		if (bChangeLeader)
			DGClearPartyWaitListForOldLeader(szLeaderName);
	}
	else
	{
		nResult = 0;  // New leader not found in party
	}

	// Check if new leader is on a different channel
	int IsOhterChannelLeader = 0;
	if (nNewLeaderChannelIndex > -1)
		IsOhterChannelLeader = (nOldLeaderChannelIndex != nNewLeaderChannelIndex);

	// Build response packet
	pMsg.h.set((unsigned char*)&pMsg.h, 0xA4, 0x13, sizeof(pMsg));
	//pMsg.h.set(0xA4, 0x13, sizeof(pMsg));  // headcode=0xA4, subcode=0x13, size=20
	pMsg.nOldLeaderIndex = lpMsg->nOldUserIndex;
	pMsg.nNewLeaderIndex = nNewLeaderIndex;
	pMsg.nResult = nResult;
	pMsg.btChangeType = 0;

	// Send response to requesting server
	if (iServerIndex >= 0 && iServerIndex <= MAX_SERVEROBJECT - 1)
	{
		if (IsOhterChannelLeader)
			pMsg.btChangeType = 1;  // Different channel

		WzIoEngine->send(gsm.m_Obj[iServerIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_stAnsChangePartyLeader));
		DGSendPartyMember(szNewLeaderName, 0);
	}
	else
	{
		cLog.Add("error-L2 : Invalid index number  %s %d", __FILE__, __LINE__);
	}

	// If new leader is on a different channel and change succeeded, notify their server
	if (IsOhterChannelLeader && nResult)
	{
		if (nNewLeaderServerIndex >= 0 && nNewLeaderServerIndex <= 99)
		{
			pMsg.btChangeType = 2;  // Cross-channel notification
			WzIoEngine->send(gsm.m_Obj[nNewLeaderServerIndex].lpPHD, (BYTE*)&pMsg.h, sizeof(_stAnsChangePartyLeader));
			DGSendPartyMember(szNewLeaderName, 0);
		}
		else
		{
			cLog.Add("[PMATCHING] not connected [%s]  %s %d",
				szNewLeaderName, __FILE__, __LINE__);
		}
	}
}

void GDReqSendPartyMemberList(__stReqSendPartyMemberList* lpMsg)
{
	char szLeaderName[16]; // [esp+D0h] [ebp-14h] BYREF

	memset(szLeaderName, 0, MAX_IDSTRING + 1);
	memcpy(szLeaderName, lpMsg->szLeaderName, MAX_IDSTRING);
	DGSendPartyMember(szLeaderName, 0);
}

void DGClearPartyWaitListForOldLeader(char* szLeaderName)
{
	_stAnsCancePartyMatching pCancelMsg;

	pCancelMsg.h.set((LPBYTE)&pCancelMsg.h, 0xA4, 0x06, sizeof(_stAnsCancePartyMatching));

	pCancelMsg.nResult = 1;
	pCancelMsg.btType = 0;

	_PARTY_MEMBER_WAIT_LIST stList[10];

	memset(stList, 0, sizeof(stList));

	int nListCount = g_PartyMatching_DBSet.GetPartyMatchingWaitListForLeader(szLeaderName, stList);

	if (nListCount <= 0)
	{
		return;
	}

	for (int i = 0; i < nListCount && i < 10; i++)
	{
		_FRIEND_INFO_STRUCT* lpFindMember = CFriendManager.Search(stList[i].szName);

		if (lpFindMember == NULL)
		{
			continue;
		}

		int aIndex = lpFindMember->sIndex;
		pCancelMsg.nUserIndex = lpFindMember->Number;

		g_PartyMatching_DBSet.DeleteWaitList(stList[i].szName);

		if (aIndex >= 0 && aIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[aIndex].lpPHD, (LPBYTE)&pCancelMsg.h, sizeof(_stAnsCancePartyMatching));
		}
		else
		{
			cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
		}
	}
}

void DGSendDelTypePartyMember(char* szLeaderName, char* szTargetName, BYTE btDelType)
{
	_PARTY_INFO_STRUCT* lpPartyInfo = g_PartyMatching.SearchParty(szLeaderName);

	if (lpPartyInfo == NULL)
	{
		return;
	}

	if (lpPartyInfo->nCount < 1)
	{
		return;
	}

	for (int i = 0; i < 5; i++)
	{
		if (lpPartyInfo->stPartyMember[i].bLogOn == FALSE)
		{
			continue;
		}

		if (lpPartyInfo->stPartyMember[i].bUse == FALSE)
		{
			continue;
		}

		_stAnsPartyMemberDelType pMsgSendList;
		pMsgSendList.h.set((LPBYTE)&pMsgSendList.h, 0xA4, 0x14, sizeof(_stAnsPartyMemberDelType));
		pMsgSendList.nRecvUserIndex = lpPartyInfo->stPartyMember[i].nUserIndex;
		pMsgSendList.btDelType = btDelType;
		memcpy(pMsgSendList.szDelName, szTargetName, sizeof(pMsgSendList.szDelName));

		int nSendServerIndex = lpPartyInfo->stPartyMember[i].nServerIndex;

		if (nSendServerIndex >= 0 && nSendServerIndex <= MAX_SERVEROBJECT - 1)
		{
			WzIoEngine->send(gsm.m_Obj[nSendServerIndex].lpPHD, (LPBYTE)&pMsgSendList.h, sizeof(_stAnsPartyMemberDelType));
		}
		else
		{
			cLog.Add("error-L2 : Invalid index number %s %d", __FILE__, __LINE__);
		}
	}
}