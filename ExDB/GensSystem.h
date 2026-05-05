// GensSystem.h
#pragma once
#include "Protocol.h"

// ----------------------------------------------------------------------
// GensSystem
// ----------------------------------------------------------------------

class GensSystem
{
public:
	GensSystem();
	virtual ~GensSystem();

public:
	int ReqRegGensMember(_tagPMSG_REQ_REG_GENS_MEMBER_EXDB* lpMsg);
	int ReqSecedeGensMember(_tagPMSG_REQ_SECEDE_GENS_MEMBER_EXDB* lpMsg);
	int ReqGensInfo(_tagPMSG_REQ_GENS_INFO_EXDB* lpMsg,_tagGensMemberInfo* pST_GensMemberInfo);
	int ReqSaveContributePoint(_tagPMSG_REQ_SAVE_CONTRIBUTE_POINT_EXDB* lpMsg);
	int ReqSaveAbusingKillUserName(_tagPMSG_REQ_SAVE_ABUSING_KILLUSER_EXDB* lpMsg);
	int ReqAbusingInfo(_tagPMSG_REQ_ABUSING_INFO* lpMsg,_tagPMSG_ANS_ABUSING_INFO* pAnsAbusingInfo);
	void ReqGensSecedeDateModify(_tagPMSG_REQ_SECEDE_GENS_DATE_MODIFY* lpMsg);
	int ReqGensReward(_tagPMSG_REQ_GENS_REWARD_EXDB* lpMsg,_tagPMSG_ANS_GENS_REWARD_EXDB* pAnsGensReward);
	int ReqGensRewardComplete(_tagPMSG_REQ_GENS_REWARD_COMPLETE_EXDB* lpMsg);
	int ReqGensMemberCount(_tagPMSG_REQ_GENS_MEMBER_COUNT_EXDB* lpMsg,_tagPMSG_ANS_GENS_MEMBER_COUNT_EXDB* pAnsGensMemberCount);
	int ReqSetGensRewardDay(_tagPMSG_REQ_SET_GENS_REWARD_DAY_EXDB* lpMsg);
	int ReqSetGensRanking(_tagPMSG_REQ_SET_GENS_RANKING_EXDB* lpMsg);
	int ReqGensRewardDayCheck(_tagPMSG_REQ_GENS_REWARD_DAY_CHECK_EXDB* lpMsg,_tagPMSG_ANS_GENS_REWARD_DAY_CHECK_EXDB* pAnsMsg);
};

extern GensSystem g_GensSystem;