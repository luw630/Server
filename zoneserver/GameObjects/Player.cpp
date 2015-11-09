#include "StdAfx.h"
#include "player.h"
#include "region.h"
#include "eventregion.h"
#include "fightarena.h"
#include "prison.h"
#include "biguanarea.h"
#include "area.h"
#include "UpgradeSys.h"
#include "Npc.h"
#include "Monster.h"
#include "Pet.h"
#include "Trigger.h"
#include "DFIghtData.h"
#include "DTelergyData.h"
#include "FactionManager.h"
#include "GameWorld.h"
#include "item.h"
#include "sportsarea.h"
#include "DSpcialItemData.h"
#include "GoatMutateData.h"
#include "ScriptManager.h"
#include "building.h"
#include "prisonex.h"
#include "GlobalFunctions.h"

#include "networkmodule\cardpointmodifymsgs.h"
#include "networkmodule\accountmsgs.h"

#include "networkmodule\movemsgs.h"
#include "networkmodule\scriptmsgs.h"
#include "networkmodule\upgrademsgs.h"
#include "networkmodule\refreshmsgs.h"
#include "networkmodule\regionmsgs.h"
#include "networkmodule\tongmsgs.h"
#include "networkmodule\chatmsgs.h"
#include "networkmodule\RelationMsgs.h"
#include "networkmodule\ItemTypedef.h"
#include "networkmodule/fightmsgs.h"
#include "networkmodule/UnionMsgs.h"
#include "plugingame.h"
#include "networkmodule/MenuMsgs.h"
#include "networkmodule/ItemMsgs.h"
#include "networkmodule/SectionMsgs.h"
#include "NETWORKMODULE/DailyMsgs.h"

#include "pub/rpcop.h"

#include <time.h>
#include "pub/traceinfo.h"
#include "DSpecialPlayerRcd.h"

#include "DSkillStudyTable.h"
#include "RoleLevelUpTbl.h"
#include "SkillDataTable.h"
#include "FlyDataTable.h"
#include "ProtectedDataTable.h"
#include "fightListener.h"
#include "Mounts.h"
#include "./MountEvent.h"

#include <algorithm>
//#include <atlutil.h>
#include "TeamManager.h"
#include "../networkmodule/TeamMsgs.h"
#include "CPlayerDefine.h"
#include "CPlayerService.h"
#include "CItemDefine.h"
#include "CItemService.h"
#include "CSkillDefine.h"
#include "CSkillService.h"
#include "CMountService.h"
#include "CJingMaiService.h"
#include "CFightPetDefine.h"
#include "CFightPetService.h"
#include "GroundItemWinner.h"
#include "Random.h"
#include "UpdatePropertyEx.h"
#include "CFriendService.h"
#include "CTiZhiService.h"
#include "CKylinArmService.h"
#include "CItemDropService.h"
#include "RankList.h"
#include "../networkmodule/typede_v4/PlayerTypedef.h"
#include "JingMaiShareExpEvent.h"
#include "DynamicScene.h"
#include "CGloryAndTitleServer.h"
#include "DynamicDailyArea.h"
#include "SanguoCode\BlessMoudle\BlessMoudle.h"
#include "SanguoCode\Common\PubTool.h"
#include "SanguoCode\ArenaMoudel\ArenaMoudel.h"
#include "SanguoCode\Common\ConfigManager.h"
extern  CSpecialPlayerRcd    *g_pSpecialPlayerRcd;
extern  WORD TopFactionID[ 5 ];
extern  string TopFactionName[ 5 ];
extern  DWORD FactionFightInf[2][5];
extern  DWORD TopFiveVals[ 5 ][ 5 ];  
extern std::string TopFiveName[ 5 ][ 5 ];
extern std::map< std::string, SFactionData > factionManagerMap;
extern std::map< DWORD, Team > teamManagerMap;
extern int MAXPLAYERLEVEL;

#define PLAYER_DATA_VERSION		(1)
#define MAX_DISWAITTIME		    (5 * 180)			// 3分钟
#define MAX_DEADWAITTIME		(5 * 180)			// 3分钟
#define MAX_PKTIME              (5 * 150)
#define DEAD_CONSTANT_SUBTRACT  (1)
#define LOGINCHECKTIME		30 * 1000			//30秒

#define AUTOSAVETIME            (1000 * 60 * 10)

#define AUTO_PRISON_ID          (100)

#define DOCTORBUFFTIME          ( 20 )
#define WHINEBUFFTIME			( 48 )
#define MAXPOINT_PER_BUFF       ( 100 )

#define ADD_TELERGY_STEP		6	//心法增长步长


const int CONSUME_TP_JUMP =		15;	// JUMP消耗TP值

extern void UpdatePlayer(Team &teamInfo, int nNum, CPlayer *pPlayer);
extern void RefreshTeam(Team &team);
extern void OnRecvJoinMsg(DNID dnidClient, SQTeamJoinMsgG *pMsg, CPlayer *pPlayer, bool Auto = false);

extern BOOL SendRefreshPlayerMessage(int storeflag, DWORD gid, LPCSTR acc, SFixData *pData, SPlayerTempData *pTempData);
extern BOOL PutPlayerIntoDestRegion(CPlayer *pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0);
extern LPIObject GetPlayerBySID(DWORD dwStaticID);

extern BOOL ClearClient(DNID);
extern BOOL GetFactionInfo( LPCSTR szFactionName, SFactionData& stFactionInfo );
extern void RemoveFromCache(LPCSTR account);
//extern int CheckBindEquip( CPlayer *_this, SEquipment *equip, BOOL isTakl = TRUE, DWORD curtime = time( NULL ) );
BOOL   isLimitWG = FALSE;
extern BOOL wgLimit;
extern DWORD g_AccOnlineCheck[6];
#undef  TimeCompress
// 师徒系统占用的任务号
DWORD g_masterTask = 11;
DWORD g_prenticeTask = 12;
DWORD g_mastercount = 13;

// 心法的第一个ID号
const DWORD FirstXinFaID = 6000;
const DWORD CurXinFaNum	 = 12;

// 洗点的道具ID
const DWORD dwXiSuiDanID	= 4010009;

// 宠物ID管理
std::vector<DWORD>	reUseID;
std::set<DWORD>		inUseID;

TreasureShop CPlayer::tsSingle;

extern BOOL DropToGround( SRawItemBuffer &itemDrop, DWORD protectedGID, DWORD protectedTeamID, POINT pos, CRegion *region, DWORD &dropCount );

// 取得指定日期在本年的第几周
// 如果跨年,为负数,到本年第几
//  周的一个负数
int DataOfWeek( time_t date )
{
	tm *t = localtime( &date );
	if ( t == NULL )
		return 0;

	int temp = 0;
	if ( t->tm_yday < t->tm_wday )
		temp = 0;
	else 
	{
		temp = t->tm_yday / 7;
		if ( ( t->tm_yday % 7 ) >= t->tm_wday )
			temp++;
	}
	return temp;
}

bool FloatComPar(float a,float b)
{
	return  abs(a - b) > 0.01f;
}

//
CPlayer::CPlayer(void)
{
	memset(&m_PetFightPro, 0, sizeof(FightProperty));	//保存玩家变身前的玩家战斗数据
	m_RecordPreTransferTime = 0;
	m_RecordPreDeadCTime = 0;
	m_SysAutoTransToPlayer = FALSE;
	m_PlayerFightPower = GetPlayerFightPower();	//获取玩家战斗力

	m_SecondOnceAddTp = TRUE;
	m_dwLoginTime = 0;
	m_IsUseKillMonster = 0;		//不启用玩家移动杀怪
	m_EntryScenceTime = 0; 
	m_pPreFightRegion = NULL;
	m_iCurerGID = 0;        // 如果不在构造函数里初始化，可能会比较不安全。
	m_dwCureDAMTime = 0;
	m_dwCurePOWTime = 0;
	m_dwCureDEFTime = 0;
	m_dwCureAGITime = 0;
	m_bIsTalkGM		= FALSE;
	m_SportsRoomID  = 0;
	m_CurFollowIndex = 0;
	m_pItemXQD = NULL;
	m_pItemXTBL = NULL;
	m_HoleType = 0;
	ZeroMemory( &m_GMIDCheckInf, sizeof(GMIDCheckInf) );
	m_MetrialItemID = 0;
	m_dwDecDurTime = 0;
	m_bIsStarPractice	=	false;

	m_nCopySceneStata = COPYSCENS_NONE;
	mLockEvent->SetPlayer(this);
	m_iPassVenationCount = 0;

	//////////////////////////////////////////////////////////////////////////
	m_checkTick			= timeGetTime();
	m_pAttribListener	= 0;
	m_nCollectTimeLen	= 0;
	m_nCollectNpcId		= 0;
	m_RidingStatus		= 0;
	m_PetStatus			= 0;
	m_PetGID			= 0;
	m_FightPetTimeLimit = m_checkTick;
	m_SendRequestID		= 0;
	m_PlayerAttri		= 0;
	m_pCheckItem		= 0;
	m_pSingleItem = 0;
	m_pFightPet			= 0;
	//m_forgeItem			= 0;
	m_forgeMetrial		= 0;
	m_forgeType			= SQStartForge::ASF_NONE;

	m_UpdateItem		= 0;
	m_XMTJ_Metrial		= 0;
	m_LJS_Metrial		= 0;
	m_UpdateItemType	= SQDragUpdateItemMsg::DUI_NONE;

	m_MaxUpdateTimes	= 0;
	m_MaxUseTimes		= 0;

	m_MakeHoleTime				= -1;
	m_curSaledItemCount			= 0;
	m_IsPlayerUpdated			= false;
	m_PreFPIndex				= 0xff;
	m_nPkRule					= 0;		// PK模式：和平模式
	m_nChagePkRuleTime			= 0;
	m_NineWordsKill				= 0;		// 九字杀阵初始化

	m_updateSkill				= false;
	m_updateTelergy				= false;
	m_bKrlinArmDamge			= false;	// 麒麟臂标记

	m_InRidingevens = 0;		//在骑马读条的状态
	m_InUseItemevens = 0; 		//在道具读条的状态
	// 装备强化
	//m_strengthenEquip		= 0;		// 强化装备
	m_strengthenMetrial		= 0;		// 强化原材料
	m_vecStrengthenEquip.clear();
	m_vecStrengthenMetrial.clear();

	_time64(&m_dwOnlineBeginTime);			//在线时间，用于送新生礼包
	m_PartDyRegionID = 0;
	m_PartDyRegionGID = 0;	
	m_refineType			= SQAddEquip::ADE_NONE;

	m_bFashionMode = 0;
	m_bFirstLogIn = false;
	m_bTempLevel = 0;

	m_bStrShowStep = 0 ; // 表示提示已经进行到哪个阶段了!
	m_PlayerOpenClose = 0;
	// ===========套装属性加成===========
	m_ModifyTypeMark	= 0;						// 需要修改的套装属性类型标记	
	memset(m_SaveTheSuitAttriValue, 0, sizeof(m_SaveTheSuitAttriValue));
	// ================================
	m_ClickTaskFlag = 0;
	// 洗点材料初始化
	//m_resetPointMetrial		= 0;
	m_pIdentifyEquipBackUpNeedDel = 0;
	m_DynamicRegionState = 0;
	memset(m_MountAddFactor,		0, sizeof(m_MountAddFactor));
	memset(m_JingMaiFactor,			0, sizeof(m_JingMaiFactor));
	memset(m_buyBackItem,			0, sizeof(m_buyBackItem));
	memset(m_PlayerPropertyStatus,  0, sizeof(m_PlayerPropertyStatus));
	memset(m_TelergyFactor,			0, sizeof(m_TelergyFactor));
	memset(m_TelergyActiveRate,		0, sizeof(m_TelergyActiveRate));
	memset(m_VIPFactor, 0, sizeof(m_VIPFactor));

	// 排行榜数据初始化
	memset(m_RankNum,				0, sizeof(m_RankNum));	// 我的排名
	memset(m_RankList,				0, sizeof(Rank4Client) * MAX_RANKLIST_NUM * RT_MAX);
	//IsBeProcessed	= false;		// 表示未初始化处理过排行榜信息

	memset(m_newAddproperty, 0, sizeof(WORD)* 20);
	for (size_t i = 0; i < SEquipDataEx::EEA_SHANBI; i++)
	{
		m_AttriRefence[XA_UNCRIT + i] = &m_newAddproperty[i];
	}
	m_AttriRefence[XA_HIT] = &m_Hit;

	// 设置内部引用，用于加速属性的更新
	for (int i=XA_MAX_EXP; i<XA_MAX; i++)
	{
		switch (i)
		{
		case XA_MAX_EXP:			// 最大经验
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_MaxExp;
			break;
		case XA_CUR_EXP:			// 当前经验
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_Exp;
			break;
		case XA_MAX_JP:				// 最大精力
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_MaxJp;
			break;
		case XA_CUR_JP:				// 当前精力
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_CurJp;
			break;
		case XA_MAX_SP:				// 最大真气
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_MaxSp;
			break;
		case XA_CUR_SP:				// 当前真气
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_CurSp;
			break;
		case XA_BIND_MONEY:			// 绑定货币
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_BindMoney;
			break;
		case XA_UNBIND_MONEY:		// 非绑定货币
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_Money;
			break;
		case XA_ZENGBAO:			// 赠宝
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_ZengBao;
			break;
		case XA_YUANBAO:			// 元宝
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_YuanBao;
			break;
		case XA_TF_JINGGONG:		// 进攻
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_JingGong;
			break;
		case XA_TF_FANGYU:			// 防御
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_FangYu;
			break;
		case XA_TF_QINGSHEN:		// 轻身
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_QingShen;
			break;
		case XA_TF_JIANSHEN:		// 健身
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_JianShen;
			break;
		case XA_REMAINPOINT:		// 剩余点数
			m_PlayerAttriRefence[i-XA_MAX_EXP] = &m_Property.m_RemainPoint;
			break;
		case XA_FIGHTPOWER:			//战斗力
			m_PlayerAttriRefence[i - XA_MAX_EXP] = &m_PlayerFightPower;
			break;
		default:
			MY_ASSERT(0);
		}
	}
	
}


CPlayer::~CPlayer() 
{
	SAFE_DELETE(m_pIdentifyEquipBackUpNeedDel);	
}

//----------------------------------------------------------------------------------------------------------	
int CPlayer::OnCreate(_W64 long pParameter)
{
	// 在这里请求数据库数据，填充该玩家的角色数据
	SParameter *pParam = (SParameter*)pParameter;

	ZeroMemory( &m_stFacRight, sizeof( m_stFacRight ) );

	//	m_isLockedTarget = false;

	if (pParam == NULL) return false;

	if (pParam->dwVersion != PLAYER_DATA_VERSION) return false;

	m_ClientIndex = pParam->dnidClient;

	m_PrevTrapID = 0;

	m_dwTeamID = 0;

	m_bInit = false;

	m_DisWaitTime = -1;

	ZeroMemory(&m_CountData, sizeof(_PLAYERDATA));
	m_bCountFlag = FALSE;
	m_wScriptState = 0;
	m_wScriptIcon = 0;

	m_dwUseTelergyVal = 0;

	m_OnlineState = OST_NOT_INIT;

	m_wScapegoatID  = 0;
	m_wGoatEffectID = 0;
	m_wMutateID     = 0;
	m_wMonsterCountOnGoat = 0;
	m_dwAutoAddMCountTime = 0;

	m_stAutoUseItemSet.bSetQXD = true;
	m_stAutoUseItemSet.bSetLZ  = true;

	m_ProRefTime = 0;  
	m_bProtect = false;
	m_bIsTeamLeader = false;
	m_dwFactionTitleID = 0;

	m_wTeamMemberCount = 0;

	//m_qwMoneyPointTime = 0;

	selected_item_x = selected_item_y = 8;

	ATM_temporaryVerifier = 0;
	temporaryVerifier = 0;
	buildVerify = 0;
	isErrPlayer = 0;


	dialEndIndex = 0;

	for ( int i = 0; i < 6; i++)
		m_wWhineBUFFData[i] = 0;

	m_tempItem.clear();
	timeOutChk = 0;
	isAutoFight = false;
	isUseRose = false;

	for ( int i = 0; i < 6; ++i )
		m_pItemLZ[i] = NULL;

	ZeroMemory( m_nLZEffectVal, sizeof(m_nLZEffectVal) );

	m_dUpXMValTimes          = 0;
	m_bHaveXMBuff            = false;
	m_dwLookGID      = 0;

	m_dwLookStartTime = 0;
	m_dwLookDisable = 0;

	m_dwLastHPResTime = 0;
	m_dwLastMPResTime = 0;
	m_dwLastTPResTime = 0;

	m_dwLastSPResTime=0;

	m_StartJumpTime	  = 0;

	m_nOnlineFriendTime = 0;
	m_nOnlineFriendExp = 0 ;
	m_nOnlineExpCount = 0 ;
	m_nOnlineFriendLastExp = 0 ;

	///正在装备道具的侠客索引
	m_nEquipfpIndex = -1;



	//改变名字颜色计时
	m_dChangeColorTime = 0xffffffff;
	m_bNameColor = SAPNameColorChangeMsg::COLOR_NORMAL;

	m_nImpelFightPetTime = timeGetTime();	// 激励值获取时间
	for(int i= 0;i<MAX_FIGHTPET_NUM;i++)    // 侠客回复疲劳时间
	{
		m_nRestoreTiredTime[i] = timeGetTime();
	}

	m_nfpLearnSkillTime = 0xffffffff;//侠客开始学习技能的时间
	m_CountTA = 0;

	m_bCurDeadreginID = 0;
	m_wCurDeadCount = 0;
	m_dUpdateState = 0xffffffff;

	wReliveTime[0] = 300;
	wReliveTime[1] = 180;
	wReliveTime[2] = 0;
	wReliveTime[3] = 0;

	wReliveMoney[0] = 0;
	wReliveMoney[1] = 0;
	wReliveMoney[2] = 2000;
	wReliveMoney[3] = 3000;

	m_dLoginCheckTime = GetTickCount();

	//m_fpAtkType = FP_FOLLOW;
	return CItemUser::OnCreate( pParameter );
}

void ImmOverlapItem(SPackageItem *items, CPlayer *_this = NULL)
{
	// 	// 对老单个道具和新 多个道具的合并支持 仓库部分
	// 	for ( int i=0; i<64; i++ )
	// 	{
	// 		if ( ( items[i].size >= sizeof( SItemBase ) ) && ( items[i].wIndex != 0 ) )
	// 		{
	// 			const SItemData *itemData = CItem::GetItemData( items[i].wIndex );
	// 			if ( itemData && ITEM_IS_OVERLAP( itemData->byType ) )
	// 			{
	// 				SOverlap &overlap = reinterpret_cast< SOverlap& >( static_cast< SItemBase& >( items[i] ) );
	// 				if ( overlap.size == sizeof( SItemBase ) )
	// 				{
	// 					overlap.number = 1;
	// 					overlap.size = sizeof( SOverlap );
	// 				}
	// 			}
	// 		}
	// 	}
}
//----------------------------------------------------------------------
bool CPlayer::GetTelergyLevelEx(int index,int* level)
{
	//*level = m_Property.m_Telergy[ index ].byTelergyLevel;

	return true;
}

bool CPlayer::SetTelergyLevel(int index, int level)
{
	// 	if (m_Property.m_Telergy[index].byTelergyLevel == level)
	// 		return false;
	// 
	// 	int levelAdd = level - (int)m_Property.m_Telergy[index].byTelergyLevel;
	// 	if (levelAdd == 0)
	// 		return false;
	// 
	// 	WORD telergyID = m_Property.m_Telergy[index].wTelergyID;
	// 	TelergyData* pTelergy = g_pTelergyData->GetTelergy(telergyID);
	// 	for (int i = 0; i < TelergyDataXiaYiShiJie::MAX_TELERGY_XIA_YI_SHI_JIE; ++i)
	// 	{
	// 		if (m_Property.m_Telergy[index].bStateTel == STelergy::STATETEL_EQUI)
	// 		{
	// 			m_TelergyAddFactor[i] += pTelergy->m_data[i] * levelAdd;
	// 		}
	// 	}
	// 	m_Property.m_Telergy[index].byTelergyLevel = level;
	// 	SendStatePanelDataMsg();
	return true;
}

//----------------------------------------------------------------------
bool CPlayer::InitAddTelergyAddFactorTbl()
{
	// 	memset(m_TelergyAddFactor, 0, sizeof(m_TelergyAddFactor));
	// 	for (int i = 0; i < MAX_EQUIPTELERGY; i++)
	// 	{
	// 		if (m_Property.m_Telergy[i].byTelergyLevel && m_Property.m_Telergy[i].bStateTel == STelergy::STATETEL::STATETEL_EQUI)
	// 		{
	// 			TelergyData* pTelergy = g_pTelergyData->GetTelergy(m_Property.m_Telergy[i].wTelergyID);
	// 			if (pTelergy == NULL)
	// 				continue;
	// 
	// 			INT32 AddLevel = m_Property.m_Telergy[i].byTelergyLevel;
	// 			for (int j = 0; j < TelergyDataXiaYiShiJie::MAX_TELERGY_XIA_YI_SHI_JIE; j++)
	// 			{
	// 				TelergyDataXiaYiShiJie* data = (TelergyDataXiaYiShiJie*)pTelergy;
	// 				m_TelergyAddFactor[j] += data->m_data[j] * AddLevel;
	// 			}
	// 		}
	// 	}

	return true;
}

bool CPlayer::InitVenaAddFactors()
{
	// 	memset(m_VenaAddFactor, 0, sizeof(m_VenaAddFactor));
	// 	for (int i = 0; i < 9; ++i)
	// 	{
	// 		DWORD dwVenaState = m_Property.m_dwVenapointState[i];
	// 		int nVenaCount = (int)g_pVenapointData->GetVenapointCount((BYTE)i);
	// 		for (int v = 0; v < nVenaCount; ++v)
	// 		{
	// 			SVenapointData* pVenaData = g_pVenapointData->GetVenapoint(i, v);
	// 			if (dwVenaState & (0x1 << v))
	// 				AddValueByVenaData(pVenaData);
	// 		}
	// 
	// 		if (IsPassVenation(i))
	// 		{
	// 			SVenapointData* pVenaData = g_pVenapointData->GetVenapoint(i, 31);
	// 			AddValueByVenaData(pVenaData);
	// 		}
	// 	}

	return true;
}

BOOL CPlayer::SetFixData(SFixData *pData)
{
	memset(&m_FixData, 0, sizeof(m_FixData));
	memcpy(&m_FixData, pData, sizeof(SFixData));
	m_bInit = true;
	string accountStr(GetAccount());

	if (accountStr.substr(0, 2) == "gm")
		CSanguoPlayer::SetGMFlag(true);

	CSanguoPlayer::SetSanguoPlayerDNID(m_ClientIndex);
	
	CSanguoPlayer::SetCplayerPtr(this);
	CSanguoPlayer::SetSanguoPlayerData(&m_FixData);
	// 填充打印标志
	if (g_Count.DelPlayerFromList(GetName(), false))
		m_bCountFlag = TRUE;

	// 绑定一些查询用数据
	BindNameRelation(m_FixData.m_Name, self.lock());
	BindSIDRelation(m_FixData.m_dwStaticID, self.lock());
	InitLuaData();
	//OnInitFriendData(this);

	m_dwSecondTick = timeGetTime();     // 初始化时间间隔 1秒
	m_dwMountTimer = timeGetTime();     // 初始化时间间隔 10秒
	m_dwMinuteTick = (DWORD)time(NULL) / 60; // 初始化分钟间隔
	m_dwDecDurTime = timeGetTime();			// 5分钟间隔

	return TRUE;
}

BOOL CPlayer::SetFixProperty(SFixProperty *pData)
{

	QWORD puid = m_Property.puid;
	memset(&m_Property, 0, sizeof(m_Property));
	memcpy(&m_Property, pData, sizeof(SFixProperty));
	m_Property.puid = puid;
	{
		WORD taskID = 1;
		if ( m_Property.IsComplete(taskID) == 0 )
		{
			for (int i=0; i<sizeof(m_Property.m_PlayerTask)/sizeof(SPlayerTask); ++i)
			{
				if (m_Property.m_PlayerTask[i].m_TaskID == 1)
				{
					for (int j = 0; j < m_Property.m_PlayerTask[i].m_FlagCount; ++j)
					{
						if ( m_Property.m_PlayerTask[i].m_Flags[j].m_Complete == 0 && m_Property.m_PlayerTask[i].m_Flags[j].m_Type == 100)
						{
							m_Property.m_PlayerTask[i].m_Flags[j].m_Complete = 1;
							//m_Property.m_PlayerTask[i].m_CreateTime = 0xffffffff;
							break;
						}
					}
					break;
				}
			}
		}

	}


	// 侠义道3
	m_curX		= m_Property.m_X;
	m_curY		= m_Property.m_Y;
	m_curZ		= m_Property.m_Z;
	m_Direction	= m_Property.m_dir;
	m_curTileX	= (DWORD)(m_Property.m_X) >> TILE_BITW;
	m_curTileY	= (DWORD)(m_Property.m_Y) >> TILE_BITH;

	m_DeadWaitTime	= MAX_DEADWAITTIME;
	m_DeadTime		= 0;

	// 填充打印标志
	if (g_Count.DelPlayerFromList(GetName(), false))
		m_bCountFlag = TRUE;

	// 绑定一些查询用数据
	BindNameRelation(GetName(), self.lock());
	BindSIDRelation(GetSID(), self.lock());

	if (0 == m_dwSaveTime)
		m_dwSaveTime = timeGetTime() + AUTOSAVETIME;    // 初始化时间间隔

	// 这一部分是直接被战斗影响的数据
	m_PlayerAttri = CPlayerService::GetInstance().GetPlayerUpdateData(m_Property.m_School,m_Property.m_Level);
	if (!m_PlayerAttri)
		return FALSE;

	// 恢复当前血量（和战斗相关的）
	m_CurHp = m_Property.m_CurHp;
	m_CurMp = m_Property.m_CurMp;
	m_CurTp = m_Property.m_CurTp;
	m_Level = m_Property.m_Level;

	//恢复PK值
	m_wPKValue = m_Property.m_PKValue;
	//应为玩家名字颜色不只是和pk挂钩，所以先放开
	//if (m_wPKValue)
	//{
	m_bNameColor = GetPlayerNameColor();
	m_dChangeColorTime = timeGetTime();
	SendPlayerNameColor();
	//}

	//配合共享内存恢复玩家死亡的状态，不再使用断言，改为自动复活
/*	if (m_CurHp == 0) Relive();*/
	//MY_ASSERT(m_CurHp);
	

	//m_HpRSInterval	 = m_PlayerAttri->m_HpRecoverInterval;
	//m_MpRSInterval	 = m_PlayerAttri->m_MpRecoverInterval;
	m_TpRSInterval	 = m_PlayerAttri->m_TpRecoverInterval;
// 	m_HpRecoverSpeed = m_PlayerAttri->m_HpRecoverSpeed;
// 	m_MpRecoverSpeed = m_PlayerAttri->m_MpRecoverSpeed;
	//m_TpRecoverSpeed = m_PlayerAttri->m_TpRecoverSpeed;
	m_Hit = m_PlayerAttri->m_Hit;

	UpdateBaseProperties();
	
	// 获得当前最大经验值
	m_MaxExp = m_PlayerAttri->m_Exp;
	m_MaxJp  = m_PlayerAttri->m_BaseJp;
	m_MaxSp	 = 0;
	m_PlayerPropertyStatus[XA_MAX_EXP-XA_MAX_EXP] =
		m_PlayerPropertyStatus[XA_MAX_JP -XA_MAX_EXP] =
		m_PlayerPropertyStatus[XA_MAX_SP -XA_MAX_EXP] = true;

	// 恢复任务数据（但不同步）
	RecoverTask();

	if (m_Property.m_SPlayerLimtTime.flag == 0)//创建账号的时候需要用
	{	
		std::string currtimestr;
		GetSysTimeTick(currtimestr);
		memcpy(m_Property.m_SPlayerLimtTime.m_CompleteTime,currtimestr.c_str(),30);
		m_Property.m_SPlayerLimtTime.flag = 1;
	}
	else
	{
		std::string currtimestr1;
		GetSysTimeTick(currtimestr1);
		std::string currtimestr = m_Property.m_SPlayerLimtTime.m_CompleteTime;
		if ( IsCompleteDayTime(currtimestr) )//大于他那么
		{
			//m_Property.m_SPlayerLimtTime.UpdataLimtTimeData();

		}
		memcpy(m_Property.m_SPlayerLimtTime.m_CompleteTime,currtimestr1.c_str(),30);
	}
	// 恢复道具数据（但不同步）
	SetPlayerInitItems(this, true);

	// 恢复装备数据（但不同步）
	InitEquipmentData();
	
	// 恢复经脉数据（但不同步）
	InitJingMaiData();


	//加载称号数据
	InitTitleData();

	//add by ly 2014/5/16 初始化VIP附加数据
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("InitVipData"))();
	g_Script.CleanCondition();
	
	// 恢复骑乘数据（但不同步）
	SPlayerMounts::Mounts *pM = GetMounts(GetEquipMount());
	if (pM)
		ChangeMountFactor(pM, 1);

	// 填充好友的数据
//	SetPlayerRelations_new(&m_Property);

	//填充送花记录
//	SetPlayerRoseRecod(&m_Property);


	if (m_Property.m_bStopTime == 0)
	{
		m_Property.m_FightPetActived = 0xff;	//玩家是否在变身宠物形态下
		m_Property.m_TransPetIndex = 0xff;	//默认玩家没有变身宠物
		m_Property.m_FollowPetIndex = 0xff;	//默认玩家没有随身的宠物
	}
	m_dwSecondTick = timeGetTime();     // 初始化时间间隔
	m_dwMountTimer = timeGetTime();     // 初始化时间间隔
	m_dwMinuteTick = (DWORD)time( NULL ) / 60; // 初始化分钟间隔

	m_bInit = true;


	//////////////////////////////////////////////////////////////////////////
	// [2012-7-26 15-19 gw: +为初始的装备和包裹中的物品添加一个全局唯一ID]
	{
		int iLen = sizeof(m_Property.m_Equip) / sizeof(m_Property.m_Equip[0]);
		int i = 0;
		for (i = 0; i < iLen; ++i)
		{
			if (0 == m_Property.m_Equip[i].wIndex)
				continue;
			if (0 == m_Property.m_Equip[i].uniID)
			{
				extern BOOL GenerateNewUniqueId(SItemBase &item);
				// 说明是数据库服务器默认产生的，那么在这里产生一个UniqueID
				if (!GenerateNewUniqueId(m_Property.m_Equip[i]))
					return FALSE;
			}
		}

		iLen = sizeof(m_PackageItems) / sizeof(m_PackageItems[0]);
		for (i = 0; i < iLen; ++i)
		{
			if (0 == m_PackageItems[i].m_Item.wIndex)
				continue;
			if (0 == m_PackageItems[i].m_Item.uniID)
			{
				extern BOOL GenerateNewUniqueId(SItemBase &item);
				// 说明是数据库服务器默认产生的，那么在这里产生一个UniqueID
				if (!GenerateNewUniqueId(m_PackageItems[i].m_Item))
					return FALSE;
			}
		}		
	}
	InitLuaData();
	if (m_CurHp == 0)
	{
		Relive();
	}
	return TRUE;
}

void CPlayer::RecoverTask()
{
	// 任务的填充
	m_KillTask.clear();
	m_ItemTask.clear();
	m_UseItemTask.clear();
	m_GiveItemTask.clear();
	m_GetItemTask.clear();
	m_MapTask.clear();
	m_SceneTask.clear();
	m_TaskInfo.clear();
	m_MenuInfo.clear();
	m_ItemInfo.clear();
	m_TaskManager.ClearAllRoleTask();

	for (int i=0; i<sizeof(m_Property.m_PlayerTask)/sizeof(SPlayerTask); ++i)
	{
		if (m_Property.m_PlayerTask[i].m_TaskID > 0)
		{
			CRoleTask task;
			task.m_Index		= m_Property.m_PlayerTask[i].m_TaskID;
			task.m_CreateTime	= m_Property.m_PlayerTask[i].m_CreateTime;

			for (int j=0; j<m_Property.m_PlayerTask[i].m_FlagCount; ++j)
			{
				// 根据旗标的类型，初始化相应的数据结构
				switch (m_Property.m_PlayerTask[i].m_Flags[j].m_Type)
				{
				case TT_ITEM:
					m_ItemTask[m_Property.m_PlayerTask[i].m_Flags[j].m_TaskDetail.Item.ItemID].push_back(m_Property.m_PlayerTask[i].m_Flags[j].m_Index);
					break;

				case TT_GIVE:
					// 对于永久完成的给予旗标，不加入队列
					if (2 != m_Property.m_PlayerTask[i].m_Flags[j].m_Complete)
						m_GiveItemTask[m_Property.m_PlayerTask[i].m_Flags[j].m_TaskDetail.GiveGet.ItemID].push_back(m_Property.m_PlayerTask[i].m_Flags[j].m_Index);
					break;

				case TT_GET:
					m_GetItemTask[m_Property.m_PlayerTask[i].m_Flags[j].m_TaskDetail.GiveGet.ItemID].push_back(m_Property.m_PlayerTask[i].m_Flags[j].m_Index);
					break;

				case TT_KILL:
					m_KillTask[m_Property.m_PlayerTask[i].m_Flags[j].m_TaskDetail.Kill.MonsterID].push_back(m_Property.m_PlayerTask[i].m_Flags[j].m_Index);
					break;

				case TT_USEITEM:
					m_UseItemTask[m_Property.m_PlayerTask[i].m_Flags[j].m_TaskDetail.UseItem.ItemID].push_back(m_Property.m_PlayerTask[i].m_Flags[j].m_Index);
					break;

				case TT_REGION:
					m_MapTask[m_Property.m_PlayerTask[i].m_Flags[j].m_TaskDetail.Region.DestMapID].push_back(m_Property.m_PlayerTask[i].m_Flags[j].m_Index);
					break;

				case TT_SCENE:
					m_SceneTask[m_Property.m_PlayerTask[i].m_Flags[j].m_TaskDetail.Scene.DestSceneID].push_back(m_Property.m_PlayerTask[i].m_Flags[j].m_Index);
					break;

				case TT_ADD:
					m_AddTask[m_Property.m_PlayerTask[i].m_Flags[j].m_TaskDetail.TaskADD.btype].push_back(m_Property.m_PlayerTask[i].m_Flags[j].m_Index);
					break;


				}

				task.m_flagList.push_back(m_Property.m_PlayerTask[i].m_Flags[j]);
			}
			m_TaskManager.AddRoleTask(&task);
		}
	}
}

void CPlayer::ReleaseMountSkillCD()
{
	std::map<int,EventMgr::EventBase*>::iterator i = m_MountSkillColdTimeTbl.begin();
	for(i; i!=m_MountSkillColdTimeTbl.end(); i++)
	{
		i->second->Reset( );	//中断计时,从事件管理器中退出. 防止出错.	
		delete (i->second);		//删除对象.
	}
	m_MountSkillColdTimeTbl.clear( );
}

void CPlayer::InitMountSkillColdTime()
{
}

BOOL CPlayer::HangupLoginInit()
{
	// 挂机时进入游戏时必要的初始化动作
	if (!this)
		return FALSE;

	// 更新分布的角色数据
	//UpdateDistributedSimplePlayer(GetName(), this);

	// 在这里恢复掉线后的Buff数据
	if (m_Property.m_BlessOnce == 1)
	{
		LockItemCellTwo(m_Property.m_BlessSPackageitem.wCellPos,false);
		OnAllNotice();
		m_Property.m_BlessOnce = 0;
	}
	if (!SendInitMsgs())
		return false;

// 	//add by ly 2014/4/28  处理玩家的每日签到操作
// 	g_Script.SetCondition(NULL, this, NULL, NULL);
// 	LuaFunctor(g_Script, FormatString("OnHandleSignInOpt"))[2]();	//玩家挂起登陆
// 	g_Script.CleanCondition();
// 
// 	//add by ly 2014/5/13 玩家登陆后发送商城信息
// 	g_Script.SetCondition(NULL, this, NULL, NULL);
// 	LuaFunctor(g_Script, FormatString("QuestShopListInfo"))();
// 	g_Script.CleanCondition();

	return true;
}

BOOL CPlayer::FirstLoginInit()
{
	if (!this)
		return FALSE;


	// 通知结义兄弟
	// 	SUnionOnlineMsg _msg;
	// 	_msg.eState = SUnionOnlineMsg::UNION_STATE_ONLINE;
	// 	dwt::strcpy(_msg.cName, GetName(), CONST_USERNAME);
	// 	GetGW()->m_UnionManager.UnionOnline(&_msg, GetSID());
	// 	GetGW()->m_UnionManager.UnionOnline(this);

	//InitBeginBuffData();	// 重新启动BUFF

	//三国仓库的初始化
	m_StroageIndex.clear();
	for (int i = 0; i < MAX_ITEM_NUM; i++)
	{
		if (m_FixData.m_ItemList[i].m_dwItemID == 0)
			break;
		SSanguoItem *tempItem = &m_FixData.m_ItemList[i];
		m_StroageIndex[tempItem->m_dwItemID] = i;
	}

	//20150126 wk 首次登陆 添加玩家lua数据初始化到lua环境,dbMgr[sid]={}
	g_Script.SetCondition(0, this, 0);
	LuaFunctor(g_Script, "OnPlayerFristLogin")[m_FixData.m_dwStaticID]();
	g_Script.CleanCondition();
	CSanguoPlayer::FristLogin();
	if (!SendInitMsgs())
		return false; 

	// 通告好友信息,上线通知
	// 	SFriendOnlineMsg msg;
	// 	msg.eState = SFriendOnlineMsg::FRIEND_STATE_ONLINE;
	// 	dwt::strcpy(msg.cName, GetName(), CONST_USERNAME);
	// 	FriendsOnline_new(&msg);
	// 通告临时好友信息,上线通知
	//FriendsOnline_new(&msg,RT_TEMP);


	//恢复侠客的装备数据,同时同步到客户端
// 	size_t index = 0;
// 	for (;index<MAX_FIGHTPET_NUM;index++)
// 	{
// 		if (0 == m_Property.m_FightPets[index].m_fpID)continue;
// 		if (_fpSetEquipFightPet(index))
// 		{
// 			_fpInitEquipmentData();
// 			_fpUpdateAllProperties();
// 			_fpInitEquipFightPetIndex();
// 		}	
// 	}

	// 登录触发控制
// 	g_Script.SetCondition(0, this, 0);
// 	extern BOOL wgLimit;
// 	if (wgLimit & 4)		// 向脚本提交外挂信息
// 		LuaFunctor(g_Script, "OnPlayerOnline")[isErrPlayer ? -1 : m_OnlineState]();
// 	else
// 		LuaFunctor(g_Script, "OnPlayerOnline")[m_OnlineState]();
// 	g_Script.CleanCondition();

	// 第一次登陆的时候设置聊天时间间隔信息
	m_dwFactionLimitTime = 
		m_dwPublicLimitTime	 = 
		m_dwSchoolLimitTime  = 
		m_dwTeamLimitTime	 = 
		m_dwRumorLimitTime	 = 
		m_dwWhisperLimitTime = 
		m_dwGlobalLimitTime = timeGetTime();

	// =====发送九字杀阵的初始化=======
	//m_NineWordsKill		= 0;

	//add by ly 2014/5/13 玩家登陆后发送商城信息
	//g_Script.SetCondition(NULL, this, NULL, NULL);
	//LuaFunctor(g_Script, FormatString("QuestShopListInfo"))();
	//g_Script.CleanCondition();

	return true;
}

BOOL CPlayer::SendInitMsgs()
{
	/*SYSTEMTIME currentTime;
	GetSystemTime(&currentTime);
	rfalse("%d秒,%d毫秒  SendToClient_time_start", currentTime.wSecond, currentTime.wMilliseconds);*/
	
	if (!m_bInit)
		return FALSE;

	///更新相关的玩家
	CSanguoPlayer::InitDataOfLogin();
	SAChDataMsg chdmsg;
	chdmsg.dwPlayerGlobalID	= GetGID();
	chdmsg.dwPlayerStaticID = m_FixData.m_dwStaticID;
	//SectionMessageManager::getInstance().sendMessageWithSection(m_ClientIndex, &chdmsg, sizeof(chdmsg));
	chdmsg.dwRegionGlobalID = 123;
	if (m_FixData.m_bNewPlayer == true)//如果是新玩家
	{
		m_FixData.m_dwZoneID = GetGroupID();//区域服id，用于充值或合服
		memset(m_FixData.m_FactionName, 0, 32 * sizeof(byte)); //TODO 为老数据做兼容，正式上线需要修改
		//m_FixData.m_dFactionID[0] = 0;//		玩家工会ID
	}
		
	chdmsg.fixData = (SSanGuoPlayerFixData)m_FixData;
	chdmsg.fixData.m_dwEnterZoneServerTime = time(NULL); ///向客户端发送服务器时间

	 CArenaMoudel::getSingleton()->GetBuyChallengeCount(m_FixData.m_dwStaticID,chdmsg.BuyChallengeCount);
	 CArenaMoudel::getSingleton()->GetReSetCountDownCount(m_FixData.m_dwStaticID, chdmsg.ReSetCountDownCount);

	if (CArenaMoudel::getSingleton()->GetLastChallengTimespan(m_FixData.m_dwStaticID, chdmsg.dwLastArenaChallengeTimeSpan) ==false)//竞技场的时间间隔
		chdmsg.dwLastArenaChallengeTimeSpan = 99999999;

	if (CArenaMoudel::getSingleton()->GetArenaChallengCount(m_FixData.m_dwStaticID, chdmsg.dwArenaChallengeCount) == false)
		chdmsg.dwArenaChallengeCount = 0;

	chdmsg.dwBestArenaRank = CArenaMoudel::getSingleton()->GetBestRank(m_FixData.m_dwStaticID);
	chdmsg.dwServerStartTime = CConfigManager::Instance()->m_ServerStartTime; //设置开服时间

	g_StoreMessage(m_ClientIndex, &chdmsg, sizeof(SAChDataMsg));


	//20150505 wk 玩家上线同步相关lua功能数据到前台
	g_Script.SetCondition(0, this, 0);
	if (m_FixData.m_bNewPlayer == true)//如果是新玩家
	{
		LuaFunctor(g_Script, "set_createTime")[m_FixData.m_dwStaticID]();
	}
	LuaFunctor(g_Script, "OnPlayerOnline")[m_FixData.m_dwStaticID]();
	g_Script.CleanCondition();

	m_FixData.m_bNewPlayer = false; ///临时处理
	 ///三国竞技场数据发送给客户端

	//if (m_ParentRegion)	// 重绑定
	//{
	//	chdmsg.wCurMapID = m_ParentRegion->m_wMapID;
	//	chdmsg.dwRegionGlobalID = m_ParentRegion->GetGID();
	//}
	//else				// 初次登陆
	//{
	//	chdmsg.wCurMapID = m_Property.m_CurRegionID;
	//
	//	CRegion *pRegion = (CRegion*)FindRegionByID(m_Property.m_CurRegionID)->DynamicCast(IID_REGION);
	//	if (pRegion)
	//		chdmsg.dwRegionGlobalID = pRegion->GetGID();
	//	else
	//		chdmsg.dwRegionGlobalID = -1;
	//}
	//
	//chdmsg.PlayerBaseData	= (SXYD3FixData)m_Property;
	//chdmsg.mountsData		= (SPlayerMounts)m_Property;
	//memcpy(chdmsg.taskInfo, m_Property.m_flags, sizeof(chdmsg.taskInfo));
	//memcpy(chdmsg.venInfo.m_JingMai, m_Property.m_JingMai, sizeof(chdmsg.venInfo));
	////闭关奖励领取时间
	//INT64 now;
	//_time64(&now);
	//chdmsg.PlayerBaseData.m_lastBiguanTime = now - m_Property.m_lastBiguanTime;
	//chdmsg.petsData = (SPlayerPets)m_Property;
	//
	/////好友数据
	//chdmsg.m_playerrelation=(SPlayerRelation)m_Property;
	//SectionMessageManager::getInstance().sendMessageWithSection(m_ClientIndex,&chdmsg,sizeof(chdmsg));
	//int msgNum = SectionMessageManager::evaluateDevidedAmount(sizeof(SAChDataMsg));
	//SSectionMsg* sectionMsgs = new SSectionMsg[msgNum];
	//SectionMessageManager::devideMessage(&chdmsg,sizeof(chdmsg),sectionMsgs,msgNum);
	//for (int i = 0;i < msgNum;i++)
	//{
	//g_StoreMessage(m_ClientIndex,&sectionMsgs[i],sizeof(SSectionMsg));
	//}
	//g_StoreMessage(m_ClientIndex, &chdmsg, sizeof(chdmsg));
	// 体质数据同步
	//SendTiZhiData();
	// 麒麟臂数据同步
	//SendKylinArmData();
	// 道具的同步
	//SetPlayerInitItems(this, false);
	// 心法存入初始化
	//InitTelergyAndSkill();

	// 同步任务
	//m_TaskManager.SendAllRoleTaskInfo(this);

	// 恢复组队（由于掉线）
	/*if (m_dwTeamID)
	{
		rfalse(2, 1, "有队伍信息哦~~~");
	}

	// 将属性同步一道
	UpdateAllProperties();
	// 同步所有的好友在线状态（包含黑名单仇人等）
	RefreshFriends_new();*/

	////召回侠客。这里可能会再玩家掉线后进行一次初始化
	//if (m_pFightPet)
	//{
	//	BYTE fpindex = m_pFightPet->m_index;
	//	CallBackFightPet(true);
	//	CallOutFightPet(fpindex);
	//}

	//登录成功后这里设置时间
	m_nchecklivetime = 0xffffffff; 

	// 从脚本中计算修为值
	//	UpdatePlayerXwzMsg();

	m_bFashionMode = m_Property.m_FashionMode;
	if (m_dwLoginTime == 0)
		_time64(&m_dwLoginTime); //当前的登陆时间。用于记录
	//add by ly 2014/4/14
	m_AddOnceTpFlag = TRUE;
	m_Property.m_LeaveTime += m_dwLoginTime - m_Property.m_dLeaveTime64;	//计算玩家不在游戏时间总和了

	/*GetSystemTime(&currentTime);
	rfalse("%d秒,%d毫秒  SendToClient_time_end", currentTime.wSecond, currentTime.wMilliseconds);*/
	return TRUE;
}

std::string GetAccountPrefix( std::string &account )
{
	int pos = (int)account.find( 30 );
	if ( pos == string::npos )
		return "";

	return account.substr( 0, pos );
}

std::string GetAccountPrefixV2( std::string &account )
{
	int pos = (int)account.find( 31 );
	if ( pos == string::npos )
		return "";

	return account.substr( 0, pos );
}

LPCSTR GetNotPrefixAccount( std::string &account )
{
	int pos = (int)account.find( 30 );
	if ( pos == string::npos )
		return account.c_str();

	return account.c_str() + pos + 1;
}

void CPlayer::SetAccount(const char * szSetAccount)
{
	m_szAccount = szSetAccount;
	m_szAccount_lv1_prefix = ::GetAccountPrefix( m_szAccount );
	m_szAccount_lv2_prefix = ::GetAccountPrefixV2( m_szAccount );
	m_szAccount_without_prefix = ::GetNotPrefixAccount( m_szAccount );
}

void CPlayer::OnClose()
{
	return;
}

void CPlayer::Backup(SFixProperty &data)
{
	MY_ASSERT(m_bInit);

	m_Property.m_dir	= m_Direction;
	m_Property.m_X		= m_curX;
	m_Property.m_Y		= m_curY;
	m_Property.m_Z		= m_curZ;

	if (m_ParentRegion)
	{
		if ((GetCurActionID() == EA_DEAD) || (m_ParentRegion->DynamicCast(IID_DYNAMICREGION)))
		{
			m_Property.m_X = m_ParentRegion->m_ptReLivePoint.x << 5;
			m_Property.m_Y = m_ParentRegion->m_ptReLivePoint.y << 5;
			m_Property.m_CurRegionID = m_ParentRegion->m_wReLiveRegionID;
		}
	}

	m_Property.m_CurHp	= m_CurHp;
	m_Property.m_CurMp	= m_CurMp;
	m_Property.m_CurTp	= m_CurTp;

	// 道具的填充
	SPackageItem *pItems = m_Property.m_BaseGoods;
	memset(pItems, 0, PackageAllCells * sizeof(SPackageItem));
	for (DWORD i = 0; i < PackageAllCells; i++, pItems++)
	{
		SPackageItem *curItem = FindItemByPos(i, XYD_FT_WHATEVER);
		if (curItem)
			*pItems = *curItem;
	}

	// 任务的填充
	SPlayerTask *pTask = m_Property.m_PlayerTask;
	memset(pTask, 0, sizeof(m_Property.m_PlayerTask));

	int TaskPos = 0;

	for (CRoleTaskManager::RTLIST::iterator it = m_TaskManager.m_taskList.begin(); it != m_TaskManager.m_taskList.end(); ++it)
	{
		pTask[TaskPos].m_TaskID		= it->m_Index;
		pTask[TaskPos].m_CreateTime = it->m_CreateTime;
		pTask[TaskPos].m_FlagCount	= it->m_flagList.size();

		if (pTask[TaskPos].m_FlagCount >TASK_MAX)
			rfalse("[严重错误]：任务%d的旗标数为0或者大于8，当前值为%d", pTask[TaskPos].m_TaskID, pTask[TaskPos].m_FlagCount);

		int FlagPos = 0;
		for (CRoleTask::TELIST::iterator ti = it->m_flagList.begin(); ti != it->m_flagList.end(); ++ti)
		{
			pTask[TaskPos].m_Flags[FlagPos++] = (*ti);
		}

		TaskPos++;
	}

	data = m_Property;

	// 暂时用m_dwPowerVal来保存GID用于角色恢复！！！
	data.m_dwPowerVal = GetGID();
}

void CPlayer::SendData(BOOL isSave)
{
	SPlayerTempData tdata;
	memset(&tdata, 0, sizeof(tdata));

	if (!m_bInit)
	{
		// 可能是异常退出的情况，由于没有初始化，所以只需要做LOGOUT
		SendRefreshPlayerMessage(SARefreshPlayerMsg::ONLY_LOGOUT, GetGID(), m_szAccount.c_str(), &m_FixData, &tdata);

		// 同时需要取消该数据
		Logout();
		return;
	}

	if (m_ParentRegion)
	{
		if (isSave == SARefreshPlayerMsg::SAVE_AND_LOGOUT)
		{
			if (m_ParentRegion)
			{
				// 在死亡或者动态场景中退出
				if (0 == m_CurHp || GetCurActionID() == EA_DEAD)
				{
					TimeToRelive(m_ParentRegion->m_wReLiveRegionID, m_ParentRegion->m_ptReLivePoint.x, m_ParentRegion->m_ptReLivePoint.y, 1);
					//m_CurHp = m_BaseMaxHp / 2;
				}
				CDynamicRegion *dyregion = (CDynamicRegion*)m_ParentRegion->DynamicCast(IID_DYNAMICREGION);
				if (dyregion)
				{
					PutPlayerIntoDestRegion(this,m_PartprevRegID,m_PartprevX, m_PartprevY, 0);
				}
				// 将自己从世界中清除
				m_ParentRegion->DelObject(this->self.lock());
			}
		}
	}

	m_Property.m_X		= m_curX;
	m_Property.m_Y		= m_curY;
	m_Property.m_dir	= m_Direction;
	m_Property.m_CurHp	= m_CurHp;
	m_Property.m_CurMp	= m_CurMp;
	m_Property.m_CurTp	= m_CurTp;
	m_Property.m_PKValue = m_wPKValue;
	m_Property.m_FashionMode = m_bFashionMode ;
	m_Property.m_bStopTime = 1;			//更新新玩家数据
	//buff 数据的记录
	//SetBuffSaveData();

	//// 玩家离线时间保存
	//m_Property.m_LeaveTime = timeGetTime();

	// 保存玩家道具
	SPackageItem *pItems = m_Property.m_BaseGoods;
	memset(pItems, 0, PackageAllCells * sizeof(SPackageItem));
	for (DWORD i = 0; i < PackageAllCells; i++, pItems++)
	{
		SPackageItem *curItem = FindItemByPos(i, XYD_FT_WHATEVER);
		if (curItem)
			*pItems = *curItem;
	}

	///保存玩家好友
//	SavePlayerRelations_new(&m_Property);

	//保存玩家送花记录
	//SavePlayerRoseRecod(&m_Property);

	// 任务的填充
	SPlayerTask *pTask = m_Property.m_PlayerTask;
	memset(pTask, 0, sizeof(m_Property.m_PlayerTask));

	int TaskPos = 0;


	for (CRoleTaskManager::RTLIST::iterator it = m_TaskManager.m_taskList.begin(); it != m_TaskManager.m_taskList.end(); ++it)
	{
		pTask[TaskPos].m_TaskID		= it->m_Index;
		pTask[TaskPos].m_CreateTime = it->m_CreateTime;
		pTask[TaskPos].m_FlagCount	= it->m_flagList.size();

		if (pTask[TaskPos].m_FlagCount > TASK_MAX )
			rfalse("[严重错误]：任务%d的旗标数为0或者大于8，当前值为%d", pTask[TaskPos].m_TaskID, pTask[TaskPos].m_FlagCount);

		int FlagPos = 0;
		for (CRoleTask::TELIST::iterator ti = it->m_flagList.begin(); ti != it->m_flagList.end(); ++ti)
		{
			if (ti->m_Type == TT_EMPTY)
			{
				if (ti->m_Complete == 0)
				{
					ti->m_Complete = 1;
				}
			}
			pTask[TaskPos].m_Flags[FlagPos++] = (*ti);
		}

		TaskPos++;
	}

	// 刷新在orb-server上的数据
	if (isSave == SARefreshPlayerMsg::ONLY_BACKUP)
	{

	}
	else
	{
		m_dwSaveTime = timeGetTime() + AUTOSAVETIME;

		if (isSave == SARefreshPlayerMsg::SAVE_AND_LOGOUT || 
			isSave == SARefreshPlayerMsg::ONLY_LOGOUT)
		{
			LogoutChecks();
		}
		else if (isSave == SARefreshPlayerMsg::BACKUP_AND_FLAG)
		{
			if (m_OnlineState != OST_HANGUP)
			{
				// 在挂机的状态下，不让玩家断线离开
				m_OnlineState = OST_LINKBREAK;
			}
		}
		else if (isSave == SARefreshPlayerMsg::SAVE_AND_HANGUP)
		{
			m_OnlineState = OST_HANGUP;
			isAutoFight   = 0;

			// clear all state
			// 			if ((GetCurActionID() == EA_ATTACK) || 
			// 				(GetBackupActionID() == EA_ATTACK) || 
			// 				bMsgComein)
			// 			{
			// 				SetCurActionID(EA_ZAZEN);
			// 
			// 				SetBackupActionID(0xff);
			// 				
			// 				bMsgComein = false;
			// 			}

			LogoutChecks();
		}
	}

	GetTempData(&tdata);
	{
		std::string currofflinetime;
		GetSysTimeTick(currofflinetime);
		memset(m_Property.m_SPlayerLimtTime.m_CompleteTime,0,30);
		memcpy(m_Property.m_SPlayerLimtTime.m_CompleteTime,currofflinetime.c_str(),currofflinetime.size());
	}

	// 保存数据
	SendRefreshPlayerMessage(isSave, GetGID(), m_szAccount.c_str(), &m_FixData, &tdata);
}

void CPlayer::SendUpdateTaskFlagMsg(STaskFlag &flag, bool updateStatus)
{
	SAUpdateTaskFlag msg;
	msg.flag = flag;

	g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));
}

BOOL CPlayer::SetTempData(SPlayerTempData *pData)
{
	if (pData == NULL)
		return FALSE;

	m_dwTeamID = pData->m_dwTeamID;


	if(0 != pData->m_dwTimeLeaving)
		m_dwHalfHourTime = timeGetTime() + pData->m_dwTimeLeaving;

	if(0 != pData->m_dwOneHourLeaving)
		m_dwOneHourTime = timeGetTime() + pData->m_dwOneHourLeaving;

	for (int i=0; i<30; i++)
	{
		if (pData->m_PkList[i].GID == 0)
			break;

		m_PkMap[pData->m_PkList[i].GID] = pData->m_PkList[i].lasttime;
	}

	// 	for (int i=0; i<100; i++)
	// 	{
	// 		if (pData->PlayerTask[i].wTaskID == 0)
	// 			break;
	// 
	// 		m_TaskList.push_back(pData->PlayerTask[i]);
	// 	}

	// m_bInit = pData->m_IsRegionChange;

	//m_dwExtraState = pData->m_dwExtraState;
	//	m_dwExtraState |= pData->m_dwExtraState & SHOWEXTRASTATE_NEWMODEL;
	//if ( m_dwExtraState & SHOWEXTRASTATE_NEWMODEL )
	//		m_wMutateID = (m_Property.m_Sex == 0 ) ? 99 : 98;
	//	else
	//		m_wMutateID    = 0;
	//m_wMutateID = pData->m_wMutateID;

	m_wScapegoatID = pData->m_wScapegoatID;
	m_wGoatEffectID= pData->m_wGoatEffectID;
	//m_wMonsterCountOnGoat = pData->m_wMonsterCountOnGoat;
	m_wGoatMutateID= pData->m_wGoatMutateID;

	if (pData->m_dwSaveTime == 0)
	{
		// 初始化时间间隔
		m_dwSaveTime = timeGetTime() + AUTOSAVETIME; 
	}
	else
	{
		// 更新时间间隔
		m_dwSaveTime = timeGetTime() + pData->m_dwSaveTime;
	}

	// 	m_stAutoUseItemSet.sSetHP = pData->m_stAutoUseItemSet[0];
	// 	m_stAutoUseItemSet.sSetMP = pData->m_stAutoUseItemSet[1];
	// 	m_stAutoUseItemSet.sSetSP = pData->m_stAutoUseItemSet[2];

	m_wCheckID = pData->m_wCheckID;

	if ((pData->m_dwTeamID > 0) && (pData->m_wTeamMemberCount > 0) && (pData->m_wTeamMemberCount < 9))
	{
		ChangeTeamSkill((BYTE)pData->m_wTeamSkillID, pData->m_wTeamMemberCount);
	}

	//m_qwMoneyPointTime = pData->m_qwMoneyPointTime;

	//	memcpy( m_Property.checkPoints, pData->checkPoints, 
	//		min( sizeof( m_Property.checkPoints ), sizeof( pData->checkPoints ) ) );

	return TRUE;
}

BOOL CPlayer::GetTempData(struct SPlayerTempData *pData)
{
	if (pData == NULL)
		return FALSE;

	memset(pData, 0, sizeof(SPlayerTempData));

	pData->m_dwTeamID = m_dwTeamID;

	pData->m_dwTimeLeaving = m_dwHalfHourTime - timeGetTime();
	pData->m_dwOneHourLeaving = m_dwOneHourTime - timeGetTime();

	{
		int i = 0;
		map<DWORD, DWORD>::iterator it = m_PkMap.begin();
		while ((it != m_PkMap.end()) && (i < 30))
		{
			pData->m_PkList[i].GID = it->first;
			pData->m_PkList[i].lasttime = it->second;

			it++;
			i++;
		}
	}

	{
		// 		int i = 0;
		// 		std::list<SRoleTask>::iterator it = m_TaskList.begin();
		// 		while ((it != m_TaskList.end()) && (i < 100))
		// 		{
		// 			if ((*it).bSave != true) 
		// 			{
		// 				pData->PlayerTask[i] = *it;
		// 				i++;
		// 			}
		// 
		// 			it++;
		// 		}
	}

	pData->m_IsRegionChange = m_bInit;

	pData->m_dwExtraState = m_dwExtraState;

	//     DWORD tempExtraState = GetSaleState();
	//     pData->m_dwExtraState &= ~(SHOWEXTRASTATE_SALE | SHOWEXTRASTATE_BUY);
	//     m_dwExtraState |= tempExtraState;

	/*  if( GetSaleState() != 0 )
	{
	pData->m_dwExtraState |= SHOWEXTRASTATE_SALE;
	}
	else
	{
	pData->m_dwExtraState &= ~SHOWEXTRASTATE_SALE;
	}*/

	pData->m_wMutateID    = m_wMutateID;
	pData->m_wScapegoatID = m_wScapegoatID;
	pData->m_wGoatEffectID= m_wGoatEffectID;
	pData->m_wMonsterCountOnGoat = m_wMonsterCountOnGoat;
	pData->m_wGoatMutateID= m_wGoatMutateID;

	pData->m_dwSaveTime = m_dwSaveTime - timeGetTime();

	// 	pData->m_stAutoUseItemSet[0] = m_stAutoUseItemSet.sSetHP;
	// 	pData->m_stAutoUseItemSet[1] = m_stAutoUseItemSet.sSetMP;
	// 	pData->m_stAutoUseItemSet[2] = m_stAutoUseItemSet.sSetSP;

	pData->m_wCheckID = m_wCheckID;
	pData->m_bQuestFullData = false;

	pData->m_wTeamMemberCount = m_wTeamMemberCount;

	//	memcpy( pData->checkPoints, m_Property.checkPoints, 
	//		min( sizeof( pData->checkPoints ), sizeof( m_Property.checkPoints ) ) );

	return TRUE;
}

void CPlayer::OnRecvMoveMsg(struct SQSynPathMsg *pMsg)
{
	if (m_StartJumpTime > 0)
	{
		//rfalse("OnRecvSynPosMsg: 大于0");
		//return;
	}
	else
	{
		//rfalse("OnRecvSynPosMsg: <=0");
	}

	if (GetGID() != pMsg->m_GID && (!m_pFightPet || m_pFightPet->GetGID() != pMsg->m_GID))
		return;

	if (pMsg->m_Path.m_wayPoints < 2)
		return;


	(GetGID() == pMsg->m_GID) ? (this->InitPlayerMove(pMsg)) : (this->m_pFightPet->InitPlayerMove(pMsg));

	if (m_followObject.size()>0)
	{
		std::map<DWORD, CFightObject*>::iterator iter = m_followObject.begin();
		while (iter != m_followObject.end())
		{
			CFightObject *pfight = iter->second;
			if (pfight&&pfight->isValid())
			{
				CMonster *pmonster = (CMonster*)pfight->DynamicCast(IID_MONSTER);
				if (pmonster)
				{
					pmonster->SendMove2TargetForAttackMsg(100, -1, this);   //告诉所有追击的怪物玩家已经改变了方向
				}
			}
			iter++;
		}
	}

	return;
}

void CPlayer::OnRecvSynPosMsg(struct SQSynPosMsg *pMsg)
{
	if (pMsg->m_GID != GetGID())
		return;
	// [2012-6-18 11-15 gw: +此处应该判断正确性]

	if (FloatComPar(m_curX, pMsg->m_X) || FloatComPar(m_curY, pMsg->m_Y))
	{
		pMsg->m_Action = EA_STAND;
		SetCurActionID((EActionState)pMsg->m_Action);
		SetBackupActionID(pMsg->m_Action);
		// 更新坐标
		m_curX		= pMsg->m_X;
		m_curY		= pMsg->m_Y;
		//	m_curZ		= pMsg->m_Z;
		m_curTileX	= (DWORD)(m_curX) >> TILE_BITW;
		m_curTileY	= (DWORD)(m_curY) >> TILE_BITH;
		//	m_Direction	= pMsg->m_Dir;

		// 将轨迹转发给有效区域的所有玩家
		CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
		if (pArea)
		{
			//////////////////////////////////////////////////////////////////////////
			// [2012-6-18 11-14 gw: +自己不需要发送]
			pArea->SendAdj(pMsg, sizeof(SASynPosMsg), m_ClientIndex);
		}
	}

		rfalse(2, 1, "同步了ID为%d的坐标：[%f, %f]", GetGID(), m_curX, m_curY);

	return;
}

void CPlayer::OnRecvJumpMsg(struct SAQSynJumpMsg *pMsg)
{
	if (pMsg->dwGlobalID != GetGID())
		return;

	if (!m_bPlayerAction[CST_CANJUMP])
		return;

	if (m_StartJumpTime)
		return;

	// 扣除跳跃消耗TP.每次跳跃消耗15
	if (m_CurTp < CONSUME_TP_JUMP)
	{
		TalkToDnid(m_ClientIndex, "体力值不足，无法跳跃！");
		return;
	}
	ConsumeTP(CONSUME_TP_JUMP);

	CancelCollect();		// 取消采集
	//BreakOperation();		// 中断操作
	m_BatchReadyEvent.BreakOperationReadyEvent(Event::EEventInterrupt_Jump);

	// 将跳跃轨迹发给有效区域的所有玩家
	CArea *pArea = (CArea *)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea)
		pArea->SendAdj(pMsg, sizeof(SAQSynJumpMsg), -1);

	m_curX = pMsg->x;
	m_curY = pMsg->y;

	m_curTileX = (DWORD)(m_curX) >> TILE_BITW;
	m_curTileY	= (DWORD)(m_curY) >> TILE_BITH;

	rfalse(2,1,"玩家跳跃坐标X%d,Y%d",m_curTileX,m_curTileY);
	m_StartJumpTime	= timeGetTime();
	m_LastingTime	= 2350;

	//rfalse(2, 1, "玩家开始跳跃咯....");

	return;
}

void CPlayer::OnRecvUnity3DJumpMsg(struct SAQUnity3DJumpMsg *pMsg)
{
	if (pMsg->dwGlobalID != GetGID())
		return;

	if (!m_bPlayerAction[CST_CANJUMP])
		return;

	if (m_StartJumpTime)
		return;

	// 扣除跳跃消耗TP.每次跳跃消耗15
	// 	if (m_CurTp < CONSUME_TP_JUMP)
	// 	{
	// 		TalkToDnid(m_ClientIndex, "体力值不足，无法跳跃！");
	// 		return;
	// 	}
	// 	ConsumeTP(CONSUME_TP_JUMP);

	CancelCollect();		// 取消采集
	//BreakOperation();		// 中断操作
	m_BatchReadyEvent.BreakOperationReadyEvent(Event::EEventInterrupt_Jump);

	// 将跳跃轨迹发给有效区域的所有玩家
	CArea *pArea = (CArea *)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea)
		pArea->SendAdj(pMsg, sizeof(SAQUnity3DJumpMsg), -1);


	//rfalse("启动unity3d中的跳跃");
	m_StartJumpTime	= timeGetTime();
	m_LastingTime	= 1350;

	//rfalse(2, 1, "玩家开始跳跃咯....");

	return;
}

void CPlayer::OnRecvSynZMsg(struct SQSynZMsg *pMsg)
{
	if (NULL == pMsg || pMsg->m_GID != GetGID())
		return;

	// 更新高度坐标
	m_curZ	= pMsg->m_Z;
	return;
}

void CPlayer::OnMoveMsg(SQSynWayTrackMsg *pMsg)
{
}

void CPlayer::OnJumpMsg(SQASynWayJumpMsg *pMsg)
{
	if ( ( isLimitWG & 0x02 ) && ( isErrPlayer == 1 ) )
	{
		//TalkToDnid( m_ClientIndex, "非法使用第三方软件, 跳跃被限制!" );
		return;
	}

	// 停止攻击锁定
	//	m_isLockedTarget = false;

	if( !CheckAction( ECA_CANJUMP ) )   
		return ;

	// 判断附加状态
	// 	if ( m_tvDizzy.Validate() )
	// 	{
	// 		SendErrorMsg( SABackMsg::B_BEDIZZY );
	// 		return;
	// 	}

	//取消采集
	if ( EA_COLLECT==GetCurActionID())
		CancelCollect();

	// 设置跳跃路径
	if ( !SetJump(pMsg) )
	{
		// 如果失败的话就停下来
		Stand();
	}
	else
	{
		//成功的话就下马
		DownMounts();
	}
}

// 通过当前身上装备的玉石的总和来计算角色的特效颜色
// 返回- 没有玉石
BYTE GetColorByBowlder( CPlayer *_this )
{
	return 0;
	//int num = 0;
	//for ( int i = 0; i < 16; i ++ )
	//{
	//	for( int j = 0; j < 5; j  ++ )
	//	{
	//		if( _this->CItemUser::m_Equip[i].slots[j].value > 1 )
	//			num += _this->CItemUser::m_Equip[i].slots[j].value;
	//	}
	//}

	//if ( num < 140 )
	//	return 0;

	//if ( num < 420 ) return 1;
	//else if ( num < 700 ) return 2;
	//else if ( num < 980 ) return 3;
	//else if ( num < 1260 ) return 4;
	//else if ( num < 1540 ) return 5;
	//else if ( num < 1682 ) return 6;

	//return 7;
}

/** 计算玩家心法等级
*/
BYTE GetMentalType( int telery )
{
	if( telery < 20 ) return 0;
	else if( telery < 40 ) return 1;
	else if( telery < 60 ) return 2;
	else if( telery < 80 ) return 3;
	else return 4;
}

SASynPlayerMsg *CPlayer::GetStateMsg()
{
	static SASynPlayerMsg msg;

	// 侠义道3 同步数据
	memcpy(msg.sName, m_Property.m_Name, CONST_USERNAME);

	msg.dwGlobalID	= GetGID();
	msg.dwShowState	= m_Property.m_ShowState;
	msg.byBRON		= m_Property.m_BRON;
	msg.bySex		= m_Property.m_Sex;
	msg.bySchool	= m_Property.m_School;
	msg.byFace		= m_Property.m_FaceID;
	msg.byHair		= m_Property.m_HairID;
	msg.MountID		= m_RidingStatus;
	msg.PetID		= m_PetStatus;
	msg.PetGlobalID	= m_PetGID;
	msg.PetTransferID = m_Property.m_NewPetData[m_Property.m_FightPetActived].m_PetID; 
	memcpy(msg.sTranPetName, m_Property.m_NewPetData[m_Property.m_FightPetActived].m_PetName, PETNAMEMAXLEN);

	msg.wEquipIndex[SASynPlayerMsg::SPT_WEAPON] = m_Property.m_Equip[EQUIP_P_WEAPON].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_WRIST]	= m_Property.m_Equip[EQUIP_P_WRIST].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_SHOSE]	= m_Property.m_Equip[EQUIP_P_SHOSE].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_CLOTH]	= m_Property.m_Equip[EQUIP_P_CLOTH].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_MANTLE]	= m_Property.m_Equip[EQUIP_P_MANTLE].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_PENDANTS1] = m_Property.m_Equip[EQUIP_P_PENDANTS1].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_PENDANTS2] = m_Property.m_Equip[EQUIP_P_PENDANTS2].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_PRECIOUS] = m_Property.m_Equip[EQUIP_P_PRECIOUS].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_HAT]	= m_Property.m_Equip[EQUIP_P_HAT].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_SHIZHUANG_HAT1]	= m_Property.m_Equip[EQUIP_P_SHIZHUANG_HAT1].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_SHIZHUANG_CLOTH1]	= m_Property.m_Equip[EQUIP_P_SHIZHUANG_CLOTH1].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_PAT] = m_Property.m_Equip[EQUIP_P_PAT].wIndex;
	msg.wEquipIndex[SASynPlayerMsg::SPT_WAIST] = m_Property.m_Equip[EQUIP_P_WAIST].wIndex;
	 

	msg.wAtkSpeed	= m_AtkSpeed;
	//msg.wAtkSpeed	= m_CurAtkSpeed;
	msg.mMoveSpeed = PLAYER_BASIC_SPEED ;
	msg.direction	= m_Direction;
	msg.bNameColor = m_bNameColor;
	msg.bFashionMode = m_bFashionMode;
	FillSynPos(&msg.ssp); 
	
	BYTE byState = 0;	//元素的状态
	BYTE byTitleID = 0;	//元素的ID
	int TitleSize = CONST_USERNAME * MAX_TITLE;
	for (int i = 0; i < TitleSize; i++)		//从第1个位置查找称号。
	{
		byState = (BYTE)(*(m_Property.m_Title[i / CONST_USERNAME] + i % CONST_USERNAME)) >> 7;	//获取元素的状态
		BYTE byTempTitleID = ((BYTE)((*(m_Property.m_Title[i / CONST_USERNAME] + i % CONST_USERNAME)) << 1)) >> 1;
		if (byState == 1)
		{
			byTitleID = byTempTitleID;
			break;
		}
	}
	msg.byCurTitle = byTitleID;

	return &msg;
}


// 获取武功熟练度
DWORD CPlayer::GetSkillProficiency( DWORD dSkillID )
{
	for (int i = 0; i < MAX_SKILLCOUNT; i++)
	{
		if (m_Property.m_pSkills[i].wTypeID == dSkillID && m_Property.m_pSkills[i].byLevel > 0)
			return m_Property.m_pSkills[i].dwProficiency;
	}
	return -1;
}

BOOL CPlayer::ActiveFly( WORD wFlyID, LPCSTR info, bool bForce )
{
	return TRUE;
}

BOOL CPlayer::ActiveProtected( WORD wProtected, LPCSTR info, bool bForce )
{
	return TRUE;
}

bool CPlayer::StartJumpBuff()
{
	return false;
}

bool CPlayer::SetTelergyState( SQSetTelergyStateMsg *pMsg )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CPlayer::OnGetPracticeRequest(SAPracRequesttMsg * pMsg)
{
}

void CPlayer::SendPracticeResult(bool bSuccess,WORD wType)
{
	SAPracAnswertMsg msg;
	msg.dwGID	=	GetGID();
	msg.bSuccess	=	bSuccess;
	g_StoreMessage( m_ClientIndex, &msg, sizeof(SAPracAnswertMsg) );
	//同步给区域内别的玩家，修炼开始
	if (bSuccess)
	{
		SAPracStarIDMsg	smsg;
		smsg.dwPlayID	=	GetGID();
		smsg.bIsStar	=	true;
		smsg.wSkillType	= wType;
		CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
		if (pArea && DynamicCast( IID_PLAYER ) )
			pArea->SendAdj(&smsg, sizeof(SAPracStarIDMsg), -1);
	}
}

bool CPlayer::CheckPlayPracticeCondition(DWORD dwGID, WORD wType,WORD wID )
{
	//保存
	// 	m_Property.lStarTime= m_lStarTime;
	// 	m_Property.bIsPractice = true;
	// 	m_Property.wPracticeType = wType;
	// 	m_Property.wPracticeID	=	wID;


	return true;
}

void CPlayer::AccountPracticeResult(WORD wType,WORD wSKillID)
{
	//计算数据
	// 	DWORD lTempTime = m_Property.lEndTime - m_Property.lStarTime;	//得到结果，单位 秒
	// 	float addPro = (float)lTempTime / (float)60 ; //得到分钟
	// 	DWORD addProficiency	=	0;
	// 	switch (wType)
	// 	{
	//   	case TYPE_COMMONSKILL:
	//   		{
	//   			//AddSkillProficiency(wSKillID,addPro);
	// 			addProficiency	=	m_dwSkillProNum;
	// 			SendSkillPro(wType,wSKillID,addProficiency);
	// 			m_dwSkillProNum= 0;
	//   		}	
	//   		break;
	// 	case TYPE_FLYSKILL:
	// 		{
	// 			
	//  			const CFlyDataTable::SFlyData* data = CFlyDataTable::GetData(wSKillID);
	// 			if (data == NULL)
	// 				return;
	// 
	//  			if (data->type == 1)	//轻功提速(每个门派第一个技能)
	//  			{
	// 				addPro *= 60;
	//  				addProficiency = (DWORD)FlyFormula::GetPracticeSpeed(data,m_wCurSkillLevel, (DWORD)addPro);
	// 				m_pFlyBuffs[data->type]->AddProficiency(addProficiency);
	// 				//AddProficiency
	//  			}
	//  			else	//非提速
	//  			{
	//  				addProficiency = m_dwSkillProTectNum ;//addProficiency = FlyFormula::GetPracriceProficiency(data,m_wCurSkillLevel,addPro);
	//  			}
	// 			//AddFly(wSKillID,addProficiency);
	// 			
	// 			//addProficiency = m_dwSkillProTectNum ;
	// 			SendSkillPro(wType,wSKillID,addProficiency);
	// 			m_dwSkillProTectNum = 0;
	// 		}
	// 		break;
	// 	case TYPE_PROSKILL:
	// 		{
	// 			//AddProtected(wSKillID,addProficiency);
	// 			//addProficiency	=	m_dwSkillProNum;
	// 			
	// 			addProficiency = m_dwSkillProTectNum;
	// 			SendSkillPro(wType,wSKillID,addProficiency);
	// 			m_dwSkillProTectNum	=	0;
	// 		}
	// 		break;
	// 	case TYPE_TELSKILL:
	// 		{
	// 			addProficiency =  m_dwSkillProNum;//DTelergyData::GetCurPractice((WORD)addPro );//时间不对
	// 			//AddTelergy(addProficiency);	//进入修炼心法的房间，自动修炼正在修炼的心法
	// 			SendSkillPro(wType,wSKillID,addProficiency);
	// 			m_dwSkillProNum	=	0;
	// 		}
	// 		break;
	// 	}
	// 	//同步给区域内别的玩家
	// 	SAPracStarIDMsg	smsg;
	// 	smsg.bIsStar	=	false;
	// 	smsg.dwPlayID	=	GetGID();
	// 	smsg.wSkillType	= 0;
	// 	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	// 	if (pArea && DynamicCast( IID_PLAYER ) )
	// 		pArea->SendAdj(&smsg, sizeof(SAPracStarIDMsg), -1);

}

// 内部测试接口，不要使用
void CPlayer::_SetPropertyTemp(int type, int data)
{
	// 	switch (type)
	// 	{
	// 	// 生命最大
	// 	case 0: m_dwMaxHP = (DWORD)data; break;
	// 	// 内力最大
	// 	case 1:	 m_wMaxMP = (WORD)data; break;
	// 	// 真气最大
	// 	case 2: m_wMaxSP = (WORD)data; break;
	// 	// 身法
	// 	case 3: m_wAgile = (WORD)data; break;
	// 	// 杀气
	// 	case 4: m_wAmuck = (WORD)data; break;
	// 	// 外攻基础
	// 	case 5: m_ADBaseValueMin[SPlayerXiaYiShiJie::OUT_ATTACK_INDEX] = (WORD)data; m_ADBaseValueMax[SPlayerXiaYiShiJie::OUT_ATTACK_INDEX] = (WORD)data; break;
	// 	// 内攻基础
	// 	case 6: m_ADBaseValueMin[SPlayerXiaYiShiJie::INNER_ATTACK_INDEX] = (WORD)data; m_ADBaseValueMax[SPlayerXiaYiShiJie::INNER_ATTACK_INDEX] = (WORD)data; break;
	// 	// 外防基础
	// 	case 7: m_ADBaseValueMin[SPlayerXiaYiShiJie::OUT_DEFENSE_INDEX] = (WORD)data; m_ADBaseValueMax[SPlayerXiaYiShiJie::OUT_DEFENSE_INDEX] = (WORD)data; break;
	// 	// 内防基础
	// 	case 8: m_ADBaseValueMin[SPlayerXiaYiShiJie::INNER_DEFENSE_INDEX] = (WORD)data; m_ADBaseValueMax[SPlayerXiaYiShiJie::INNER_DEFENSE_INDEX] = (WORD)data; break;
	// 	// 外攻精通
	// 	case 9: m_ADMasteryValueMin[SPlayerXiaYiShiJie::OUT_ATTACK_INDEX] = (WORD)data; m_ADMasteryValueMax[SPlayerXiaYiShiJie::OUT_ATTACK_INDEX] = (WORD)data; break;
	// 	// 内攻精通
	// 	case 10: m_ADMasteryValueMin[SPlayerXiaYiShiJie::INNER_ATTACK_INDEX] = (WORD)data; m_ADMasteryValueMax[SPlayerXiaYiShiJie::INNER_ATTACK_INDEX] = (WORD)data; break;
	// 	// 外防精通
	// 	case 11: m_ADMasteryValueMin[SPlayerXiaYiShiJie::OUT_DEFENSE_INDEX] = (WORD)data; m_ADMasteryValueMax[SPlayerXiaYiShiJie::OUT_DEFENSE_INDEX] = (WORD)data; break;
	// 	// 内防精通
	// 	case 12: m_ADMasteryValueMin[SPlayerXiaYiShiJie::INNER_DEFENSE_INDEX] = (WORD)data; m_ADMasteryValueMax[SPlayerXiaYiShiJie::INNER_DEFENSE_INDEX] = (WORD)data; break;
	// 	default: break;
	// 	};
}

WORD CPlayer::GetCurPracticeSkillLevel( WORD wType,WORD wID )
{
	// 	switch (wType)
	// 	{
	// 	case TYPE_COMMONSKILL:
	// 		{
	// 			for (int i = 0;i < MAX_SKILLCOUNT; ++i)
	// 			{
	// 				if (m_Property.m_pSkills[i].wTypeID == wID)
	// 				{
	// 					return m_Property.m_pSkills[i].byLevel;
	// 				}
	// 			}
	// 			return 1;
	// 		}
	// 		break;
	// 	case TYPE_FLYSKILL:
	// 		{
	// 			for (int i = 0;i < EFST_MAX; ++i)
	// 			{
	// 				if (m_Property.m_FlySkills[i].wTypeID == wID)
	// 				{
	// 					return m_Property.m_FlySkills[i].byLevel;
	// 				}
	// 			}
	// 			return 1;
	// 		}
	// 		break;
	// 	case TYPE_PROSKILL:
	// 		{
	// 			for (int i = 0;i < EPST_MAX; ++i)
	// 			{
	// 				if (m_Property.m_ProtectedSkills[i].wTypeID == wID)
	// 				{
	// 					return m_Property.m_ProtectedSkills[i].byLevel;
	// 				}
	// 			}
	// 			return 1;
	// 		}
	// 		break;
	// 	case TYPE_TELSKILL:
	// 		{
	// 			for (int i = 0;i < TELERGY_NUM; ++i )
	// 			{
	// 				if (m_Property.m_Telergy[i].wTelergyID == wID)
	// 				{
	// 					return m_Property.m_Telergy[i].byTelergyLevel;
	// 				}
	// 			}
	// 			return 1;
	// 		}
	// 		break;
	// 	}
	return 0;
}

DWORD CPlayer::GetSkillProForMP( WORD wType,WORD wSkillID,DWORD dwPro )
{
	return 1;
}

SSkill* CPlayer::GetSkillInfoforID( WORD wskillID )
{
	for (int i = 0;i < MAX_SKILLCOUNT; ++i)
	{
		if (m_Property.m_pSkills[i].byLevel > 0 && m_Property.m_pSkills[i].wTypeID == wskillID)
		{
			return &m_Property.m_pSkills[i];
		}
	}
	return NULL;
}

bool CPlayer::CheckBoxInfo( WORD wID ,DWORD wstOldBox[])
{
	for (int j = 0;j < 6; ++j)
	{
		if (wstOldBox[j] == wID)
		{
			return true;
		}
	}

	return false;
}

//保存闭关面板的格子信息
//void CPlayer::UpDatePracticeBox(SUpdatePracticeBox_C2S_MsgBody *msg)
//{
//直接加到stBox这个结构体中了
// 	for (int i = 0;i < 6;++i)
// 	{
// 		m_Property.stPracBox[ i ].wID = msg->stBox[ i ].wID;
// 		m_Property.stPracBox[ i ].wType = msg->stBox[ i ].wType;
// 	}
//}
//////////////////////////////////////////////////////////////////////////



void CPlayer::LogOutUnPractice()
{
	//离线挂机处理
	// 	SAPracRequesttMsg  pMsg;
	// 	memset(&pMsg,0,sizeof(pMsg));
	// 	pMsg.stPracValue[0].wType		=	m_Property.wPracticeType;
	// 	pMsg.stPracValue[0].wID	=	m_Property.wPracticeID;
	// 	pMsg.dwGID			=	GetGID();
	// 	pMsg.bPractice		=	false;
	// 	OnGetPracticeRequest(&pMsg);
}

void CPlayer::SendSkillPro(WORD wType,WORD wSkillID,DWORD dwPro )
{
	//告诉客户端增加了多少熟练度
	if (dwPro > 0 )
	{
		SAPracResultMsg	msg;
		msg.wType		=	wType;
		msg.wSkillID	=	wSkillID;
		msg.wResultValue=	dwPro;
		g_StoreMessage( m_ClientIndex, &msg, sizeof(SAPracResultMsg) );
	}
}

// void CPlayer::SetSkillContinue()
// {
// 	m_AtkContext.m_enableNextSkill = true;
// }

void CPlayer::SendPlayerPracticeResult()
{
	// 	if (m_Property.bIsPractice)
	// 	{
	// 		if (m_Property.wPracticeType == TYPE_FLYSKILL)
	// 		{
	// 			SQSkillFPMsg	 msg;
	// 			memset(&msg,0,sizeof(SQSkillFPMsg));
	// 			msg.byType	=	(BYTE)m_Property.wPracticeType;
	// 			msg.dwSkillIndex	=	m_Property.wPracticeID;
	// 			ProcessFly( &msg );
	// 		}
	// 		else if (m_Property.wPracticeType == TYPE_PROSKILL)
	// 		{
	// 			SQSkillFPMsg msg;
	// 			memset(&msg,0,sizeof(SQSkillFPMsg));
	// 			msg.byType	=	(BYTE)m_Property.wPracticeType;
	// 			msg.dwSkillIndex	=	m_Property.wPracticeID;
	// 			ProcessProtected(&msg);
	// 		}
	// 	}
	// 		
}

void CPlayer::UnPractice()
{
	//保存
	// 	m_bIsStarPractice = false;
	// 	m_Property.bIsPractice = false;
	// 	m_Property.wPracticeType	=	0;
	// 	m_Property.wPracticeID	=	0;
}

void CPlayer::SendSynPlayMount(WORD index, BYTE level, BYTE bAction)
{
	SAMountAction smsg;
	smsg.dwGID		= GetGID();
	smsg.bAction	= bAction;
	smsg.index		= index;
	smsg.level		= level;

	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea && DynamicCast(IID_PLAYER))
		pArea->SendAdj(&smsg, sizeof(SAMountAction), -1);
}

bool CPlayer::AddSkillProficiency(WORD wSkillID, int iAddVal, LPCSTR info)
{
	// 	if (iAddVal <= 0)
	// 		return false;
	// 
	// 	if (IsNormalSkill(wSkillID))	// 普通攻击不升级
	// 		return true;
	// 
	// 	INT32 skillIndex = GetSkillSlot(this, wSkillID);
	// 	if (skillIndex < 0)
	// 		return false;
	// 
	// 	SSkill &skill = m_Property.m_pSkills[skillIndex];
	// 
	// 	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(wSkillID, skill.byLevel);
	// 	if (!pData)
	// 		return false;
	// 
	// 	// 如果技能已满级并且熟练度已满，则不升级 
	// 	if (skill.byLevel == 8 && skill.dwProficiency >= pData->m_ShuLianDu)
	// 		return true;
	// 
	// 	if (skill.dwProficiency < pData->m_ShuLianDu)
	// 		skill.dwProficiency += iAddVal;
	// 
	// 	SASkillUpdate msg;
	// 	msg.byWhat = SASkillUpdate::SSU_UPDATE_SHULIANDU;
	// 	msg.wPos   = skillIndex;
	// 
	// 	// 首先判断是否达到了技能升级条件
	// 	bool canUpdate = false;
	// 
	// 	if (skill.byLevel < 8 && skill.dwProficiency >= pData->m_ShuLianDu)
	// 		canUpdate = true;
	// 
	// 	if (skill.byLevel < 8 && canUpdate)			// 如果熟练度够，判断心法是否OK
	// 	{
	// 		canUpdate = false;
	// 
	// 		const SSkillBaseData *pNext = CSkillService::GetInstance().GetSkillBaseData(wSkillID, skill.byLevel+1);
	// 		if (pNext)
	// 		{
	// 			WORD curLevel = GetTelergyLevel(pNext->m_XinFaID);
	// 
	// 			if (curLevel >= pNext->m_XinFaLevel)
	// 				canUpdate = true;
	// 		}
	// 	}
	// 
	// 	if (skill.byLevel < 8 && canUpdate)
	// 	{
	// 		// 升级
	// 		m_Property.m_pSkills[skillIndex].byLevel++;
	// 		msg.byWhat = SASkillUpdate::SSU_LEVELUP;
	// 
	// 		SASetEffectMsg skillUpdate;
	// 		skillUpdate.effect = SASetEffectMsg::EEFF_UPDATESKILL;
	// 		skillUpdate.data.Skill.dwGID	= GetGID();
	// 		skillUpdate.data.Skill.wID		= m_Property.m_pSkills[skillIndex].wTypeID;
	// 		skillUpdate.data.Skill.bLevel	= m_Property.m_pSkills[skillIndex].byLevel;
	// 
	// 		SendEffectiveMsg(skillUpdate);
	// 	}
	// 
	// 	memcpy(&msg.stSkill, &skill, sizeof(msg.stSkill));
	// 
	// 	g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));

	return true;
}

bool CPlayer::AddSkillProficiency(DWORD dSkillID,  int iAddVal)
{
	for (int i = 0; i < MAX_SKILLCOUNT; i++)
	{
		if (m_Property.m_pSkills[i].wTypeID == dSkillID && m_Property.m_pSkills[i].byLevel > 0)
		{
			m_Property.m_pSkills[dSkillID].dwProficiency += iAddVal;
			return true;
		}
	}
	return false;
}

bool CPlayer::SetMutate( BYTE byMutateType, WORD	wMutateID )
{
	// 判断变身条件...
	// 	if(byMutateType != 0 && byMutateType != 4 && byMutateType != 5 && !CheckAction( ECA_CANMUTATE ))
	// 	{
	// 		return false;
	// 	}
	// 
	// 	// 开始变身，并将变身消息发往客户端
	// 	switch( byMutateType )
	// 	{
	// 	case 1:
	// 		m_dwExtraState |= SHOWEXTRASTATE_ONMOSMUTATE;
	// 		break;
	// 	case 2:
	// 		m_dwExtraState |= SHOWEXTRASTATE_ONNPCMUTATE;
	// 		break;
	// 	case 3:
	// 		m_dwExtraState |= SHOWEXTRASTATE_ONGOATMUTATE;
	// 		break;
	// 	case 4:
	// 		m_dwExtraState |= SHOWEXTRASTATE_ONESPMUTATE;
	// 		break;
	// 	case 5:
	// 		m_dwExtraState |= SHOWEXTRASTATE_NEWMODEL;
	// 		break;
	// 	default:
	// 		m_dwExtraState &= ~(SHOWEXTRASTATE_ONNPCMUTATE|SHOWEXTRASTATE_ONMOSMUTATE|SHOWEXTRASTATE_ONGOATMUTATE|SHOWEXTRASTATE_ONESPMUTATE|SHOWEXTRASTATE_NEWMODEL);
	// 		break;
	// 	}
	// 	m_wMutateID = wMutateID;
	// 	AddCheckID();
	// 
	// 	SendChangeMutate( byMutateType, wMutateID );

	return  true;
}

// 设置替身状态
void    CPlayer::SetScapegoat( WORD	wScapegoatID, WORD wGoatEffID )
{
	// 判断替身条件...
	if( wScapegoatID==m_wScapegoatID && wGoatEffID==m_wGoatEffectID )  
		return;

	// 无门派装备的等级20相当于6
	if( wScapegoatID<14  )          return;
	if( wScapegoatID<100 )
	{
		wScapegoatID -= 14; 
	}

	//if( (wScapegoatID%100)<6 && (wScapegoatID%100)!=0 )    // 小于6的套装不加替身
	//    return;

	// 设置替身状态
	m_wScapegoatID = wScapegoatID;
	m_wGoatEffectID= wGoatEffID;

	// 发送替身消息
	SendScapegoat( m_wScapegoatID, m_wGoatEffectID );
}

DWORD   CPlayer::GetSpouseSID(void)
{
	// 	SRoleTask   *pTask;
	// 	pTask = FindTask( 15 );
	// 	if( pTask )
	// 	{
	// 		return  pTask->wComplete;
	// 	}

	return  0;
}
///////////////////////////////// 属性取值接口
BYTE CPlayer::GetAmuck(void)   // 获取杀气值
{   
	// 	int aucmk = 0;
	// 	if ( _EquipData.suitCheck.type == 24 && _EquipData.suitCheck.ucount == 2 )
	// 		aucmk = 12;
	// 
	// 	// 混沌灵珠 杀气 可抵消
	// 	int tmpLZEffect = 0;
	// 	if ( GetAutoUseItemSetLZ() == TRUE && m_pItemLZ[5] != NULL )
	// 	{
	// 		BYTE tempDeclare = ( (SScriptItem *)((SItemBase *)( m_pItemLZ[5])) )->declare;
	// 		if ( tempDeclare && (tempDeclare & 0x4) )  // 有修为第3位有值
	// 			tmpLZEffect = m_nLZEffectVal[5];
	// 	}
	// 
	// 	return m_tvAddAmuck.GetVal( m_wAmuckAD + m_wAmuck + equipAttribute.amuck + aucmk + m_Property.m_byValueLastAmuck + tmpLZEffect );

	return 0;
}

// 获取血内体提升速度
// WORD    CPlayer::GetAddHPSpeed(void)
// {
// 	return  ( m_wHPValOnEquipment + m_wAddHP + m_Property.m_wAddHPSpeed ) + equipAttribute.HPSpeed;
// }

// WORD    CPlayer::GetAddMPSpeed(void)
// {
// 	return  ( m_wMPValOnEquipment + m_wAddMP + m_Property.m_wAddMPSpeed ) + equipAttribute.MPSpeed;
// }

// WORD    CPlayer::GetAddSPSpeed(void)
// {
// 	return  ( m_wSPValOnEquipment + m_wAddSP + m_Property.m_wAddSPSpeed ) + equipAttribute.SPSpeed;
// }

BOOL CPlayer::ActivaTelergy(WORD wTelergyID, LPCSTR info)
{
	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (!pPlayer)
		return FALSE;

	const SXinFaData *pData = CSkillService::GetInstance().GetXinFaBaseData(wTelergyID);
	if (!pData)
		return FALSE;

	//if (IsTelegryExisted(pPlayer, wTelergyID))
	//	return TalkToDnid(pPlayer->m_ClientIndex, "您已经学会了该心法!"), FALSE;

	INT32 freeSlot = GetFreeTelergySlot(pPlayer, 0);	
	if (-1 == freeSlot || freeSlot >= MAX_SKILLCOUNT)
		return FALSE;

	pPlayer->m_Property.m_Xyd3Telergy[freeSlot].m_TelergyID		= wTelergyID;
	pPlayer->m_Property.m_Xyd3Telergy[freeSlot].m_TelergyLevel	= 1;

	SATelergyUpdate msg;

	msg.pos								= freeSlot;			
	msg.bResult							= SATelergyUpdate::SAT_LEARNED;
	msg.m_Telergy.m_TelergyID			= m_Property.m_Xyd3Telergy[freeSlot].m_TelergyID;
	msg.m_Telergy.m_TelergyLevel		= pPlayer->m_Property.m_Xyd3Telergy[freeSlot].m_TelergyLevel;

	g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));

	// 增加心法附加属性值
	switch(wTelergyID)
	{
	case FirstXinFaID:
		m_TelergyFactor[TET_ATTACK]		+= pData->m_AddAttack;
		break;
	case FirstXinFaID + 1:
		m_TelergyFactor[TET_DEFENCE]	+= pData->m_AddDefence;
		break;

	case FirstXinFaID + 2:
		m_TelergyFactor[TET_HP]			+= pData->m_AddLife;
		break;

	case FirstXinFaID + 3:
		m_TelergyFactor[TET_MP]			+= pData->m_AddInterforce;
		break;

	case FirstXinFaID + 4:
		m_TelergyFactor[TET_TP]			+= pData->m_AddPhysiforce;
		break;

	case  FirstXinFaID + 5:
		m_TelergyFactor[TET_SHANBI]		+= pData->m_AddDodgeforce;
		break;

	case FirstXinFaID + 6:
		m_TelergyFactor[TET_BAOJI]		+= pData->m_AddCriceforce;
		break;

	case FirstXinFaID + 7:
		m_TelergyFactor[TET_SUCKHP_PER] += pData->m_AddPctBloodSucker;
		m_TelergyActiveRate[0]	+= 50;
		break;

	case FirstXinFaID + 8:
		m_TelergyFactor[TET_SUCKMP_PER] += pData->m_AddPctCutInterforce;
		m_TelergyActiveRate[1]	+= 50;
		break;

	case FirstXinFaID + 9:
		m_TelergyFactor[TET_SUCKTP_PER] += pData->m_AddPctCutPhysiforce;
		m_TelergyActiveRate[2]	+= 50;
		break;

	case FirstXinFaID + 10:
		m_TelergyFactor[TET_SUCKATTK]	+= pData->m_AddPctCutAttakNum;
		m_TelergyActiveRate[3]	+= 50;
		break;

	case FirstXinFaID + 11:
		m_TelergyFactor[TET_SUCKDEF]	+= pData->m_AddPctCutDefecNum;
		m_TelergyActiveRate[4]	+= 50;
		break;

	default:
		break;
	}

	UpdateAllProperties();

	return TRUE;

	//BOOL bRet = FALSE;
	//
	//const SXinFaData *pData = CSkillService::GetInstance().GetXinFaBaseData(wTelergyID);
	//if (!pData)
	//	return bRet;

	//const std::vector<DWORD> *pSkills = CSkillService::GetInstance().GetTelegrySkillMap(wTelergyID);
	//if (!pSkills)
	//	return bRet;

	//// 先检查是否已经学会了心法
	//for (int i = 0; i < MAX_TELEGRYNUM; ++i)
	//{
	//	if (wTelergyID == m_Property.m_Xyd3Telergy[i].m_TelergyID)
	//	{
	//		MY_ASSERT(m_Property.m_Xyd3Telergy[i].m_TelergyLevel > 0 && m_Property.m_Xyd3Telergy[i].m_TelergyLevel <= MaxXinFaLevel);
	//		return TalkToDnid(m_ClientIndex, "您已经学会了该心法!"), FALSE;
	//	}
	//}

	//for (int i = 0; i < MAX_TELEGRYNUM; ++i)
	//{
	//	if (0 == m_Property.m_Xyd3Telergy[i].m_TelergyID)
	//	{
	//		MY_ASSERT(0 == m_Property.m_Xyd3Telergy[i].m_TelergyLevel);

	//		m_Property.m_Xyd3Telergy[i].m_TelergyID    = wTelergyID;
	//		m_Property.m_Xyd3Telergy[i].m_TelergyLevel = 1;
	//		SendCurTelChange(&m_Property.m_Xyd3Telergy[i], i, true);

	//		bRet = TRUE;
	//		break;
	//	}
	//}

	//// 心法学习完毕后，自动学会心法对应的技能
	//if (TRUE == bRet)
	//{
	//	for (std::vector<DWORD>::const_iterator it = pSkills->begin(); it != pSkills->end(); ++it)
	//	{
	//		MY_ASSERT(*it);

	//		// 判断技能学习条件
	//		ActivaSkill(*it, "学习心法自动学习技能！");
	//	}
	//}

	//return bRet;
}

BOOL CPlayer::SetTelergyProfic( WORD wTelergyID, DWORD dwProfic )
{
	// 	for( int i = 0; i < MAX_EQUIPTELERGY;i++ )
	// 	{
	// 		STelergy &t = m_Property.m_Telergy[ i ];
	// 		if ( t.wTelergyID == wTelergyID && t.byTelergyLevel != 0 )
	// 		{
	// 			m_Property.m_Telergy[ i ].dwTelergyVal = dwProfic;
	// 			SendTelergyChange();
	// 			return  TRUE;
	// 		}
	// 	}
	return  FALSE;
}

BOOL CPlayer::SetTelergyProficM(WORD wTelergyID, DWORD dwProfic, LPCSTR info )
{
	return  FALSE;
}

// SSkill* CPlayer::GetFlySkillById( int id, EFlySKillType& type )
// {
// 	for ( int i=0; i< EFST_MAX; ++i)
// 	{
// 		if ( id == m_Property.m_FlySkills[i].wTypeID )
// 		{
// 			type = (EFlySKillType)i;
// 			return &m_Property.m_FlySkills[i];
// 		}
// 	}
// 
// 	return 0;
// }

// SSkill* CPlayer::GetProtectSkillById( int id, EProtectedSkillType& type )
// {
// 	for ( int i=0; i< EPST_MAX; ++i)
// 	{
// 		if ( id == m_Property.m_ProtectedSkills[i].wTypeID )
// 		{
// 			type = (EProtectedSkillType)i;
// 			return &m_Property.m_ProtectedSkills[i];
// 		}
// 	}
// 	return 0;
// }

BOOL CPlayer::AddFly( int id, int iAddVal, LPCSTR info )
{
	return TRUE;
}

BOOL CPlayer::AddProtected( int id, int iAddVal, LPCSTR info )
{
	return TRUE;
}

BOOL CPlayer::IsMoneyEnough(DWORD type, DWORD value)
{
	MY_ASSERT(value);

	if (XYD_UM_ONLYBIND == type)  // 这里不改，保持原有 ahj
		return m_Property.m_BindMoney >= value ? TRUE : FALSE;

	if (XYD_UM_ONLYUNBIND == type)
		return m_Property.m_Money >= value ? TRUE : FALSE;

	if (XYD_UM_ZENGBAO == type)
		return m_Property.m_ZengBao >= value ? TRUE : FALSE;

	// 元宝，还是特殊处理，以后屏蔽
	if (XYD_UM_YUANBAO == type)
		return m_Property.m_YuanBao >= value ? TRUE : FALSE;

	return FALSE;
}

BOOL CPlayer::CheckPlayerMoney(DWORD type, DWORD value, bool OnlyCheck)
{
	if (0 == value)
		return TRUE;

	if (!IsMoneyEnough(type, value))
		return FALSE;

	if (OnlyCheck)
		return TRUE;

	if (XYD_UM_ONLYBIND == type)
	{
		m_Property.m_BindMoney -= value;
		m_PlayerPropertyStatus[XA_BIND_MONEY-XA_MAX_EXP] = true;
	}
	else if (XYD_UM_ONLYUNBIND == type)
	{
		m_Property.m_Money -= value;
		m_PlayerPropertyStatus[XA_UNBIND_MONEY-XA_MAX_EXP] = true;
	}
	else if (XYD_UM_ZENGBAO == type)
	{
		m_Property.m_ZengBao -= value;
		m_PlayerPropertyStatus[XA_ZENGBAO - XA_MAX_EXP] = true;
	}
	else if (XYD_UM_YUANBAO == type)
	{
		m_Property.m_YuanBao -= value;
		m_PlayerPropertyStatus[XA_YUANBAO - XA_MAX_EXP] = true;
	}
	//else if (XYD_UM_BINDFIRST == type)
	//{
	//	if (m_Property.m_BindMoney >= value)
	//	{
	//		m_Property.m_BindMoney -= value;
	//	}
	//	else
	//	{
	//		m_Property.m_Money -= (value - m_Property.m_BindMoney);
	//		m_Property.m_BindMoney = 0;
	//		m_PlayerPropertyStatus[XA_UNBIND_MONEY-XA_MAX_EXP] = true;
	//	}

	//	m_PlayerPropertyStatus[XA_BIND_MONEY-XA_MAX_EXP] = true;
	//}

	return TRUE;
}

DWORD CPlayer::AddPlayerSp(DWORD value)
{
	if (0 == value)
		return 0;

	m_Property.m_CurSp += value;
	m_PlayerPropertyStatus[XA_CUR_SP - XA_MAX_EXP] = true;
	

	return m_Property.m_CurSp;
}

BOOL CPlayer::CheckPlayerSp(DWORD value, bool OnlyCheck)
{
	if (value > m_Property.m_CurSp)
		return FALSE;

	if (OnlyCheck)
		return TRUE;

	m_Property.m_CurSp -= value;
	m_PlayerPropertyStatus[XA_CUR_SP-XA_MAX_EXP] = true;

	return TRUE;
}

DWORD CPlayer::AddPlayerMoney(DWORD type, DWORD value)
{
	if (0 == value)
		return 0;

	DWORD remain = 0;
	DWORD left = value;

	if (XYD_UM_ONLYBIND == type)			// 绑定满了就不加了
	{
		remain = 0xffffffff - m_Property.m_BindMoney;
		if (remain)
		{
			m_Property.m_BindMoney += (remain >= value) ? value : (remain);
			left -= ((remain >= value) ? value : (remain));

			m_PlayerPropertyStatus[XA_BIND_MONEY - XA_MAX_EXP] = true;
		}
	}
	else if (XYD_UM_ONLYUNBIND == type)		// 非绑定满了就不加了
	{
		remain = 0xffffffff - m_Property.m_Money;
		if (remain)
		{
			m_Property.m_Money += (remain >= value) ? value : (remain);
			left -= ((remain >= value) ? value : (remain));

			m_PlayerPropertyStatus[XA_UNBIND_MONEY - XA_MAX_EXP] = true;
		}
	}
	else if (XYD_UM_ZENGBAO == type)		// 加礼券
	{
		remain = 0xffffffff - m_Property.m_ZengBao;
		if (remain)
		{
			m_Property.m_ZengBao += (remain >= value) ? value : (remain);
			left -= ((remain >= value) ? value : (remain));

			m_PlayerPropertyStatus[XA_ZENGBAO - XA_MAX_EXP] = true;
		}
	}
	// 这一块以后运营的时候是在账号服务器里面处理的，以后要屏蔽这一块~ahj
	else if (XYD_UM_YUANBAO == type)
	{
		remain = 0xffffffff - m_Property.m_YuanBao;
		if (remain)
		{
			m_Property.m_YuanBao += (remain >= value) ? value : remain;
			left -= ((remain >= value) ? value : remain);

			m_PlayerPropertyStatus[XA_YUANBAO - XA_MAX_EXP] = true;
		}
	}
	//else if (XYD_UM_BINDFIRST == type)		// 非绑定满了，往绑定里面加	
	//{
	//	remain = 0xffffffff - m_Property.m_Money;
	//	if (remain)
	//	{
	//		m_Property.m_Money += (remain >= value) ? value : (remain);
	//		left -= ((remain >= value) ? value : (remain));

	//		m_PlayerPropertyStatus[XA_UNBIND_MONEY-XA_MAX_EXP] = true;

	//		if (left)		// 绑定未完全容纳
	//		{
	//			remain = 0xffffffff - m_Property.m_BindMoney;
	//			m_Property.m_BindMoney += (remain >= left) ? left : (remain);
	//			left -= (remain >= left) ? left : (remain);

	//			m_PlayerPropertyStatus[XA_BIND_MONEY-XA_MAX_EXP] = true;
	//		}
	//	}
	//}
	//else
	//{
	//	MY_ASSERT(0);
	//}

	return left;	
}

BOOL CPlayer::AddPlayerPoint(WORD type, WORD point)
{
	if (type < XA_TF_JINGGONG || type > XA_REMAINPOINT)
		return FALSE;

	if (0 == point)
		return FALSE;

	switch (type)
	{
	case XA_TF_JINGGONG:
		m_Property.m_JingGong += point;
		break;
	case XA_TF_FANGYU:
		m_Property.m_FangYu += point;
		break;
	case XA_TF_QINGSHEN:
		m_Property.m_QingShen += point;
		break;
	case XA_TF_JIANSHEN:
		m_Property.m_JianShen += point;
		break;
	case XA_REMAINPOINT:
		m_Property.m_RemainPoint += point;
		break;
	default:
		MY_ASSERT(0 && "AddPlayerPoint:Invalid Type!!!");
	}

	m_PlayerPropertyStatus[type-XA_MAX_EXP] = true;

	return TRUE;
}

BOOL CPlayer::AddTelergy( int iAddVal, LPCSTR info )
{
	BOOL bRet = FALSE;
	return bRet;
}

int CPlayer::GetTelergyLevel(void)
{
	// 	if( m_Property.m_CurTelergy >= 0 && m_Property.m_CurTelergy < MAX_EQUIPTELERGY )
	// 	{
	// 		return m_Property.m_Telergy[ m_Property.m_CurTelergy ].byTelergyLevel;
	// 	}
	// 	else
	// 	{
	// 		return 0;
	// 	}
	return 0;
}

int CPlayer::GetTelergyLevel(WORD wTelergyID)
{
	for (int i = 0; i < MAX_TELEGRYNUM; ++i)
	{
		if (m_Property.m_Xyd3Telergy[i].m_TelergyID == wTelergyID && 0 != m_Property.m_Xyd3Telergy[i].m_TelergyLevel)
		{
			return m_Property.m_Xyd3Telergy[i].m_TelergyLevel;
		}
	}

	return 0;
}

// 获取心法攻击系数
int     CPlayer::GetTelergyCoef( BOOL bIsAttack )
{
	DWORD   dwRet = 0;

	if( bIsAttack )
		dwRet = GetTelergyCalculate( TELERGYR_wDamCoef );
	else
		dwRet = GetTelergyCalculate( TELERGYR_wPowCoef );

	return  dwRet;  // 1000*TelergyCoef
}

// 设置当前使用的心法
BOOL    CPlayer::SetCurTelergyNum( BYTE byCurTelergyNum )
{
	// 	int nIndex = -1;
	// 	for (int i = 0;i < MAX_EQUIPTELERGY;++i)
	// 	{
	// 		if (byCurTelergyNum == m_Property.m_Telergy[i].wTelergyID)
	// 		{
	// 			m_Property.m_Telergy[i].bStateTel = STelergy::STATETEL::STATETEL_USE;  //add by xj
	// 			nIndex = i;
	// 			break;
	// 		}
	// 	}
	// 	if( nIndex>=0 && nIndex<MAX_EQUIPTELERGY )
	// 	{
	// 		if( m_Property.m_Telergy[nIndex].byTelergyLevel>0 )
	// 		{
	// 			m_Property.m_CurTelergy = (BYTE)m_Property.m_Telergy[nIndex].wTelergyID;
	// 			return  TRUE;
	// 		}
	// 	}
	// 	m_Property.m_CurTelergy	= 255;
	return  FALSE;
}

// 设置当前使用的武功
BOOL    CPlayer::SetCurSelSkills( WORD cur )
{
	return  TRUE;
}

//-----------------------------------------------------------------
BOOL CPlayer::DeleteTeletgy( BYTE byTelergyNum )
{
	// 	if( byTelergyNum >= 0 || byTelergyNum < MAX_EQUIPTELERGY )
	// 	{
	// 		if( m_Property.m_Telergy[ byTelergyNum ].byTelergyLevel )
	// 		{
	// 			WORD id = m_Property.m_Telergy[ byTelergyNum ].wTelergyID;
	// 			BYTE level = m_Property.m_Telergy[ byTelergyNum ].byTelergyLevel;
	// 			DWORD value = m_Property.m_Telergy[ byTelergyNum ].dwTelergyVal;
	// 			
	// 			m_Property.m_Telergy[ byTelergyNum ].wTelergyID = 0;
	// 
	// 			//m_Property.m_Telergy[ byTelergyNum ].byTelergyLevel = 0;
	// 			SetTelergyLevel(m_Property.m_CurTelergy,0);
	// 			
	// 			m_Property.m_Telergy[ byTelergyNum ].dwTelergyVal = 0;
	// 			
	// 			SendTelergyChange();
	// 			
	// 		//	SendCurTelChange(&m_Property.m_Telergy[ byTelergyNum ],false);
	// 
	// 			CalculateTelergyEffects();
	// 			SendPlayerAbility();
	// 
	// 			rfalse(4, 1, "[%s][%s]删除心法[%d, %d, %d]", GetAccount(), GetName(), id, level, value);
	// 
	// 			return  TRUE;
	// 		}
	// 	}

	return  FALSE;
}

// 根据心法类型计算当前装备心法的相应值
int CPlayer::GetTelergyCalculate( E_TOTAL_TELERGY iDataType )
{
	return GetTelergyEffect( iDataType );
	/*
	int     iRet = 0, i;
	WORD    *pwVal, wVal;
	TelergyData     *pTelergy;

	if( iDataType<=TELERGYR_NONE || iDataType>=TELERGYR_MAX )
	return  iRet;

	for( i=0; i<MAX_EQUIPTELERGY; i++ )
	{
	if( m_Property.m_Telergy[i].byTelergyLevel )
	{
	pTelergy = g_pTelergyData->GetTelergy( m_Property.m_Telergy[i].wTelergyID );
	if( pTelergy )
	{
	pwVal = &( pTelergy->wDamCoef );
	wVal = *( pwVal + iDataType - TELERGYR_wDamCoef );

	// 根据所要访问的数据进行相应的计算
	if( iDataType>=TELERGYR_wDamCoef && iDataType<=TELERGYR_wPowCoef )
	{
	iRet += (int)( (m_Property.m_Telergy[i].byTelergyLevel + 99) * wVal );// / 1000.0 );
	}
	else if( iDataType>=TELERGYR_wDamDam && iDataType<=TELERGYR_wAgiDef )
	{
	iRet += (int)( m_Property.m_Telergy[i].byTelergyLevel * wVal / 10.0 ); // 这个是因为经脉伤害的心法层数为10层
	}
	else 
	{
	iRet += (int)( m_Property.m_Telergy[i].byTelergyLevel * wVal / 100.0 );
	}
	}
	}
	}

	return  iRet;
	*/
}

int CPlayer::GetTelergyEffect( E_TOTAL_TELERGY iDataType )
{
	// 	if ( iDataType <= TELERGYR_NONE || iDataType >= TELERGYR_MAX )
	// 		return 0;
	// 
	// 	return telergyEffects[ iDataType ];
	return 0;
}

void CPlayer::CalculateTelergyEffects()
{
	// 	memset( &telergyEffects, 0, sizeof( telergyEffects ) );
	// 
	// 	int maxLoop[2] = { MAX_EQUIPTELERGY, MAX_EXTRATELERGY };
	// 	STelergy *data[2] = { m_Property.m_Telergy, m_Property.m_ExtraTelergy.telergy };
	// 
	// 	for ( int loop = 0; loop < 2; loop ++ )
	// 	{
	// 		int max = maxLoop[loop];
	// 		STelergy *telergy = data[loop];
	// 
	// 		for ( int i = 0; i < max; i ++ )
	// 		{
	// 			// 34扩展心法的限制,如果关闭则没有效果
	// 			if ( loop == 1 && ( i == 2 || i == 3))
	// 			{
	// 				if ( m_Property.m_ExtraTelergy.state[i] == 0 )
	// 					continue;
	// 			}
	// 
	// 			if ( telergy[i].byTelergyLevel )
	// 			{
	// 				if ( TelergyData *pTelergy = g_pTelergyData->GetTelergy( telergy[i].wTelergyID ) )
	// 				{
	// 					// 将心法最大等级截断, 避免部分玩家心法等级异常时造成更大的错误..
	// 					int level = telergy[i].byTelergyLevel;
	// 					if ( (DWORD)level > pTelergy->m_maxLevel )
	// 						level = pTelergy->m_maxLevel;
	// 
	// 					// 心法加层
	// 					if ( telergy[i].wTelergyID == ( equipAttribute.telergyLevel >> 8 ) )
	// 						level += ( equipAttribute.telergyLevel & 0xff );
	// 
	// 					// zeb 2009.11.19
	// 					//WORD *pwVal = &( pTelergy->wDamCoef );
	// 					//for ( int j=TELERGYR_wDamCoef; j<TELERGYR_MAX; j++, pwVal++ )
	// 					//{
	// 					//	// 根据所要访问的数据进行相应的计算
	// 					//	if( j<=TELERGYR_wPowCoef )
	// 					//		telergyEffects[j] += (int)( ( level + 99 ) * ( *pwVal ) );
	// 					//	else 
	// 					//	{
	// 					//		if ( *pwVal != 0 && level != 0 && telergy[i].wTelergyID >= 38 && telergy[i].wTelergyID <= 41 )
	// 					//			telergyEffects[j] += (int)( 100 + level * ( *pwVal ) / 100.0 );
	// 					//		else
	// 					//			telergyEffects[j] += (int)( level * ( *pwVal ) / 100.0 );
	// 					//	}
	// 					//}
	// 					// zeb 2009.11.19
	// 				}
	// 			}
	// 		}
	// 	}
}

//-----------------------------------------------------------------
bool CPlayer::ReduceTelergyVal( BYTE byTelergyID, DWORD dwUseVal, BOOL bIsReduce, LPCSTR info )
{
	// 	DWORD dwTotalVal = 0;
	// 	DWORD dwMaxTelergy;
	// 
	// 	int index = GetTelergyIndexByTeleryID(byTelergyID);
	// 
	// 	if (index >= 0 && index < TELERGY_NUM)
	// 	{
	// 		if (m_Property.m_Telergy[index].byTelergyLevel)
	// 		{
	// 			// 先减当前等级心法值
	// 			if (m_Property.m_Telergy[index].dwTelergyVal >= dwUseVal)
	// 			{
	// 				if (bIsReduce)
	// 				{
	// 					m_Property.m_Telergy[index].dwTelergyVal -= dwUseVal;
	// 
	// 					// 发送心法数据的改变
	// 					SendTelergyChange();
	// 				}
	// 
	// 				return  true;
	// 			}
	// 
	// 			// 取得总的心法值
	// 			TelergyData* pTelergy = g_pTelergyData->GetTelergy(m_Property.m_Telergy[index].wTelergyID);
	// 			if (pTelergy == NULL)
	// 				return 0;
	// 
	// 			dwMaxTelergy = DTelergyData::GetCurLevelMax(pTelergy,m_Property.m_Telergy[ index ].byTelergyLevel);
	// 			dwTotalVal = dwMaxTelergy * (m_Property.m_Telergy[index].byTelergyLevel - 1) 
	// 				+ m_Property.m_Telergy[index].dwTelergyVal;
	// 
	// 			// 总的心法值不够
	// 			if (dwTotalVal < dwUseVal)
	// 				return false;
	// 
	// 			if (bIsReduce)
	// 			{
	// 				// 减掉心法值
	// 				dwTotalVal -= dwUseVal;
	// 				BYTE prevLevel = m_Property.m_Telergy[index].byTelergyLevel;
	// 
	// 				SetTelergyLevel(index, (dwTotalVal / dwMaxTelergy) + 1);
	// 
	// 				m_Property.m_Telergy[index].dwTelergyVal = dwTotalVal % dwMaxTelergy;
	// 
	// 				// 等级是否发生了改变？
	// 				if (prevLevel != m_Property.m_Telergy[index].byTelergyLevel)
	// 				{
	// 					if (info != NULL)
	// 					{
	// 						CPlayer *pPlayer = this;
	// 						if (pPlayer->m_ParentRegion == NULL)
	// 							return false;
	// 
	// 						int mapid = pPlayer->m_ParentRegion->m_wRegionID;
	// 						int x = pPlayer->m_wCurX;
	// 						int y = pPlayer->m_wCurY;
	// 
	// 						BEGIN_SNOTIFY( 100 )( 0, ( int )pPlayer->GetSID() )( 1, 4 )
	// 							( 2, ( int )prevLevel  )
	// 							( 3, ( int )pPlayer->m_Property.m_Telergy[index].byTelergyLevel )
	// 							( 4, ( int )pPlayer->m_Property.m_Telergy[index].wTelergyID )
	// 							( 7, mapid )( 8, x )( 9, y )( 10, pPlayer->GetAccount() )( 11, info )END_SNOTIFY;
	// 					}
	// 
	// 					//CalculateTelergyEffects();
	// 					SendPlayerAbility();
	// 				}
	// 
	// 				// 发送心法数据的改变
	// 				SendTelergyChange();
	// 			}
	// 
	// 			return  true;
	// 		}
	// 	}

	return  false;
}

BOOL CPlayer::DamageToVenation( CPlayer *pAttacker, BYTE &byWhoVenation, BYTE &byVenationState, DWORD extraPos )
{
	// 	if ( ( pAttacker == NULL ) || ( extraPos >= 4 ) )
	// 		return false;
	// 
	// 	// 扩展心法未激活
	// 	if ( ( pAttacker->m_Property.m_ExtraTelergy.state[extraPos] == 0 ) || 
	// 		( pAttacker->m_Property.m_ExtraTelergy.telergy[extraPos].byTelergyLevel == 0 ) )
	// 		return false;
	// 
	// 	int whoVenation = -1;
	// 
	// 	// 非死亡状态时,考虑心法伤害...
	// 	TelergyData *pTelergy = g_pTelergyData->GetTelergy( 
	// 		pAttacker->m_Property.m_ExtraTelergy.telergy[extraPos].wTelergyID );
	// 
	// 	if ( pTelergy == NULL )
	// 		return false;
	// 
	// 	// 这里是一个特殊处理，直接处理64bit来判断是否至少有1个经脉的伤害效果
	// 	// zeb 2009.11.19
	// 	//if ( ( __int64 )&pTelergy->wDamDam == 0 )
	// 	//	return false;
	// 	// zeb 2009.11.19
	// 
	// 	// 获取攻击者所选择的伤害心法
	// 	// zeb 2009.11.19
	// 	//if ( pTelergy->wDamDam )
	// 	//	whoVenation = 0;
	// 	//else if ( pTelergy->wPowDam )
	// 	//	whoVenation = 1;
	// 	//else if ( pTelergy->wDefDam )
	// 	//	whoVenation = 2;
	// 	//else if ( pTelergy->wAgiDam )
	// 	//	whoVenation = 3;
	// 	// zeb 2009.11.19
	// 
	// 	//// 目标等级超过自己50级则无效。。。
	// 	//if ( ( int )m_byLevel - ( int )pAttacker->m_byLevel > 50 ) 
	// 	//    return false;
	// 
	// 	// 获取自己的命中几率和对方的防御几率
	// 	int hitRating = pAttacker->GetTelergyCalculate( ( E_TOTAL_TELERGY )( TELERGYR_wDamDam + whoVenation ) );
	// 	int avoidRating = GetTelergyCalculate( ( E_TOTAL_TELERGY )( TELERGYR_wDamDef + whoVenation ) );
	// 	hitRating -= avoidRating;
	// 	if ( hitRating <= 0 )
	// 		return false;
	// 
	// 	// 如果不能命中，则失败返回
	// 	if ( !IN_ODDS( hitRating ) )
	// 		return false;
	// 
	// 	// 不会造成经脉伤害，直接返回！
	// 	if ( whoVenation == -1 )
	// 		return false;
	// 
	// 	// 计算伤害的程度,默认通过命中导致的受伤程度固定为1
	// 	int venaDamage = 1;
	// 
	// 	BYTE *venation = NULL;
	// 	switch ( whoVenation ) 
	// 	{
	// 	case 0: venation = &m_Property.m_byENState; break;  // 手太阳伤害   
	// 	case 1: venation = &m_Property.m_byINState; break;  // 手少阳伤害
	// 	case 2: venation = &m_Property.m_bySTState; break;  // 足太阴伤害
	// 	case 3: venation = &m_Property.m_byAGState; break;  // 足少阴伤害
	// 	}
	// 
	// 	BYTE prevVenation = *venation;
	// 	*venation += venaDamage;
	// 	if ( *venation > 4 )
	// 		*venation = 4;
	// 
	// 	if ( prevVenation != *venation )
	// 	{
	// 		SendVenationState( whoVenation, *venation );
	// 		SendPlayerAbility();
	// 	}
	// 
	// 	byWhoVenation = whoVenation;
	// 	byVenationState = *venation;

	return true;
}

///////////////////////////////// 经脉计算
BOOL    CPlayer::DamageToVenation( CPlayer *pAttacker, int iDamage, BOOL bIsDead,
	BYTE &byWhoVenation, BYTE &byVenationState )//根据伤害值计算经脉所受到的伤害
{
	// 	int whoVenation = -1;
	// 	BOOL result = FALSE;
	// 
	// 	// 非死亡状态时,考虑心法伤害...
	// 	if ( !bIsDead && pAttacker && ( pAttacker->m_Property.m_CurTelergy < MAX_EQUIPTELERGY ) )
	// 	{
	// 		// 先处理两个扩展心法
	// 		result |= DamageToVenation( pAttacker, byWhoVenation, byVenationState, 0 );
	// 		result |= DamageToVenation( pAttacker, byWhoVenation, byVenationState, 1 );
	// 		result |= DamageToVenation( pAttacker, byWhoVenation, byVenationState, 2 );
	// 		result |= DamageToVenation( pAttacker, byWhoVenation, byVenationState, 3 );
	// 
	// 		TelergyData *pTelergy = NULL;
	// 		STelergy &t = pAttacker->m_Property.m_Telergy[ pAttacker->m_Property.m_CurTelergy ];
	// 		if ( t.byTelergyLevel != 0 )
	// 			pTelergy = g_pTelergyData->GetTelergy( t.wTelergyID );
	// 
	// 		if ( pTelergy == NULL )
	// 			return result;
	// 
	// 		// 这里是一个特殊处理，直接处理64bit来判断是否至少有1个经脉的伤害效果
	// 		// zeb 2009.11.19
	// 		//if ( ( __int64 )&pTelergy->wDamDam == 0 )
	// 		//	return result;
	// 		// zeb 2009.11.19
	// 
	// 		// 获取攻击者所选择的伤害心法
	// 		// zeb 2009.11.19
	// 		//if ( pTelergy->wDamDam )
	// 		//	whoVenation = 0;
	// 		//else if ( pTelergy->wPowDam )
	// 		//	whoVenation = 1;
	// 		//else if ( pTelergy->wDefDam )
	// 		//	whoVenation = 2;
	// 		//else if ( pTelergy->wAgiDam )
	// 		//	whoVenation = 3;
	// 		// zeb 2009.11.19
	// 
	// 		//// 目标等级超过自己50级则无效。。。	
	// 		//if ( ( int )m_byLevel - ( int )pAttacker->m_byLevel > 50 ) 
	// 		//    return false;
	// 
	// 		// 获取自己的命中几率和对方的防御几率
	// 		int hitRating = pAttacker->GetTelergyCalculate( ( E_TOTAL_TELERGY )( TELERGYR_wDamDam + whoVenation ) );
	// 		int avoidRating = GetTelergyCalculate( ( E_TOTAL_TELERGY )( TELERGYR_wDamDef + whoVenation ) );
	// 		hitRating -= avoidRating;
	// 		if ( hitRating <= 0 )
	// 			return result;
	// 
	// 		// 如果不能命中，则失败返回
	// 		if ( !IN_ODDS( hitRating ) )
	// 			return result;
	// 	}
	// 	// 如果是目标死亡,则随机抽
	// 	else if ( bIsDead )
	// 		whoVenation = rand() % 4;
	// 
	// 	// 不会造成经脉伤害，直接返回！
	// 	if ( whoVenation == -1 )
	// 		return result;
	// 
	// 	// 计算伤害的程度,默认通过命中导致的受伤程度固定为1
	// 	int venaDamage = 1;
	// 
	// 	// 如果死亡，伤害则随机
	// 	if ( bIsDead )
	// 	{ 
	// 		if ( IN_ODDS( 10 ) )
	// 			venaDamage = 4;
	// 		else if ( IN_ODDS( 40 ) )
	// 			venaDamage = 3;
	// 		else if ( IN_ODDS( 80 ) )
	// 			venaDamage = 2;
	// 	}
	// 
	// 	BYTE *venation = NULL;
	// 	switch ( whoVenation ) 
	// 	{
	// 	case 0: venation = &m_Property.m_byENState; break;  // 手太阳伤害   
	// 	case 1: venation = &m_Property.m_byINState; break;  // 手少阳伤害
	// 	case 2: venation = &m_Property.m_bySTState; break;  // 足太阴伤害
	// 	case 3: venation = &m_Property.m_byAGState; break;  // 足少阴伤害
	// 	}
	// 
	// 	BYTE prevVenation = *venation;
	// 	*venation += venaDamage;
	// 	if ( *venation > 4 )
	// 		*venation = 4;
	// 
	// 	if ( prevVenation != *venation )
	// 	{
	// 		SendVenationState( whoVenation, *venation );
	// 		SendPlayerAbility();
	// 	}
	// 
	// 	byWhoVenation = whoVenation;
	// 	byVenationState = *venation;

	return true;
}


// 被治疗经脉受伤
BOOL    CPlayer::CureSomebodyVenation( DWORD dwDestGID )
{
	CPlayer *pCurer;
	pCurer = (CPlayer*)GetPlayerByGID( dwDestGID )->DynamicCast(IID_PLAYER);

	if( pCurer )
		return  pCurer->BeCureVenation( this );

	return  FALSE;
}

// 被治疗经脉受伤
BOOL    CPlayer::BeCureVenation( CPlayer *pCurer )
{
	// 	DWORD   dwTempTime,dwTelergyTime;
	// 
	// 	if( pCurer==NULL )      
	// 		return  FALSE;
	// 	if( GetCurActionID()!=EA_ZAZEN || pCurer->GetCurActionID() != EA_ZAZEN )
	// 	{
	// 		SendErrorMsg( SABackMsg::B_CURE_NOZAZEN );
	// 		pCurer->SendErrorMsg( SABackMsg::B_CURE_NOZAZEN );
	// 		return  FALSE;
	// 	}
	// 	// 其他治疗判断条件...
	// 	// 被医疗着被治疗 发送给医生提示
	// 	if ( m_dwExtraState&SHOWEXTRASTATE_BECURE )
	// 	{
	// 		pCurer->SendErrorMsg( SABackMsg::B_BECURED );
	// 		return  FALSE;
	// 	}
	// 	// 医生要换人就先站起来
	// 	if ( pCurer->m_dwExtraState&SHOWEXTRASTATE_CURE )
	// 	{
	// 		pCurer->SendErrorMsg( SABackMsg::B_CUREOTHER );
	// 		return FALSE;
	// 	}
	// 
	// 	LPCSTR szName = "";
	// 	//if (m_Property.m_byENState != 0)
	// 	//    szName = "手太阳";
	// 	//else if(m_Property.m_byINState != 0)
	// 	//    szName = "手少阳";
	// 	//else if(m_Property.m_bySTState != 0)
	// 	//    szName = "足太阴";
	// 	//else if(m_Property.m_byAGState != 0)
	// 	//    szName = "足少阴";
	// 	//else
	// 	if (m_Property.m_byENState == 0 &&
	// 		m_Property.m_byINState == 0 &&
	// 		m_Property.m_bySTState == 0 &&
	// 		m_Property.m_byAGState == 0)
	// 	{
	// 		TalkToDnid(pCurer->m_ClientIndex, "对方好像没有受伤哦-____-bb");
	// 		return FALSE;
	// 	}
	// 
	// 	int GetDir(WORD sx, WORD sy, WORD dx, WORD dy);
	// 	int GetPtLength(WORD sx, WORD sy, WORD dx, WORD dy);
	// 
	// 	//    if (abs(pCurer->m_wCurX-m_wCurX) > 1 || abs(pCurer->m_wCurY-m_wCurY) > 1)
	// 	if (GetPtLength(pCurer->m_wCurX, pCurer->m_wCurY, m_wCurX, m_wCurY) > 1)
	// 	{
	// 		pCurer->SendErrorMsg( SABackMsg::B_CURE_FARAWAY );
	// 		return  FALSE;
	// 	}
	// 
	// 	// 同时只能治疗一种经脉
	// 	if( pCurer->GetTelergyCalculate( TELERGYR_wDamMed ) && m_Property.m_byENState )
	// 	{
	// 		// 手太阳治疗成立(设置状态，计算治疗时间)
	// 		szName = "手太阳";
	// 		dwTempTime      = (600>>(4-m_Property.m_byENState));
	// 		dwTelergyTime   = pCurer->GetTelergyCalculate( TELERGYR_wDamMed );
	// 		if( dwTempTime > dwTelergyTime )
	// 			m_dwCureDAMTime = (dwTempTime - dwTelergyTime) * 5;     // 20 = 20（次/秒）， 断裂*100，重伤*50，中伤*25，轻伤*12
	// 		else
	// 			m_dwCureDAMTime = 300;
	// 		//m_dwCureDAMTime = (80>>(4-m_Property.m_byENState)) * 1000 / 
	// 		//    pCurer->GetTelergyCalculate( TELERGYR_wDamMed );
	// 	}
	// 	else    if( pCurer->GetTelergyCalculate( TELERGYR_wPowMed ) && m_Property.m_byINState )
	// 	{
	// 		// 手少阳治疗成立(设置状态，计算治疗时间)
	// 		szName = "手少阳";
	// 		dwTempTime      = (600>>(4-m_Property.m_byINState));
	// 		dwTelergyTime   = pCurer->GetTelergyCalculate( TELERGYR_wPowMed );
	// 		if( dwTempTime > dwTelergyTime )
	// 			m_dwCurePOWTime = (dwTempTime - dwTelergyTime) * 5;     // 20 = 20（次/秒）， 断裂*100，重伤*50，中伤*25，轻伤*12
	// 		else
	// 			m_dwCurePOWTime = 300;
	// 		//m_dwCurePOWTime = (80>>(4-m_Property.m_byINState)) * 1000 / 
	// 		//    pCurer->GetTelergyCalculate( TELERGYR_wPowMed );
	// 	}
	// 	else    if( pCurer->GetTelergyCalculate( TELERGYR_wDefMed ) && m_Property.m_bySTState )
	// 	{
	// 		// 足太阴治疗成立(设置状态，计算治疗时间)
	// 		szName = "足太阴";
	// 		dwTempTime      = (600>>(4-m_Property.m_bySTState));
	// 		dwTelergyTime   = pCurer->GetTelergyCalculate( TELERGYR_wDefMed );
	// 		if( dwTempTime > dwTelergyTime )
	// 			m_dwCureDEFTime = (dwTempTime - dwTelergyTime) * 5;     // 20 = 20（次/秒）， 断裂*100，重伤*50，中伤*25，轻伤*12
	// 		else
	// 			m_dwCureDEFTime = 300;
	// 		//m_dwCureDEFTime = (80>>(4-m_Property.m_bySTState)) * 1000 / 
	// 		//    pCurer->GetTelergyCalculate( TELERGYR_wDefMed );
	// 	}
	// 	else if( pCurer->GetTelergyCalculate( TELERGYR_wAgiMed ) && m_Property.m_byAGState )
	// 	{
	// 		// 足少阴治疗成立(设置状态，计算治疗时间)
	// 		szName = "足少阴";
	// 		dwTempTime      = (600>>(4-m_Property.m_byAGState));
	// 		dwTelergyTime   = pCurer->GetTelergyCalculate( TELERGYR_wAgiMed );
	// 		if( dwTempTime > dwTelergyTime )
	// 			m_dwCureAGITime = (dwTempTime - dwTelergyTime) * 5;     // 20 = 20（次/秒）， 断裂*100，重伤*50，中伤*25，轻伤*12;
	// 		else
	// 			m_dwCureAGITime = 300;
	// 		//m_dwCureAGITime = (80>>(4-m_Property.m_byAGState)) * 1000 / 
	// 		//    pCurer->GetTelergyCalculate( TELERGYR_wAgiMed );
	// 	}
	// 	else 
	// 	{
	// 		TalkToDnid(pCurer->m_ClientIndex, FormatString("你不会[%s]的治疗方法-____-bb", szName));
	// 		return FALSE;
	// 	}
	// 
	// 	LPCSTR info = FormatString("开始对[%s]进行治疗。。。", szName);
	// 	TalkToDnid(pCurer->m_ClientIndex, info);
	// 	TalkToDnid(m_ClientIndex, info);
	// 
	// 	pCurer->m_byDir = GetDir(pCurer->m_wCurX, pCurer->m_wCurY, m_wCurX, m_wCurY);
	// 	pCurer->SendMyState();
	// 
	// 	m_dwExtraState |= SHOWEXTRASTATE_BECURE;
	// 	pCurer->m_dwExtraState |= SHOWEXTRASTATE_CURE;
	// 	AddCheckID();
	// 	pCurer->AddCheckID();
	// 	m_iCurerGID = pCurer->GetGID();
	// 
	// 	SendErrorMsg( SABackMsg::B_BECUREING );
	// 	pCurer->SendErrorMsg( SABackMsg::B_CUREING );
	// 	SendExtraState();
	// 	pCurer->SendExtraState();
	return  TRUE;
}

// 每秒20次更新治疗经脉的时间
void CPlayer::UpdateCureVenation( void )
{
}

// 判断一条脉线是否已经打通
bool    CPlayer::IsPassVenation( BYTE byVenaNum )
{
	// 	int     iVenapointCount;
	// 
	// 	iVenapointCount = g_pVenapointData->GetVenapointCount( byVenaNum );
	// 	if( m_Property.m_dwVenapointState[byVenaNum]== ((0x01<<iVenapointCount) - 1) )
	// 	{
	// 		return true;
	// 	}
	// 	return  false;
	return false;
}

// 取得已经打通的经脉数量，byCount是需要统计的数量，按照经脉的顺序统计
int     CPlayer::GetPassVenationCount( BYTE byCount )
{
	int     i,iRet = 0;

	if( byCount>8 )     
		return  0;

	for( i=0; i<byCount; i++ )
	{
		if( IsPassVenation( i ) )
		{
			iRet ++;
		}
	}

	return  iRet;
}

int CPlayer::CheckPlusPoints()
{
	int result = 0;
	//int isfull = 0;
	//for ( int i = 0; i < 9; i++ )
	//{
	//	int max = g_pVenapointData->GetVenapointCount( i );
	//	for ( int j = 0; j < max; j++ )
	//	{
	//		if ( !IsPassVenation( i, j ) )
	//			continue;

	//		LPSVenapointData data = g_pVenapointData->GetVenapoint( i, j );

	//		result += data->wAddAGI;
	//		result += data->wAddPOW;
	//		result += data->wAddDEF;
	//		result += data->wAddDAM;
	//	}

	//	// 全脉加成
	//	if ( IsPassVenation( i ) )
	//	{
	//		LPSVenapointData data = g_pVenapointData->GetVenapoint( i, 31 );

	//		result += data->wAddAGI;
	//		result += data->wAddPOW;
	//		result += data->wAddDEF;
	//		result += data->wAddDAM;

	//		if ( i < 8 )
	//			isfull++;
	//	}
	//}

	//if ( isfull == 8 )
	//	result += 80;

	return result;
}

// 根据穴道数据增加属性值
void    CPlayer::AddValueByVenaData( LPSVenapointData pVenaData )
{
	if( pVenaData==NULL )   return;

	// 改变数据需要发送到客户端
	m_VenaAddFactor[SVenapointData::MAX_HP] += pVenaData->data[SVenapointData::MAX_HP];
	m_VenaAddFactor[SVenapointData::MAX_MP] += pVenaData->data[SVenapointData::MAX_MP];
	m_VenaAddFactor[SVenapointData::MAX_SP] += pVenaData->data[SVenapointData::MAX_SP];


	m_VenaAddFactor[SVenapointData::RE_HP] += pVenaData->data[SVenapointData::RE_HP];
	m_VenaAddFactor[SVenapointData::RE_MP] += pVenaData->data[SVenapointData::RE_MP];
	m_VenaAddFactor[SVenapointData::RE_SP] += pVenaData->data[SVenapointData::RE_SP];

	m_VenaAddFactor[SVenapointData::OUT_ATTA_BASE] += pVenaData->data[SVenapointData::OUT_ATTA_BASE];
	m_VenaAddFactor[SVenapointData::OUT_ATTA_MASTERY] += pVenaData->data[SVenapointData::OUT_ATTA_MASTERY];
	m_VenaAddFactor[SVenapointData::INNER_ATTA_BASE] += pVenaData->data[SVenapointData::INNER_ATTA_BASE];
	m_VenaAddFactor[SVenapointData::INNER_ATTA_MASTERY] += pVenaData->data[SVenapointData::INNER_ATTA_MASTERY];

	m_VenaAddFactor[SVenapointData::OUT_DEF_BASE] += pVenaData->data[SVenapointData::OUT_DEF_BASE];
	m_VenaAddFactor[SVenapointData::OUT_DEF_MASTERY] += pVenaData->data[SVenapointData::OUT_DEF_MASTERY];
	m_VenaAddFactor[SVenapointData::INNER_DEF_BASE] += pVenaData->data[SVenapointData::INNER_DEF_BASE];
	m_VenaAddFactor[SVenapointData::INNER_DEF_MASTERY] += pVenaData->data[SVenapointData::INNER_DEF_MASTERY];

	m_VenaAddFactor[SVenapointData::AGILE] += pVenaData->data[SVenapointData::AGILE];
	m_VenaAddFactor[SVenapointData::AMUCK] += pVenaData->data[SVenapointData::AMUCK];
}


int CPlayer::GetVenaAddFactor(SVenapointData::VENA_PROP_DEF type)
{
	if (type < 0 || type >= SVenapointData::VENA_PROPERTY_MAX)
		return 0;

	return m_VenaAddFactor[type];
}

INT32 CPlayer::GetTelergyFactorByIndex(TelergyDataXiaYiShiJie::TELE_PROP_DEF index)
{
	if (index < 0 || index >= TelergyDataXiaYiShiJie::MAX_TELERGY_XIA_YI_SHI_JIE)
		return 0;

	return m_TelergyAddFactor[ index ];
}

// 设置一个穴道被打通
bool    CPlayer::SetPassVenapoint( BYTE byVenaNum, BYTE byPointNum )
{
	return false;

	// 	LPSVenapointData    pVenaData = NULL;
	// 
	// 	if( byVenaNum>8 || byPointNum>32 )
	// 		return  false;
	// 
	// 	pVenaData = g_pVenapointData->GetVenapoint( byVenaNum, byPointNum );
	// 	if( pVenaData==NULL )   
	// 		return  false;
	// 
	// 	// 如果想用多余的穴道来骗取经脉加点，少来！！~：（
	// 	if( IsPassVenation( byVenaNum ) )
	// 	{
	// 		//.. "你的这个穴道已经打通了的"
	// 		SendErrorMsg( SABackMsg::B_VENA_PASSED );
	// 		return  false;
	// 	}
	// 
	// 	// 判断穴道是否已经打通 
	// 	if( m_Property.m_dwVenapointState[byVenaNum] & (0x01<<byPointNum) )
	// 	{
	// 		//.. "你的这个穴道已经打通了的"
	// 		SendErrorMsg( SABackMsg::B_VENA_PASSED );
	// 		return  false;
	// 	}
	// 
	// 	// 判断打通穴道的条件是否成立
	// 	if( m_Property.m_byVenapointCount==0 )
	// 	{
	// 		//.. "你的充穴点数不够"
	// 		SendErrorMsg( SABackMsg::B_VENA_COUNTNOT );
	// 		return  false;
	// 	}
	// 
	// 	// 判断心法值是否够用（最后一个判断条件）
	// 	if( !ReduceTelergyVal( m_Property.m_CurTelergy, pVenaData->dwUseTelergy, FALSE, "" ) )
	// 	{
	// 		// 你的心法不够用于打通这个穴道
	// 		SendErrorMsg( SABackMsg::B_VENA_LOWTELERGY );
	// 		return  false;
	// 	}
	// 	else
	// 	{
	// 		//TalkToDnid( m_ClientIndex, FormatString( "当前心法消耗了[%d]点", pVenaData->dwUseTelergy ) );
	// 	}
	// 
	// 	if( !FindItemByIndex( pVenaData->wNum ) ) // 判断物品栏里有没有相应的物品
	// 	{       
	// 		//  判断有没5个小人。
	// 		g_Script.SetCondition( NULL, this, NULL );
	// 		LuaFunctor( g_Script, "HaveFiveWM" )[pVenaData->wNum][pVenaData->szName][byVenaNum][byPointNum]();
	// 		g_Script.CleanCondition();
	// 		SendErrorMsg( SABackMsg::B_VENA_NOTITEM );
	// 		return false;
	// 
	// 		//// 你的心法不够用于打通这个穴道
	// 		//TalkToDnid( m_ClientIndex, "你没有相应的铜人!" );
	// 		//return false;
	// 	}
	// 
	// 	/*
	// 	// 如果是任督二脉还需要判断之前的经脉是否都打通了
	// 	if( byVenaNum==6 || byVenaNum==7 )
	// 	{
	// 	if( m_Property.m_dwVenapointState[byVenaNum]==0 )//减少每次遍历的负担
	// 	{
	// 	if( GetPassVenationCount( 6 )!=6 )
	// 	{
	// 	// "等你打通前面的六个经脉再来吧！"
	// 	SendErrorMsg( SABackMsg::B_VENA_PASSNOT6 );
	// 	return  false;
	// 	}
	// 	}
	// 	}
	// 	// 判断当前穴道之前的穴道是否都打通（对于任督二脉）//经外奇脉没有此条件）
	// 	if( byVenaNum==6 || byVenaNum==7 )// )byVenaNum!=8 )//
	// 	{
	// 	if( m_Property.m_dwVenapointState[byVenaNum] != ((0x01<<byPointNum) - 1) )
	// 	{
	// 	//.. "强制冲脉"
	// 
	// 	g_Script.SetCondition( NULL, this, NULL );
	// 	g_Script.CallFunc( "ForciblyOpenVena", byVenaNum, byPointNum, pVenaData->szName, pVenaData->wNum ); 
	// 	g_Script.CleanCondition();
	// 	return  false;
	// 	}
	// 	}
	// 	*/
	// 
	// 	// 只要是不符合规则的任督二脉冲穴都认为是强制冲穴
	// 	if( byVenaNum==6 || byVenaNum==7 )
	// 	{
	// 		BOOL bIsForcibly = FALSE;
	// 		//if( m_Property.m_dwVenapointState[byVenaNum] == 0 )//减少每次遍历的负担
	// 		//{
	// 		// 前面的六个没打通完的
	// 		if( GetPassVenationCount( 6 ) != 6 )
	// 			bIsForcibly = TRUE;
	// 		//}
	// 
	// 		//if( m_Property.m_dwVenapointState[byVenaNum] != ((0x01<<byPointNum) - 1) )
	// 		DWORD checker = ( 0x01 << byPointNum ) - 1;
	// 		if ( ( m_Property.m_dwVenapointState[byVenaNum] & checker ) != checker )
	// 			bIsForcibly = TRUE; // 上面的穴道没打通的
	// 
	// 		if( bIsForcibly )
	// 		{
	// 			//.. "强制冲脉"
	// 			g_Script.SetCondition( NULL, this, NULL );
	// 			g_Script.CallFunc( "ForciblyOpenVena", byVenaNum, byPointNum, pVenaData->szName, pVenaData->wNum ); 
	// 			g_Script.CleanCondition();
	// 			return  false;
	// 		}
	// 	}  
	// 
	// 	// 删除物品栏里相应的物品
	// 	DelItem( pVenaData->wNum, "充脉删除" );
	// 	g_pSpecialItemData->OnUseedItem( pVenaData->wNum, 1 );      // 消耗物品侦测
	// 
	// 	return OpenVena( pVenaData, byVenaNum, byPointNum );
}

bool CPlayer::GmOpenVena( BYTE byVenaNum, BYTE byPointNum )
{
	return true;
}

bool CPlayer::OpenVena( LPSVenapointData pVenaData, BYTE byVenaNum, BYTE byPointNum )
{
	return true;
}

// 检测动作执行的条件
BOOL CPlayer::CheckAction(int eCA)
{
	BOOL bRet = TRUE;

	if( m_CurHp==0 ) 
		return FALSE;

	switch( eCA )
	{
	case EA_STAND:///当前为站立状态应该切换为打坐
		return (GetCurActionID() ==EA_STAND );
		break;
	case EA_ZAZEN:///当前为打坐状态应该切换为站立
		return ( GetCurActionID()!=EA_ZAZEN ) ;
		break;
	default:
		break;
	}
	return  0;
}


BOOL CPlayer::RecvAutoUseItem(SQAutoUseItemMsg* pMsg)
{
	//if (m_byLevel < 20) //45级以下不能自动吃药
	//{
	//	TalkToDnid(m_ClientIndex, "20级以下不能自动吃药！");
	//	return FALSE;
	//}
	//	BOOL oldbSetLZ = m_stAutoUseItemSet.bSetLZ;
	//	memcpy(&m_stAutoUseItemSet, pMsg, sizeof(SQAutoUseItemMsg));
	//	if ( m_stAutoUseItemSet.bSetLZ != oldbSetLZ )
	//		SendAddPlayerLevel(0, 0);
	return TRUE;
}

void CPlayer::UpdateUseItemTime()
{
}

bool CPlayer::CheckAndUpdateOnlineState()
{
	// 意外的数值
	if ((m_OnlineState > 3) || (m_OnlineState < 0))
		return false;

	int st = m_OnlineState;
	if (isAutoFight) 
		st = 4;
	g_AccOnlineCheck[st]++;

	if (m_OnlineState == OST_NOT_INIT) 
	{
		if (GetTickCount() - m_dLoginCheckTime > LOGINCHECKTIME) //登录延迟验证时间超时
		{
			SPlayerTempData tdata;
			memset(&tdata, 0, sizeof(tdata));

			// 可能是异常退出的情况，由于没有初始化，所以只需要做LOGOUT
			SendRefreshPlayerMessage(SARefreshPlayerMsg::ONLY_LOGOUT, GetGID(), m_szAccount.c_str(), &m_FixData, &tdata);
		
			CheckTimeWhenLogout();

			ClearClient(m_ClientIndex);

			m_ClientIndex = 0;

			RemoveFromCache(GetAccount());

			// 将自己设为无效，这将会被管理器清除
			m_bValid = false;

			RemoveAccountRelation(GetAccount());
			rfalse(2, 1, "LogOut 异常玩家 %s",GetAccount());
			// 通知登录服务器区域服务器登录完成,这里也以为登录处理，排除排队队列
			SQPlayerLoginComplte sPlayerLoginComplte;
			strcpy(sPlayerLoginComplte.account, GetAccount());
			GetGW()->SendMsgToLoginSrv(&sPlayerLoginComplte, sizeof(SQPlayerLoginComplte));
		
		}
	}

	//	if (m_Property.m_byEngineFlage)
	//		g_AccOnlineCheck[5]++;

	return true;
}

//----------------------------------------------------------------------------------------------------------	
bool CPlayer::CheckUseRose()
{
	if (isUseRose)
	{
		if((m_bProtect) && ((int)(timeGetTime() - m_ProRefTime) > 5000))
		{
			m_bProtect = false;
		}
	}
	else 
	{
		if((m_bProtect) && ((int)( timeGetTime() - m_ProRefTime) > 5000))
		{
			m_bProtect = false;
		}
	}
	// 如果设置了向帐号服务器提交消息，并且客户端在10秒内没有返回模块信息列表！则禁止该客户端的AI操作！！！
	if ((wgLimit & 8) && timeOutChk != 0)
	{
		if((int)( timeGetTime() - timeOutChk) > 10000)
		{
			return true;
		}
	}

	return false;
}

void CPlayer::Relive()
{
	// 复活的时候，设置成STAND状态
	SetCurActionID(EA_STAND);
	SetBackupActionID(EA_STAND);

	m_CurHp = m_MaxHp;
	m_CurMp	= m_MaxMp;

	m_FightPropertyStatus[XA_CUR_HP] = true;
	m_FightPropertyStatus[XA_CUR_MP] = true;
	if (0xff != m_PreFPIndex)
	{
		CallOutFightPet(m_PreFPIndex);
		m_PreFPIndex = 0xff;
	}
}

void CPlayer::SpecialProcessDeadState()
{
	if (EA_DEAD != GetCurActionID())
		return;

	MY_ASSERT(0 == m_CurHp);

	// 修改计数器
	if (m_DeadWaitTime > 0)
	{
		m_DeadWaitTime--;
		return;
	}

	// 重置计数器
	m_DeadWaitTime = MAX_DEADWAITTIME;

	TimeToRelive(1, 400, 400, SQPlayerDeadMsg::PDM_BACK_CITY);
}

void CPlayer::QuestToRelive(BYTE type)
{
	if (0 != m_CurHp)
		return;

	MY_ASSERT(EA_DEAD == GetCurActionID());

	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("PlayerQuestRelive"))
	{
		g_Script.PushParameter(type);
		g_Script.PushParameter(GetGID());
		g_Script.Execute();
	}
	g_Script.CleanPlayer();

// 	if (type > SQPlayerDeadMsg::PDM_HERE_FULLLIFE )return;
// 
// 	SAPlayerDeadResultMsg sResultMsg;
// 
// 	if (SQPlayerDeadMsg::PDM_BACK_CITY == type)
// 	{
// 		TimeToRelive(1, 622, 791, type);// 回城
// 	}		
// 	else if (SQPlayerDeadMsg::PDM_HERE_FREE == type)
// 	{
// 		DWORD wRelivet = wReliveTime[SQPlayerDeadMsg::PDM_HERE_FREE];
// 		wRelivet += m_wCurDeadCount > 2 ? (m_wCurDeadCount -2) * 30 : 0;
// 		wRelivet *= 10;//测试阶段暂时较短的时间复活
// 		if (timeGetTime() - m_DeadTime < wRelivet)
// 		{		
// 			TalkToDnid(m_ClientIndex,"还需要等待一段时间");
// 			sResultMsg.bResult = SAPlayerDeadResultMsg::RELIVE_FAIL_TIME;
// 			g_StoreMessage(m_ClientIndex,&sResultMsg,sizeof(SAPlayerDeadResultMsg));
// 			return;
// 		}
// 		TimeToRelive(m_Property.m_CurRegionID, m_curTileX, m_curTileY, type);// 原地
// 	}		
// 	else if (type > SQPlayerDeadMsg::PDM_HERE_FREE)
// 	{
// 		WORD wReliveM = wReliveMoney[type];
// 		wReliveM += m_wCurDeadCount > 2 ? (m_wCurDeadCount -2) * 1000 : 0;
// 		if (CheckPlayerMoney(XYD_UM_ONLYUNBIND,wReliveM,false)){
// 			TimeToRelive(m_Property.m_CurRegionID, m_curTileX, m_curTileY, type);// 原地
// 		}
// 		else{
// 			TalkToDnid(m_ClientIndex,"你的金钱不足");
// 			sResultMsg.bResult = SAPlayerDeadResultMsg::RELIVE_FAIL_MONEY;
// 			g_StoreMessage(m_ClientIndex,&sResultMsg,sizeof(SAPlayerDeadResultMsg));
// 			return;
// 		}	
// 	}			

	return;
}

void CPlayer::TimeToRelive(WORD regionID, WORD wX, WORD wY, WORD type)
{
	if (0 != m_CurHp)return;

	Relive();

	TalkToDnid(m_ClientIndex, "你刚刚复活了！");

	EnableProtect();

	m_wCheckID++;
	   
	if (CDynamicRegion *pDynamic = (CDynamicRegion *)m_ParentRegion->DynamicCast(IID_DYNAMICREGION))
	{
		PutPlayerIntoDestRegion(this, 0, wX, wY, pDynamic->GetGID());
	}
	else
	{
		PutPlayerIntoDestRegion(this, m_ParentRegion->m_wReLiveRegionID, m_ParentRegion->m_ptReLivePoint.x, m_ParentRegion->m_ptReLivePoint.y);
	}
		


// 	if (SQPlayerDeadMsg::PDM_HERE_FREE == type)
// 	{
// 		if (CDynamicRegion *pDynamic = (CDynamicRegion *)m_ParentRegion->DynamicCast(IID_DYNAMICREGION))
// 			PutPlayerIntoDestRegion(this, 0, wX, wY, pDynamic->GetGID());
// 		else
// 			PutPlayerIntoDestRegion(this, regionID, wX, wY);
// 
// 		//m_ParentRegion->m_wRegionID
// 		// 玩家复活触发脚本，如果有返回值就处理
// 		lite::Variant ret;
// 		g_Script.SetCondition(NULL, this, NULL, NULL);
// 
// 		LuaFunctor(g_Script, "OnPlayerRelive")(&ret);
// 		g_Script.CleanCondition();
// 	}
// 	else if (SQPlayerDeadMsg::PDM_HERE_PAY == type)
// 	{
// 		if (CDynamicRegion *pDynamic = (CDynamicRegion *)m_ParentRegion->DynamicCast(IID_DYNAMICREGION))
// 			PutPlayerIntoDestRegion(this, 0, wX, wY, pDynamic->GetGID());
// 		else
// 			PutPlayerIntoDestRegion(this, regionID, wX, wY);
// 		// 玩家复活触发脚本，如果有返回值就处理
// 		lite::Variant ret;
// 		g_Script.SetCondition(NULL, this, NULL, NULL);
// 
// 		LuaFunctor(g_Script, "OnPlayerRelive")(&ret);
// 		g_Script.CleanCondition();
// 
// 	}else if (SQPlayerDeadMsg::PDM_HERE_FULLLIFE == type)
// 	{
// 		if (CDynamicRegion *pDynamic = (CDynamicRegion *)m_ParentRegion->DynamicCast(IID_DYNAMICREGION))
// 			PutPlayerIntoDestRegion(this, 0, wX, wY, pDynamic->GetGID());
// 		else
// 			PutPlayerIntoDestRegion(this, regionID, wX, wY);
// 	}
// 	else
// 	{
// 		PutPlayerIntoDestRegion(this, regionID, wX, wY);
// 	}

	SAPlayerDeadResultMsg sResultMsg;
	sResultMsg.bResult = SAPlayerDeadResultMsg::RELIVE_SUCCESS;
	g_StoreMessage(m_ClientIndex,&sResultMsg,sizeof(SAPlayerDeadResultMsg));

	return;
}

//----------------------------------------------------------------------------------------------------------	
void CPlayer::SendTestMsg()
{
}

//----------------------------------------------------------------------------------------------------------	
void CPlayer::CheckStatisticsFlag()
{
	// 统计标志检查
	if(m_bCountFlag) {
		m_CountData.dwLevel = m_Property.m_Level;
		//		m_CountData.dwAG = m_Property.m_wAG;
		//m_CountData.dwEN = m_Property.m_wEN;
		m_CountData.dwExper = (DWORD)m_Property.m_Exp;
		//m_CountData.dwIN = m_Property.m_wIN;
		m_CountData.dwMoney = m_Property.m_Money;
		//m_CountData.dwST = m_Property.m_wST;
		//m_CountData.dwStoreMoney = m_Property.m_dwStoreMoney;
		memcpy(m_CountData.stSkill, m_Property.m_pSkills, sizeof(m_Property.m_pSkills));

		g_Count.UpdatePlayerDataCount(&m_CountData, m_Property.m_Name, g_Count.GetUpdateTime(CCount::CCT_PLAYERDATA)/*TIMECOUNT_HOURS*/);
	}
}

//----------------------------------------------------------------------------------------------------------	
void CPlayer::ProcessAutoUseItem()
{
	if ((GetCurActionID() == EA_DEAD) || (m_CurHp == 0))
		return;

	/* For XYD3
	//if (m_byLevel < 20) //20级以下不能自动吃药
	//	goto end;

	//	if ( m_stAutoUseItemSet.sSetHP.bUse && ( m_CurHp < m_stAutoUseItemSet.sSetHP.wPoint ) && ( m_nUseItemTimeHP == 0 ) )
	//		AutoUseItem( m_stAutoUseItemSet.sSetHP.wWhich );

	//	if ( m_stAutoUseItemSet.sSetMP.bUse && ( m_wMP < m_stAutoUseItemSet.sSetMP.wPoint ) && ( m_nUseItemTimeMP == 0 ) )
	//		AutoUseItem( m_stAutoUseItemSet.sSetMP.wWhich );

	//	if ( m_stAutoUseItemSet.sSetSP.bUse && ( m_wSP < m_stAutoUseItemSet.sSetSP.wPoint ) && ( m_nUseItemTimeSP == 0 ) )
	//		AutoUseItem( m_stAutoUseItemSet.sSetSP.wWhich );

	BOOL autoUseItemRuslet = -1;
	//	int oldUseItemTimeHPXQDx = m_nUseItemTimeHPXQDx;
	//	int oldUseItemTimeHPXQDd = m_nUseItemTimeHPXQDd;
	//	int oldUseItemTimeHPXQDt = m_nUseItemTimeHPXQDt;

	if (m_stAutoUseItemSet.bSetQXD == FALSE && m_Property.m_dwTempMaxHPByXQD != 0)//关掉气血丹后降低最大生命上限
	{
	m_Property.m_dwTempMaxHPByXQD = 0;
	SendAddPlayerLevel(0, 0);
	}
	if (m_stAutoUseItemSet.bSetQXD == TRUE && m_pItemXQD != NULL)//打开气血丹如果有已经使用的气血丹
	{
	if (m_pItemXQD->wIndex == 40138 && m_Property.m_dwTempMaxHPByXQD != 1000)
	{
	m_Property.m_dwTempMaxHPByXQD = 1000;
	SendAddPlayerLevel(0, 0);
	}
	else if (m_pItemXQD->wIndex == 40139 && m_Property.m_dwTempMaxHPByXQD != 2000)
	{
	m_Property.m_dwTempMaxHPByXQD = 2000;
	SendAddPlayerLevel(0, 0);
	}
	else if (m_pItemXQD->wIndex == 40381 && m_Property.m_dwTempMaxHPByXQD != 2500)
	{
	m_Property.m_dwTempMaxHPByXQD = 2500;
	SendAddPlayerLevel(0, 0);
	}
	}

	// 首先使用小血气
	if ( m_nUseItemTimeHPXQDx == 0 && m_stAutoUseItemSet.bSetQXD )
	CheckUseXQD( 40138, 100, 1000, autoUseItemRuslet );

	// 如果没有小血气
	if ( m_nUseItemTimeHPXQDd == 0 && autoUseItemRuslet == FALSE && m_stAutoUseItemSet.bSetQXD )
	CheckUseXQD( 40139, 200, 2000, autoUseItemRuslet );

	// 如果没有大血气
	if ( m_nUseItemTimeHPXQDt == 0 && autoUseItemRuslet == FALSE && m_stAutoUseItemSet.bSetQXD )
	CheckUseXQD( 40381, 200, 2000, autoUseItemRuslet );

	// 因该有却 没找到，锁定等使用失败的情况,3为没激活的
	if ( m_Property.m_dwTempMaxHPByXQD != 0 && (autoUseItemRuslet == FALSE || autoUseItemRuslet == 3 || m_pItemXQD == NULL ) )
	{
	if ( (oldUseItemTimeHPXQDx == 0 && m_Property.m_dwTempMaxHPByXQD == 1000 ) 
	|| (oldUseItemTimeHPXQDd == 0 && m_Property.m_dwTempMaxHPByXQD == 2000 ) 
	|| (oldUseItemTimeHPXQDt == 0 && m_Property.m_dwTempMaxHPByXQD == 2500 )  )
	{
	m_Property.m_dwTempMaxHPByXQD = 0;
	SendAddPlayerLevel(0, 0);
	}
	}

	for ( int i = 0; i < 6; ++i )
	{
	// 消耗灵珠武功修为
	if (  m_nUseItemTimeLZ[i] == 0 && GetAutoUseItemSetLZ() == TRUE )
	{     
	if ( m_pItemLZ[i] && m_pItemLZ[i]->wIndex == 40317 + i )              
	AutoUseItem( m_pItemLZ[i], 1 );           
	}
	}//*/
}

void CPlayer::OnRun()
{

	CSanguoPlayer::OnRun();



	//20150130 wk 因没初始化m_ParentRegion 导致下面运行不了,暂时不用m_ParentRegion
	/*
	if (!CheckAndUpdateOnlineState() || !m_ParentRegion || CheckDisWait() || !m_bInit)
	return;

	//SpecialProcessDeadState();
	if (m_IsUseKillMonster == 1 && m_ParentRegion->DynamicCast(IID_DYNAMICREGION) != NULL)	//测试用，用于时时杀死参加中的怪物
	{
	time_t t = time(NULL);
	if (m_pPreFightRegion != m_ParentRegion)
	{
	m_pPreFightRegion = m_ParentRegion;
	m_EntryScenceTime = t;
	pret = t + 10;
	}
	if (static_cast<int>(t - pret) > 1)
	{
	m_ParentRegion->KillMonster(this);
	pret = t;
	}
	else if (t - m_EntryScenceTime > 5)
	{
	m_ParentRegion->KillMonster(this);
	pret = t;
	}
	}

	*/
	if (!CheckAndUpdateOnlineState()  || CheckDisWait() || !m_bInit)
		return;

	// 执行时间检查
	CheckOnRunTime();

	//执行宠物魂力恢复检测
	CheckResumePetHunli();

	//add by ly 2014/4/28  处理玩家的每日签到操作
	//判断是否可以重置
// 	g_Script.SetCondition(NULL, this, NULL);
// 	if (g_Script.PrepareFunction("OnHandleSignInOpt"))
// 	{
// 		g_Script.PushParameter(0);
// 		g_Script.Execute();
// 	}
// 	g_Script.CleanCondition();

	// 流程调用
	CFightObject::OnRun();

}

void CPlayer::OnEveryDayManagerRun(int nIndex)
{
	g_Script.SetCondition( NULL, this, NULL );
	LuaFunctor( g_Script, "PlayerEveryDayRun" )[nIndex]();
	g_Script.CleanCondition();
}

BOOL CPlayer::CheckTrapTrigger()
{
	// 	if (m_ParentRegion == NULL) 
	// 		return FALSE;
	// 
	// 	DWORD tileMark = m_ParentRegion->GetTileMark( m_wCurX, m_wCurY );
	// 	WORD id = ( WORD )( tileMark >> 8 ) & 0xffff;
	// 
	// 	if ( m_PrevTrapID != id )
	// 	{
	// 		// 说明进入或者离开了陷阱
	// 		m_PrevTrapID = id;
	// 		if ( m_PrevTrapID != 0 )
	// 		{
	// 			//说明有陷阱存在，id即为陷阱编号
	// 			// 目前暂时写死在这里
	// 			OnTriggerActivated( m_ParentRegion->GetTrapTrigger( id ) );
	// 			return TRUE;
	// 		}
	// 	}

	return FALSE;
}

bool CPlayer::IsOnSportsArea(void)
{
	if (m_ParentRegion == NULL)
		return false;

	if(m_ParentRegion->DynamicCast(IID_SPORTSAREA))
		return true;

	return false;
}

void CPlayer::UpdateTempItemToClient(BOOL nullUpdate)
{
}

void CPlayer::OnClearTempItem()
{
	// 	DWORD dorpCount = 0;
	// 	for(std::list<tempAddItem>::iterator it = m_tempItem.begin(); it != m_tempItem.end();)
	// 	{
	// 		std::list<tempAddItem>::iterator tit = it++;
	// 		SItemBase *bItem = (SItemBase*)tit->itemBuf;
	// 		if( m_ParentRegion )
	// 		{
	// 			POINT pos;
	// 			DWORD count = 1;
	// 			pos.x = m_wCurX; pos.y = m_wCurY; // [注意]后面要重新计算位置...
	// 			DropToGround( *( (SRawItemBuffer*)bItem ), 0xffffffff, (DWORD)0, pos, m_ParentRegion, dorpCount );
	// 			// 通知玩家添加到包裹失败，并掉落到地上 ...
	// 			TalkToDnid( m_ClientIndex, "你的临时物品将全部自动掉落到地上！！！" );
	// 			m_tempItem.erase( tit );
	// 		}
	// 		else
	// 			;// 异常
	// 	}
	// 
	// 	m_tempItem.clear();
	// 	UpdateTempItemToClient();
}

void CPlayer::CheckOnRunTime()
{
	if (m_OnlineState == OST_NOT_INIT)
	{
		rfalse(2, 1, "异常玩家[%s] CheckOnRunTime",GetAccount());
	}

	DWORD dwCurTick = timeGetTime();
	DWORD dt = dwCurTick - m_checkTick;
	m_checkTick = dwCurTick;


	// 更新每5秒的点击指令检查数据
	clickLimitChecker.Update();

	
		//20150130 wk 三国没有地图,检测中有参数没有会打印错误导致超时,注释掉
	// 检查跑地图任务信息   
	//UpdateMapTaskFlag();
	

	UpDataCycData();
	SendPropertiesUpdate();

	//5分钟间隔
	if (abs((int)dwCurTick - (int)m_dwDecDurTime) > 1000*60*5 ) //先写一秒测试
	{
		m_dwDecDurTime = dwCurTick;
		//备份玩家脚本数据到c
		g_Script.SetPlayer(this);
		g_Script.CallFunc("SI_savePlayerData", this->GetSID());
		g_Script.CleanPlayer();
	}
	// 1秒的间隔
	if (abs((int)dwCurTick - (int)m_dwOneSecondTick) > 1000)
	{
		m_dwOneSecondTick = dwCurTick;

		// 组队自动拒绝机制
		if (m_TeamGetIn.size())
		{
			CPlayer::TeamQuestList tempCopy(m_TeamGetIn);

			for (CPlayer::TeamQuestList::iterator it = tempCopy.begin(); it != tempCopy.end(); ++it)
			{
				if (timeGetTime() - it->second.first < 20000)
					continue;

				CPlayer *pQuest = (CPlayer *)GetPlayerByGID(it->first)->DynamicCast(IID_PLAYER);
				if (pQuest)
				{
					// 直接拒绝
					SQTeamJoinMsgG rejectMsg;
					rejectMsg.bResult	= SQTeamJoinMsgG::QTJ_REJECT;
					rejectMsg.type		= it->second.second;
					rejectMsg.dwGID		= it->first;

					OnRecvJoinMsg(m_ClientIndex, &rejectMsg, this, true);
				}
				else
				{
					MY_ASSERT(0);
					m_TeamGetIn.erase(it->first);			// 找不到玩家，也删除其索引
				}
			}
		}

		// 交易自动拒绝机制
// 		if (m_ChxGetIn.size())
// 		{
// 			CPlayer::CHXQuestList tempCopy(m_ChxGetIn);
// 
// 			for (CPlayer::CHXQuestList::iterator it = tempCopy.begin(); it != tempCopy.end(); ++it)
// 			{
// 				if (timeGetTime() - it->second < 20000)
// 					continue;
// 
// 				CPlayer *pQuest = (CPlayer *)GetPlayerByGID(it->first)->DynamicCast(IID_PLAYER);
// 				if (pQuest)
// 				{
// 					MY_ASSERT(pQuest->m_CurHp);
// 
// 					// 直接拒绝
// 					SQQuestExchangeMsg rejectMsg;
// 					rejectMsg.bAccept    = false;
// 					rejectMsg.dnidClient = 0;
// 					rejectMsg.dwDestGID  = GetGID();
// 					rejectMsg.dwSrcGID   = it->first;
// 
// 					RecvQuestExchangeMsg(&rejectMsg, true);
// 				}
// 				else
// 				{
// 					MY_ASSERT(0);
// 					m_ChxGetIn.erase(it->first);
// 				}
// 			}
// 		}
// 
// 		// 判断交易的距离
// 		if (InExchange())
// 		{
// 			CPlayer *pDest = (CPlayer *)GetPlayerByGID(m_dwOtherPlayerGID)->DynamicCast(IID_PLAYER);
// 			if (pDest)
// 			{
// 				DWORD distance = sqrt(pow((m_curX-pDest->m_curX),2)+pow((m_curY-pDest->m_curY),2));
// 				if (distance > 500)
// 				{
// 					SQExchangeOperationMsg cancelEx;
// 					cancelEx.operation	= SQExchangeOperationMsg::CANCEL;
// 					cancelEx.dwSrcGID	= m_dwSrcPlayerGID;
// 					cancelEx.dwDestGID	= m_dwDestPlayerGID;
// 					RecvExchangeOperationMsg(&cancelEx);
// 				}
// 			}
// 		}
	}

	/* 手游版本|不检测NPC距离的问题
	// 如果距离超出范围，则NPC关联取消
	if (m_CommNpcGlobalID)
	{
	MY_ASSERT(m_CurHp);
	CNpc *curNpc = GetCurrentNpc();
	if (curNpc)
	{
	DWORD distance = sqrt(pow((m_curX-curNpc->m_curX),2)+pow((m_curY-curNpc->m_curY),2));
	if (distance > 500)
	{
	m_CommNpcGlobalID = 0;
	temporaryVerifier = 0;
	}
	}
	else
	{
	m_CommNpcGlobalID = 0;
	temporaryVerifier = 0;
	}
	}
	*/

	// 查看摆摊自动关闭
	if (0 != dwOtherGID)
	{
		MY_ASSERT(m_CurHp);
		CPlayer *pDestPlayer = (CPlayer *)GetPlayerByGID(dwOtherGID)->DynamicCast(IID_PLAYER);
		if (pDestPlayer)
		{
			// 如果超出距离，那么关闭
			DWORD distance = sqrt(pow((m_curX-pDestPlayer->m_curX),2)+pow((m_curY-pDestPlayer->m_curY),2));
			if (distance > 500)
			{
				pDestPlayer->DelCurPlayerFromList(GetGID());
				dwOtherGID = 0;
			}
		}
	}

	// 查看包裹自动关系
	if (m_pCheckItem)
	{
		float itemX = m_pCheckItem->m_xTile << TILE_BITW;
		float itemY = m_pCheckItem->m_yTile << TILE_BITH;

		DWORD distance = sqrt(pow((m_curX-itemX),2)+pow((m_curY-itemY),2));
		if (distance > 500)
			CloseGroundItem();
	}

	// 实时监测取消监听对象
	if (m_pAttribListener && m_pAttribListener->GetSubject()->isValid())
	{
		if ((char *)m_ParentRegion != (char *)m_pAttribListener->GetSubject()->m_ParentRegion ||
			abs(m_ParentArea->m_X-m_pAttribListener->GetSubject()->m_ParentArea->m_X) >= 2 ||
			abs(m_ParentArea->m_Y-m_pAttribListener->GetSubject()->m_ParentArea->m_Y) >= 2)
		{
			// 不在一张地图上或超出了同步范围
			ProcessForceQusetEnemyInfo();
		}
	}

	// 打孔的间隔时间
// 	if (0xffffffff != m_MakeHoleTime && dwCurTick - m_MakeHoleTime > 1000)
// 	{
// 		StartMakeHole();
// 		m_MakeHoleTime = 0xffffffff;
// 	}

	// 跳跃的判断
	if (m_StartJumpTime && (dwCurTick - m_StartJumpTime > m_LastingTime))
	{
		m_StartJumpTime = m_LastingTime = 0;
		rfalse(2, 1, "跳跃结束~~~~");
	}

	// 生命、内力、体力自动回复处理
	__OnCheckAutoRestore(dwCurTick);
	///根据在线好友的多少定时送经验
	OnCheckFriendNum(dwCurTick);
	//根据在线时间获取激励值
	//GetImpelValueOntime(dwCurTick);
	OnTimeLimitTask(); //任务时间检查
	//UpDataPlayerRdTaskInfo();//检查任务是否超过一天，随机出现的师门任务
	//OnCheckNameColor();//杀孽值减少
	// 20150130 wk 注释掉
	//_L_CheckOnRunTime();//玩家在线时的脚本回调
	//OnProcessDead();//处理玩家死亡
	//OnCheckTimeLimit(dwCurTick);//防沉迷检测
	return;
}

// void CPlayer::CheckRunOnTime(void)
// {
// 	if ( m_ParentRegion == NULL )
// 		return;
// 	
// 	BOOL    bChange = FALSE;
// 	DWORD   dwCurTick = timeGetTime();
// 	DWORD	dt = dwCurTick - m_checkTick;	//计算从上一帧到这一帧经过了多少时间
// 	m_checkTick = dwCurTick;
// 	// ===== 角色身上的物品冷却时间计算
// 	int ckNumber = (int)m_tempItem.size();
// 	for( std::list<tempAddItem>::iterator it = m_tempItem.begin(); it != m_tempItem.end(); )
// 	{
// 		std::list<tempAddItem>::iterator dit = it ++;
// 		// 冷却时间到了
// 		int times = dit->dwItemCoolingTime;
// 	}
// 
// 	if ( ckNumber != m_tempItem.size() )
// 		UpdateTempItemToClient( true );
// 
// 
// 
// 	// 因为只有站立不动的时候才能回血
// 	// 结果策划说随时都可以回血
// 	if (GetCurActionID() == EA_DEAD)
// 	{
// 
// 		m_dwSecondTick = timeGetTime();
// 
// 		// 1秒的间隔
// 		if ( abs( (int)dwCurTick - (int)m_dwOneSecondTick ) > 1000 )
// 		{
// 			m_dwOneSecondTick = timeGetTime();
// 		}
// 
// 		return;
// 	}
// 
// 	if ( (int)(timeGetTime() - m_dwSaveTime) > 0 )
// 	{
// 		m_dwSaveTime = timeGetTime() + AUTOSAVETIME;
// 
// 		if (CWareHouse::m_dwOperationSerial == 0)
// 		{
// 			// 只有在非仓库状态才能及时保存数据
// 			SendData(SARefreshPlayerMsg::BACKUP_AND_SAVE);
// 		}
// 	}
// 
// 	if ( (int)(timeGetTime() - m_dwOneHourTime) > 0 )
// 	{
// 		m_dwOneHourTime = timeGetTime() + 1000 * 60 * 60;
// 		UpdateFriends(m_ClientIndex);
// 	}
// 
// 	if ( (int)(timeGetTime() - m_dwHalfHourTime) > 0 )
// 	{
// 		m_dwHalfHourTime = timeGetTime() + 1000 * 60 * 30;
// 
// 		bChange = false;
// 
// // 		if (m_Property.m_sXValue != 0)
// // 		{
// // 			if ((m_Property.m_sXValue >= 1) && (m_Property.m_sXValue < 150))
// // 			{
// // 				bChange = true;
// // 				m_Property.m_sXValue ++;
// // 			}
// // 			else if ((m_Property.m_sXValue > -150) && (m_Property.m_sXValue <= -1))
// // 			{
// // 				bChange = true;
// // 				m_Property.m_sXValue --;
// // 			}
// // 		}
// 
// // 		if ( m_Property.m_byPKValue > 0 /*&& m_OnlineState == OST_NORMAL*/ )  // 只有在线才会减少
// // 		{
// // 			bChange = true;
// // 			m_Property.m_byPKValue --;
// // 			CheckAndSendHellioMaskBUFF();
// // 		}
// 
// // 		if (bChange)
// // 		{
// // 			// 广播自己的狭义值
// // 			SAXVauleChangeMsg msg;
// // 			msg.dwGlobalID = GetGID();
// // 			msg.sXvalue = m_Property.m_sXValue;
// // 			msg.byPKValue = m_Property.m_byPKValue;
// // 
// // 			if (m_ParentArea != NULL)
// // 				m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
// // 		}
// 
// 		// 如果在监狱中，PK清到了4，就自动放出去
// // 		if ( ( m_ParentRegion != NULL ) && ( m_ParentRegion->m_wRegionID == AUTO_PRISON_ID ) )
// // 			if ( m_Property.m_byPKValue <= 4 )
// // 				PutPlayerIntoDestRegion( this, 1, 100, 100 );
// 	}
// 
// 	// 道具检测
// //	__int64 r = recycleTime;
// //	__int64 now = time( NULL );
// //	if ( ( recycleTime != 0xffffffff ) && ( r - now ) <= 0 )
// //		UpdateTimeRecycleItem();
// 
// 	// 1秒的间隔
// 	if ( abs( (int)dwCurTick - (int)m_dwOneSecondTick ) > 1000 )
// 	{
// 		m_dwOneSecondTick = timeGetTime();
// 
// 		//用状态检测方式，因需求有变化，暂时注释掉 20100127_lm
// 		//if (m_bCheckerStateFlag = TRUE)
// 		//{
// 			//if (EA_USEITEM != GetCurActionID())
// 			//{	
// 			//	if ( (EA_DEAD == GetCurActionID()) || (EA_WALK == GetCurActionID()) || (EA_RUN == GetCurActionID()) || (EA_JUMP == GetCurActionID()) \
// 			//		|| (EA_ATTACK  == GetCurActionID()) || (EA_SKILL_ATTACK  == GetCurActionID()) )
// 			//	{
// 			//		CheckBreak(TRUE);
// 			//	}
// 			//	else
// 			//	{
// 			//		CheckBreak();
// 			//	}
// 			//}
// 		//			
// 		//}
// 		//利用这个1秒检查物品使用预处理时间
// 		//CheckPreTime();
// 				
// 	}
// 
// 	// 3秒的间隔
// 	//     if ( abs( ( int )dwCurTick - ( int )m_dwHellionBuffTick ) >= 3000 )
// 	//     {
// 	//         m_dwHellionBuffTick = timeGetTime();
// 	//         DWORD value = 0;
// 	//         if ( m_dwTempState & GetESBit( EXTRASTATE_HELLION_MARK ) ) // 有恶人印记
// 	//             value = 2000; // 减2000血
// 	//         else if ( m_dwTempState & GetESBit( EXTRASTATE_MAX_HELLION_MARK ) )  // 有大恶人印记
// 	//             value = 3000; // 减3000血
// 	// 
// 	//         DWORD oldHP = m_CurHp;
// 	//         if ( ( INT64 )m_CurHp - value <= 1000 )
// 	//         {
// 	//             if ( m_CurHp > 1000 ) m_CurHp = 1000;
// 	//         }
// 	//         else
// 	//             m_CurHp -= value;
// 	// 
// 	//     }
// 
// 	if ( GetCurActionID() != EA_ZAZEN )  // 站立状态下才会自动增长剑气
// 	{
// 		if ( MAX_MONSTER_ONGOAT <= m_wMonsterCountOnGoat )
// 			m_dwAutoAddMCountTime = m_dwOneSecondTick;
// 		// 1.2分钟间隔
// 		if ( abs( (int)m_dwAutoAddMCountTime - (int)m_dwOneSecondTick ) > (AUTO_ADDMC_ONGOAT * 1000 * 100) / MAX_MONSTER_ONGOAT )
// 		{
// 			m_dwAutoAddMCountTime = timeGetTime();
// 		}
// 	}
// 	else
// 		m_dwAutoAddMCountTime = m_dwOneSecondTick;  // 保证切换后时间不会累积
// 
// 	// 刷新医德附加过程持续时间
// 	if ( m_wDoctorBuffTime > 0 )
// 	{
// // 		CPlayer* pDoctor = ( CPlayer* )GetPlayerByGID( m_dwDoctorGID )->DynamicCast( IID_PLAYER );
// // 		if ( pDoctor == NULL || GetCurActionID() != EA_ZAZEN || pDoctor->GetCurActionID() != EA_ZAZEN ) // 不准站起来哟 - -
// // 		{
// // 			m_wDoctorBuffTime = 0;
// // 			// 清除客户端状态( 加BUFF、被加BUFF )
// // 			m_dwExtraState &= ~SHOWEXTRASTATE_BEADDDOCTORBUFF;
// // 			SendExtraState();
// // 			if ( pDoctor  ) pDoctor->m_dwExtraState &= ~SHOWEXTRASTATE_ADDDOCTORBUFF;
// // 			if ( pDoctor  ) pDoctor->SendExtraState();
// // 		}
// // 		else if ( abs( ( int )dwCurTick - ( int )m_dwDoctorBuffTick ) > ( m_wDoctorBuffTime * 1000 ) ) // m_wDoctorBuffTime这么多时间的间隔
// // 		{
// // 			m_dwDoctorBuffTick = timeGetTime();
// // 			m_wDoctorBuffTime = 0;
// // 			DoAddDoctorBuff( pDoctor, false );  // 执行BUFF附加
// // 		}
// 	}
// 
// 	// zeb 2009-12-8
// 	// 自动回复计算
// 	__OnCheckAutoRestore(dwCurTick);
// 	// zeb 2009-12-8
// 
// 	// 检查是否需要同步锁定目标
// 	//__OnCheckSyncTarget(); dc. 2010-1-27
// 
// 		
// 
// 	if( (int)(dwCurTick-m_dwSecondTick) >= 10000 )     // 十秒一次
// 	{
// 		m_dwSecondTick += 10000;
// 
// 		// zeb 2009-12-8
// 		//if(IsOnSportsArea() == false)    // 不在竞技场才能恢复
// 		//{
// 		//	bChange = false;
// 
// 		//	// 每10秒一次长血
// 		//	if ( m_CurHp < m_dwMaxHP )
// 		//	{
// 		//		ModifyCurrentHP(GetAddHPSpeed());
// 
// 		//		if (GetCurActionID() == EA_ZAZEN) 
// 		//			ModifyCurrentHP(GetAddHPSpeed());
// 
// 		//		bChange = true;
// 		//	}
// 
// 		//	if ( !m_tvLimitUMP.Validate() && ( m_wMP<m_wMaxMP ) )
// 		//	{
// 		//		ModifyCurrentMP(GetAddMPSpeed());
// 
// 		//		if (GetCurActionID() == EA_ZAZEN) 
// 		//			ModifyCurrentMP(GetAddMPSpeed());
// 
// 		//		bChange = true;
// 		//	}
// 
// 		//	if ( !m_tvLimitUSP.Validate() && ( m_wSP < m_wMaxSP ) )
// 		//	{
// 		//		ModifyCurrentSP(GetAddSPSpeed());
// 
// 		//		if (GetCurActionID() == EA_ZAZEN) 
// 		//			ModifyCurrentSP(GetAddSPSpeed());
// 
// 		//		bChange = true;
// 		//	}
// 
// 		//	if( IsMonsterMutate() && !IsGoatMutate() && !IsEspMutate() )  
// 		//	{
// 		//		// 考虑在替身变身中不扣体力
// 		//		if( m_wSP<50 )  m_wSP = 0;
// 		//		else            m_wSP -= 50;
// 
// 		//		if( m_wSP==0 )   
// 		//		{
// 		//			if (IsNewModel())
// 		//			{
// 		//				SetMutate( 0, 0 );
// 		//				SetMutate( 5, (m_Property.m_Sex == 0) ? 99 : 98);
// 		//			}
// 		//			else
// 		//				SetMutate( 0, 0 );
// 		//		}
// 
// 		//		bChange = true;
// 		//	}
// 
// 		//}
// 		// zeb 2009-12-8
// 
// 		// 更新多倍得经验、金钱时间
// 		if ( multipletime.m_byMultipleTime > 0 )
// 			UpdateMultipleTime();
// 
// 		// 10秒更新一次
// 		if ( m_wDoctorBuffTime == 0 && ( m_wDoctorDefBuff > 0 || m_wDoctorDamBuff > 0 || m_wDoctorPowBuff > 0 || m_wDoctorAgiBuff > 0 ) )
// 			UpdateDoctorBuffTime();
// 
// 		if ( m_wWhineBUFFType > 0)
// 			UpdateWhineBuffTime();
// 
// 		//十秒检查一下物品变化
// 		CheckItemChange();
// 
// #ifdef TimeCompress
// 
// 	}
// 
// 
// 
// 	// 这里改为时间压缩60倍，就只有每秒近来检查了
// 	if ( m_Property.m_pTroopsSkills[2].dwProficiency != time( NULL ) ) 
// 	{
// 		m_Property.m_pTroopsSkills[2].dwProficiency = time( NULL );
// 
// 		if ( limitedState && m_OnlineState == OST_NORMAL )
// 		{
// 			m_OnlineTime += 60; // 1秒进一次， 一次加60秒， 压缩60倍
// 
// 			if ( ( m_OnlineTime % ( 15*60 ) ) <= 59 ) // ...
// 			{
// 
// #else
// 
// 		if ( limitedState && m_OnlineState == OST_NORMAL )
// 		{
// 			m_OnlineTime += 10; // 10秒进一次， 一次加10秒， 正常速度
// 
// 			if ( ( m_OnlineTime % ( 15*60 ) ) <= 9 ) // 因为为10的步长, 所以只需要判断是否余数小于等于9就可以了
// 			{
// 
// #endif
// 				// 现在防沉迷系统属于每在线时间每累加满15分钟就向账号服务器发起一个状态检测, 
// 				// 等返回值确认后, 再设置状态和显示提示
// 				SQUpdateTimeLimit msg;
// 				try {
// 					lite::Serializer slm( msg.streamBuffer, sizeof( msg.streamBuffer ) );
// 					slm( GetAccount() );
// 					SendToLoginServer( &msg, long(sizeof( msg ) - slm.EndEdition()) );
// 				}
// 				catch ( lite::Xcpt& )
// 				{
// 				}
// 			}
// 		}
// 	}
// 
// 	if ( m_ParentRegion->m_wSkillAccess == 1 || m_ParentRegion->m_wSkillAccess == 3 )
// 		return;
// 
// 	extern DWORD timeTick32;
// 	DWORD curMinute = timeTick32 / 60;
// 	// 注意, 这个地方修改为按 19xx年开始以秒计算的一个值, 除60就是分钟
// 	// 如果分钟数不一致,就说明已经过分钟了,就该执行操作了
// 	if ( m_dwMinuteTick != curMinute ) 
// 	{
// 		// 增长在线时间
// 		if ( m_OnlineState != CPlayer::OST_HANGUP  )
// 			m_Property.m_OnlineTime ++;
// 
// 		m_dwMinuteTick = curMinute;
// 
// 		// 一分钟一次长心法
// 		int add =/* m_Property.m_byTelergyStep*/ + 0;
// 
// 		// 打坐的状态下翻倍
// 		if (GetCurActionID() == EA_ZAZEN)
// 			add *= 2;
// 
// 		// 闭关的情况下再翻倍
// 		// 但是挂机的情况下不行
// 		if (m_OnlineState != OST_HANGUP)
// 			//if (m_ParentRegion->DynamicCast(IID_BIGUANAREA) != NULL)
// 			add *= m_ParentRegion->GetExperMul();
// 		//挂机心法，速度4倍
// // 		if (m_bIsStarPractice && m_Property.stPracBox[0].wType == TYPE_TELSKILL)
// // 		{
// // 			add *= 4;
// // 			m_dwSkillProNum +=add;
// // 		}
// 		AddTelergy( add, "定时增长" );
// 
// 
//         // 每分钟扣一次持久
//         // 检查当前装备的道具是否存在时间道具
//         //bool bchange = false;
//         //for ( int i = 0; i < 16; i++ )
//         //{
//         //    if ( m_Equip[i].prefix == 0 )
//         //        continue;
// 
//         //    if (  m_Equip[i].prefix == 1 && CheckBindEquip( this, &m_Equip[i], 0  ) )
//         //    {
//         //        SAEquipInfoMsg msg;
//         //        msg.stEquip = m_Equip[i];
//         //        msg.byPos = i;
//         //        g_StoreMessage( m_ClientIndex, &msg, sizeof( msg ) );
//         //    }
//         //    else if ( m_Equip[i].prefix == 2 && CheckBindEquip( this, &m_Equip[i], 0  )  )
//         //    {
//         //        SAMoveItemMsg msg;
//         //        msg.wWhat = 11; 
//         //        msg.wIndex = i;
//         //        g_StoreMessage( m_ClientIndex, &msg, sizeof( msg ) );
//         //        ZeroMemory( &m_Equip[i], sizeof( m_Equip[i] ) );
//         //        ZeroMemory( &m_Property.m_Equip[i], sizeof( m_Property.m_Equip[i] ) );
//         //        bchange = true;
// 
//         //        if ( i == 15 )
//         //        {
//         //            SAEquipItemMsg  EquipItemMsg;
//         //            EquipItemMsg.dwGlobalID = GetGID();
//         //            EquipItemMsg.byEquipPos = i;
//         //            EquipItemMsg.wItemIndex = 0;
// 
//         //            // 将消息发给有效区域的所有玩家
//         //            if ( CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA) )
//         //                pArea->SendAdj( &EquipItemMsg, sizeof(SAEquipItemMsg), -1 );
//         //        }
//         //    }
//         //}
//         //            
//         //if ( bchange )
//         //    CheckEquip();
//     }
// 
//     // 先更新状态
// //     if ( m_Property.dwDecDurTimeEnd != 0 &&( (DWORD)time(NULL) > m_Property.dwDecDurTimeEnd )  )
// //         m_Property.dwDecDurTimeEnd = 0;
// 
//     // 5分钟执行一次
// //    if ( m_OnlineState == CPlayer::OST_NORMAL && m_Property.dwDecDurTimeEnd == 0 && abs( (int)timeGetTime() - (int)m_dwDecDurTime ) > 1000 * 60 * 5 )
//   //  {
//   //      m_dwDecDurTime = timeGetTime();
// //		bool eqDurEmpty = false;
// 
//   //      for ( int i = 0; i < 16; ++i )
// 		//{
// 		//	if ( m_Equip[ i ].wIndex != 0 )
// 		//	{
// 		//		const SItemData *itemData = CItem::GetItemData( m_Equip[ i ].wIndex );
// 
// 		//		// zeb 2009.11.19
// 		//		//if ( itemData && itemData->wWearPrime != 0xffff )
// 		//		//{
// 		//		//	SEquipment::Attribute &eqAtt = m_Equip[ i ].attribute;
// 		//		//	WORD oldDuranceCur = eqAtt.curWear;
// 		//		//	WORD oldDuranceMax = eqAtt.maxWear;     
// 
// 		//		//	if ( eqAtt.maxWear > 0 && eqAtt.curWear > 0 )
// 		//		//		eqAtt.curWear--;
// 
// 		//		//	if ( eqAtt.maxWear != 0xffff && eqAtt.maxWear > itemData->wWearPrime )
// 		//		//		eqAtt.maxWear = itemData->wWearPrime;
// 
// 		//		//	if ( eqAtt.curWear != 0xffff && eqAtt.curWear > itemData->wWearPrime )
// 		//		//		eqAtt.curWear = 0;
// 
// 		//		//	if ( eqAtt.curWear > eqAtt.maxWear )
// 		//		//		eqAtt.curWear = eqAtt.maxWear;
// 
// 		//		//	if ( (oldDuranceCur != eqAtt.curWear && eqAtt.curWear % 10 == 0 ) || oldDuranceMax != eqAtt.maxWear )
// 		//		//	{
// 		//		//		SAUpdateCurEqDuranceMsg updateWDurmsg;
// 		//		//		updateWDurmsg.wEqIndex = m_Equip[ i ].wIndex;
// 		//		//		updateWDurmsg.byPos = i;
// 		//		//		updateWDurmsg.wCurEqDuranceMax = eqAtt.maxWear;
// 		//		//		updateWDurmsg.wCurEqDuranceCur = eqAtt.curWear;
// 		//		//		g_StoreMessage( m_ClientIndex, &updateWDurmsg, sizeof( updateWDurmsg ) );
// 
// 		//		//		if ( eqAtt.curWear == 0 || eqAtt.maxWear == 0 )
// 		//		//			eqDurEmpty = true;
// 		//		//	}
// 		//		//}
// 		//		// zeb 2009.11.19
// 		//	}
// 		//}
// 
//         // 每分钟检查一次
//         // 检查当前装备的道具是否存在时间道具
//         //bool bchange = false;
//         //for ( int i = 0; i < 16; i++ )
//         //{
//         //    if ( m_Equip[i].prefix == 0 )
//         //        continue;
//         //    if (  m_Equip[i].prefix == 1 && CheckBindEquip( this, &m_Equip[i], 0  ) )
//         //    {
//         //        SAEquipInfoMsg msg;
//         //        msg.stEquip = m_Equip[i];
//         //        msg.byPos = i;
//         //        g_StoreMessage( m_ClientIndex, &msg, sizeof( msg ) );
//         //    }
//         //    else if ( m_Equip[i].prefix == 2 && CheckBindEquip( this, &m_Equip[i], 0  )  )
//         //    {
//         //        SAMoveItemMsg msg;
//         //        msg.wWhat = 11; 
//         //        msg.wIndex = i;
//         //        g_StoreMessage( m_ClientIndex, &msg, sizeof( msg ) );
//         //        ZeroMemory( &m_Equip[i], sizeof( m_Equip[i] ) );
//         //        ZeroMemory( &m_Property.m_Equip[i], sizeof( m_Property.m_Equip[i] ) );
//         //        bchange = true;
//         //        if ( i == 15 )
//         //        {
//         //            SAEquipItemMsg  EquipItemMsg;
//         //            EquipItemMsg.dwGlobalID = GetGID();
//         //            EquipItemMsg.byEquipPos = i;
//         //            EquipItemMsg.wItemIndex = 0;
//         //            // 将消息发给有效区域的所有玩家
//         //            if ( CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA) )
//         //                pArea->SendAdj( &EquipItemMsg, sizeof(SAEquipItemMsg), -1 );
//         //    }
//         //}
// 		//}
//         //            
//         //if ( bchange )
//         //    CheckEquip();
// //	}
// 
// 
// 	
// 	//1分钟一次
// 	//坐骑饥饿度，暂时改为每10秒减一次
// 	if( abs( (int)(dwCurTick-m_dwMountTimer)) >= MOUNT_HUNGER_TIME )
// 	{
// // 		m_dwMountTimer = timeGetTime();
// // 		//马的饥饿度
// // 		for ( int i = 0; i < SPlayerMounts::MAX_MOUNTS; i++)
// // 		{
// // 			SPlayerMounts::Mounts*	pMount = GetMounts( i);
// // 			if(pMount->ID == 0) break;
// // 			Mounts::CMountTemplates::_QualityAddition addition;
// // 			INSTANCE(Mounts::CMountTemplates)->GetMountsQualityAddition(addition,*pMount);
// // 			if ( pMount->State & SPlayerMounts::State_Equip
// // 				|| pMount->State & SPlayerMounts::State_Riding)
// // 			{
// // 				pMount->UpdateTimer++;
// // 				if ( pMount->UpdateTimer >= addition.UpdateTimer)
// // 				{
// // 					pMount->UpdateTimer = 0;
// // 					pMount->Hunger -= addition.DeHunger;
// // 					if ( pMount->Hunger < 0.0f)
// // 						pMount->Hunger = 0.0f;
// // 					SASetMountHunger shunger;
// // 					shunger.showTips = 0;
// // 					shunger.index = i;
// // 					shunger.hunger = (int)( pMount->Hunger + 0.5);
// // 					g_StoreMessage( this->m_ClientIndex, &shunger, sizeof(SASetMountHunger));
// // 				}
// // 				if ( pMount->Hunger == 0.0f)
// // 				{
// // //*					UnEquipMount();*/
// // 				}
// // 			}
// // 			else if ( pMount->State == SPlayerMounts::State_None)
// // 			{
// // 				pMount->UpdateTimer++;
// // 				if ( pMount->UpdateTimer >= addition.UpdateTimer)
// // 				{
// // 					pMount->UpdateTimer = 0;
// // 					pMount->Hunger += addition.DeHunger;
// // 					if ( pMount->Hunger > 100.0f)
// // 						pMount->Hunger = 100.0f;
// // 
// // 					SASetMountHunger shunger;
// // 					shunger.showTips = 0;
// // 					shunger.index = i;
// // 					shunger.hunger = (int)( pMount->Hunger + 0.5);
// // 					g_StoreMessage( this->m_ClientIndex, &shunger, sizeof(SASetMountHunger));
// // 				}
// // 			}
// // 		}
// 	}
// 	// 时间判断部分在这个函数内部
// 	UpdateExtraTelerge();
// 
// }

bool CPlayer::__OnCheckHPAutoRestore(DWORD dwCurTick)
{
	// 打坐5秒间隔,站立10秒间隔
	DWORD RecoverInterval = (EA_STAND == GetCurActionID()) ? m_HpRSInterval : m_HpRSInterval / 2;

	if (dwCurTick < m_dwLastHPResTime + RecoverInterval)
		return false;

	m_dwLastHPResTime = dwCurTick;

	if (m_CurHp >= m_MaxHp)
		return false;

	// 每次最大生命的3%
	ModifyCurrentHP(m_MaxHp * 0.03, 0, this);

	return true;
}

bool CPlayer::__OnCheckMPAutoRestore(DWORD dwCurTick)
{
	// 打坐5秒间隔,站立10秒间隔
	DWORD RecoverInterval = (EA_STAND == GetCurActionID()) ? m_MpRSInterval : m_MpRSInterval / 2;

	if (dwCurTick < m_dwLastMPResTime + RecoverInterval)
		return false;

	m_dwLastMPResTime = dwCurTick;

	if (m_CurMp >= m_MaxMp)
		return false;

	ModifyCurrentMP(m_MaxMp * 0.03, 0);

	return true;
}

bool CPlayer::__OnCheckTPAutoRestore(DWORD dwCurTick)
{
	// 打坐5秒间隔,站立10秒间隔
	DWORD RecoverInterval = (EA_STAND == GetCurActionID()) ? m_TpRSInterval : m_TpRSInterval / 2;

	if (dwCurTick < m_dwLastTPResTime + RecoverInterval)
		return false;

	m_dwLastTPResTime = dwCurTick;

	if (m_CurTp >= m_MaxTp)
		return false;

	// 每次恢复量 10 点
	ModifyCurrentTP(5, 0);

	return true;
}

bool CPlayer::__OnCheckSPAutoRestore(DWORD dwCurTick)
{
	if (0 == m_CurHp || m_IsInFight)
		return false;

	if (EA_ZAZEN == GetCurActionID())
	{
		if (dwCurTick-m_dwLastSPResTime>PLAYER_SP_RESTIME *1000)
		{
			AddPlayerSp(PLAYER_SP_RESVALUE);
			m_dwLastSPResTime=dwCurTick;
			return true;
		}
	}
	return false;		// 侠义道三不回复
}

void CPlayer::__OnCheckAutoRestore(DWORD dwCurTick)
{
	if (0 == m_CurHp)
		return;

	// 在战斗状态中重新计时
	if (m_IsInFight)
	{
		m_dwLastHPResTime = dwCurTick;
		return;
	}

	// 站立跟打坐的时候才可以恢复
	if ( EA_ZAZEN == GetCurActionID())
	{
		__OnCheckHPAutoRestore(dwCurTick);
		//__OnCheckMPAutoRestore(dwCurTick);
		//__OnCheckTPAutoRestore(dwCurTick);
	}

	return;
}

void CPlayer::ClearDisConnectStatus()
{
	if (!m_bInit)
		return;

	ClearPlayerStatus(true);

	// 组队的判断
	GetGW()->m_TeamManager.TeamLoseStatus(this, SATeamCancelQuestMsg::TCQ_DISCONNECT);
}

void CPlayer::ClearLogoutStatus()
{
	if (!m_bInit)
		return;

	ClearPlayerStatus(true);
	ClearPlayerStatus(false);

	// 下面是玩家退出时特殊的状态清楚
	GetGW()->m_TeamManager.TeamLoseStatus(this, SATeamCancelQuestMsg::TCQ_LOGOUT);

	if (m_OwnedChangeItems.size())
	{
		for (OwnedChanceItems::iterator it = m_OwnedChangeItems.begin(); it != m_OwnedChangeItems.end(); ++it)
			CGroundItemWinner::GetInstance().PlayerLogOut(this, it->first, it->second);

		m_OwnedChangeItems.clear();
	}
}

void CPlayer::SaveLogoutBuffList()
{
	TSaveBuffData *pSaveBuffData = &m_Property.m_tSaveBuffData[0];
	m_buffMgr.GetSaveBuffList(pSaveBuffData, SPlayerSaveBuff::msc_byMaxSaveBuffCount);	 
}

void CPlayer::Logout(bool SaveAndLogOut)
{
	/// 玩家下线时,调用脚本 20150131
	{
		CScriptState s(NULL, this, NULL, NULL);
		LuaFunctor(g_Script, "OnPlayerLogout")();
	}
	SanguoLogout();//三国逻辑登出
	ClearLogoutStatus();

	CheckTimeWhenLogout();


	SaveLogoutBuffList(); // [11-13-2012 17:48 gw]+退出时保存Buff列表 
	LPIObject PTemp = self.lock();

	if (SaveAndLogOut)
		SendData(SARefreshPlayerMsg::SAVE_AND_LOGOUT);

	ClearClient(m_ClientIndex);

	m_ClientIndex = 0;

// 	// 将自己从世界中清除
// 	if (m_ParentRegion)
// 		m_ParentRegion->DelObject(PTemp);

	extern void RemoveFromCache(LPCSTR account);
	RemoveFromCache(GetAccount());

	// 将自己设为无效，这将会被管理器清除
	m_bValid = false;


	if (m_bInit) 
	{
		RemoveNameRelation(GetName());
		RemoveSIDRelation(GetSID());
	}

	RemoveAccountRelation(GetAccount());
}

void CPlayer::OnDisconnect()
{
	if (m_OnlineState != OST_HANGUP)
	{
		// 断线后，会进入这里，设置断线等待时间
		m_DisWaitTime = MAX_DISWAITTIME;
	}

	// 将自己的网络编号清除
	m_ClientIndex = 0;

	// 取消一些状态
	ClearDisConnectStatus();

	// 同时还要向登陆服务器报告该角色掉线的消息
	SendData(SARefreshPlayerMsg::BACKUP_AND_FLAG);
}

struct ExtraLuaFunctions
{
	static int L_CancelWaitingState( lua_State *L );
};

BOOL CPlayer::isWaitingLockedResult( BOOL useAutoUnlock, BOOL showDialog )
{
	// 百宝阁道具购买和脚本扣点都归入锁定等待状态！
	BOOL ret = ( ( !ATM_temporaryScriptVariant.empty() ) ? 1 : 0 )
		| ( m_bWaitBuyResult ? 2 : 0 );

	extern CScriptManager g_Script;
	extern int autoUnlock;
	if ( ret && useAutoUnlock && autoUnlock )
	{
		extern dwt::stringkey<char [256]> szDirname;
		if ( ATM_temporaryVerifier == 0 ) 
			TraceInfo( szDirname + "decpoint.log", "[%s]自动解锁了一个异常的对象！[%d][%s]", 
			GetStringTime(), ret, GetAccount() );
		else if ( abs( (int)( timeGetTime() - ATM_temporaryVerifier ) ) <= 1000*60*5 )
			return ret; // 该锁定对象还有效且锁定时间未超时！

		// 在条件满足的情况下,直接解锁返回
		g_Script.SetCondition( NULL, this, NULL );
		ExtraLuaFunctions::L_CancelWaitingState( NULL );
		g_Script.CleanCondition();
		return 0;
	}

	// 如果效验值有效,则说明还存在被锁定的数据！
	if ( ATM_temporaryVerifier != 0 ) 
	{
		if ( showDialog ) 
		{
			// 如果5分钟后都没有结果返回，则通过脚本提示玩家是否取消本次操作
			if ( abs( (int)( timeGetTime() - ATM_temporaryVerifier ) ) > 1000*60*5 )
			{
				// 通知玩家是否取消当前还在等待返回的扣点调用！
				g_Script.SetCondition( NULL, this, NULL );
				g_Script.CallFunc( "OpenCancelWaitingStateWnd" ); 
				g_Script.CleanCondition();
			}
		}

		if ( ret == 0 )
			return 4;       // 特殊情况，不应该发生！！！
	}
	// 如果效验值无效,则ret就不应该有值！！！
	else if ( ret != 0 )
	{
		return 0x80 | ret;  // 特殊情况，不应该发生！
	}

	return ret;
}

bool CPlayer::CheckDisWait()
{
	// 这个地方是特殊处理的逻辑，表示该玩家任务号已满，需要立即踢下线
	if (m_wCheckID == 0xfedc)
		goto _force_logout;

	if (m_DisWaitTime == -1)
		return false;

	if (m_DisWaitTime == 0)
	{
_force_logout:
		Logout(true);
		return true;
	}
	else
	{
		m_DisWaitTime--;
		return false;
	}
}

BOOL CPlayer::EndPrevAction()
{
	/*
	CFightObject::EndPrevAction(); 
	//*/

	return TRUE;
}

BOOL CPlayer::DoCurAction()
{
	return CFightObject::DoCurAction();
}

void CPlayer::DeleteTaskFlagMap(const STaskFlag& flag)
{
	switch (flag.m_Type)
	{
	case TT_ITEM:
		{
			DWORD itemID = flag.m_TaskDetail.Item.ItemID;
			CPlayer::TASKITEM::iterator ti = m_ItemTask.find(itemID);

			if (ti != m_ItemTask.end())
			{
				std::list<DWORD>::iterator flagit = find((ti->second).begin(), (ti->second).end(), flag.m_Index);
				(ti->second).erase(flagit);


			}
		}
		break;

	case TT_GIVE:
		{
			DWORD itemID = flag.m_TaskDetail.GiveGet.ItemID;
			CPlayer::TASKGIVEITEM::iterator ti = m_GiveItemTask.find(itemID);

			if (ti != m_GiveItemTask.end())
			{
				std::list<DWORD>::iterator flagit = find((ti->second).begin(), (ti->second).end(), flag.m_Index);
				(ti->second).erase(flagit);
			}
		}
		break;

	case TT_GET:
		{
			DWORD itemID = flag.m_TaskDetail.GiveGet.ItemID;
			CPlayer::TASKGETITEM::iterator ti = m_GetItemTask.find(itemID);

			if (ti != m_GetItemTask.end())
			{
				std::list<DWORD>::iterator flagit = find((ti->second).begin(), (ti->second).end(), flag.m_Index);
				(ti->second).erase(flagit);
			}
		}
		break;

	case TT_USEITEM:
		{
			DWORD itemID = flag.m_TaskDetail.UseItem.ItemID;
			CPlayer::TASKUSEITEM::iterator ti = m_UseItemTask.find(itemID);

			if (ti != m_UseItemTask.end())
			{
				std::list<DWORD>::iterator flagit = find((ti->second).begin(), (ti->second).end(), flag.m_Index);
				(ti->second).erase(flagit);
				SPackageItem *packitem = NULL;
				for (DWORD i = 0; i < PackageAllCells; i++)
				{
					packitem = FindItemByPos(i,XYD_FT_WHATEVER);
					if (packitem&&packitem->wIndex == itemID)break;
				}
				if (packitem){
					DelItem(*packitem,"任务放弃道具删除");
				}
			}
		}
		break;

	case TT_KILL:
		{
			DWORD monID = flag.m_TaskDetail.Kill.MonsterID;
			CPlayer::TASKKILL::iterator ti = m_KillTask.find(monID);

			if (ti != m_KillTask.end())
			{
				std::list<DWORD>::iterator flagit = find((ti->second).begin(), (ti->second).end(), flag.m_Index);
				(ti->second).erase(flagit);
			}
		}
		break;

	case TT_REGION:
		{
			WORD mapID = flag.m_TaskDetail.Region.DestMapID;
			CPlayer::TASKMAP::iterator ti = m_MapTask.find(mapID);

			if (ti != m_MapTask.end())
			{
				std::list<DWORD>::iterator flagit = find((ti->second).begin(), (ti->second).end(), flag.m_Index);
				(ti->second).erase(flagit);
			}
		}
		break;

	case TT_ADD:
	{
		BYTE btype = flag.m_TaskDetail.TaskADD.btype;
		CPlayer::TASKADD::iterator ti = m_AddTask.find(btype);

		if (ti != m_AddTask.end())
		{
			std::list<DWORD>::iterator flagit = find((ti->second).begin(), (ti->second).end(), flag.m_Index);
			(ti->second).erase(flagit);
		}
	}
		break;
	}
}

void CPlayer::DeleteTaskItemMap(WORD taskID)
{
	if (CRoleTask *task = g_Script.m_pPlayer->m_TaskManager.GetRoleTask(taskID))
	{
		for (CRoleTask::TELIST::iterator it = task->m_flagList.begin(); it != task->m_flagList.end(); ++it)
		{
			DeleteTaskFlagMap(*it);
		}
	}
}

void CPlayer::OnTaskMsg(SRoleTaskBaseMsg *pMsg)
{
	CNpc *curNpc = GetCurrentNpc();
	if (SRoleTaskBaseMsg::EPRO_GIVEUPTASK != pMsg->_protocol && !curNpc)
		return;

	switch (pMsg->_protocol)
	{
	case SRoleTaskBaseMsg::EPRO_ADDROLETASK:
		{
			SQAddRoleTask *addMsg = (SQAddRoleTask *)pMsg;

			if (addMsg->dwGlobalID != GetGID())
				return;

			// 不能超过20个任务
			if (m_TaskManager.GetTaskCount() >= 20)
				return;

			if (0 == addMsg->wTaskID)
				return;
			m_ClickTaskFlag = 1;
			g_Script.SetCondition(NULL, this, curNpc);

			// 获取即将调用的函数名，并做调用准备
			
			if (!g_Script.PrepareFunction("AcceptTask"))
			{
				return;
			}
			g_Script.PushParameter(curNpc->m_Property.m_wClickScriptID);
			g_Script.PushParameter(addMsg->wTaskID);

			lite::Variant scriptResult;
			BOOL execResult = g_Script.Execute(&scriptResult);

			if (execResult)
			{
				int result = (int)scriptResult;

				if (result)		// 如果接受成功，开始联动任务的处理和NPC卷轴状态的处理
				{
					g_Script.CallFunc("CommonNpcClick", curNpc->m_Property.m_wClickScriptID);
				}
			}

			g_Script.CleanCondition();
			break;
		}

	case SRoleTaskBaseMsg::EPRO_HANDINTASK:
		{
			SQHandInRoleTask *Handmsg = (SQHandInRoleTask *)pMsg;

			if (Handmsg->dwGlobalID != GetGID())
				return;

			if (0 == Handmsg->wTaskID)
				return;
			m_ClickTaskFlag = 1;
			g_Script.SetCondition(NULL, this, curNpc);

			
			if (!g_Script.PrepareFunction("HandInTask"))
			{
				return;
			}
			g_Script.PushParameter(Handmsg->wTaskID);
			g_Script.PushParameter(Handmsg->wAwardCount);

			lite::Variant scriptResult;
			BOOL execResult = g_Script.Execute(&scriptResult);

			if (execResult)
			{
				int result = (int)scriptResult;

				if (result)
				{
					g_Script.CallFunc("CommonNpcClick", curNpc->m_Property.m_wClickScriptID);
				}
			}

			g_Script.CleanCondition();
			break;
		}

	case SRoleTaskBaseMsg::EPRO_GIVEUPTASK:
		{
			SQGiveUpTask *giveMsg = (SQGiveUpTask *)pMsg;

			if (giveMsg->dwGlobalID != GetGID())
				return;

			if (1 == giveMsg->wTaskID)
			{
				TalkToDnid(m_ClientIndex, "无法放弃主线起始任务！");
				return;
			}

			if (0 == giveMsg->wTaskID)
				return;

			g_Script.SetCondition(NULL, this, NULL);

			
			if (!g_Script.PrepareFunction("GiveUpTask"))
			{
				return;
			}
			g_Script.PushParameter(giveMsg->wTaskID);

			lite::Variant scriptResult;
			BOOL execResult = g_Script.Execute(&scriptResult);

			if (execResult)
			{
				int result = (int)scriptResult;

				if (result)
				{
				}
			}

			g_Script.CleanCondition();
			break;
		}

	case SRoleTaskBaseMsg::EPRO_FINISHTALK:
		{
			SQFinishTalk *talkMsg = (SQFinishTalk *)pMsg;

			if (GetGID() != talkMsg->dwGolbalID)
				break;

			if (CRoleTask *task = m_TaskManager.GetRoleTask(talkMsg->wTaskID))
			{
				for (CRoleTask::TELIST::iterator it = task->m_flagList.begin(); it != task->m_flagList.end(); ++it)
				{
					if (TT_TALK == it->m_Type && talkMsg->wTalkDes == it->m_TaskDetail.Talk.NpcID && it->m_Complete == 0)
					{
						it->m_TaskDetail.Talk.CurTimes = it->m_TaskDetail.Talk.Times;
						it->m_Complete = 1;

						// 发送更新旗标的消息
						SendUpdateTaskFlagMsg(*it, true);

						g_Script.SetCondition(NULL, this, curNpc);
						g_Script.CallFunc("CommonNpcClick", curNpc->m_Property.m_wClickScriptID);
						g_Script.CleanCondition();

						break;
					}
				}					
			}
			break;
		}

	case SRoleTaskBaseMsg::EPRO_FINISHGIVE:
		{
			SQFinishGive *giveMsg = (SQFinishGive *)pMsg;

			if (GetGID() != giveMsg->dwGolbalID)
				break;

			if (CRoleTask *task = m_TaskManager.GetRoleTask(giveMsg->wTaskID))
			{
				bool canFinish = true;

				// 首先做全局的验证
				for (CRoleTask::TELIST::iterator it = task->m_flagList.begin(); it != task->m_flagList.end(); ++it)
				{
					if (TT_GIVE == it->m_Type && giveMsg->wGiveDes == it->m_TaskDetail.GiveGet.NpcID)
					{
						MY_ASSERT(2 != it->m_Complete);

						if (0 == it->m_Complete)
						{
							canFinish = false;
							break;
						}

						DWORD itemID  = it->m_TaskDetail.GiveGet.ItemID;
						WORD itemNum  = it->m_TaskDetail.GiveGet.ItemNum;

						if (!CheckGoods(itemID, itemNum, true))			// 要扣除的道具被什么原因锁定了
						{
							canFinish = false;
							break;
						}
					}
				}	

				// 说明物品足够，可以完成给予
				if (canFinish)
				{
					for (CRoleTask::TELIST::iterator it = task->m_flagList.begin(); it != task->m_flagList.end(); ++it)
					{
						if (TT_GIVE == it->m_Type && giveMsg->wGiveDes == it->m_TaskDetail.GiveGet.NpcID)
						{
							MY_ASSERT(1 == it->m_Complete);

							DWORD itemID  = it->m_TaskDetail.GiveGet.ItemID;
							WORD itemNum  = it->m_TaskDetail.GiveGet.ItemNum;

							// 在此处删除旗标映射
							DeleteTaskFlagMap(*it);

							it->m_Complete = 2;						// 标记为永久完成的旗标
							SendUpdateTaskFlagMsg(*it, true);		// 旗标更新

							CheckGoods(itemID, itemNum, false);		// 扣除物品
						}
					}

					g_Script.SetCondition(NULL, this, curNpc);
					g_Script.CallFunc("CommonNpcClick", curNpc->m_Property.m_wClickScriptID);
					g_Script.CleanCondition();
				}
			}
			else
			{
				rfalse("任务找不到，已交或未接！！");
			}
		}
		break;

	case SRoleTaskBaseMsg::EPRO_FINISHGET:
		{
			SQFinishGet *getMsg = (SQFinishGet *)pMsg;

			if (GetGID() != getMsg->dwGolbalID)
				break;

			if (CRoleTask *task = m_TaskManager.GetRoleTask(getMsg->wTaskID))
			{
				std::list<SAddItemInfo> itemList;

				for (CRoleTask::TELIST::iterator it = task->m_flagList.begin(); it != task->m_flagList.end(); ++it)
				{
					if (TT_GET == it->m_Type && getMsg->wGetDes == it->m_TaskDetail.GiveGet.NpcID)
					{
						// 统计需要的背包格子数
						DWORD itemID  = it->m_TaskDetail.GiveGet.ItemID;
						WORD itemNum  = it->m_TaskDetail.GiveGet.ItemNum;

						DWORD count = GetItemNum(itemID, XYD_FT_WHATEVER);

						if (count < itemNum)
						{
							MY_ASSERT(count == it->m_TaskDetail.GiveGet.CurItemNum);

							itemList.push_back(SAddItemInfo(itemID, itemNum-count));
						}
					}
				}	

				if (!CanAddItems(itemList))
				{
					TalkToDnid(this->m_ClientIndex, "背包已满！无法拾取！");
				}
				else
				{
					StartAddItems(itemList);

					g_Script.SetCondition(NULL, this, curNpc);
					g_Script.CallFunc("CommonNpcClick", curNpc->m_Property.m_wClickScriptID);
					g_Script.CleanCondition();
				}
			}
			else
			{
				rfalse("任务找不到，已交或未接！！");
			}
		}
		break;

	case SRoleTaskBaseMsg::EPRO_SET_SPEF:
		{
			SQSetSpefTask *smsg = (SQSetSpefTask *)pMsg;

			if (smsg->dwPlrID != GetGID())
				return;

			if (0 == smsg->wTaskID)
				return;

			m_TaskManager.ClearAllRoleTask();
			m_TaskInfo.clear();
			m_MenuInfo.clear();
			m_ItemTask.clear();
			m_KillTask.clear();
			m_UseItemTask.clear();
			m_GiveItemTask.clear();
			m_GetItemTask.clear();
			m_MapTask.clear();

			for (int i=smsg->wTaskID; i<=MAX_WHOLE_TASK; i++)
			{
				m_Property.MarkUnComplete(i);
			}

			for (int i=1; i<smsg->wTaskID; i++)
			{
				m_Property.MarkComplete(i);
			}
		}
		break;
	case  SRoleTaskBaseMsg::EPRO_FINISHTADDTASK:
		{
			SQFinishAddTask *sfinishtask = (SQFinishAddTask *)pMsg;
			if (CRoleTask *task = m_TaskManager.GetRoleTask(sfinishtask->wTaskID))
			{
				for (CRoleTask::TELIST::iterator it = task->m_flagList.begin(); it != task->m_flagList.end(); ++it)
				{
					if ( it->m_TaskDetail.TaskADD.btype == sfinishtask->btype && it->m_TaskDetail.TaskADD.dIndex == sfinishtask->dIndex)
					{
						if (it->m_TaskDetail.TaskADD.wCount >= sfinishtask->wCount)
						{
							it->m_TaskDetail.TaskADD.wCount -= sfinishtask->wCount;
						}
						else
						{
							it->m_TaskDetail.TaskADD.wCount = 0;
						}
						if (it->m_TaskDetail.TaskADD.wCount == 0)
						{
							it->m_Complete = 1;
						}
							// 发送更新旗标的消息
							SendUpdateTaskFlagMsg(*it, true);
					}
				}
			}
		}
		break;
	}
}

void CPlayer::StoreClickMsg(SQClickObjectMsg *pMsg)
{
	if (pMsg->dwGlobalID != GetGID()) 
		return;

	m_CommNpcGlobalID 	= pMsg->dwDestGlobalID;
	memcpy(m_aClickParam, pMsg->dymmy, sizeof(m_aClickParam));

	if (m_ParentRegion && m_ParentArea)
	{
		//LPIObject Object = m_ParentRegion->SearchObjectListInAreas(pMsg->dwDestGlobalID, m_ParentArea->m_X, m_ParentArea->m_Y);
		LPIObject Object = m_ParentRegion->SearchNpcByScriptID(pMsg->dwDestGlobalID); //手游版本找NPC，直接在Region中,通过ScriptID查找

		if (Object)
		{
			m_CommNpcGlobalID = Object->GetGID();
			OnClickSomebody(Object);
		}
		else
		{
			m_CommNpcGlobalID = 0;
			rfalse(2, 1, "!!!无法找到NPC:%d在九宫格【%d，%d】", pMsg->dwDestGlobalID, m_ParentArea->m_X, m_ParentArea->m_Y);

			Object = m_ParentRegion->LocateObject(pMsg->dwDestGlobalID);
			if (!Object)
			{
				rfalse(2, 1, "我日，NPC根本不存在！");
			}
		}
	}
}

void CPlayer::OnClickSomebody(LPIObject pObj)
{
	// 取消采集
	CancelCollect();

	CActiveObject *object = (CActiveObject*)pObj->DynamicCast(IID_ACTIVEOBJECT);

	/* dj modify - 手游版本|取消距离检测
	if (object && object->m_ParentRegion)
	{
	BOOL ok_gid = object->m_ParentRegion == m_ParentRegion;

	WORD x = 0, y = 0;
	object->GetCurPos(x, y);
	BOOL ok_pos = (abs((int)x - (int)m_curTileX) <= 9) && (abs((int)y - (int)m_curTileY) <= 9);

	if (!ok_pos || !ok_gid)
	{
	rfalse(2, 1, "对象距离过远，玩家【%d，%d】，NPC【%d，%d】, gid_ok?:%d, pos_ok?:%d", m_curTileX, m_curTileY, x, y, ok_gid, ok_pos);
	m_CommNpcGlobalID 	= 0;
	return;
	}
	}
	*/

	CTrigger trigger;

	if (m_aClickParam[0] == 0)
	{
		CNpc *pNpc = (CNpc*)pObj->DynamicCast(IID_NPC);
		if (!pNpc) 
		{
			m_CommNpcGlobalID	= 0;
			return;
		}

		trigger.m_dwType			= CTrigger::TT_CLICK_ON_NPC;
		trigger.SClickOnNpc.pNpc	= pNpc;
		trigger.SClickOnNpc.pPlayer = this;	
	}
	/*
	else if ( m_aClickParam[0] == 1 ) 
	{
	CPlayer *pPlayer = (CPlayer*)pObj->DynamicCast(IID_PLAYER);
	if(pPlayer == NULL) 
	return;

	if ((m_ParentRegion->DynamicCast(IID_PRISON) != NULL) || 
	m_ParentRegion->DynamicCast(IID_PRISONEX) != NULL ||
	(m_ParentRegion->DynamicCast(IID_SPORTSAREA) != NULL) ||
	(m_ParentRegion->DynamicCast(IID_FIGHTARENA) != NULL))
	{
	// 监狱和竞技场里不允许使用玩家脚本！
	m_CommNpcGlobalID  = 0;
	return;
	}

	if ((pPlayer->GetScriptState() != 0) &&
	(m_ParentRegion->m_dwSpecialAccess != 0))
	// 当前场景不允许使用玩家脚本！
	{
	trigger.m_dwType = CTrigger::TT_CLICK_ON_PLAYER;
	trigger.SClickOnPlayer.dwScriptID = pPlayer->GetScriptState();
	trigger.SClickOnPlayer.dwPPlayerID = m_DestClickID;
	trigger.SClickOnPlayer.pPlayer = this;

	}
	else
	{
	m_CommNpcGlobalID  = 0;
	return;
	}
	}
	// 点击的是建筑物!
	else if ( m_aClickParam[0] == 'b' ) 
	{
	CBuilding *pBuilding = ( CBuilding* )pObj->DynamicCast( IID_BUILDING );
	if ( pBuilding == NULL ) 
	return;

	if ( m_aClickParam[1] )
	{
	// 如果 m_aClickParam[1] 有值, 表示属于强制攻击... 直接构造攻击状态数据
	m_CommNpcGlobalID  = 0;

	//			m_isLockedTarget = false;   // 先取消攻击锁定
	bMsgComein = true;
	SQAFightObjectMsg &msg = m_PendingFightObjectMsg;
	msg.dwDestGlobalID = pBuilding->GetGID();
	ZeroMemory( &msg.sa, sizeof( msg.sa ) );
	msg.sa.dwGlobalID = GetGID();
	msg.sa.wPosX = m_wCurX;
	msg.sa.wPosY = m_wCurY;
	msg.sa.byFightID = m_aClickParam[2];
	return;
	}

	// 由于不知道建筑物的更多细节数据，所以通过脚本来控制
	// 脚本中可以通过特殊的函数来将当前的 m_aClickParam[2] 中的攻击类型转出来, 直接设置玩家的攻击状态数据!
	trigger.m_dwType = CTrigger::TT_CLICK_ON_BUILDING;
	trigger.SClickOnBuilding.pBuilding = pBuilding;
	trigger.SClickOnBuilding.pPlayer = this;
	} 
	else if( m_aClickParam[0]=='n' )	//如果点击NPC使用物品
	{
	CNpc *pNpc = (CNpc*)pObj->DynamicCast(IID_NPC);
	if(pNpc == NULL) 
	return;

	trigger.m_dwType = CTrigger::TT_USEITEM_ON_NPC;
	trigger.SUseItemOnNpc.pNpc = pNpc;
	trigger.SUseItemOnNpc.pPlayer = this;

	memcpy( &trigger.SUseItemOnNpc.dwItemID, &m_aClickParam[1], sizeof(DWORD));		
	} 
	else if( m_aClickParam[0]=='m' ) 
	{ //如果点击Monster使用物品
	CMonster* pMonster = (CMonster*)pObj->DynamicCast(IID_MONSTER);
	if ( pMonster==NULL)
	return;

	trigger.m_dwType = CTrigger::TT_USEITEM_ON_MONSTER;
	trigger.SUseItemOnMonster.pMonster = pMonster;
	trigger.SUseItemOnMonster.pPlayer = this;

	memcpy( &trigger.SUseItemOnMonster.dwItemID, &m_aClickParam[1], sizeof(DWORD));
	}
	//*/
	m_ClickTaskFlag = 0;
	OnTriggerActivated(&trigger);
}

void CPlayer::OnRecvLevelUp(SQLevelChangeMsg *msg)
{
	WORD  UpCount = 0;
	QWORD needExp = CPlayerService::GetInstance().GetPlayerUpdateExp(m_Property.m_School,m_Property.m_Level);

	while (m_Property.m_Exp >= needExp && (m_Property.m_Level+UpCount) != MAXPLAYERLEVEL)
	{
		m_Property.m_Exp -= needExp, UpCount++;
		needExp = CPlayerService::GetInstance().GetPlayerUpdateExp(m_Property.m_School,m_Property.m_Level + UpCount);
	}

	if (0 != UpCount)
	{
		SendAddPlayerLevel(UpCount, SASetEffectMsg::EEFF_LEVELUP, "Notice, plr lvl up");

		if (MAXPLAYERLEVEL == m_Property.m_Level)
			m_Property.m_Exp = 0;

		///升级在此通知好友
		SFriendOnlineMsg msg;
		msg.eState = SFriendOnlineMsg::FRIEND_STATE_LEVELUP;
		memcpy(msg.cName, GetName(), CONST_USERNAME);
		msg.byFriendLevel=m_Property.m_Level;
		FriendsOnline_new(&msg);
		// 通告临时好友信息,升级通知
		FriendsOnline_new(&msg,RT_TEMP);

		///如果到达指定等级，给所有在线好友发送送道具的消息
		DWORD itemid=CFriendService::GetInstance().GetFriendRewarditem(m_Property.m_Level);
// 		if (itemid)
// 		{
// 			msg.eState = SFriendOnlineMsg::FRIEND_STATE_LEVELUPWITHITEM;
// 			msg.dwgiveItemID=itemid;
// 			FriendsOnline_new(&msg);
// 		}

		//升级以后属性改变更新玩家修为值
		UpdatePlayerXwzMsg();

		//玩家升级时脚本回调
		_L_SafeCallLuaFunction("OnPlayerLevelUp");
		// 		lite::Variant ret;
		// 		g_Script.SetCondition(NULL, this, NULL);
		// 		LuaFunctor(g_Script, "OnPlayerLevelUp")(&ret);
		// 		g_Script.CleanCondition();
	}
}

void CPlayer::OnRecvSetPoint(SQSetPointMsg *pMsg)
{
	WORD tempPoints = 0;
	for (size_t i=0; i< 4; i++)
		tempPoints += pMsg->wPoints[i];

	if (tempPoints > m_Property.m_RemainPoint)
		return;

	for (size_t i= 0; i < 4; i++)
		AddPlayerPoint(XA_TF_JINGGONG + i, pMsg->wPoints[i]);

	m_Property.m_RemainPoint -= tempPoints;
	m_PlayerPropertyStatus[XA_REMAINPOINT-XA_MAX_EXP] = true;

	UpdateBaseProperties();

	UpdateAllProperties();

	return;
}

// 洗点函数
BOOL CPlayer::ResetPlayerPoint(BYTE type, WORD point)
{
	MY_ASSERT(type >= SQResetPointMsg::PT_JINGONG && type < SQResetPointMsg::PT_ALL);

	switch(type)
	{
	case SQResetPointMsg::PT_JINGONG:
		m_Property.m_JingGong = m_Property.m_JingGong >= point ? m_Property.m_JingGong - point : 0;
		m_PlayerPropertyStatus[XA_TF_JINGGONG - XA_MAX_EXP] = true;
		break;
	case SQResetPointMsg::PT_FANGYU:
		m_Property.m_FangYu = m_Property.m_FangYu >= point ? m_Property.m_FangYu - point : 0;
		m_PlayerPropertyStatus[XA_TF_FANGYU - XA_MAX_EXP] = true;
		break;

	case SQResetPointMsg::PT_QINGSHEN:
		m_Property.m_QingShen = m_Property.m_QingShen >= point ? m_Property.m_QingShen - point : 0;
		m_PlayerPropertyStatus[XA_TF_QINGSHEN - XA_MAX_EXP] = true;
		break;

	case SQResetPointMsg::PT_JIANSHEN:
		m_Property.m_JianShen = m_Property.m_JianShen >= point ? m_Property.m_JianShen - point : 0;
		m_PlayerPropertyStatus[XA_TF_JIANSHEN - XA_MAX_EXP] = true;
		break;

	case SQResetPointMsg::PT_ALL:
		break;

	default:
		rfalse("洗点异常退出！");
		return FALSE;
	}

	//MY_ASSERT(m_Property.m_JingGong >= 0 && m_Property.m_FangYu >= 0 && m_Property.m_QingShen >= 0 && m_Property.m_JianShen >= 0);
	return TRUE;
}

// 玩家洗点
void CPlayer::OnRecvResetPoint(struct SQResetPointMsg* pMsg)
{
	// 	if (pMsg->type < SQResetPointMsg::PT_JINGONG || pMsg->type > SQResetPointMsg::PT_ALL || !pMsg->point)
	// 		return;
	// 
	// 	SAResetPointMsg msg;
	// 	
	// 	WORD TotalItemNum	= 0;
	// 	WORD PointCount		= pMsg->point;
	// 	// 只有当大于30级的时候，才遍历背包是否有道具
	// 	if (m_Property.m_Level > 30)
	// 	{
	// 		// 存储背包里所有洗髓丹
	// 		vector<SPackageItem*> resetPointItem;
	// 		// 遍历道具表,这里还要加一个没有道具的判断
	// 		for (DWORD i = 0; i < PackageAllCells; ++i)
	// 		{
	// 			ValidationPos(i);
	// 			SPackageItem *pItem = FindItemByPos(i, XYD_FT_ONLYUNLOCK);
	// 			if (!pItem)
	// 				continue;
	// 
	// 			if (dwXiSuiDanID == pItem->wIndex)
	// 			{
	// 				resetPointItem.push_back(pItem);
	// 				TotalItemNum += pItem->overlap;
	// 			}
	// 		}
	// 
	// 		if (!pMsg->bBuy)
	// 		{
	// 			MY_ASSERT(pMsg->point - pMsg->itemCount == TotalItemNum);
	// 			
	// 			DWORD needMoney = pMsg->itemCount * 1000;
	// 			if (!CheckPlayerMoney(XYD_UM_ONLYUNBIND, needMoney, false))
	// 			{
	// 				msg.result = SAResetPointMsg::RPM_FAIL;
	// 				g_StoreMessage(m_ClientIndex, &msg, sizeof(SAResetPointMsg));
	// 				return;
	// 			}
	// 
	// 			// 全部删除背包里道具
	// 			if (resetPointItem.size())
	// 			{
	// 				for (vector<SPackageItem*>::iterator iter = resetPointItem.begin(); iter != resetPointItem.end(); ++iter)
	// 					DelItem(*(*iter), "洗点消耗", false);
	// 			}
	// 		}
	// 		else
	// 		{
	// 			MY_ASSERT(pMsg->point <= TotalItemNum);
	// 
	// 			for (vector<SPackageItem*>::iterator iter = resetPointItem.begin(); iter != resetPointItem.end(); ++iter)
	// 			{
	// 				if (PointCount >= (*iter)->overlap)
	// 				{
	// 					PointCount -= ((*iter)->overlap);
	// 					DelItem(*(*iter), "洗点消耗", false);
	// 				}
	// 				else
	// 				{
	// 					(*iter)->overlap -= PointCount;
	// 					SendItemSynMsg(*iter, XYD_FT_ONLYUNLOCK);
	// 					break;
	// 				}
	// 			}		
	// 		}
	// 
	// 		resetPointItem.clear();
	// 	}
	// 
	// 	// 循环判断是否是全部洗点
	// 	WORD	tempPoints = 0;
	// 	
	// 	// 全部洗点
	// 	if (SQResetPointMsg::PT_ALL == pMsg->type)
	// 	{
	// 		tempPoints = m_Property.m_JingGong + m_Property.m_FangYu + m_Property.m_JianShen + m_Property.m_QingShen;
	// 		
	// 		if (tempPoints != PointCount)
	// 			return;
	// 
	// 		m_Property.m_JingGong	= 0;
	// 		m_Property.m_FangYu		= 0;
	// 		m_Property.m_QingShen	= 0;
	// 		m_Property.m_JianShen	= 0;
	// 
	// 		for (size_t i = XA_TF_JINGGONG; i <= XA_TF_JIANSHEN; ++i)
	// 			m_PlayerPropertyStatus[i - XA_MAX_EXP] = true;
	// 	}
	// 	else
	// 	{
	// 		// 单个洗点
	// 		ResetPlayerPoint(pMsg->type, PointCount);
	// 	}
	// 
	// 	// 更新剩余点数
	// 	m_Property.m_RemainPoint += PointCount;
	// 	m_PlayerPropertyStatus[XA_REMAINPOINT - XA_MAX_EXP] = true;
	// 
	// 	// 更新玩家属性
	// 	UpdateBaseProperties();
	// 	UpdateAllProperties();
	// 
	// 	msg.result = SAResetPointMsg::RPM_SUCCESS;
	// 	g_StoreMessage(m_ClientIndex, &msg, sizeof(SAResetPointMsg));

	return;
}

void CPlayer::OnRecvEquipColor( SQAEquipColorMsg *pMsg )    // 接收到玩家改变装束颜色
{
	// 	if (pMsg == NULL) return;
	// 	if (pMsg->byEquipID>=2*EEC_MAX || pMsg->byEquipID<0) return;
	// 	if (pMsg->byEquipID<EEC_MAX) 
	// 	{
	// 		// 设置颜色
	// 		m_Property.m_byEquipColCount[pMsg->byEquipID] ++;
	// 		m_Property.m_wEquipCol[pMsg->byEquipID] = pMsg->wEquipCol;
	// 	}
	// 	else
	// 	{
	// 		// 清除颜色
	// 		m_Property.m_byEquipColCount[pMsg->byEquipID-4] = 0;
	// 	}
	// 
	// 	pMsg->dwGlobalID = GetGID();
	// 
	// 	if (CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA))
	// 		pArea->SendAdj(pMsg, sizeof(SQAEquipColorMsg), -1);

	return ;
}

/** 夫妻传送消息处理
客户端只做了是否装备了夫妻传送需要的道具，
具体是否已经结婚或者配偶是否在线都在服务端做计算
*/
void CPlayer::OnWifeRemitMsg() 
{
	return;
}

void CPlayer::OnRecvZazenMsg(SQSetZazenMsg *pMsg)
{
	if (pMsg->dwGlobalID != GetGID()) 
		return;
	///检测骑乘状态或者摆摊状态,交易状态,跳跃状态
	if (m_RidingStatus!=0||IsInStall()|| InExchange()||IsInJump())return;

	if (pMsg->byAction!=EA_STAND&&pMsg->byAction!=EA_ZAZEN)return;

	if (CheckAction( pMsg->byAction )) m_dwLastSPResTime=timeGetTime();

	SetZazen(CheckAction( pMsg->byAction ) );



	// 	if( CheckAction( pMsg->byAction ) )  // 判断打坐可行否
	// 	{
	// 		// 开始打坐的时候，清空坐立时的状态
	// // 		if (m_dwExtraState != 0)
	// // 		{
	// // 			m_dwExtraState &= ~0x07;     // 去掉前面三个状态（摆摊、治疗、被治疗）
	// // 
	// // 			AddCheckID();
	// // 
	// // 			SendExtraState();
	// // 		}
	// 
	// 		SetZazen( pMsg->byAction==EA_ZAZEN ? true : false );
	// 	}
}

void CPlayer::UpdateSingleSkill(SQSkillUpdate *SkillMsg)	//处理升级单个技能
{
	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(SkillMsg->wID);

	if (!pData)
	{
		rfalse("技能配置表中找不到该ID！");
		return;
	}

	if (!IsSkillExisted(this, SkillMsg->wID))
	{
		rfalse(1, 2, "技能ID不存在！");
		return;
	}

	WORD pos = -1;
	for (int i = 0; i < MAX_SKILLCOUNT; ++i)
	{
		if (m_Property.m_pSkills[i].wTypeID == SkillMsg->wID)
		{
			pos = i;
			break;
		}
	}

	if (-1 == pos)
	{
		rfalse("技能索引错误！");
		return;
	}

	// 得到当前技能等级
	WORD skillLevel = m_Property.m_pSkills[pos].byLevel;

	if (MaxSkillLevel == skillLevel)
	{
		TalkToDnid(m_ClientIndex, "该技能已经满级,无法继续升级!");
		return;
	}

	// 交易中不能升级
	if (InExchange())
	{
		TalkToDnid(m_ClientIndex, "亲~交易中不能升级技能哦！");
		return;
	}

	// 普通攻击退出
	if (SPT_COMMAN == pData->m_SkillPropertyType)
		return;

	//玩家等级没有达到升级该技能的要求
	if (m_Level < pData->m_NeedPlayerLevel)
	{
		TalkToDnid(m_ClientIndex, "玩家等级不够，不能升级该技能！");
		return;
	}

	// 计算技能组升级消耗 
	g_Script.SetCondition(NULL, this, NULL);
	//这是之前调用的函数
	/*LuaFunctor(g_Script, FormatString("UpdateSkill%d", pData->m_SkillPropertyType))[skillLevel]();*/
	LuaFunctor(g_Script, "UpdateSkill")[skillLevel][pData->m_UpNeedSp][pData->m_UpNeedMoney]();
	g_Script.CleanCondition();

	if (m_updateSkill)
	{
		UpdateAllProperties();

		// 增加技能等级
		++m_Property.m_pSkills[pos].byLevel;

		MY_ASSERT(m_Property.m_pSkills[pos].byLevel <= MaxSkillLevel);

		if (SKT_PASSITIVE == pData->m_SkillType)
		{
			g_Script.SetCondition(0, this, 0);
			LuaFunctor(g_Script, FormatString("AttachPasstiveSkillBuff%d", pData->m_ID))[m_Property.m_pSkills[pos].byLevel]();
			g_Script.CleanCondition();
		}

		SASkillUpdate msg;

		msg.byWhat = SASkillUpdate::SSU_LEVELUP;
		msg.wPos = pos;
		msg.stSkill.wTypeID = m_Property.m_pSkills[pos].wTypeID;
		msg.stSkill.byLevel = m_Property.m_pSkills[pos].byLevel;
		msg.stSkill.dwProficiency = m_Property.m_pSkills[pos].dwProficiency;

		g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));

		m_updateSkill = false;

		g_Script.SetPlayer(this);
		if (g_Script.PrepareFunction("OnRecvUpdateSkill"))
		{
			g_Script.PushParameter(msg.stSkill.wTypeID);
			g_Script.Execute();
		}
		g_Script.CleanPlayer();
	}
}

void CPlayer::UpdateGroupSkill(SQSkillUpdate *SkillMsg)		//处理升级一组技能
{
	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(SkillMsg->wID);
	const SSkillBaseData *pData1 = CSkillService::GetInstance().GetSkillBaseData(SkillMsg->wID1);
	const SSkillBaseData *pData2 = CSkillService::GetInstance().GetSkillBaseData(SkillMsg->wID2);

	if (!pData || !pData1 || !pData2)
	{
		rfalse("技能配置表中找不到其中一个技能ID，不能升级！");
		return;
	}

	if (!IsSkillExisted(this, SkillMsg->wID) || !IsSkillExisted(this, SkillMsg->wID1) || !IsSkillExisted(this, SkillMsg->wID2))
	{
		rfalse(1, 2, "技能ID不存在！");
		return;
	}

	WORD pos = -1;
	WORD pos1 = -1;
	WORD pos2 = -1;
	for (int i = 0; i < MAX_SKILLCOUNT; ++i)
	{
		if (m_Property.m_pSkills[i].wTypeID == SkillMsg->wID)
			pos = i;
		if (m_Property.m_pSkills[i].wTypeID == SkillMsg->wID1)
			pos1 = i;
		if (m_Property.m_pSkills[i].wTypeID == SkillMsg->wID2)
			pos2 = i;
	}

	if (-1 == pos || -1 == pos1 || -1 == pos2)
	{
		rfalse("技能索引错误！");
		return;
	}

	// 得到当前技能等级，技能组中的技能等级相同
	WORD skillLevel = m_Property.m_pSkills[pos].byLevel;

	if (MaxSkillLevel == skillLevel)
	{
		TalkToDnid(m_ClientIndex, "该技能组已经满级,无法继续升级!");
		return;
	}

	// 交易中不能升级
	if (InExchange())
	{
		TalkToDnid(m_ClientIndex, "亲~交易中不能升级技能组哦！");
		return;
	}

	// 普通攻击退出
	if (SPT_COMMAN == pData->m_SkillPropertyType || SPT_COMMAN == pData1->m_SkillPropertyType || SPT_COMMAN == pData2->m_SkillPropertyType)
		return;

	//玩家等级没有达到升级该技能的要求
	if (m_Level < pData->m_NeedPlayerLevel)
	{
		TalkToDnid(m_ClientIndex, "玩家等级不够，不能升级该技能组！");
		return;
	}

	//最新消耗方式，只消耗普通形态下技能的消耗
	DWORD CousumeAllJinBi = 0, CousumeAllSp = 0;
	CousumeAllJinBi += pData->m_UpNeedMoney;
	//CousumeAllJinBi += pData1->m_UpNeedMoney;
	//CousumeAllJinBi += pData2->m_UpNeedMoney;
	CousumeAllSp += pData->m_UpNeedSp;
	//CousumeAllSp += pData1->m_UpNeedSp;
	//CousumeAllSp += pData2->m_UpNeedSp;

	// 计算技能组升级消耗 
	g_Script.SetCondition(NULL, this, NULL);
	//这是之前调用的函数
	/*LuaFunctor(g_Script, FormatString("UpdateSkill%d", pData->m_SkillPropertyType))[skillLevel]();*/
	LuaFunctor(g_Script, "UpdateSkill")[skillLevel][CousumeAllSp][CousumeAllJinBi]();
	g_Script.CleanCondition();

	if (m_updateSkill)
	{
		UpdateAllProperties();

		// 增加技能等级
		++m_Property.m_pSkills[pos].byLevel;
		++m_Property.m_pSkills[pos1].byLevel;
		++m_Property.m_pSkills[pos2].byLevel;

		SASkillUpdate msg;
		msg.byIsRefreshPain = 1;
		msg.byWhat = SASkillUpdate::SSU_LEVELUP;
		msg.wPos = pos;
		msg.stSkill.wTypeID = m_Property.m_pSkills[pos].wTypeID;
		msg.stSkill.byLevel = m_Property.m_pSkills[pos].byLevel;
		msg.stSkill.dwProficiency = m_Property.m_pSkills[pos].dwProficiency;
		g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));
		g_Script.SetPlayer(this);
		if (g_Script.PrepareFunction("OnRecvUpdateSkill"))
		{
			g_Script.PushParameter(msg.stSkill.wTypeID);
			g_Script.Execute();
		}
		g_Script.CleanPlayer();

		SASkillUpdate msg1;
		msg1.byIsRefreshPain = 1;
		msg1.byWhat = SASkillUpdate::SSU_LEVELUP;
		msg1.wPos = pos1;
		msg1.stSkill.wTypeID = m_Property.m_pSkills[pos1].wTypeID;
		msg1.stSkill.byLevel = m_Property.m_pSkills[pos1].byLevel;
		msg1.stSkill.dwProficiency = m_Property.m_pSkills[pos1].dwProficiency;
		g_StoreMessage(m_ClientIndex, &msg1, sizeof(msg1));
		g_Script.SetPlayer(this);
		if (g_Script.PrepareFunction("OnRecvUpdateSkill"))
		{
			g_Script.PushParameter(msg1.stSkill.wTypeID);
			g_Script.Execute();
		}
		g_Script.CleanPlayer();

		SASkillUpdate msg2;
		msg2.byWhat = SASkillUpdate::SSU_LEVELUP;
		msg2.wPos = pos2;
		msg2.stSkill.wTypeID = m_Property.m_pSkills[pos2].wTypeID;
		msg2.stSkill.byLevel = m_Property.m_pSkills[pos2].byLevel;
		msg2.stSkill.dwProficiency = m_Property.m_pSkills[pos2].dwProficiency;
		g_StoreMessage(m_ClientIndex, &msg2, sizeof(msg2));
		g_Script.SetPlayer(this);
		if (g_Script.PrepareFunction("OnRecvUpdateSkill"))
		{
			g_Script.PushParameter(msg2.stSkill.wTypeID);
			g_Script.Execute();
		}
		g_Script.CleanPlayer();

		m_updateSkill = false;
	}
}

// 玩家技能组升级 
void CPlayer::OnRecvUpdateSkill(SQSkillUpdate *SkillMsg)
{
	if (SkillMsg->dwGlobalID != GetGID())
		return;
	if (SkillMsg->UpdateSkillType == 1)	//升级单个技能
		UpdateSingleSkill(SkillMsg);
	else if (SkillMsg->UpdateSkillType == 2)	//升级一组技能
		UpdateGroupSkill(SkillMsg);
	else
		TalkToDnid(m_ClientIndex, "升级技能失败！");
	return;
}

// 升级心法
void CPlayer::OnRecvUpdateTelegry(SQTelergyUpdate *TelMsg)
{
	if (TelMsg->dwGlobalID != GetGID())
		return;

	const SXinFaData *pData = CSkillService::GetInstance().GetXinFaBaseData(TelMsg->wID);
	if (!pData)
	{
		rfalse("心法配置表中找不到该心法ID！");
		return;
	}

	WORD pos = IsTelegryExisted(this, TelMsg->wID);		// 心法ID错误
	if (0xffff == pos)
		return;

	// 交易中不能升级
	if (InExchange())
	{
		TalkToDnid(m_ClientIndex, "亲~交易中不能升级技能或心法哦！");
		return;
	}

	// 得到要升级的心法的等级
	WORD telergyLevel = m_Property.m_Xyd3Telergy[pos].m_TelergyLevel;
	if (MaxXinFaLevel == telergyLevel)
	{
		TalkToDnid(m_ClientIndex, "该心法已经满级,无法在继续升级!");
		return;
	}

	// 心法升级消耗跟附加属性添加
	g_Script.SetCondition( NULL, this, NULL );
	LuaFunctor(g_Script, FormatString("UpdateTelergy%d", TelMsg->wID))[telergyLevel]();
	LuaFunctor(g_Script, FormatString("TelergyProperty%d", TelMsg->wID))[telergyLevel]();
	g_Script.CleanCondition();

	if (m_updateTelergy)
	{
		++m_Property.m_Xyd3Telergy[pos].m_TelergyLevel;
		MY_ASSERT(m_Property.m_Xyd3Telergy[pos].m_TelergyLevel <= MaxXinFaLevel);

		SATelergyUpdate msg;

		msg.pos							= pos;
		msg.bResult						= SATelergyUpdate::SAT_LEVELUP;
		msg.m_Telergy.m_TelergyID		= m_Property.m_Xyd3Telergy[pos].m_TelergyID;
		msg.m_Telergy.m_TelergyLevel	= m_Property.m_Xyd3Telergy[pos].m_TelergyLevel;

		g_StoreMessage(m_ClientIndex, &msg, sizeof(SATelergyUpdate));

		// 更新属性
		UpdateAllProperties();

		m_updateTelergy = false;
	}

	return;
}

// 接受打通穴道请求消息
void CPlayer::OnRecvPassVenapointAsk( BYTE byVenaNum, BYTE byPointNum )
{
	// 大周天
	if ( ( byVenaNum >= 10 ) && ( byVenaNum < 20 ) && ( byPointNum < 32 ) )
	{
		LPSVenapointDataEx data = g_pVenapointData->GetVenapointEx( byVenaNum - 10, byPointNum );
		if ( data == NULL || data->szName == NULL )
			return;

		g_Script.SetCondition( NULL, this, NULL );
		LuaFunctor( g_Script, "OnPlayerOpenVenapoint" )[ ( byVenaNum - 10 ) ][ byPointNum ][data->szName]();
		g_Script.CleanCondition();
		return;
	}

	// 执行打通穴道的操作
	if( SetPassVenapoint( byVenaNum, byPointNum ) )
	{
		// 返回打通穴道的确认消息
		SendPassVenapoint( byVenaNum, byPointNum );
	}
	else
	{
		//.. 别的原因无法打通穴道
	}

}

////////////////////////////////////////////////
void CPlayer::MyTitleChanged(char* pNewTitle)
{
	SATitleChangedMsg msg;
	msg.dwPlayerChangedID = GetGID();
	memcpy(msg.cChangedTitle, pNewTitle, 10);
	msg.cChangedTitle[10] = 0;

	if (CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA))
		pArea->SendAdj(&msg, sizeof(SATitleChangedMsg), -1);
}
////////////////////////////////////////////////

void CPlayer::ChangeTeamSkill(BYTE byNewTeamSkill,WORD wMemberCount)
{
}

BOOL CPlayer::AddPKiller(CPlayer *pPKiller)
{
	if (pPKiller == NULL)
		return FALSE;

	// 已经是PK对象了
	if (IsPKiller(pPKiller))
		return TRUE;

	// 添加PK对象，并且设定防卫时间
	m_PkMap[pPKiller->GetGID()] = MAX_PKTIME;

	return TRUE;
}

BOOL CPlayer::IsPKiller(CPlayer *pPKiller)
{
	if (pPKiller == NULL)
		return FALSE;

	return m_PkMap.find(pPKiller->GetGID()) != m_PkMap.end();
}

BOOL CPlayer::CheckPKiller(CPlayer *pDestPlayer,BOOL bIsAttack)
{
	// bAttacked==TRUE命中的时候，否则是攻击的时候；pDestPlayer=NULL表示攻击范围，或对象是怪物

	// 	if (pDestPlayer == this)
	// 		return TRUE;
	// 
	// 	if( pDestPlayer==NULL ) 
	// 		return  TRUE;
	// 
	// 	if ( m_ParentRegion == NULL )
	// 		return TRUE;
	// 
	// 	if ( pDestPlayer->m_ParentRegion == NULL )
	// 		return  TRUE;
	// 
	// 	if( pDestPlayer->m_ParentRegion->m_IsChildProtect && pDestPlayer->m_Property.m_Level < 60 )
	// 	{
	// 		TalkToDnid( this->m_ClientIndex, "你不能攻击小于60级的玩家" );;
	// 		return FALSE;
	// 	}
	// 
	// 	if( pDestPlayer->m_ParentRegion->m_IsChildProtect && this->m_byLevel < 60 )
	// 	{
	// 		TalkToDnid( this->m_ClientIndex, "阁下修为不到60级，不能攻击其他玩家！" );;
	// 		return FALSE;
	// 	}
	// 
	// 	if(m_dwTeamID == pDestPlayer->m_dwTeamID && m_dwTeamID != 0)
	// 	{
	// 		if( bIsAttack )
	// 		{
	// 			SendErrorMsg( SABackMsg::B_ATTACKTEAM );
	// 			return FALSE;
	// 		}
	// 	}
	// 
	// 	bool bInMatchMode = false; //在比武场模式，并且不在同一队伍，允许PK
	// 	DWORD dwOtherTeam = pDestPlayer->m_dwExtraState & 0x0000f000;
	// 	DWORD dwMyTeam = m_dwExtraState & 0x0000f000;
	// 	if ((dwMyTeam != 0) && (dwOtherTeam != 0) && (dwMyTeam != dwOtherTeam)) //两个人都在比武场，并且不在一个队
	// 	{
	// 		bInMatchMode = true;
	// 	}
	// 
	// 	// PK规则 0:不避所有  1:避开所有   2:避开帮派  3:避开正魔  4:门派模式
	// 	if (!bInMatchMode)	// 比武场模式不适用该规则
	// 		switch(m_Property.m_byPKRule)
	// 	{
	// 		case 0:
	// 			break;//不避所有
	// 		case 1:
	// 			return FALSE;//避开所有
	// 		case 2:
	// 			if (strcmp(m_Property.m_szTongName, pDestPlayer->m_Property.m_szTongName) == 0 && 
	// 				strcmp( m_Property.m_szTongName, "" ) != 0 )//避开帮派
	// 				return FALSE;
	// 			break;
	// 		case 3:
	// 			if (m_Property.m_sXValue * pDestPlayer->m_Property.m_sXValue < 0)//避开正魔 
	// 				return FALSE;
	// 			break;
	// 		case 4:
	// 			{
	// 				if ( (m_Property.m_szTongName[ 0 ] == 0 && pDestPlayer->m_Property.m_szTongName[ 0 ] == 0) && (m_Property.m_School == pDestPlayer->m_Property.m_School) )
	// 					return FALSE;
	// 			}
	// 			break;
	// 	}
	// 
	// 	// PK相关 启动
	// 	if ( bIsAttack ) // 如果系统不允许PK
	// 	{
	// 		// 如果场景有限制，就不能PK
	// 		if ( ( m_ParentRegion != NULL ) && ( m_ParentRegion->m_dwPKAvailable == CRegion::PAT_NULL ) /*&& ( pDestPlayer->m_Property.m_byPKValue < 8 )*/ )
	// 		{
	// 			SendErrorMsg(SABackMsg::SYS_CAN_NOT_DO_PK);
	// 			return FALSE;
	// 		}
	// 
	// 		if (bInMatchMode)	// 比武场模式
	// 			return TRUE;
	// 
	// 		// 如果自己在10等级以下，也无法对别人进行PK
	// 		if ( pDestPlayer->m_ParentRegion->m_IsChildProtect && m_Property.m_Level < 10 )
	// 		{
	// 			SendErrorMsg( SABackMsg::B_LEVELABLE );
	// //			m_isLockedTarget = false;
	// 			return FALSE;
	// 		}
	// 
	// 		//if( bAttacked )     // 命中以后判断
	// 		{
	// 			// 如果对方在10等级以下，无法进行PK
	// 			if ( pDestPlayer->m_ParentRegion->m_IsChildProtect && pDestPlayer->m_Property.m_Level < 10 )
	// 			{
	// 				SendErrorMsg( SABackMsg::B_LEVELABLE );
	// //				m_isLockedTarget = false;
	// 				return FALSE;
	// 			}
	// 
	// 			// 如果对方还在保护保护状态，不能PK
	// 			if ( pDestPlayer->GetProtect() || ( ( m_ParentRegion->m_dwPKAvailable == CRegion::PAT_NULL ) /*&& ( pDestPlayer->m_Property.m_byPKValue < 8 )*/ ) )
	// 			{
	// //				m_isLockedTarget = false;
	// 				return FALSE;
	// 			}
	// 			// 如果自己在保护状态，也不能PK
	// 			if ( GetProtect() || ( ( m_ParentRegion->m_dwPKAvailable == CRegion::PAT_NULL ) /*&& ( pDestPlayer->m_Property.m_byPKValue < 8 )*/ ) )
	// 			{
	// //				m_isLockedTarget = false;
	// 				return FALSE;
	// 			}
	// 			// 如果自己的PK列表中有对方的ID，就说明自己是正当防卫，否则就算自己启动PK
	// 			if ( !IsPKiller(pDestPlayer) )
	// 				pDestPlayer->AddPKiller(this);
	// 		}
	// 	}

	return  TRUE;
}

void CPlayer::UpdatePkMap()
{
	queue<DWORD> delete_queue;
	map<DWORD, DWORD>::iterator it = m_PkMap.begin();
	while (it != m_PkMap.end())
	{
		if (it->second > 0)
			it->second--;

		if (it->second == 0)
			delete_queue.push(it->first);

		it++;
	}

	while (delete_queue.size())
	{       
		it = m_PkMap.find(delete_queue.front());
		if (it != m_PkMap.end())
			m_PkMap.erase(it);
		//m_PkMap.erase(delete_queue.front());
		delete_queue.pop();
	}
}

void CPlayer::ResetState()  // 在放入场景哪里调用的
{
	OnClearTempItem();
	CExchangeGoods::TheEnd();

	// 	DWORD perExteraState = m_dwExtraState;
	// 	m_dwExtraState = 0; // 切换场景，变身状态还原
	// 	if ( ( perExteraState & SHOWEXTRASTATE_SCRIPT ) )
	// 		m_dwExtraState |= SHOWEXTRASTATE_SCRIPT;
	// 	if( perExteraState & SHOWEXTRASTATE_D3D_EQUIP )
	// 		m_dwExtraState |= SHOWEXTRASTATE_D3D_EQUIP;
	// 	if ( ( perExteraState & SHOWEXTRASTATE_NEWMODEL) )
	// 		m_dwExtraState |= SHOWEXTRASTATE_NEWMODEL;
	// 
	// 	if ( ( perExteraState & SHOWEXTRASTATE_SCORES_LEVEL ) )
	// 		m_dwExtraState |= SHOWEXTRASTATE_SCORES_LEVEL;
	// 	if( perExteraState & SHOWEXTRASTATE_SCORES_MONEY )
	// 		m_dwExtraState |= SHOWEXTRASTATE_SCORES_MONEY;
	// 	if ( ( perExteraState & SHOWEXTRASTATE_SCORES_FAMEG) )
	// 		m_dwExtraState |= SHOWEXTRASTATE_SCORES_FAMEG;
	// 	if ( ( perExteraState & SHOWEXTRASTATE_SCORES_FAMEX) )
	// 		m_dwExtraState |= SHOWEXTRASTATE_SCORES_FAMEX;
	// 	if ( ( perExteraState & SHOWEXTRASTATE_SCORES_MEDICALETHICS) )
	// 		m_dwExtraState |= SHOWEXTRASTATE_SCORES_MEDICALETHICS;

	/*
	LPIObject pOtherObj = GetOther();
	if(pOtherObj != NULL)
	{ 
	CPlayer *pOther = (CPlayer *)pOtherObj->DynamicCast(IID_PLAYER);
	if(pOther != NULL)
	{
	pOther->SetOther(NULL);
	pOther->SetBeExchanging(false);
	pOther->InitExchangePanel();
	}
	}
	SetOther(NULL);
	SetBeExchanging(false);
	InitExchangePanel();
	*/
}

// DWORD CPlayer::CheckItemRequest(const SItemData *pItem)
// {

// zeb 2009.11.19
// 	if (pItem == NULL)
// 		return SABackMsg::B_FAIL;
// 
// 	//如果有门派，则判断门派是否相同
// 	if (pItem->byGroupEM != 0)
// 	{
// 		if (!(pItem->byGroupEM & (1 << m_Property.m_School)))
// 			return SABackMsg::B_NOTSCHOOL;	//门派不同
// 	}
// 
// 	if (pItem->byLevelEM > m_Property.m_Level)
// 		return SABackMsg::B_LV_NOTENOUGH;	//等级不足

//if (pItem == NULL)
//	return SABackMsg::B_FAIL;

//if (pItem->byGroupEM != 0)
//	if (pItem->byGroupEM != m_Property.m_School+1)
//		return SABackMsg::B_NOTSCHOOL;

//if (pItem->byLevelEM > m_Property.m_Level)
//	return SABackMsg::B_LV_NOTENOUGH;

//if (pItem->wENEM > m_Property.m_wEN)
//	return SABackMsg::B_EN_NOTENOUGH;

//if (pItem->wINEM > m_Property.m_wIN)
//	return SABackMsg::B_IN_NOTENOUGH;

//if (pItem->wSTEM > m_Property.m_wST)
//	return SABackMsg::B_ST_NOTENOUGH;
// zeb 2009.11.19

//if (pItem->wAGEM > m_Property.m_wAG)
//		return SABackMsg::B_AG_NOTENOUGH;

// 	return SABackMsg::B_SUCCESS;
// }

BOOL CPlayer::CheckCanTalk(DWORD type, DWORD time)
{
	if ((int)(timeGetTime() - m_dwChatSkipTime[type]) > 0 )
	{
		m_dwChatSkipTime[type] = timeGetTime() + 1000 * time;
		return TRUE;
	}

	return FALSE;
}



// 掉落特殊物品变化概率的控制
void    CPlayer::InitDropItemBaseRand(void)  
{
	m_DropItemBaseRand = g_pFightData->m_DropItemBaseRand.wMax;// + (rand() % 4000);
}

void    CPlayer::RdcDropItemBaseRand(void)   
{   
	// 	m_DropItemBaseRand -= g_pFightData->m_DropItemBaseRand.wReduceVal;
	// 
	// 	if( m_DropItemBaseRand<g_pFightData->m_DropItemBaseRand.wMin )
	// 	{
	// 		m_DropItemBaseRand = g_pFightData->m_DropItemBaseRand.wMin;
	// 	}
}

// 发送心法数据的改变
void    CPlayer::SendTelergyChange( BOOL extraOnly )
{
	// 	SASetExtrTelergyMsg msg;
	// 	msg.m_ExtraTelergy = m_Property.m_ExtraTelergy;
	// 	msg.nTelergy = 0xffff;
	// 
	// 	if ( !extraOnly )
	// 	{
	// 		msg.nTelergy = 8;
	// 		memcpy( msg.m_Telergy, m_Property.m_Telergy, sizeof( msg.m_Telergy ) );
	// 	}
	// 
	// 	g_StoreMessage( m_ClientIndex, &msg, sizeof( msg ) );

	/*
	SATetergyChangeMsg  TetergyMsg;

	TetergyMsg.dwGlobalID = GetGID();
	memcpy( TetergyMsg.Tetergy, m_Property.m_Telergy, sizeof(STelergy)*MAX_EQUIPTELERGY );

	g_StoreMessage( m_ClientIndex, &TetergyMsg, sizeof( TetergyMsg ) );
	*/
}

// 发送技能更新消息
void CPlayer::SendUpdataSkill(BYTE bySkillPosID)
{
	if (bySkillPosID >= MAX_SKILLCOUNT)
		return;

	SASkillUpdate msg;
	msg.byWhat					= SASkillUpdate::SSU_LEARNED;
	msg.wPos					= bySkillPosID;
	msg.stSkill.wTypeID			= m_Property.m_pSkills[bySkillPosID].wTypeID;
	msg.stSkill.byLevel			= m_Property.m_pSkills[bySkillPosID].byLevel;
	msg.stSkill.dwProficiency	= m_Property.m_pSkills[bySkillPosID].dwProficiency;

	g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));
}

// 发送变身消息
void    CPlayer::SendChangeMutate( BYTE byMutateType, WORD	wMutateID )
{
	SAChangeMutateMsg   Msg;

	Msg.dwGlobalID   = GetGID();
	Msg.byMutateType = byMutateType;
	Msg.wMutateID    = wMutateID;

	if (m_ParentArea != NULL) 
		m_ParentArea->SendAdj(&Msg, sizeof(SAChangeMutateMsg), -1);
}

// 发送替身消息
void    CPlayer::SendScapegoat( WORD wScapegoatID, WORD wGoatEffID )
{
	SASetScapegoatMsg   Msg;

	Msg.dwGlobalID    = GetGID();
	Msg.wScapegoatID  = wScapegoatID;
	Msg.wGoatEffectID = wGoatEffID;

	if (m_ParentArea != NULL) 
		m_ParentArea->SendAdj(&Msg, sizeof(SASetScapegoatMsg), -1);
}

void    CPlayer::SendOnGoatMonsterCount( WORD wCount, WORD wMutateTime )
{
	SAOnGoatMonsterCountMsg     Msg;

	Msg.dwGlobalID          = GetGID();
	Msg.wOnGoatMonsterCount = wCount;
	Msg.wMutateTime         = wMutateTime;

	g_StoreMessage( m_ClientIndex, &Msg, sizeof(SAOnGoatMonsterCountMsg) );
}

void    CPlayer::SendRefreshOnUp(void)
{
	SARefreshOnUpMsg    Msg;
	Msg.dwGlobalID = GetGID();
	Msg.m_wMonsterCountOnGoat = m_wMonsterCountOnGoat;

	g_StoreMessage( m_ClientIndex, &Msg, sizeof(SARefreshOnUpMsg) );

	BYTE    byMutateType = 0;
	WORD	wMutateID = 0;

	//	if( m_dwExtraState&SHOWEXTRASTATE_ONMOSMUTATE )             byMutateType = 1;
	//	else if( m_dwExtraState&SHOWEXTRASTATE_ONNPCMUTATE )        byMutateType = 2;
	//	else if( m_dwExtraState&SHOWEXTRASTATE_ONGOATMUTATE )       byMutateType = 3;
	//	else if( m_dwExtraState&SHOWEXTRASTATE_ONESPMUTATE )        byMutateType = 4;
	wMutateID = m_wMutateID;

	if( byMutateType )
		SendChangeMutate( byMutateType, wMutateID );
}

void CPlayer::ClearPlayerStatus(bool IsCommon)
{
	if (IsCommon)
	{
		// 取消当前正在查看的玩家
		if (m_pAttribListener)
		{
			MY_ASSERT(m_pAttribListener->GetSubject());
			m_pAttribListener->GetSubject()->RemoveListener(m_pAttribListener);
			m_pAttribListener = 0;

			SShowEnemyInfo_S2C_MsgBody msg;
			msg.mEnemyGID	= -1;
			msg.mPlayerGID	= GetGID();
			g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));
		}

		StallLoseStatus();		// 取消摆摊（自己/查看别个）
		CloseGroundItem();		// 取消查看包裹
		ExchgLoseStatus();		// 取消当前交易（拒绝，取消请求，取消交易）
		WRLostStatus();			// 关闭仓库
		CancelCollect();		// 取消采集
		CallBackFightPet(false);		// 收回战斗宠物

		m_CommNpcGlobalID = 0;	// 取消NPC关联
		temporaryVerifier = 0;

		SQEndForge endMsg;
		endMsg.bOps = SQEndForge::QEF_CLOSE;
		RecvEndForgeItem(&endMsg);
	}
	else
	{
		DownMounts();			// 收回骑乘
		CallBackPet();			// 收回宠物
		CallBackFightPet(false);		// 收回战斗宠物
		// BreakOperation();		// 取消当前操作
		m_BatchReadyEvent.BreakOperationReadyEvent(Event::EEventInterrupt_Combinate_ALL); // 取消全部事件操作
	}
}

void CPlayer::OnDead(CFightObject *PKiller)
{
	if (!m_ParentRegion)
		return;
	if (m_Property.m_TransPetIndex != 0xff)
	{
		time_t CurTime = time(NULL);
		DWORD DieTranIntervalTime = _L_GetLuaValue("ReturnPlayerDieTranLimit");
		DWORD TranIntervalTime = _L_GetLuaValue("ReturnPlayerTranLimit");
		if (CurTime - m_RecordPreDeadCTime >= DieTranIntervalTime && CurTime - m_RecordPreTransferTime >= TranIntervalTime
			&&!(m_fightState & FS_DIZZY) && !(m_fightState & FS_LIMIT_SKILL) && !(m_fightState & FS_HYPNOTISM) && !(m_fightState & FS_DINGSHENG)
			&& !(m_fightState & FS_JITUI))
		{
			m_SysAutoTransToPlayer = TRUE;
			BYTE State;// = GetTransformersLevel(0);
			if (State == 1)
			{
				TransferPlayer();
				m_Property.m_FightPetActived = 0xff;
//				SetTransformersLevel(0, 0);
				SendMyState();
			}
			else
			{
				PlayerTransferPet(m_Property.m_TransPetIndex);
				m_Property.m_FightPetActived = m_Property.m_TransPetIndex;
//				SetTransformersLevel(0, 1);
				SendMyState();
			}
			SAUnTransformers Res;
			g_StoreMessage(m_ClientIndex, &Res, sizeof(SAUnTransformers));
			m_RecordPreDeadCTime = CurTime;
			return;
		}
	}

	m_DeadTime = timeGetTime();		// 记录死亡时间（在CFightObject::OnDead之前！）

	if (m_ParentRegion->m_wRegionID != m_bCurDeadreginID)
	{
		m_bCurDeadreginID = m_ParentRegion->m_wRegionID ;
		m_wCurDeadCount= 0;
	}
	m_wCurDeadCount ++;

	CFightObject::OnDead(PKiller);

// 	SAPlayerDeadAddMsg PlayerDeadAddMsg;
// 
// 	for(int i = 0;i<4;i++)
// 	{
// 		PlayerDeadAddMsg.wreliveconType[i].bMoneytype = XYD_UM_ONLYUNBIND;
// 		PlayerDeadAddMsg.wreliveconType[i].wReliveMoney = wReliveMoney[i];
// 		PlayerDeadAddMsg.wreliveconType[i].wReliveTime = wReliveTime[i];
// 		if (i==SQPlayerDeadMsg::PDM_HERE_FREE){
// 			PlayerDeadAddMsg.wreliveconType[i].wReliveTime += m_wCurDeadCount > 2 ? (m_wCurDeadCount - 2) * 30 : 0;//增加30秒
// 		}
// 		else if (i>SQPlayerDeadMsg::PDM_HERE_FREE){
// 			PlayerDeadAddMsg.wreliveconType[i].wReliveMoney += m_wCurDeadCount > 2 ? (m_wCurDeadCount - 2) * 1000 : 0;//每次增加1000
// 		}
// 	}
// 	dwt::strcpy(PlayerDeadAddMsg.killname,PKiller->getname(),CONST_USERNAME);
// 	PlayerDeadAddMsg.skillid = PKiller->m_AtkContext.m_SkillData.m_ID;
// 	g_StoreMessage(m_ClientIndex,&PlayerDeadAddMsg,sizeof(PlayerDeadAddMsg));


	ClearPlayerStatus(true);
	ClearPlayerStatus(false);

	CPlayer *pPlayerKiller = 0;
	if (PKiller)
	{
		pPlayerKiller = (CPlayer*)PKiller->DynamicCast(IID_PLAYER);
		if (pPlayerKiller == this)
			pPlayerKiller = 0;

		if(!pPlayerKiller)
		{
			//如果是侠客，算到侠客的主人头上
			CFightPet *fpet = (CFightPet*)PKiller->DynamicCast(IID_FIGHT_PET);
			if (fpet){
				pPlayerKiller = fpet->m_owner;
			}
		}
	}

	MY_ASSERT(0 == m_CurHp);

	// 清空九字杀阵
	SANineWordMsg msg;
	m_NineWordsKill = 0;
	msg.num = m_NineWordsKill;

	g_StoreMessage(m_ClientIndex, &msg, sizeof(SANineWordMsg));

	// 所有的装备当前耐久度减去5%
	for (int i = EQUIP_P_WEAPON; i <= EQUIP_P_PRECIOUS; ++ i)
	{
		// 减少攻击者装备的耐久
		if (0 != m_Property.m_Equip[i].wIndex && 0 != m_Property.m_Equip[i].attribute.currWear)
		{
			const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(m_Property.m_Equip[i].wIndex);
			if (!itemData)
				return;

			// 判断是否是永不磨损的装备
			if (itemData->m_MaxWear)
				UpdateEquipWear(i, itemData->m_MaxWear * 0.05);
		}
	}

	OnDropItem();//玩家死亡掉落物品

	//玩家死亡脚本回调
	// 	lite::Variant ret;
	// 	g_Script.SetCondition(NULL, this, NULL);
	// 	LuaFunctor(g_Script, "OnPlayerDead")(&ret);
	// 	g_Script.CleanCondition();
	g_Script.SetPlayer(this);

	CDynamicDailyArea *pDailyScene = (CDynamicDailyArea *)m_ParentRegion->DynamicCast(IID_DYNAMICDAILYAREA);
	if (pDailyScene != NULL)
	{
		if (g_Script.PrepareFunction("OnDailyObjectDead"))
		{
			g_Script.PushParameter(GetGID());	//自己gid
			g_Script.PushParameter(GetSceneRegionID()); //所在地图gid
			g_Script.PushParameter(m_ParentRegion->m_wRegionID);//地图regionid
			if (PKiller)
			{
				g_Script.PushParameter(PKiller->GetGID());	//杀死玩家的gid
			}
			g_Script.PushParameter(this->GetGID());	//当前玩家的gid
			g_Script.Execute();
		}
	}
	else
	{
		if (g_Script.PrepareFunction("OnObjectDead"))
		{
			g_Script.PushParameter(GetGID());	//自己gid
			g_Script.PushParameter(GetSceneRegionID()); //所在地图gid
			g_Script.PushParameter(m_ParentRegion->m_wRegionID);//地图regionid
			if (PKiller)
			{
				g_Script.PushParameter(PKiller->GetGID());	//杀死玩家的gid
			}
			g_Script.Execute();
		}
	}
	g_Script.CleanPlayer();

	// 	// 如果是玩家杀死了玩家
	if ( pPlayerKiller && m_ParentRegion->m_nSafeZoneInfo != CRegion::PK_NOTPKVALUE )
	{
		// 在这里加入判断,看对方是否在自己的仇人中，如果已经存在加一点被杀次数
		// 不然将pKiller者加入到仇人名单中zgc，通知玩家自己被谁杀死了
		// 通知玩家被谁杀死了
		int nDegree = 0 ;
		//在仇人列表中直接返回
		if (FindRelation_new(pPlayerKiller->m_Property.m_Name,RT_ENEMIES))return;

		// [2012-7-3 16-20 gw: +记录当前是否是好友：用于好友度从大于0变成小于等于0的情况，解除好友关系]
		bool bIsFriend = false;
		//在好友中则扣除好友度
		if (FindRelation_new(pPlayerKiller->m_Property.m_Name,RT_FRIEND)&&pPlayerKiller->FindRelation_new(m_Property.m_Name,RT_FRIEND))
		{
			bIsFriend = true;
			nDegree = -500;
			///减少好友度
			ChangeDegree(m_ClientIndex,pPlayerKiller->m_Property.m_Name,nDegree );
			pPlayerKiller->ChangeDegree(pPlayerKiller->m_ClientIndex,m_Property.m_Name,nDegree );
			nDegree = GetFriendDegree_new( pPlayerKiller->m_Property.m_Name);
		}

		//如果于被杀的玩家好友度低于0，自动添加到仇人中
		LPCSTR info = NULL;
		if ( nDegree <= 0 )
		{
			if ( AddRelation_new( pPlayerKiller->m_Property.m_Name, RT_ENEMIES ) )
				info = FormatString( "你被%s杀死了，已经把他加入到了仇人名单中", pPlayerKiller->m_Property.m_Name );
			else 
				info = FormatString( "你被%s杀死了，但因为仇人名单已满，添加失败", pPlayerKiller->m_Property.m_Name );
			TalkToDnid(m_ClientIndex,info);
		}

		//如果与被杀的玩家好友度低于0，删除好友
		nDegree = pPlayerKiller->GetFriendDegree_new( m_Property.m_Name);
		if (bIsFriend && nDegree<=0){
			pPlayerKiller->RemoveRelation_new(m_Property.m_Name,RT_FRIEND);
			info = FormatString( "你与%s的好友度等于0，已经删除好友关系", m_Property.m_Name );
			TalkToDnid(pPlayerKiller->m_ClientIndex,info);
		}
	}

}

BOOL CPlayer::SendMyState()
{
	if (!m_ParentArea)
		return FALSE;

	m_ParentArea->SendAdj(GetStateMsg(), sizeof(SASynPlayerMsg), 0);

	return TRUE;
}

void CPlayer::ChangeMountFactor(SPlayerMounts::Mounts *pMount, WORD type)
{
	if (1 == type)			// 装备
	{
		for (size_t i = 0; i < SPlayerMounts::SPM_SPEED; i++)
			m_MountAddFactor[i] = pMount->Points[i];
	}
	else if (2 == type)		// 上马
	{
		const SMountBaseData *pMountData = CMountService::GetInstance().GetMountBaseData(pMount->ID, pMount->Level);
		if (pMountData)
			m_MountAddFactor[SPlayerMounts::SPM_SPEED] = pMountData->m_SpeedUp;
	}
	else if (3 == type)		// 卸装
	{
		memset(&m_MountAddFactor, 0, sizeof(m_MountAddFactor)-sizeof(WORD));
	}
	else if (4 == type)		// 下马（只改变速度）
	{
		m_MountAddFactor[SPlayerMounts::SPM_SPEED] = 0;
	}

	return;
}

WORD CPlayer::GetMountFactor(WORD index)
{
	MY_ASSERT(index >=0 && index <SPlayerMounts::SPM_MAX);

	return m_MountAddFactor[index];
}

BOOL CPlayer::LoginChecks()
{
	m_OnlineState = OST_NORMAL;
	
	return TRUE;
}

BOOL CPlayer::LogoutChecks()
{
	// 释放组队加成状态.
	//	ChangeTeamSkill( 0, 0 );

	//	GetGW()->m_FactionManager.MemberLogout(this);

	///好友下线的通知
	SFriendOnlineMsg msg;
	msg.eState = ( m_OnlineState == OST_HANGUP ) ? 
		SFriendOnlineMsg::FRIEND_STATE_HANGUP : SFriendOnlineMsg::FRIEND_STATE_OUTLINE;
	memcpy(msg.cName, GetName(), CONST_USERNAME);
	FriendsOnline_new(&msg);
	// 通告临时好友信息,下线通知
	FriendsOnline_new(&msg,RT_TEMP);

	// 通知结义兄弟
	//	SUnionOnlineMsg _msg;
	//	_msg.eState = ( m_OnlineState == OST_HANGUP ) ? 
	//		SUnionOnlineMsg::UNION_STATE_OUTLINE : SUnionOnlineMsg::UNION_STATE_ONLINE;;
	//	dwt::strcpy( _msg.cName, GetName(), CONST_USERNAME );
	//	GetGW()->m_UnionManager.UnionOnline( &_msg, GetSID() );

	return TRUE; 
}

void CPlayer::SetCountFlag(BOOL bPrint)
{
	if(bPrint) 
		g_Count.AddPlayerToList(m_Property.m_Name);
	else
		g_Count.DelPlayerFromList(m_Property.m_Name);

	m_bCountFlag = bPrint; 
}

void CPlayer::CheckMulTime()
{
	extern BYTE GetMultiTimeCount();

	if ( multipletime.m_dwLastTime == 0 )
	{
		// initialize state
		multipletime.m_byLeaveTime = GetMultiTimeCount();
		multipletime.m_dwLastTime = (DWORD)time( NULL );
		multipletime.m_byCurWeekUseTime = 0;
		multipletime.m_byMultipleTime = 0;
		multipletime.m_fMultiple = 1;
		multipletime.m_reserve1 = 0;
		//multipletime.m_reserve2 = 0;
		return ;
	}

	// 如果现在还在双倍状态
	double dElapsed_time = difftime( time( NULL ), ( time_t )multipletime.m_dwLastTime );

	// 如果正在使用双倍，或者最后领取时间大于当前时间。
	if ( ( dElapsed_time / 3600 ) <  multipletime.m_byMultipleTime ) 
		return ;

	time_t tLastTime = ( time_t )multipletime.m_dwLastTime;
	time_t tCurTime = time( NULL );

	tm *temp = localtime( &tLastTime );
	if ( temp == NULL )
		return ;

	tm oldt = *temp;
	temp = localtime( &tCurTime );
	if ( temp == NULL )
		return ;

	tm newt = *temp;
	int year = newt.tm_year - oldt.tm_year;

	// 取得本年的第几周
	int lastweek = DataOfWeek( tLastTime );
	int currweek = DataOfWeek( tCurTime );
	int diffweek = 0;

	if ( year == 0 )
		diffweek = currweek - lastweek;
	else if ( year > 0 )
		diffweek = ( currweek + ( year * 52 ) ) - lastweek;
	else
	{
		rfalse( 3, 1, "双倍时间异常year[%d][%d][%d]", year, tCurTime, tLastTime ); 
		return ;
	}

	// 如果跨年,为负数，到本年第几周的一个负数
	if ( diffweek > 0 )
	{
		// 如果最后一次领取时间大于7天或者过了一周

		int mulTimeMax = GetMultiTimeCount();

		// 双倍的最大上限 每周领取小时 + ( 等级 - 60 )
		if ( m_Property.m_Level > 60 )
		{
			mulTimeMax = GetMultiTimeCount() + ( m_Property.m_Level - 60 );
			if ( mulTimeMax > 255 ) 
				mulTimeMax = 255;
		}

		int curMulTime = diffweek * GetMultiTimeCount();
		if ( multipletime.m_byLeaveTime + curMulTime > mulTimeMax )
			multipletime.m_byLeaveTime = mulTimeMax;
		else
			multipletime.m_byLeaveTime += curMulTime;

		multipletime.m_dwLastTime = (DWORD)time( NULL );
		multipletime.m_byCurWeekUseTime = 0;
		multipletime.m_byMultipleTime = 0; 
		multipletime.m_fMultiple = 1;
	}
}


void CPlayer::UpdateMultipleTime()
{
	double dElapsed_time = abs( difftime( time( NULL ), ( time_t )multipletime.m_dwLastTime ) ); 
	if ( ( dElapsed_time / 3600 ) >=  multipletime.m_byMultipleTime ) 
	{
		multipletime.m_byMultipleTime = 0;
		multipletime.m_fMultiple = 1;
	}
}

void  CPlayer::SendMultimeInfo(void)
{
	if(multipletime.m_byMultipleTime/*m_byMultipleTime */> 0)
	{
		double dElapsed_time = difftime(time(NULL), (time_t)multipletime.m_dwLastTime /*m_nMulStartTime*/);

		SAMulTimeInfoMsg msg;
		msg.dTime = multipletime.m_byMultipleTime * 3600  - dElapsed_time; 
		msg.wMulTime = multipletime.m_fMultiple;
		msg.wMulTime = (msg.wMulTime << 8) & 0xff00; // 如果有2.5倍这里就别的处理
		msg.wMulTime |= multipletime.m_byMultipleTime & 0xff ;  
		g_StoreMessage( m_ClientIndex, &msg, sizeof(SAMulTimeInfoMsg) );
	}
}

// 发送心法状态，包括受伤、治疗
void    CPlayer::SendVenationState( BYTE byWhichvena, BYTE byState, BYTE byIsUpdate )
{
	SAVenationStateMsg  msg ;
	msg.dwGlobalID      = GetGID();
	msg.byWhoVenation   = byWhichvena;
	msg.byState         = byState;
	msg.byIsUpdateMsg   = byIsUpdate;

	if( byIsUpdate )    // 更新消息要同时告诉周围的人
	{
		if (m_ParentArea != NULL) 
			m_ParentArea->SendAdj(&msg, sizeof(SAVenationStateMsg), -1);
	}
	else
	{
		g_StoreMessage( m_ClientIndex, &msg, sizeof(SAVenationStateMsg) );
	}
}

// 发送心法治疗的确认消息，可能包括治疗动作，所以是广播
void    CPlayer::SendOnCureing(DWORD dwGID)
{
	// 	SACureVenationOKMsg     msg;
	// 	CPlayer     *pDestPlayer;
	// 	pDestPlayer = (CPlayer *)GetPlayerByGID(dwGID)->DynamicCast(IID_PLAYER);
	// 
	// 	extern  int g_GetRigorDir(int x1,int y1,int x2,int y2);
	// 	if( pDestPlayer )
	// 	{
	// 		msg.dwGlobalID  = GetGID();
	// 		msg.dwDestGID   = dwGID;
	// 		msg.byDir       = g_GetRigorDir( m_wCurX, m_wCurY, pDestPlayer->m_wCurX, pDestPlayer->m_wCurY );
	// 
	// 		if (m_ParentArea != NULL)
	// 			m_ParentArea->SendAdj(&msg, sizeof(msg), -1);
	// 	}
}

// 发送附加状态，摆摊=0x01（没用），治疗经脉=0x02，被治疗经脉=0x04
void CPlayer::SendExtraState()
{
	SASetExtraStateMsg  ExtraStateMsg;
	ExtraStateMsg.dwGlobalID = GetGID();
	ExtraStateMsg.dwExtraState = m_dwExtraState;

	if (m_ParentArea != NULL)
		m_ParentArea->SendAdj(&ExtraStateMsg, sizeof(ExtraStateMsg), -1);
}

// 发送打通穴道确认消息
void    CPlayer::SendPassVenapoint( BYTE byVenaNum, BYTE bypointNum )
{
	// 	SAPassVenapointOKMsg    PassMsg;
	// 
	// 	PassMsg.dwGlobalID      = GetGID();
	// 	PassMsg.byVenationCount = m_Property.m_byVenapointCount;
	// 	PassMsg.byVenationNum   = byVenaNum;
	// 	PassMsg.byVenapointNum  = bypointNum;
	// 
	// 	PassMsg.dwUseTelergyVal = m_dwUseTelergyVal;
	// 	m_dwUseTelergyVal = 0;
	// 
	// 	g_StoreMessage( m_ClientIndex, &PassMsg, sizeof(SAPassVenapointOKMsg) );
}

BOOL CPlayer::UpdatePlayerProperty(SFixBaseData * pFixBaseData)     // GM 指令更新玩家资料
{
	return TRUE;
}

void CPlayer::EnableProtect(void)
{
	m_ProRefTime = timeGetTime();
	m_bProtect = true;
}

BOOL CPlayer::IsTeamLeader()
{
	return m_bIsTeamLeader;
}

//void CPlayer::UpdateMoneyPointTime()
//{
//	if(m_qwMoneyPointTime == 0)
//		return;
//
//	double dElapsed_time = difftime(time(NULL), (time_t)m_qwMoneyPointTime);
//    if( dElapsed_time >=  QUEST_MONEYPOINT_TIME) 
//    {
//        m_qwMoneyPointTime = 0; 
//    }
//
//}

void CPlayer::SetPlayerConsumePoint(DWORD dwPoint)
{

	// 	m_Property.m_dwConsumePoint += dwPoint; //更新本地数据，并且通知客户端更新数据
	// 	SAConsumePoint CPmsg;
	// 	CPmsg.dwConsumePoint = dwPoint;
	// 	g_StoreMessage(m_ClientIndex,&CPmsg,sizeof(SAConsumePoint));

}

BOOL ClickLimitChecker::isValid() 
{ 
	return clickCount <= 80; 
}

void ClickLimitChecker::Update() 
{ 
	// 每30秒一次更新
	if ( abs( ( int )( timeGetTime() - clickLimitCheckTime ) ) < ( 30000 ) )
		return;

	// 现在的规则是，每30秒每个玩家最多发送80个操作指令，多余的指令直接抛弃

	clickLimitCheckTime = timeGetTime();
	clickCount = ( ( clickCount > 120 ) ? 40 : 0 );
}

BOOL QueryAccountPoints( CPlayer *player ) 
{
	if ( player == NULL )
		return FALSE;

	DWORD staticId = player->GetSID();
	//记得是传取消了前缀的账号到账号服务器上哟.
	LPCSTR account = player->GetNotPrefixAccount();

	SQGameServerRPCOPMsg msg;

	lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );

	try
	{
		// 该函数是请求获取与该账号直接相关的各类型点数，很多时候用于前期检测并没有用于关键数据判断
		// 所以如果这个请求出错，也不用回传失败信息

		// 准备存储过程（查询）
		slm [OP_PREPARE_STOREDPROC] ( "AccountManager.sp_QueryPointsNewXYD" ) 

			// 设定调用参数
			[OP_BEGIN_PARAMS]
		(1) ( account )     // 查询账号
			[OP_END_PARAMS]

		// 调用存储过程
		[OP_CALL_STOREDPROC]

		// 初始化返回数据盒
		[OP_INIT_RETBOX]    ( 1024 )

			// 数据盒的第一个值
			[OP_BOX_VARIANT]    ( (int)SMessage::EPRO_ACCOUNT_MESSAGE )
			[OP_BOX_VARIANT]    ( (int)SAccountBaseMsg::EPRO_REFRESH_MONEY_PIONT )
			[OP_BOX_VARIANT]    ( staticId )

			// 返回第2个OUTPUT参数，是一个Dot分割的字符串
			[OP_BOX_PARAM]      ( 2 )

			// 返回数据盒
			[OP_RETURN_BOX]

		// 结束
		[OP_RPC_END];

		// 全都准备好后，再发送消息到登陆服务器！并且由登陆服务器再转到账号服务器
		msg.dstDatabase = SQGameServerRPCOPMsg::ACCOUNT_DATABASE;
		dwt::strcpy( ( char* )msg.GetExtendBuffer(), player->GetAccountPrefix(), 16 );
		if ( !SendToLoginServer( &msg, long(sizeof( msg ) - slm.EndEdition()) ) )
			return FALSE;

		// 因为只是查询与刷新，所以就不用做LOG
		//extern dwt::stringkey<char [256]> szDirname;
		//TraceInfo( szDirname + "decpoint.log", "[%s]启动扣点操作！[%s][%08X][%d][%d][%d][%s]", 
		//    GetStringTime(), account, staticId, type, number, optype, opinfo );
	}
	catch ( lite::Xcpt & )
	{
		return FALSE;
	}

	return TRUE;
}

void CPlayer::InitTimeLimit( DWORD limitedState, DWORD online, DWORD offline )
{
	this->m_OnlineTime = online;
	this->offlineTime = offline;
	this->limitedState = limitedState;
}

void CPlayer::NotifyTimeLimit( DWORD online )
{
	// 更新在线时间，以确保在每间隔15分钟后进行数据更新验证
	m_OnlineTime = online;

	// ################防沉迷系统################
	DWORD prevShowStep = showStep;
#ifdef TimeCompress
	if ( online >= 5*60 )
	{
		// 每15分钟提示一次！
		showStep = 7 + ( online - 5*60 ) / ( 15 );
	}
	else if ( online > 3*60 )
	{
		// 每30分钟提示一次
		showStep = 3 + ( online - 3*60 ) / ( 30 );
	}
	else 
	{
		// 每60分钟提示一次
		showStep = online / ( 60 );
	}
#else
	if ( online >= 5*60*60 )
	{
		// 每15分钟提示一次！
		showStep = 7 + ( online - 5*60*60 ) / ( 15*60 );
		limitedState = LIMITED_ZERO;
	}
	else if ( online > 3*60*60 )
	{
		// 每30分钟提示一次
		showStep = 3 + ( online - 3*60*60 ) / ( 30*60 );
		limitedState = LIMITED_HALF;
	}
	else 
	{
		// 每60分钟提示一次
		showStep = online / ( 60*60 );
		limitedState = LIMITED_ALREADY;
	}
#endif

	if ( showStep != prevShowStep )
	{
		if ( showStep < 3 )
		{
			TalkToDnid( m_ClientIndex, FormatString( "您累计在线时间已满%d小时。", showStep ) );
			return;
		}
		else 
		{
			if ( showStep < 7 ){
				TalkToDnid( m_ClientIndex, "您已经进入疲劳游戏时间，您的游戏收益将降为正常值的50%。为了您的健康，请尽快下线休息，做适当身体活动，合理安排学习生活。" );
			}
			else{
				TalkToDnid( m_ClientIndex, "您已进入不健康游戏时间，为了您的健康，请您立即下线休息。如不下线，您的身体将受到损害，您的收益已降为零，直到您的累计下线时间满5小时后，才能恢复正常。" );
			}

			extern std::list< std::string > extraNetLimitedInfo;
			if ( extraNetLimitedInfo.empty() )
				return;

			for ( std::list< string >::iterator it = extraNetLimitedInfo.begin(); it != extraNetLimitedInfo.end(); it ++ )
				TalkToDnid( m_ClientIndex, it->c_str() );
		}
	}
	// ##########################################
}

void CPlayer::OnRegionChanged(CRegion *newRegion, bool Before)
{
	if (Before)
	{
		CheckStopBullet(2);		// 强制子弹失效，仅对我发射的有效
		CloseGroundItem();
		CallBackFightPet(false);	// 收回侠客
	}
	else
	{
		EnableProtect();

		if (0xff != m_PreFPIndex)
		{
			CallOutFightPet(m_PreFPIndex);
			m_PreFPIndex = 0xff;
		}

		//update by ly 2014/7/14 判断玩家变身宠物说在场景改变时，更新变身宠物数据，当在主场景时，变身宠物数据位玩家数据，当在副本时为宠物数据
		if (m_ParentRegion->DynamicCast(IID_DYNAMICREGION) != NULL && m_Property.m_FightPetActived != 0xff)
		{
			PlayerTransferPet(m_Property.m_FightPetActived);
		}
		else if (m_ParentRegion->DynamicCast(IID_DYNAMICREGION) == NULL && m_Property.m_FightPetActived != 0xff)
		{
			TransferPlayer();
		}

		CheckResumeFullPetHpAndMp();	//核对回复满宠物血量和内力
		CheckResumeFullPlayerHpAndMp();//核对回复满玩家血量和内力
	}
	//if (m_followObject.size()>0)  //切换场景清除
	{
		//ClearFollow();
	}
}

void CPlayer::UpdateFaction()
{
	GetGW()->m_FactionManager.UpdateMember( NULL, this, TRUE );
}

void CPlayer::UpdateMemberData( SimFactionData::SimMemberInfo *member, 
	LPCSTR factionName, WORD factionId, BOOL doSynchro )
{
	// 	if( member == NULL )
	// 	{
	// 		m_Property.factionId = 0;
	// 		//m_Property.m_dwSysTitle[1] = 0;
	// 		m_Property.m_szTongName[0] = 0;
	// 		m_dwFactionTitleID = 0;
	// 		ZeroMemory( &m_stFacRight, sizeof( m_stFacRight ) );
	// 	}
	// 	else
	// 	{
	// 		doSynchro = false;
	// 
	// 		if ( factionId != 0 && factionId != m_Property.factionId ) {
	// 			m_Property.factionId = factionId;
	// 			doSynchro = true;
	// 		}
	// 		if ( m_Property.m_szTongName[0] == 0 && factionName != NULL && 
	// 			dwt::strcmp( m_Property.m_szTongName, factionName, sizeof( m_Property.m_szTongName ) ) )
	// 		{
	// 			dwt::strcpy( m_Property.m_szTongName, factionName, sizeof( m_Property.m_szTongName ) );
	// 			doSynchro = true;
	// 		}
	// 
	// 		// 如果挂牌已经存在，则帮派部分不主动更新挂牌图标
	// 		int number = IsInFactionFirst() == TRUE ? 4000 : 3000;
	// 
	// 		if ( m_Property.m_dwSysTitle[1] != member->Title + number ) {
	// 			m_Property.m_dwSysTitle[1] = member->Title + number;
	// 			doSynchro = true;
	// 		}
	// 
	// 		memcpy( &m_stFacRight, member, sizeof( m_stFacRight ) );
	// 	}
	// 
	// 	if ( doSynchro )
	// 		SendMyState();
}

BOOL CPlayer::SendMsg(void *msg, size_t size)
{
	if (size & 0xffff0000)
		return FALSE;

	return g_StoreMessage(m_ClientIndex, msg, (WORD )size);
}

void CPlayer::UpdateExtraTelerge()
{	
}

void CPlayer::OnExtraTelergyModify( SQSetExtraTelergyMsg *pMsg )
{
	// 	if ( pMsg == NULL )
	// 		return;
	// 
	// 	SExtraTelergy &et = m_Property.m_ExtraTelergy;
	// 
	// 	// 1.如果没有充值, 激活, 是不能操作扩展心法的
	// 	if ( et.charge <= 6 || pMsg->state[0] > 2 || pMsg->state[1] > 2 || pMsg->state[2] > 2 || pMsg->state[3] > 2 )
	// 		return;
	// 
	// 	// ########################################################################
	// 	// 注意，在服务器端：空格子表示为 byTelergyLevel == 0
	// 	// 客户端：空格子表示为 wTelergyID == 0xffff
	// 	// 由于这种不一致性，导致了很多判断复杂度增加!
	// 	// ########################################################################
	// 	// 现进行方案修正, 使用临时数据, 将 ID 统一为 0:空 大于0:有效
	// 	// ########################################################################
	// 
	// 	// 拷贝一个临时数据区
	// 	WORD tids[ MAX_EQUIPTELERGY ];
	// 	WORD etids[ MAX_EXTRATELERGY ];
	// 	WORD msg_etids[ NMAXTELRGY ];
	// 
	// 	for ( int i = 0; i < MAX_EQUIPTELERGY; ++i )
	// 		tids[i] = m_Property.m_Telergy[i].byTelergyLevel ? m_Property.m_Telergy[i].wTelergyID + 1 : 0;
	// 
	// 	for ( int i = 0; i < MAX_EXTRATELERGY; ++i )
	// 		etids[i] = et.telergy[i].byTelergyLevel ? et.telergy[i].wTelergyID + 1 : 0;
	// 
	// 	for ( int i = 0; i < NMAXTELRGY; ++i )
	// 		msg_etids[i] =  pMsg->wTelergyID[i] + 1;
	// 
	// 
	// 	// 2.如果内容也确实保持不变, 则本次操作就不进行处理 
	// 	if ( ( et.state[0] == pMsg->state[0] ) && ( etids[0] == msg_etids[MAX_EQUIPTELERGY + 0] ) 
	// 		&& ( et.state[1] == pMsg->state[1] ) && ( etids[1] == msg_etids[MAX_EQUIPTELERGY + 1] )
	// 		&& ( et.state[2] == pMsg->state[2] ) && ( etids[2] == msg_etids[MAX_EQUIPTELERGY + 2] ) 
	// 		&& ( et.state[3] == pMsg->state[3] ) && ( etids[3] == msg_etids[MAX_EQUIPTELERGY + 3] ) )
	// 		return;
	// 
	// 	// 3.判断12个心法是否有效 0,扩展,装备,以外的数据认为是无效的
	// 	for ( int at = 0; at < 12; ++at )
	// 	{
	// 		if ( msg_etids[at] == 0 )
	// 			continue;
	// 
	// 		int sameCount = 0;
	// 		for ( int i = 0; i< 8; ++i )
	// 		{
	// 			if ( msg_etids[at] == tids[i] )
	// 				++sameCount;
	// 		}
	// 		for ( int i = 0; i< 4; ++i )
	// 		{
	// 			if ( msg_etids[at] == etids[i] )
	// 				++sameCount;
	// 		}
	// 
	// 		if ( sameCount == 0 )
	// 			return;          
	// 	}
	// 
	// 	// 4.更新心法数据
	// 	STelergy tempTelergy[12];
	// 	memset( tempTelergy, 0, sizeof(STelergy) * 12 );
	// 
	// 	for ( int i = 0; i<NMAXTELRGY; ++i )
	// 	{
	// 		// 装备心法
	// 		for ( int j = 0; j<MAX_EQUIPTELERGY; ++j )
	// 		{
	// 			if ( msg_etids[i] != 0 && msg_etids[i] == tids[j] )
	// 			{         
	// 				memcpy( &(tempTelergy[i]), &(m_Property.m_Telergy[j]), sizeof(STelergy) );
	// 				break;
	// 			}
	// 		}
	// 
	// 		// 扩展心法
	// 		for ( int j = 0; j<MAX_EXTRATELERGY; ++j )
	// 		{         
	// 			if ( msg_etids[i] != 0 && msg_etids[i] == etids[j] )
	// 			{    
	// 				memcpy( &(tempTelergy[i]), &(et.telergy[j]), sizeof(STelergy) );
	// 				break;
	// 			}
	// 		}
	// 	}
	// 
	// 	bool check = false;
	// 	for ( int i = 0; i < MAX_EQUIPTELERGY; i++ ) {
	// 		if ( tempTelergy[i].dwTelergyVal != 0 ) {
	// 			check = true;
	// 			break;
	// 		}
	// 	}
	// 	if ( !check ) {
	// 		rfalse( 2, 1, "找到一个空的扩展心法处理" );
	// 		return;
	// 	}
	// 
	// 	for ( int telerAt = 0; telerAt<MAX_EQUIPTELERGY; ++telerAt )
	// 	{
	// 		memcpy( &(m_Property.m_Telergy[telerAt]), &(tempTelergy[telerAt]), sizeof(STelergy) );
	// 	}
	// 
	// 	for ( int etelerAt = 0; etelerAt<MAX_EXTRATELERGY; ++etelerAt )
	// 	{
	// 		memcpy( &(m_Property.m_ExtraTelergy.telergy[etelerAt]), &(tempTelergy[MAX_EQUIPTELERGY + etelerAt]), sizeof(STelergy) );
	// 	}
	// 
	// 	// 5.收尾
	// 	// 确定可成功替换后，扣除6点【目前是整个操作一次扣一点】，设置扩展心法格状态
	// 	et.charge -= 6;
	// 	reinterpret_cast< DWORD& >( et.state ) = reinterpret_cast< DWORD& >( pMsg->state );
	// 
	// 	// 判断当前心法是否有效, 如果无效, 则尝试找到一个有效的心法作为当前心法
	// 	if ( (m_Property.m_CurTelergy >= MAX_EQUIPTELERGY) || m_Property.m_Telergy[ m_Property.m_CurTelergy ].byTelergyLevel == 0 )
	// 	{
	// 		for ( int i = 0; i < MAX_EQUIPTELERGY; i ++ )
	// 		{
	// 			if ( m_Property.m_Telergy[i].byTelergyLevel )
	// 			{
	// 				m_Property.m_CurTelergy = i;
	// 				break;
	// 			}
	// 		}
	// 	}

	// 	// 同步
	// 	SendTelergyChange();
	// 	CalculateTelergyEffects();
	// 	SendAddPlayerLevel( 0, 0 );

	return;
}

void CPlayer::ArrangePackage(WORD type)
{
	int index = 0;

	SPackageItem *pPackage[PackageAllCells] = {0};
	SPackageItem final[PackageAllCells];

	// 验证时间间隔
	if (timeGetTime() - ArrangeIntervalTime < 10000)
		return;

	ArrangeIntervalTime = timeGetTime();

	WORD totalItems = 0;
	for (DWORD i = 0; i < PackageAllCells; i++)
	{
		// 如果背包中有锁定的道具，那么无法进行整理
		if (m_PackageItems[i].m_State & SItemWithFlag::IWF_LOCKED)
			return;

		// 未激活的略过
		if (0 == (m_PackageItems[i].m_State & SItemWithFlag::IWF_ACTIVED))
			continue;

		// 空格子略过
		if (0 == (m_PackageItems[i].m_State & SItemWithFlag::IWF_ITEMIN))
			continue;

		//+ 任务道具可以整理
		final[index]	= m_PackageItems[i].m_Item;
		pPackage[index] = &final[index];
		index++;
		totalItems++;
	}

	if (0 == totalItems)		// 背包为空
		return;

	// 先开始合并道具
	for (int i=0; i<index; i++)
	{
		if (!pPackage[i])
			continue;

		const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(pPackage[i]->wIndex);
		MY_ASSERT(itemData);

		// 如果无法叠加或者已是最大数，则跳过
		if (itemData->m_Overlay == pPackage[i]->overlap)
			continue;

		WORD StillCon = itemData->m_Overlay - pPackage[i]->overlap;

		for (int j=i+1; j<index; j++)
		{
			if (!pPackage[j])
				continue;

			// 判断是否可以叠加至pPackage[i]
			if ((pPackage[i]->wIndex != pPackage[j]->wIndex))
				continue;

			if (StillCon >= pPackage[j]->overlap)
			{
				pPackage[i]->overlap += pPackage[j]->overlap;
				pPackage[j] = 0;
			}
			else
			{
				pPackage[i]->overlap = itemData->m_Overlay;
				pPackage[j]->overlap -= StillCon;
			}

			StillCon = itemData->m_Overlay - pPackage[i]->overlap;

			if (0 == StillCon)
				break;
		}
	}

	DWORD finalCount = 0;
	for (int i=0; i<index; i++)
	{
		if (!pPackage[i])
			continue;

		final[finalCount++] = *pPackage[i];
	}

	// 把现在所有的道具先删除
	for (DWORD i = 0; i < PackageAllCells; i++)
	{
		// 未激活的略过
		if (0 == (m_PackageItems[i].m_State & SItemWithFlag::IWF_ACTIVED))
			continue;

		// 空格子略过
		if (0 == (m_PackageItems[i].m_State & SItemWithFlag::IWF_ITEMIN))
			continue;

		//- 任务道具可以整理
		DelItem(m_PackageItems[i].m_Item, 0, false);
	}

	// 开始排序
	// 首先进行统计，统计的第一层，将每种类型的道具放到一起
	std::map<WORD, std::list<WORD> > Level1Info;

	for (size_t i=0; i<finalCount; i++)
	{
		MY_ASSERT(final[i].wIndex);

		const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(final[i].wIndex);
		if (!itemData)
			continue;

		Level1Info[itemData->m_Type].push_back(i);
	}

	//m_ItemsState.clear();

	// 开始分析每一个类型
	for (std::map<WORD, std::list<WORD> >::iterator it = Level1Info.begin(); it != Level1Info.end(); ++it)
	{
		std::map<DWORD, std::list<WORD> > Level2Info;

		for (std::list<WORD>::iterator ti = it->second.begin(); ti != it->second.end(); ++ti)
		{
			Level2Info[final[*ti].wIndex].push_back(*ti);
		}

		// 开始分析第二层
		for (std::map<DWORD, std::list<WORD> >::iterator it3 = Level2Info.begin(); it3 != Level2Info.end(); ++it3)
		{
			// 开始执行插入
			for (std::list<WORD>::iterator ti3 = it3->second.begin(); ti3 != it3->second.end(); ++ti3)
			{
				SPackageItem *item = &final[*ti3];
				MY_ASSERT(item->wIndex);

				WORD emptyPos = FindBlankPos(XYD_PT_BASE);
				AddExistingItem(*item, emptyPos, true);
			}
		}
	}
	SANeatenMsg ANeatenMsg;
	ANeatenMsg.m_Result = 1;	//整理背包成功
	g_StoreMessage(m_ClientIndex, &ANeatenMsg, sizeof(SANeatenMsg));
}

// 根据穴道数据增加属性值
void CPlayer::AddValueByVenaDataEx( LPSVenapointDataEx pVenaData )
{
}

// 判断一条脉线或某个穴道是否已经打通
bool CPlayer::IsPassVenationEx( BYTE byVenaNum, BYTE byPointNum )
{
	return  false;
}

DNID CPlayer::GetMac()
{
	extern DNID g_GetLinkMac( DNID dnidClient );
	return g_GetLinkMac( m_ClientIndex );
}

void CPlayer::UseItemXTBL()
{
	//	AutoUseItem( m_pItemXTBL, m_Property.m_dwTempExpByXTBL );

	// 成功表示添加起了，没成功表示背包里没有经验珠
	//	m_Property.m_dwTempExpByXTBL = 0;
}

void CPlayer::SendGetGMIDInf()
{
	m_GMIDCheckInf.dwRand = rand();

	SACheckGMInfMsg msg;
	msg.dwRandNumber = m_GMIDCheckInf.dwRand;

	g_StoreMessage( m_ClientIndex, &msg, sizeof(SACheckGMInfMsg) );
}

BOOL CPlayer::AddMoney( DWORD dwYuanbao, DWORD dwGift, DWORD dwGold )
{
	if ( this == NULL )
		return FALSE;

	BOOL bRst = TRUE;
	// 	m_Property.m_dwXYDPoint   =  min( 1000000000, m_Property.m_dwXYDPoint + dwYuanbao );
	// 	m_Property.m_dwGiftCoupon =  min( 1000000000, m_Property.m_dwGiftCoupon + dwGift );
	// 	m_Property.m_Money       =  min( 1000000000, m_Property.m_Money + dwGold );
	// 
	// 	UpdateMoneyToC();

	return bRst;
}

BOOL CPlayer::AddMoney(int MoneyType, DWORD dwValue)
{
	if ( this == NULL )
		return FALSE;

	BOOL bRst = TRUE;


	return bRst;
}

BOOL CPlayer::DelMoney( DWORD dwYuanbao, DWORD dwGift, DWORD dwGold )
{
	if ( this == NULL )
		return FALSE;

	BOOL bRst = TRUE;
	// 	m_Property.m_dwXYDPoint   = ( m_Property.m_dwXYDPoint > dwYuanbao) ? ( m_Property.m_dwXYDPoint - dwYuanbao ) : 0;
	// 	m_Property.m_dwGiftCoupon = ( m_Property.m_dwGiftCoupon > dwGift)  ? ( m_Property.m_dwGiftCoupon - dwGift )  : 0;
	// 	m_Property.m_Money = ( m_Property.m_Money > dwGold ) ? ( m_Property.m_Money - dwGold ) : 0;
	// 
	// 	UpdateMoneyToC();

	return bRst;
}

BOOL CPlayer::DelMoney(int MoneyType, DWORD dwValue)
{
	if ( this == NULL )
		return FALSE;

	BOOL bRst = TRUE;


	return bRst;
}

void CPlayer::UpdateMoneyToC()
{
}

DWORD CPlayer::GetMoney(BYTE type)
{
	// 	if ( this == NULL )
	// 		return 0;
	// 
	// 	switch( type )
	// 	{
	// 	case MoneyType_Yuanbao:
	// 		return m_Property.m_dwXYDPoint;
	// 	case MoneyType_Gift:
	// 		return m_Property.m_dwGiftCoupon;
	// 	case MoneyType_Gold:
	// 		return m_Property.m_Money;
	// 	default:
	// 		return 0;
	// 	}
	return 0;
}

WORD CPlayer::GetAddMedicalEthicPoint( WORD wCureType )
{
	if ( wCureType == 0 || wCureType > 4 )
		return 0;

	//// 治疗轻伤经脉，获得1点医德值-->改为3点医德值
	//// 治疗中伤经脉，获得3点医德值-->改为9点医德值
	//// 治疗重伤经脉，获得9点医德值-->改为15点医德值
	//// 治疗断裂经脉，获得15点医德值-->改为25点医德值
	//if ( wCureType == 4 )
	//    return 25;  
	//else if ( wCureType == 1 || wCureType == 2 || wCureType == 3 )
	//    return ( 3 + ( wCureType - 1 ) * 6 );

	// 治疗轻伤经脉，获得1点医德值
	// 治疗中伤经脉，获得3点医德值
	// 治疗重伤经脉，获得9点医德值
	// 治疗断裂经脉，获得15点医德值
	if ( wCureType == 1 )
		return wCureType;  
	else if ( wCureType == 2 || wCureType == 3 || wCureType == 4 )
		return ( (wCureType * 2 - 3 ) * 3 );  // (2n-3)*3
	return 0;
}

void CPlayer::AddMedicalEthics( WORD wAddPoint, WORD wDesPLevel )
{
	// 	if ( wAddPoint == 0 )
	// 		return;
	// 
	// 	// 等级判断
	// 	if ( wDesPLevel != 0)
	// 	{
	// 		if ( m_Property.m_Level < 30 )
	// 			return TalkToDnid(m_ClientIndex, "当阁下修为达到30级，行医后将额外获得医德值！");
	// 
	// 		if ( (m_Property.m_Level > wDesPLevel) && (m_Property.m_Level - wDesPLevel > 50 ) )
	// 			return TalkToDnid(m_ClientIndex, "阁下医治低于自己50级的玩家不会获得医德值！");
	// 
	// 		if ( (m_Property.m_Level < wDesPLevel) && (wDesPLevel - m_Property.m_Level > 25 ) )
	// 			return TalkToDnid(m_ClientIndex, "阁下医治高于自己25级的玩家不会获得医德值！");
	// 	}
	// 
	// 	// 更新医德点每日最大值
	// 	UpdatePlayerDataValsPerDay( m_Property.dwLastAddMETime, &m_Property.wMaxPerDayME, ( LPDWORD )0 );
	// 
	// 	// 计算某等级意得点最大值
	// 	WORD wTempPerDayMECount = CalculateMaxPerDayME( m_Property.m_Level );
	// 
	// 	DWORD wOldMedicalEthics = m_Property.wMedicalEthics;
	// 	m_Property.wMedicalEthics += wAddPoint;
	// 	m_Property.wMaxPerDayME   += wAddPoint;
	// 
	// 	// teset
	// 	/*rfalse( 2, 1, "本次行医获得医德值%d", wAddPoint);
	// 	rfalse( 2, 1, "当前等级每日可以获得医德值上限%d", wTempPerDayMECount);
	// 	rfalse( 2, 1, "当前等级可以获得医德值上限%d", wTempPerDayMECount * 25);
	// 	rfalse( 2, 1, "1当前医德点%d", m_Property.wMedicalEthics);
	// 	rfalse( 2, 1, "1当日医德点累积%d", m_Property.wMaxPerDayME);*/
	// 
	// 	// 每日累计上限
	// 	if ( m_Property.wMaxPerDayME >= wTempPerDayMECount )
	// 	{
	// 		m_Property.wMedicalEthics = (m_Property.wMedicalEthics > (m_Property.wMaxPerDayME - wTempPerDayMECount)) 
	// 			? ( m_Property.wMedicalEthics-(m_Property.wMaxPerDayME - wTempPerDayMECount) ) : 0;
	// 
	// 		m_Property.wMaxPerDayME = wTempPerDayMECount;
	// 		//test
	// 		/*rfalse( 2, 1, "2当前医德点%d", m_Property.wMedicalEthics);
	// 		rfalse( 2, 1, "2当日医德点累积%d", m_Property.wMaxPerDayME);*/
	// 		TalkToDnid(m_ClientIndex, "今日医德值获得达到最大值，行医将不能获得更多了！");
	// 	}
	// 
	// 	// 总的累积上限
	// 	if ( m_Property.wMedicalEthics >= wTempPerDayMECount * 25 )
	// 	{
	// 		m_Property.wMaxPerDayME = (m_Property.wMaxPerDayME > (m_Property.wMedicalEthics - wTempPerDayMECount * 25))
	// 			? ( m_Property.wMaxPerDayME - ( m_Property.wMedicalEthics - wTempPerDayMECount * 25) ) : 0;
	// 
	// 		m_Property.wMedicalEthics = wTempPerDayMECount * 25;
	// 		//test
	// 		/*rfalse( 2, 1, "3当前医德点%d", m_Property.wMedicalEthics);
	// 		rfalse( 2, 1, "3当日医德点累积%d", m_Property.wMaxPerDayME);*/
	// 		TalkToDnid(m_ClientIndex, "医德值总量达到最大值，行医将不能获得更多了！");
	// 	}
	// 
	// 	char tempchar[1024] = {0};
	// 	sprintf( tempchar, "本次行医获得医德值%d点，今日累计获得%d点！", m_Property.wMedicalEthics - wOldMedicalEthics, m_Property.wMaxPerDayME );
	// 	TalkToDnid(m_ClientIndex, tempchar);
}

/*
貌似还有更简单的方法，由于time是从1970:1:1 0:0:0开始计算的秒数
而一天就是24*60*60秒，是恒定的，所以说，可以通过这个逻辑来迅速计算跨天的时间
DWORD GetTimeDayZero() {

static DWORD checkTick = 0;
static DWORD curDay0 = 0; // 当天0点数据
if ( abs( ( int )timeGetTime() - ( int )checkTick ) > 1000 ) {
checkTick = timeGetTime();
time_t ct = time( NULL );
tm *wt = localtime( &ct );          // 通过判断当前时间来计算出当天跨天时的时间点
DWORD lastHour = 23 - wt->tm_hour;  // 距当前天的0点还有多少个小时（排除分级数据）
DWORD lastMin = 59 - wt->tm_min;    // 距当前小时的0分还有多少分（排除秒级数据）
DWORD lastSec = 60 - wt->tm_sec;    // 距当前分的0秒还有多少秒
curDay0 = lastHour * 360 + lastMin * 60 + lastSec;
}
return curDay0;
}
*/

BOOL IsDayPass( DWORD oldTime, DWORD newTime ) {
	// + 8*60*60 是为了将UTC格式转为中国时区。
	return ( DWORD )( ( oldTime + 8*60*60 ) / ( 24*60*60 ) ) != ( DWORD )( ( newTime + 8*60*60 ) / ( 24*60*60 ) );
}

WORD CPlayer::CalculateMaxPerDayME( WORD wLevel )
{
	// 每日上限
	// 100级前每升1级，  上限+1
	// 101-200级每升1级，上限+2
	// 200-230级每升1级，上限+4
	// 230-250级每升1级，上限+8
	// 250-300级每升1级，上限+15

	WORD wTempPerDayMECount = 50;
	if ( wLevel <= 100 )
		wTempPerDayMECount += wLevel;
	else if ( wLevel > 100 && wLevel <= 200 )
		wTempPerDayMECount += ( 100 + (( wLevel - 100 ) << 1) );
	else if ( wLevel > 200 && wLevel <= 230 )
		wTempPerDayMECount += ( 100 + 200 + (( wLevel - 200 ) << 2) );
	else if ( wLevel > 230 && wLevel <= 250 )
		wTempPerDayMECount += ( 100 + 200 + 120 + (( wLevel - 230 ) << 3) );
	else if ( wLevel > 250 && wLevel <= 300 )
		wTempPerDayMECount += ( 100 + 200 + 120 + 160 + (( wLevel - 250 ) * 15) );

	return wTempPerDayMECount;
}

void CPlayer::OnRevDoctorAddBuffMsg( struct SQDoctorAddBuffMsg* pMsg, CPlayer* pDoctor )
{
}

void  CPlayer::DoAddDoctorBuff( CPlayer* pDoctor, bool useItem )
{
}

void CPlayer::UpdateDoctorBuffTime( bool loginCheck ) // 更新医德BUFF剩余时间
{
}

BOOL CPlayer::AddWhineBuff(WORD buffType)
{
	return TRUE;
}

void CPlayer::UpdateWhineBuffTime(bool loginCheck)	// 更新桃酒BUFF剩余时间
{
}

void CPlayer::SendFirstUseQXD() //第一次使用气血丹，让客户端把气血丹自动弄成开启
{
	SQAutoUseItemMsg QXDFirstUseMsg;
	QXDFirstUseMsg.type = 1;
	QXDFirstUseMsg.bSetQXD = true;
	g_StoreMessage( m_ClientIndex, &QXDFirstUseMsg, sizeof(SQAutoUseItemMsg) );
	return ;
}

void CPlayer::SendFirstUseLZ() //第一次使用灵珠，让客户端把灵珠自动弄成开启
{
	SQAutoUseItemMsg LZFirstUseMsg;
	LZFirstUseMsg.type = 2;
	LZFirstUseMsg.bSetLZ  = true;
	g_StoreMessage( m_ClientIndex, &LZFirstUseMsg, sizeof(SQAutoUseItemMsg) );
	return ;
}

bool CPlayer::IsWuLinChief()
{
	extern string s_WulinChiefName;
	extern BYTE s_WulinChiefSchool;
	if ( s_WulinChiefName == m_Property.m_Name && m_Property.m_School + 1 == s_WulinChiefSchool)
		return true;
	return false;
}

extern string s_FactionName;
extern DWORD s_dwFactionTitle;

BOOL CPlayer::IsInFactionFirst()
{
	bool ck1 = ( m_dwFactionTitleID && ( s_dwFactionTitle == 0 || s_dwFactionTitle == m_dwFactionTitleID ) );
	bool ck2 = ( !s_FactionName.empty() && ( s_FactionName == m_Property.m_szTongName ) );

	return ( ck1 || ck2 );
}

extern SScoreTable s_Scores;
void CPlayer::RecvGetNewXYDScoresInf( SQUpdateScoreListExMsg * pMsg )
{
	if ( pMsg == NULL )
		return;

	if ( pMsg->wScoreType >= SScoreTable::ST_MAX )
		return;

	WORD wScoresSize = 0;
	BOOL isCheck     = TRUE;
	BOOL isCheckEx   = FALSE;
	SScoreTable::SScoreRecode   *pRecode   = NULL;
	SScoreTable::SScoreRecodeEx *pRecodeEx = NULL;
	wScoresSize = MAX_SCORE_NUMBER;

	switch( pMsg->wScoreType ) 
	{
	case SScoreTable::ST_ALEVEL:        pRecode = s_Scores.ALevel;                          break;
	case SScoreTable::ST_AMONEY:        pRecode = s_Scores.AMoney;                          break;
	case SScoreTable::ST_RFAME:         pRecode = s_Scores.FAME[0];                         break;
	case SScoreTable::ST_LFAME:         pRecode = s_Scores.FAME[1];                         break;
	case SScoreTable::ST_SLEVEL:        pRecode = s_Scores.Level[ m_Property.m_School ];  break;
	case SScoreTable::ST_SMONEY:        pRecode = s_Scores.Money[ m_Property.m_School ];  break;
	case SScoreTable::ST_MEDICALETHICS: pRecode = s_Scores.MedicalEthics;                   break;
	case SScoreTable::ST_KILLVAL:       pRecode = s_Scores.PKValue;                         break;

	default:
		isCheck = FALSE;
		wScoresSize = MAX_FIVESCORE_NUMBER;
		if ( pMsg->wScoreType == SScoreTable::ST_FACTION )
			// 五强帮派
			;
		else
		{
			// 新奇榜
			isCheckEx = TRUE;
			switch ( pMsg->wScoreType )
			{
			case SScoreTable::ST_ROSENUM:   pRecodeEx = s_Scores.RoseNum;   break;
			case SScoreTable::ST_BAOTU:     pRecodeEx = s_Scores.BaoTu;     break;
			case SScoreTable::ST_ZHENFA:    pRecodeEx = s_Scores.ZhenFa;    break;
			case SScoreTable::ST_HUNTER:    pRecodeEx = s_Scores.HuntScore; break;
			case SScoreTable::ST_MIGONG:    pRecodeEx = s_Scores.MazeScore; break;	
			default:
				return;
			}
		}
	}

	if ( pRecodeEx == NULL && isCheckEx == TRUE )
		return;

	if ( pRecode == NULL && isCheck == TRUE ) 
		return; 

	SAUpdateScoreListExMsg msg;
	msg.wScoreType = (SScoreTable::ScoreType)pMsg->wScoreType;
	BOOL isSend    = FALSE;
	lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );//序列化排行榜数据
	try
	{
		for ( int i = 0; i < wScoresSize; ++i )
		{
			int row = -1;
			switch ( pMsg->wScoreType )
			{
			case SScoreTable::ST_FACTION:  // 是五强帮派
				{
					int nFanctionState = 0;
					std::map< std::string, SFactionData >::iterator iter;
					iter = factionManagerMap.find( TopFactionName[ i ] );
					if ( iter == factionManagerMap.end() )
						nFanctionState = 2; // 解散或者不存在
					else
						nFanctionState = iter->second.stFaction.byOpenRApplyJoin;   // 开启或关闭申请

					if ( !TopFactionName[ i ].empty() )
					{
						isSend = TRUE;
						slm( TopFactionName[ i ].c_str() );                         // 帮会名
						slm( nFanctionState );                                      // 帮会状态

						if ( nFanctionState != 2 )
						{                    
							slm( iter->second.stFaction.byMemberNum );              // 成员数量
							slm( iter->second.stFaction.dwFactionProsperity );      // 繁荣度            
							slm( FactionFightInf[0][i] );                           // 城战胜利次数
							slm( FactionFightInf[1][i] );                           // 城战总次数

							if ( iter->second.stMember[0].szName[0] != 0 )
								slm( iter->second.stMember[0].szName );             // 帮主姓名
							else
								slm( "未知" );
						}
					}
				}
				break;

				// 新奇榜
			case SScoreTable::ST_ROSENUM:
			case SScoreTable::ST_HUNTER:
			case SScoreTable::ST_MIGONG:
			case SScoreTable::ST_BAOTU:
			case SScoreTable::ST_ZHENFA:
				{
					if ( pRecodeEx[i].Name[0] != 0 )
						isSend = TRUE;

					slm( pRecodeEx[i].Name );
					slm( pRecodeEx[i].dwValue );
				}
				break;

			default:
				{
					if ( pRecode && ( pRecode[i].Name[0] != '\0' ) )
						isSend = TRUE;

					slm( pRecode[i].Name );
					slm( pRecode[i].dwValue );
				}
				break;
			}
		}
	}
	catch ( lite::Xcpt & )
	{ 
	}

	if ( isSend )
		g_StoreMessage( m_ClientIndex, &msg, DWORD(sizeof(msg) - slm.EndEdition()) );
}

void CPlayer::OnOpenExtraTelergy( struct SQOpenExtraTelergyMsg *pMsg )
{
	// 	if ( pMsg == NULL )
	// 		return;
	// 
	// 	if ( pMsg->number != 2 && pMsg->number != 3 )
	// 		return;
	// 
	// 	if ( pMsg->state == 1 )     // 收金质玉虚针
	// 	{
	// 
	// 		WORD number = 1;
	// 		if ( (pMsg->number == 2 && m_Property.m_ExtraTelergy.state[3] != 0) || (pMsg->number == 3 && m_Property.m_ExtraTelergy.state[2] != 0) )
	// 		{
	// 			number = 2;
	// 		}
	// 
	// 		BOOL result = CheckGoods( 40195, number, FALSE );
	// 		if ( result == FALSE )
	// 		{
	// 			TalkToDnid( m_ClientIndex, "阁下没有足够的金质玉虚针激活当前心法格！" );
	// 			TalkToDnid( m_ClientIndex, "本次修改失败！" );
	// 			return;
	// 		}      
	// 
	// 		m_Property.m_ExtraTelergy.state[pMsg->number] = pMsg->state;
	// 
	// 		// 同步客户端状态
	// 		SAOpenExtrTelergyMsg msg;
	// 		msg.number = pMsg->number;
	// 		msg.state = pMsg->state;
	// 
	// 		g_StoreMessage( m_ClientIndex, &msg, sizeof( SAOpenExtrTelergyMsg ) );
	// 	}
	// 	else if ( pMsg->state == 0 )  // 同步关闭状态
	// 	{
	// 		m_Property.m_ExtraTelergy.state[pMsg->number] = pMsg->state;
	// 
	// 		SAOpenExtrTelergyMsg msg;
	// 		msg.number = pMsg->number;
	// 		msg.state = pMsg->state;
	// 
	// 		g_StoreMessage( m_ClientIndex, &msg, sizeof( SAOpenExtrTelergyMsg ) );
	// 	}
	// 
	// 	if ( pMsg->state == 1 || pMsg->state == 0 )
	// 	{
	// // 		CalculateTelergyEffects();
	// // 		SendAddPlayerLevel( 0, 0 );
	// 	}
}

void  CPlayer::CheckAndSendHellioMaskBUFF( BOOL bIsLogin )
{
}

void CPlayer::UpdateXinMoVaule()
{
}

void CPlayer::SendUpdateXMVauleMsg()
{
}

void CPlayer::AddXinMoBuff()
{
}

void CPlayer::CheckUseXQD( WORD itemIndex, DWORD dwHPMin, DWORD dwHPMax, BOOL &autoUseItemRuslet )
{
}

void CPlayer::SendUpdateDecDurTimeMsg()
{
	//     SAUpdateDecDurTimeMsg msg;
	//     msg.dwDecDurTimeEnd = m_Property.dwDecDurTimeEnd;
	//     g_StoreMessage( m_ClientIndex, &msg, sizeof(msg) );
}

void CPlayer::OnRecvQuestAddPoints(SQuestAddPoint_C2S_MsgBody* pMsg)
{
	//做以下验证工作
	// 	for (int i = 0;i < 4;++i)
	// 	{
	// 		if(m_Property.m_LeftPoint[ i / 2 ] >= pMsg->mQuestAddPoints[ i ])
	// 		{
	// 			m_Property.m_LeftPoint[ i / 2 ] -= pMsg->mQuestAddPoints[ i ];
	// 			m_Property.m_AttackAndDefense[ i ].m_wAddPoint += pMsg->mQuestAddPoints[ i ];
	// 		}
	// 	}
	// 
	// 	SendUpdateAttackDefenseLeftPointsMsg();
	/*SendStatePanelDataMsg();*/
}

BOOL CPlayer::RecvCloseGroundItem()
{
	CloseGroundItem();	// 取消查看包裹

	return TRUE;
}

BOOL CPlayer::RecvStartTakeChance(SQGroundItemChanceBack *pMsg)
{
	if (SQGroundItemChanceBack::SQGICB_GIVEUPALL != pMsg->m_Choice)
	{
		OwnedChanceItems::iterator it = m_OwnedChangeItems.find(pMsg->m_Index);
		if (it == m_OwnedChangeItems.end())
			return FALSE;

		CGroundItemWinner::GetInstance().StartTakeChance(this, pMsg->m_Index, it->second, SQGroundItemChanceBack::SQGICB_GIVEUP == pMsg->m_Choice ? true : false);
	}
	else		// 全部放弃
	{
		OwnedChanceItems copy(m_OwnedChangeItems);

		for (OwnedChanceItems::iterator it = copy.begin(); it != copy.end(); ++it)
			CGroundItemWinner::GetInstance().StartTakeChance(this, it->first, it->second, true);
	}

	return TRUE;
}

BOOL CPlayer::RecvCheckGroundItem(SQCheckGroundItemMsg *pMsg)
{
	LPIObject Item = m_ParentRegion->SearchObjectListInArea(pMsg->dwItemGID, m_ParentArea->m_X, m_ParentArea->m_Y);
	if (!Item)
		Item = m_ParentRegion->SearchObjectListInAreas(pMsg->dwItemGID, m_ParentArea->m_X, m_ParentArea->m_Y);

	if (CSingleItem *pItem = (CSingleItem *)Item->DynamicCast(IID_SINGLEITEM))
	{
		// 		if (m_pCheckItem == pItem)				// 拒绝重复查看
		// 			return FALSE;

		// 如果有队伍保护，那么无法查看别的队伍的包裹
		if (pItem->m_dwTeamID && pItem->m_dwTeamID != m_dwTeamID)
		{
			rfalse("此包裹处于队伍%d的保护期内，无法查看！", pItem->m_dwTeamID);
			return FALSE;
		}

		///在没有队伍的情况下判断保护
		if (!pItem->m_dwTeamID&&pItem->m_dwGID!=GetGID())
		{
			//rfalse(" 你现在还无法拾取别人的包裹！");
			TalkToDnid(m_ClientIndex,"你现在还无法拾取别人的包裹！",0);
			return FALSE;
		}

		if (!pItem->m_IsDispatched && !pItem->StartDispatch(this))
			return FALSE;


		if (m_pSingleItem && m_pSingleItem->GetGID() != pItem->GetGID()){
			m_pSingleItem->RemoveChecker(this);// 移除旧的映射关系
			m_pSingleItem = pItem;	// 添加新的映射关系
			m_pSingleItem->AddChecker(this);
		}
		else{
			m_pSingleItem = pItem;	// 添加新的映射关系
			m_pSingleItem->AddChecker(this);
		}

		return OnPickupItem(m_pSingleItem);
	}

	return TRUE;
}

// 初始化装备结构
void CPlayer::InitEquipmentData()
{
	memset(m_EquipFactorValue, 0, sizeof(m_EquipFactorValue));

	// 计算随机属性
	for (int i = 0; i < EQUIP_P_MAX; ++i)
	{
		if (0 == m_Property.m_Equip[i].wIndex)
			continue;

// 		// 耐久度为0，则不加属性
// 		if (!m_Property.m_Equip[i].attribute.currWear)
// 			continue;

		const SItemFactorData *pItemFactor = CItemService::GetInstance().GetItemFactorData(m_Property.m_Equip[i].wIndex);
		if (!pItemFactor)
			continue;

		// 装备本身的属性加成
		//InitEquipAttribute(pItemFactor);

		// 装备宝石的属性加成
		InitEquipJewelData(i);

		// 装备的本身的属性属性
		InitEquipAttriData(i);
	}

	// 判定是否是套装，套装属性加成
//	InitEquipSuitAttributes();
}

bool CPlayer::SendMove2TargetForAttackMsg(INT32 skillDistance, INT32 skillIndex, CFightObject* pLife)
{
	//////////////////////////////////////////////////////////////////////////
	// [2012-5-11 18-29 gw: todo..以后需要回复，如果不能移动了，就不要发送]
	if (!m_bPlayerAction[CST_CANMOVE])
	{
		return false;
	}

	SMove2TargetForAttack_S2C_MsgBody msg;
	msg.mAttackGID		= GetGID();
	msg.mDefenderGID	= pLife->GetGID();
	msg.distance		= skillDistance;

	g_StoreMessage(m_ClientIndex, &msg, sizeof(SMove2TargetForAttack_S2C_MsgBody));

	return true;
}

INT32 CPlayer::GetSkillIDBySkillIndex(INT32 index)
{
	if (index < 0 || index >= MAX_SKILLCOUNT)
		return -1;

	SSkill *pSkill = &m_Property.m_pSkills[index];
	if (pSkill->wTypeID)
	{
		MY_ASSERT(pSkill->byLevel > 0 && pSkill->byLevel <= MaxSkillLevel);
		return pSkill->wTypeID;
	}

	return -1;
}

INT32 CPlayer::GetCurrentSkillLevel(DWORD dwSkillIndex)
{
	SSkill *pSkill = &m_Property.m_pSkills[dwSkillIndex];
	if (pSkill->wTypeID)
	{
		MY_ASSERT(pSkill->byLevel > 0 && pSkill->byLevel <= MaxSkillLevel);
		return pSkill->byLevel;
	}

	return -1;
}

void CPlayer::OnChangeState(EActionState newActionID)
{
}

void CPlayer::OnRunEnd()
{
	if (m_IsMove2Attack == SQSynWayTrackMsg::move_2_attack)
	{
		//rfalse(2, 1, "玩家攻击移动完毕，当前坐标为%f, %f,攻击目标ID:%d", m_curX, m_curY,m_backUpQusetSkillMsg.mDefenderGID);

		// 获取保持的技能请求消息，开始处理
		m_backUpQusetSkillMsg = m_AtkContext;
		__ProcessQuestSkill(&m_backUpQusetSkillMsg, true);
	}
}

void CPlayer::Move(INT32 moveTileX,INT32 moveTileY)
{
	SNotifyMove_S2C_MsgBody msg;
	msg.moveTileX = moveTileX;
	msg.moveTileY = moveTileY;
	g_StoreMessage(m_ClientIndex,&msg,sizeof(SNotifyMove_S2C_MsgBody));
}

void CPlayer::UpdateEquipWear(int pos, int value, bool IfMaxWear)
{
	if (pos < EQUIP_P_WEAPON || pos > EQUIP_P_MAX-1 || value <= 0)
		return;

	if (0 == m_Property.m_Equip[pos].wIndex || 0 == m_Property.m_Equip[pos].attribute.currWear || 0 == m_Property.m_Equip[pos].attribute.maxWear)
		return;

	// 判断是否是更新当前最大耐久度
	if (IfMaxWear)
		m_Property.m_Equip[pos].attribute.maxWear = (m_Property.m_Equip[pos].attribute.maxWear <= value ) ? 0 : (m_Property.m_Equip[pos].attribute.maxWear - value);
	else
		m_Property.m_Equip[pos].attribute.currWear = (m_Property.m_Equip[pos].attribute.currWear <= value ) ? 0 : (m_Property.m_Equip[pos].attribute.currWear - value);

	// 如果当前耐久度大于最大耐久度，则重置当前耐久度为最大耐久度
	if (m_Property.m_Equip[pos].attribute.currWear > m_Property.m_Equip[pos].attribute.maxWear)
	{
		m_Property.m_Equip[pos].attribute.currWear = m_Property.m_Equip[pos].attribute.maxWear;
	}

	// 发送更新消息
	SAUpdateCurEqDuranceMsg msg;
	msg.wEqIndex			= m_Property.m_Equip[pos].wIndex;
	msg.byPos				= pos;
	msg.wCurEqDuranceCur	= m_Property.m_Equip[pos].attribute.currWear;
	msg.wMaxEqDuranceCur	= m_Property.m_Equip[pos].attribute.maxWear;

	g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));

	// 只有耐久度为0时，才影响到玩家的属性，所以如果变为了0，则检查所有的装备
	if (!m_Property.m_Equip[pos].attribute.currWear)
	{
		InitEquipmentData();
		UpdateAllProperties();
	}
}

// 处理被攻击
void CPlayer::OnDamage(INT32* nDamage, CFightObject *pFighter)
{

	static int WearPos[EQUIP_P_MAX];

	CancelCollect();

	//BreakOperation();
	m_BatchReadyEvent.BreakOperationReadyEvent(Event::EEventInterrupt_Damaged);

	if (GetCurActionID()==EA_ZAZEN)
	{
		SetZazen(false);
	}

// 	CPlayer *attacker = (CPlayer *)pFighter->DynamicCast(IID_PLAYER);
// 	if (attacker)
// 	{
// 		// 减少攻击者武器耐久
// 		if (0 != attacker->m_Property.m_Equip[EQUIP_P_WEAPON].wIndex && 0 != attacker->m_Property.m_Equip[EQUIP_P_WEAPON].attribute.currWear)
// 		{
// 			const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(attacker->m_Property.m_Equip[EQUIP_P_WEAPON].wIndex);
// 			// 判断是否是永不磨损的装备
// 			if (0 != itemData->m_MaxWear)
// 				attacker->UpdateEquipWear(EQUIP_P_WEAPON, 1);
// 		}
// 	}

	// 随机选取一件防具减少耐久
// 	int EquipedCount = 0;
// 	for (int i = EQUIP_P_SHOSE; i <= EQUIP_P_PRECIOUS; ++i)
// 	{
// 		const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(m_Property.m_Equip[i].wIndex);
// 		if (!itemData)
// 			continue;
// 
// 		if (0 != m_Property.m_Equip[i].wIndex && 
// 			0 != m_Property.m_Equip[i].attribute.currWear && 
// 			0 != itemData->m_MaxWear)
// 			WearPos[++EquipedCount] = i;
// 	}
// 	if (EquipedCount)
// 	{
// 		int RatePos = CRandom::RandRange(1, EquipedCount);
// 		UpdateEquipWear(RatePos, 1);
// 	}


	if (m_CurHp == 0)
		return;

	//判断变身宠物魂力是否为0，当变身宠物被攻击时魂力减1
	if (m_Property.m_FightPetActived != 0xff)
	{
		if (m_Property.m_NewPetData[m_Property.m_FightPetActived].m_CurPetDurable == 0)
		{
			DWORD NeedTatalTime = _L_GetLuaValue("GetHunliResumeTime");
			if (NeedTatalTime == 0xffffffff)
				NeedTatalTime = 0;
			INT64 CurTime = _time64(NULL);
			m_Property.m_NewPetData[m_Property.m_FightPetActived].m_DurableResumeNeedTime = NeedTatalTime + (CurTime - m_dwLoginTime);
			m_SysAutoTransToPlayer = TRUE;
			TransferPlayer();
			return;
		}
		m_Property.m_NewPetData[m_Property.m_FightPetActived].m_CurPetDurable--;
		SAPetSynDurableMsg SynPetDurable;
		SynPetDurable.m_PetIndex = m_Property.m_FightPetActived;
		SynPetDurable.m_CurDurable = m_Property.m_NewPetData[m_Property.m_FightPetActived].m_CurPetDurable;
		g_StoreMessage(m_ClientIndex, &SynPetDurable, sizeof(SAPetSynDurableMsg));
	}

	// 查看伤害吸收
	/*if (m_SorbDamageValue)
	*nDamage -= m_SorbDamageValue;

	if (*nDamage < 0)
	*nDamage = 0;

	if (*nDamage && m_SorbDamagePercent)
	{
	*nDamage -= (*nDamage * 100 / m_SorbDamagePercent);
	rfalse(2, 1, "吸收了d%%%的伤害，还剩余%d", m_SorbDamagePercent, *nDamage);
	}*/

	if (!m_pAttribListener)		// 如果当前没有锁定其目标，否则继续锁定当前目标
		StoreTarget(pFighter);

	ModifyCurrentHP(-(*nDamage), 0, pFighter);

	if (m_pFightPet)
	{
		m_pFightPet->SetCurEnemy(pFighter->GetGID(),FPET_ATKPLAYER);
	}

	AddToHatelist(pFighter);
	if (g_Script.PrepareFunction("OnObjectDamage"))
	{
		g_Script.PushParameter(GetGID());
		g_Script.PushParameter(*nDamage);
		if (pFighter)
		{
			g_Script.PushParameter(pFighter->GetGID());
		}
		g_Script.Execute();
	}
	
	return;
}

void CPlayer::StoreTarget(CFightObject *pObj)
{
	if (pObj)
	{
		if (m_pAttribListener)
		{
			// 如果没有切换目标，那么直接返回
			if (m_pAttribListener->GetSubject() == pObj)
				return;
			else
				ProcessForceQusetEnemyInfo();
		}
		m_pAttribListener = new AttribListener(this);

		pObj->SetListener(m_pAttribListener);	// 监听对象属性变化
	}
	else if(m_pAttribListener)
	{
		ProcessForceQusetEnemyInfo();
	}
}

void CPlayer::LockeOperator( )
{
	// 	m_bPlayerAction[ECA_CANWALK] = false;
	// 	m_bPlayerAction[ECA_CANRUN] = false;
	// 	m_bPlayerAction[ECA_CANFIGHT] = false;
	// 
	// 	this->Stand( );
	// 
	// 	SALockPlayer lockPlayer;
	// 	lockPlayer.byLock = true;
	// 	g_StoreMessage(this->m_ClientIndex,&lockPlayer,sizeof(lockPlayer));
}
void CPlayer::UnLockeOperator( )
{
	// 	m_bPlayerAction[ECA_CANWALK] = true;
	// 	m_bPlayerAction[ECA_CANRUN] = true;
	// 	m_bPlayerAction[ECA_CANFIGHT] = true;
	// 
	// 	SALockPlayer lockPlayer;
	// 	lockPlayer.byLock = false;
	// 	g_StoreMessage(this->m_ClientIndex,&lockPlayer,sizeof(lockPlayer));
}
void CPlayer::LockEvent::OnCancel( EventMgr *mgr )
{
	if(!mPlayer) return;
	mPlayer->UnLockeOperator(); 
	LuaFunctor( g_Script, "OnUnLockeOperator" )[mPlayer->GetSID()]();
}
void CPlayer::LockEvent::OnActive( EventMgr *mgr )
{
	if(!mPlayer) return;
	mPlayer->UnLockeOperator(); 
	LuaFunctor( g_Script, "OnUnLockeOperator" )[mPlayer->GetSID()]();
}

void CPlayer::UpdateSKillBox(SUpdateSkillBox_C2S_MsgBody* msg)
{
	// 	for (int i = 0;i < 6;++i)
	// 	{
	// 		for (int j = 0;j < 9;j ++)
	// 		{
	// 			m_Property.stBox[ i ][ j ] = msg->stBox[ i ][ j ];
	// 		}
	// 	}
}
void CPlayer::UpdatePracticeBox(SUpdatePracticeBox_C2S_MsgBody * msg)
{
}

void CALLBACK CPlayer::OnCloseListener(CFightListener *p)
{
	MY_ASSERT(p);
	MY_ASSERT(p == m_pAttribListener);

	m_pAttribListener = 0;
	ProcessForceQusetEnemyInfo();
}

// 强制取消锁定
void CPlayer::ProcessForceQusetEnemyInfo()
{
	if (m_pAttribListener)
	{
		MY_ASSERT(m_pAttribListener->GetSubject());

		m_pAttribListener->GetSubject()->RemoveListener(m_pAttribListener);
		m_pAttribListener = 0;
	}

	SShowEnemyInfo_S2C_MsgBody msg;
	msg.mEnemyGID	= -1;
	msg.mPlayerGID	= GetGID();
	g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));
}

SCC_RESULT CPlayer::CheckConsume(INT32 skillIndex, int& consumeHP, int& consumeMP)
{
	if (skillIndex < 0 || skillIndex >= MAX_SKILLCOUNT)
	{
		return SCCR_INVALID_SKILL_INDEX;
	}

	SSkill* pSkill = &m_Property.m_pSkills[skillIndex];

	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(pSkill->wTypeID);
	if (!pData)
	{
		return SCCR_INVALID_SKILL_INDEX;
	}

	consumeMP = pData->m_ConsumeMP/*GetMpConsume(pData->m_SkillPropertyType, pData->m_ConsumeMP, m_Property.m_pSkills[skillIndex].byLevel)*/;

	if (0 == consumeMP)
	{
		return SCCR_OK;
	}

	if (m_CurMp < (DWORD)consumeMP)
	{
		return SCCR_MP_NOT_ENOUGH;
	}

	return SCCR_OK;
}
//SCC_RESULT CPlayer::CheckConsume(INT32 skillIndex, int& consumeHP, int& consumeMP)
//{
// 	if (skillIndex < 0 || skillIndex >= MAX_SKILLCOUNT)
// 		return SCCR_INVALID_SKILL_INDEX;
// 
//  	SSkill* pSkill = &m_Property.m_pSkills[ skillIndex ];
//  	CSkillDataTable::SkillData* pSkillData = CSkillDataTable::GetSkillData(pSkill->wTypeID);
//  	if (pSkillData)
//  		return SCCR_INVALID_SKILL_INDEX;
// 	
//	CSkillDataTable::CheckConsume(pSkillData,pSkill->byLevel, consumeHP, consumeMP, consumeSP);
// 
// 	if (m_CurHp < (DWORD)consumeHP) return SCCR_HP_NOT_ENOUGH;
// 	if (m_CurMp < (WORD)consumeMP) return SCCR_MP_NOT_ENOUGH;
// 	if (m_CurSP < (WORD)consumeSP) return SCCR_SP_NOT_ENOUGH;
//	return SCCR_OK;
//}

WORD CPlayer::GetMpConsume(BYTE type,WORD baseMPConsume, WORD telergyLevel)
{
	WORD consume = 0;

	//======================================
	//==特殊技能，策划没有给，暂时先用定值==
	//======================================
	if (SPT_SPECIAL == type || SPT_CHARGE == type)
	{
		return baseMPConsume;
	}

	// 近程、远程、范围攻击内力消耗
	switch(type)
	{
	case SPT_NEARBY:
		consume = telergyLevel * 2 + baseMPConsume;
		break;

	case SPT_LONGDISTANCE:
		consume = telergyLevel * 4 + baseMPConsume;
		break;

	case SPT_RANGE:
		consume = telergyLevel + baseMPConsume;
		break;
	case SPT_AUXILIARY_ACTIVE:
	case SPT_AUXILIARY_PASSIVE:
		consume = telergyLevel + baseMPConsume;
		break;

	}

	if (!consume)
		consume = 1;

	//TalkToDnid(m_ClientIndex, FormatString("内力消耗 %d", consume));
	return consume;
}

bool CPlayer::ConsumeHP(int consume)
{
	ModifyCurrentHP(-m_AtkContext.m_consumeHP, 0, this);
	return true;
}

bool CPlayer::ConsumeMP(int consume)
{
	ModifyCurrentMP(-m_AtkContext.m_consumeMP, 0);
	return true;
}

bool CPlayer::ConsumeTP(int consume)
{
	ModifyCurrentTP(-consume, BAM_POT);
	return true;
}

#define SWAPNUM(a,b,cur) cur = a; a = b ;b = cur;
//---------------------------------------------------------------------------------------------------------
void CPlayer::SendUpSpeed()
{
	SUpSpeed_S2C_MsgBody msg;
	msg.objectGID = GetGID();
	msg.newSpeed = mMoveSpeed;

	Synchro(&msg,sizeof(SUpSpeed_S2C_MsgBody));
}

int CPlayer::GetTaskStatus(WORD TaskID)
{
	int complete = m_Property.IsComplete(TaskID);
	MY_ASSERT(-1 != complete);

	if (complete)
		return TS_COMPLETE;

	if (CRoleTask *task = m_TaskManager.GetRoleTask(TaskID))
	{
		int rt = m_TaskManager.IsTaskFinished(task);
		//MY_ASSERT(-1 != rt);
		if (rt == -1)
		{
			return TS_FAIL;
		}
		if (1 == rt)
			return TS_FINISHED;
		else
			return TS_ACCEPTED;
	}
	else
		return TS_NOTACCEPT;
}

// 坐骑系统
int CPlayer::GetEquipMount()
{
	SPlayerMounts *pMounts = &m_Property;

	for (size_t i = 0; i < SPlayerMounts::MAX_MOUNTS; ++i)
	{
		if (pMounts->mount[i].State & SPlayerMounts::State_Equip)
			return i;
	}

	return -1;
}

int CPlayer::GetRidingMount( )
{
	SPlayerMounts *pMounts = &m_Property;

	for (size_t i = 0; i < SPlayerMounts::MAX_MOUNTS; ++i)
	{
		if (pMounts->mount[i].State & SPlayerMounts::State_Riding)
			return i;
	}

	return -1;
}

void CPlayer::InitMounts(SPlayerMounts *pFixMounts)
{
	SPlayerMounts *pPlayerMountsProperty = &m_Property;
	*pPlayerMountsProperty = *pFixMounts;

	// 读取Player的Mounts数据,如果有装备Mounts,则添加相关效果
	EquipMount( GetEquipMount() );
	UpMounts( GetRidingMount() );
}

// 创建宠物
bool CPlayer::CreatePets(int petID, BYTE quality)
{
	SPlayerPets::Pets *pP = m_Property.pets;

	int index = 0;
	// [2012-7-24 12-06 gw: -]int maxSize = m_Property.m_petSlots + 2;
	// [2012-7-24 12-04 gw: +由于没有设置imaxSize,所以暂时使用数组大小来替代]
	int maxSize = sizeof(m_Property.pets)/sizeof(m_Property.pets[0]);

	for (index; index<maxSize; ++index)
	{
		if (0 == pP[index].ID)
			break;
	}

	if (index >= maxSize)
	{
		rfalse(2, 1, "宠物栏已满，无法加入！");
		return false;
	}

	// 初始化坐骑数据
	ZeroMemory(&pP[index], sizeof(SPlayerPets::Pets));
	pP[index].ID			= petID;
	pP[index].State			= SPlayerPets::Pet_State_None;
	pP[index].UpdateTimer	= 0;
	pP[index].Level			= 1;
	dwt::strcpy(pP[index].Name, "二驴子", 10);
	pP[index].Name[10] = 0;

	// 发送给角色
	SAAddPets addmsg;
	addmsg.pet = pP[index];
	g_StoreMessage(m_ClientIndex, &addmsg, sizeof(addmsg));

	rfalse(2, 1, "成功加入了ID为%d的宠物", pP[index].ID);
	return true;
}

// 创建侠客
bool CPlayer::CreateFightPet(WORD fpID)
{
	if (0 == fpID)
		return false;

	size_t index = 0;
	for (; index < MAX_FIGHTPET_NUM; ++index)
	{
		if (0 == m_Property.m_FightPets[index].m_fpID)
			break;
	}
	if (index == MAX_FIGHTPET_NUM)
		return false;

	if (index >= m_Property.m_FightPetActived )
	{
		TalkToDnid(m_ClientIndex,"你所携带的侠客数量太多了，必须先激活格子");
		return false;
	}
	const SFightPetBaseData *pFPData = CFightPetService::GetInstance().GetFightPetBaseData(fpID);
	if (!pFPData)
		return false;

	///玩家等级必须大于等于侠客的携带等级
	if (m_Property.m_Level<pFPData->m_nLimitLevel)
	{
		TalkToDnid(m_ClientIndex,"你所携带的侠客等级太高了，你无法装备");
		return false;
	}


	memset(&m_Property.m_FightPets[index], 0, sizeof(SFightPetExt));

	// 填充数据
	dwt::strcpy(m_Property.m_FightPets[index].m_fpName, pFPData->m_Name, SFightPetBase::FP_NAME);
	m_Property.m_FightPets[index].m_fpLevel = 1;
	m_Property.m_FightPets[index].m_fpMood	= 50;			//心情
	m_Property.m_FightPets[index].m_fpTired	= 0;				//疲劳度
	m_Property.m_FightPets[index].m_fpID	= fpID;
	m_Property.m_FightPets[index].m_niconID	= pFPData->m_niconID;
	m_Property.m_FightPets[index].m_nmodelID	= pFPData->m_nmodelID;
	m_Property.m_FightPets[index].m_nMaxHp	= pFPData->m_Attri[FPA_MAXHP][0];
	m_Property.m_FightPets[index].m_nMaxMp	= pFPData->m_Attri[FPA_MAXMP][0];
	m_Property.m_FightPets[index].m_nGongJi	= pFPData->m_Attri[FPA_GONGJI][0];
	m_Property.m_FightPets[index].m_nFangYu	= pFPData->m_Attri[FPA_FANGYU][0];
	m_Property.m_FightPets[index].m_fpMaxExp =pFPData->m_nexp;
	m_Property.m_FightPets[index].m_fpRelation=0;	//亲密度
	m_Property.m_FightPets[index].m_nBringLevel=pFPData->m_nLimitLevel;	//携带等级
	m_Property.m_FightPets[index].m_bSex=pFPData->m_bSex;	//性别

	m_Property.m_FightPets[index].m_nCurHp=m_Property.m_FightPets[index].m_nMaxHp;
	m_Property.m_FightPets[index].m_nCurMp=m_Property.m_FightPets[index].m_nMaxMp;

	//初始化侠客主人姓名
	dwt::strcpy(m_Property.m_FightPets[index].m_owername, m_Property.m_Name, CONST_USERNAME);


	//初始化技能孔数量
	m_Property.m_FightPets[index].m_fpActivedSkill = 6;
	//加入侠客的普通技能
	m_Property.m_FightPets[index].m_fpSkill[0].wTypeID = 6;
	m_Property.m_FightPets[index].m_fpSkill[0].byLevel = 1;
	// 获取稀有度
	BYTE pDest =0;
	pDest = CRandom::RandRange(pFPData->m_RareMin, pFPData->m_RareMax);
	m_Property.m_FightPets[index].m_nRare=pDest;

	BYTE rare=m_Property.m_FightPets[index].m_nRare;

	// 根据稀有度获取增值
	const SFightPetRise *pFPRise = CFightPetService::GetInstance().GetFightPetRise(rare);
	if (!pFPRise)
		return false;

	// 获取暴击增值
	pDest = CRandom::RandRange(pFPRise->m_nBaojiRiseMin, pFPRise->m_nBaojiRiseMax);
	m_Property.m_FightPets[index].m_fpBaoJi=pDest;
	//CFightPetService::GetInstance().SetFightPetBaseData(fpID,FPA_BAOJI,pDest);

	// 获取闪避增值
	pDest = CRandom::RandRange(pFPRise->m_nShanBiRiseMin, pFPRise->m_nShanBiRiseMax);
	m_Property.m_FightPets[index].m_fpShanBi=pDest;
	//CFightPetService::GetInstance().SetFightPetBaseData(fpID,FPA_SHANBI,pDest);

	// 获取悟性增值
	pDest = CRandom::RandRange(pFPRise->m_nWuXinRiseMin, pFPRise->m_nWuXinRiseMax);
	m_Property.m_FightPets[index].m_fpWuXing=pDest;

	// 获取血增加系数
	pDest = CRandom::RandRange(pFPRise->m_nHpRiseMin, pFPRise->m_nHpRiseMax);
	m_Property.m_FightPets[index].m_fpHpIncRadio=pDest;

	// 获取蓝增加系数
	pDest = CRandom::RandRange(pFPRise->m_nMpRiseMin, pFPRise->m_nMpRiseMax);
	m_Property.m_FightPets[index].m_fpMpIncRadio=pDest;

	// 获取攻击增加系数
	pDest = CRandom::RandRange(pFPRise->m_nGongJiRiseMin, pFPRise->m_nGongJiRiseMax);
	m_Property.m_FightPets[index].m_fpGongJi=pDest;

	// 获取防御增加系数
	pDest = CRandom::RandRange(pFPRise->m_nFangYuRiseMin, pFPRise->m_nFangYuRiseMax);
	m_Property.m_FightPets[index].m_fpFangYu=pDest;

	//生成是设置基础属性
	// 	m_Property.m_FightPets[index].m_BaseMaxHp	= pFPData->m_Attri[FPA_MAXHP][0];
	// 	m_Property.m_FightPets[index].m_BaseMaxMp	= pFPData->m_Attri[FPA_MAXMP][0];
	// 	m_Property.m_FightPets[index].m_BaseAtk	= pFPData->m_Attri[FPA_GONGJI][0];
	// 	m_Property.m_FightPets[index].m_BaseDefence	= pFPData->m_Attri[FPA_FANGYU][0];
	m_Property.m_FightPets[index].m_BaseCtrAtk	=m_Property.m_FightPets[index].m_fpBaoJi;
	m_Property.m_FightPets[index].m_BaseEscape	=m_Property.m_FightPets[index].m_fpShanBi;
	m_Property.m_FightPets[index].m_BaseWuXing	=m_Property.m_FightPets[index].m_fpWuXing;

	m_Property.m_FightPets[index].m_fpLearnSkill = 0;//当前正在学习的技能ID
	m_nfpLearnSkillTime = 0xffffffff;


	SAAddFightPet addMsg;
	memcpy(&addMsg.fightpet, &m_Property.m_FightPets[index], sizeof(SFightPetExt));
	g_StoreMessage(m_ClientIndex, &addMsg, sizeof(SAAddFightPet));

	return true;
}

// 创建坐骑
bool CPlayer::CreateMounts(int mountId, BYTE quality)
{
	if (0 == mountId)
		return false;

	SPlayerMounts::Mounts *pM = m_Property.mount;

	size_t index = 0;
	for (; index < SPlayerMounts::MAX_MOUNTS; index++)
	{
		if (0 == pM[index].ID)
			break;
	}

	if (index == SPlayerMounts::MAX_MOUNTS)
		return false;

	const SMountBaseData *pMountData = CMountService::GetInstance().GetMountBaseData(mountId, 1);
	if (!pMountData)
		return false;

	memset(&pM[index], 0, sizeof(SPlayerMounts::Mounts));
	pM[index].ID		= mountId;
	pM[index].Level		= 1;

	// 分配属性
	for (size_t i = 0; i < EMA_MAX; i++)
		pM[index].Points[i] = CRandom::RandRange(pMountData->m_Attri[i][0], pMountData->m_Attri[i][1]);

	// 发送给角色
	SAAddMounts addmsg;
	addmsg.mounts = pM[index];
	g_StoreMessage(this->m_ClientIndex,&addmsg,sizeof(addmsg));

	return true;
}

// 装备坐骑
void CPlayer::EquipMount(BYTE mountIndex)
{
	if (0 == m_CurHp)
		return;

	SPlayerMounts::Mounts *pMount = GetMounts(mountIndex);
	if (!pMount || 0 == pMount->ID)
		return;

	int indexEquip = GetEquipMount();
	if (-1 != indexEquip && mountIndex == indexEquip)
		return;

	// 如果有不同的装备骑乘，那么先卸装
	if (-1 != indexEquip)
		UnEquipMount(indexEquip);

	pMount->State |= SPlayerMounts::State_Equip;

	ChangeMountFactor(pMount, 1);

	// 属性改变通知
	UpdateAllProperties();

	SAEquipMounts equipMsg;
	equipMsg.index = mountIndex;
	g_StoreMessage(m_ClientIndex, &equipMsg, sizeof(SAEquipMounts));
}

// 取消骑乘装备
void CPlayer::UnEquipMount(BYTE mountIndex)
{
	if (0 == m_CurHp)
		return;

	SAUnEquipMounts unEquipMount;
	unEquipMount.index = mountIndex;

	if (-1 == GetEquipMount() || mountIndex != GetEquipMount())
		return;

	SPlayerMounts::Mounts *pCurrMounts = GetMounts(mountIndex);

	if (pCurrMounts && 0 != pCurrMounts->ID && (SPlayerMounts::State_Equip & pCurrMounts->State))
	{
		pCurrMounts->State &= ~SPlayerMounts::State_Equip;

		g_StoreMessage(this->m_ClientIndex, &unEquipMount, sizeof(unEquipMount));

		ChangeMountFactor(pCurrMounts, 3);

		UpdateAllProperties();
	}

	return;
}

// 删除宠物
void CPlayer::DeletePets(BYTE petIndex)
{
	// [2012-7-24 12-05 gw: -]int maxSize = m_Property.m_petSlots + 2;
	// [2012-7-24 12-04 gw: +由于没有设置imaxSize,所以暂时使用数组大小来替代]
	int maxSize = sizeof(m_Property.pets)/sizeof(m_Property.pets[0]);

	SPlayerPets::Pets *pCurrPet = GetPets(petIndex);
	if (!pCurrPet || 0 == pCurrPet->ID)
		return;

	//	rfalse(2, 1, "删除了ID为%d的宠物", pCurrPet);

	if (petIndex < maxSize-1)
		memcpy(&m_Property.pets[petIndex], &m_Property.pets[petIndex+1], sizeof(SPlayerPets::Pets)*(maxSize - petIndex  - 1));

	ZeroMemory(&m_Property.pets[maxSize-1], sizeof(SPlayerPets::Pets));

	SADeletePets delMsg;
	delMsg.index = petIndex;
	g_StoreMessage(m_ClientIndex, &delMsg, sizeof(SADeletePets));
}

// 删除侠客
void CPlayer::DeleteFightPet(BYTE fpIndex)
{
	if (fpIndex >= MAX_FIGHTPET_NUM)
		return;

	if (0 == m_Property.m_FightPets[fpIndex].m_fpID)
		return;

	// 不能删除已经出战的
	if (m_pFightPet && m_pFightPet->m_index == fpIndex)
		return;


	if (fpIndex < MAX_FIGHTPET_NUM-1)
		memcpy(&m_Property.m_FightPets[fpIndex], &m_Property.m_FightPets[fpIndex+1], sizeof(SFightPetExt)*(MAX_FIGHTPET_NUM - fpIndex  - 1));

	ZeroMemory(&m_Property.m_FightPets[MAX_FIGHTPET_NUM - 1], sizeof(SFightPetExt));

	SADeleteFightPet delMsg;
	delMsg.index = fpIndex;
	g_StoreMessage(m_ClientIndex, &delMsg, sizeof(SADeleteFightPet));

	// 对当前已经召唤出的侠客做位置修正
	if (m_pFightPet && m_pFightPet->m_index > fpIndex)
		m_pFightPet->m_index--;

	return;
}

// 删除坐骑
void CPlayer::DeleteMounts(BYTE mountIndex)
{
	SPlayerMounts::Mounts *pCurMount = GetMounts(mountIndex);
	if (!pCurMount || 0 == pCurMount->ID)
		return;

	int indexEquip = GetEquipMount();
	int indexRide  = GetRidingMount();

	// 装备或者骑乘中的马不能放生
	if ((-1 != indexEquip && mountIndex == indexEquip) || (-1 != indexRide && mountIndex == indexRide))
		return;

	if (mountIndex < SPlayerMounts::MAX_MOUNTS-1)
		memcpy(&m_Property.mount[mountIndex], &m_Property.mount[mountIndex+1], sizeof(SPlayerMounts::Mounts)*(SPlayerMounts::MAX_MOUNTS - mountIndex - 1));

	ZeroMemory(&m_Property.mount[SPlayerMounts::MAX_MOUNTS-1], sizeof(SPlayerMounts::Mounts));

	SADeleteMounts delMsg;
	delMsg.index = mountIndex;
	g_StoreMessage(m_ClientIndex, &delMsg, sizeof(SADeleteMounts));
}

int CPlayer::__OnEquipWearReduce( EQUIP_POSITION position, WORD point)
{

	int ret = 0;/*
				SEquipment& equip = m_Property.m_Equip[position];
				if ( equip.wIndex == 0)
				return 0;
				if ( equip.attribute.currWear == 0)
				return 0;
				const SItemData* pItemData = CItem::GetItemData(equip.wIndex);
				if (pItemData->wMaxWear == 0 || pItemData->wWearReduceCount == 0)
				return 0;

				// 增加计数器
				if (equip.attribute.wearReduceCounter <= pItemData->wWearReduceCount - 1)
				{
				DWORD temp = (DWORD)equip.attribute.wearReduceCounter + (DWORD)point;
				if (temp <= 0xffff)
				equip.attribute.wearReduceCounter += point;
				else
				equip.attribute.wearReduceCounter = 0xffff;
				}

				// 检查是否需要减少耐久点数
				if (equip.attribute.wearReduceCounter >= pItemData->wWearReduceCount)
				{
				// 重置计数器
				WORD wearpoint = equip.attribute.wearReduceCounter / pItemData->wWearReduceCount;
				equip.attribute.wearReduceCounter = equip.attribute.wearReduceCounter % pItemData->wWearReduceCount;

				if (equip.attribute.currWear > 0)
				{
				// 耐久点数减少
				if ( equip.attribute.currWear <= wearpoint)
				equip.attribute.currWear = 0;
				else
				equip.attribute.currWear -= wearpoint;

				// 发送消息通知耐久点数减少
				ret = 1;

				// 如果耐久点数为零，会导致角色属性变化
				if (equip.attribute.currWear == 0)
				ret = 2;
				}
				}
				//*/
	return ret;
}

void CPlayer::OnWeaponWearReduce()
{
	// 	int ret = __OnEquipWearReduce( EQUIP_P_WEAPON, 1);
	// 	if (ret == 1)
	// 	{
	// 		__SendEquipWearChangeMsg(EQUIP_P_WEAPON);
	// 	}
	// 	else if (ret == 2)
	// 	{
	// 		__SendEquipWearChangeMsg(EQUIP_P_WEAPON);
	// 		InitEquipmentData();
	// 		UpdateAllProperties();
	// 	}
}

void CPlayer::OnEquipWearReduce( WORD point)
{
	// 	int ret = 0;
	// 	for ( int i = EQUIP_P_HAT; i <= (int)EQUIP_P_SHOSE; ++i)
	// 	{
	// 		int ret2 = __OnEquipWearReduce( (EQUIP_POSITION)i, point);
	// 		if (ret2 == 1)
	// 			__SendEquipWearChangeMsg((EQUIP_POSITION)i);
	// 
	// 		if (ret2 == 2 && ret == 0)
	// 		{
	// 			__SendEquipWearChangeMsg((EQUIP_POSITION)i);
	// 			ret = 2;
	// 		}
	// 	}
	// 
	// 	if (ret == 2)
	// 	{
	// 		InitEquipmentData();
	// 		UpdateAllProperties();
	// 	}
}

void CPlayer::SwitchFightState(bool IsIn)
{
	SAFightStateMsg msg;
	msg.dwGlobalID = GetGID();
	msg.IsInFight  = IsIn;

	g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));
}

void CPlayer::__SendEquipWearChangeMsg(EQUIP_POSITION position)
{
	SEquipment& equip = m_Property.m_Equip[position];

	SAEquipWearChange msg;
	msg.equipPosition = position;
	msg.currWear = equip.attribute.currWear;
	g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));
}

void CPlayer::SendCurTelChange(SXYD3Telergy *pTempTel, BYTE pos, bool bAddNewTel)
{
	SASetChangeTelergyMsg msg;
	memcpy(&msg.m_Telergy, pTempTel, sizeof(msg.m_Telergy));
	msg.m_bIsAddTel = bAddNewTel;
	msg.m_Pos		= pos;

	g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));
}

void CPlayer::SendPropertiesUpdate()
{
	//	static char *AttriName[XA_MAX] = {"攻击","防御","暴击","闪避","最大生命","最大内力","最大体力","当前生命","当前内力","当前体力","等级","移动速度",
	//		"攻击速度","最大经验","当前经验","最大精力","当前精力","最大真气","当前真气","绑定货币","非绑定货币","赠宝","元宝","进攻","防御","轻身","健身","剩余点数"};

	CFightObject::SendPropertiesUpdate();

	SAPropertyChange msg;
	msg.buffSize = 0;
	BYTE *buff   = msg.PropertyData;

	if (true/*m_IsFightUpdated*/)	// 有战斗属性更新了
	{
		for (size_t i=0; i<XA_MAX_EXP; i++)
		{
			if (XA_SPEED != i && XA_ATKSPEED != i && m_FightPropertyStatus[i])
			{
				//				rfalse(2, 1, "【%s】 ", AttriName[i]);
				WORD attSize;
				WORD attType = GetAttriType(static_cast<XYD3_ATTRI>(i), attSize);
				if (XAT_MAX == attType)
					continue;

				// 溢出了
				if ((msg.buffSize + attSize + 1) > SAPropertyChange::PROPERTY_BUFFSIZE)
					break;

				*buff++ = i;

				BYTE *attData = reinterpret_cast<BYTE *>(m_AttriRefence[i]);
				if (!attData)
					continue;

				memcpy(buff, attData, attSize);
				buff += attSize;

				msg.buffSize += (attSize + 1);

				//更新排行榜数据
				if (i == XA_LEVEL)
				{
					g_Script.SetPlayer(this);
					if (g_Script.PrepareFunction("RefreshRankData"))
					{
						g_Script.Execute();
					}
					g_Script.CleanPlayer();
				}
			}
		}
	}

	if (true/*m_IsPlayerUpdated*/)
	{
		for (size_t i=XA_MAX_EXP; i<XA_MAX; i++)
		{
			if (m_PlayerPropertyStatus[i-XA_MAX_EXP])
			{
				//				rfalse(2, 1, "【%s】 ", AttriName[i]);

				WORD attSize;
				WORD attType = GetAttriType(static_cast<XYD3_ATTRI>(i), attSize);		// 得到属性类型的大小
				if (XAT_MAX == attType)
					continue;

				// 溢出了
				if ((msg.buffSize + attSize + 1) > SAPropertyChange::PROPERTY_BUFFSIZE)
					break;

				*buff++ = i;

				BYTE *attData = reinterpret_cast<BYTE *>(m_PlayerAttriRefence[i-XA_MAX_EXP]);
				if (!attData)
					continue;

				memcpy(buff, attData, attSize);
				buff += attSize;

				msg.buffSize += (attSize+1);

				//更新排行榜数据
				if (i == XA_FIGHTPOWER)
				{
					g_Script.SetPlayer(this);
					if (g_Script.PrepareFunction("RefreshRankData"))
					{
						g_Script.Execute();
					}
					g_Script.CleanPlayer();
				}
			}
		}
	}

	if (msg.buffSize)
	{
		g_StoreMessage(m_ClientIndex, &msg, sizeof(SAPropertyChange)-(SAPropertyChange::PROPERTY_BUFFSIZE-msg.buffSize));
		memset(m_FightPropertyStatus, 0, sizeof(m_FightPropertyStatus));
		memset(m_PlayerPropertyStatus, 0, sizeof(m_PlayerPropertyStatus));
	}
}

void CPlayer::UpdateBaseProperties()
{
	//update by ly 2014/5/20
	// 这一部分是直接被战斗影响的数据
	if (g_ReloadPersonUpgradeFlag)	//角色升级表已经重新加载
	{
		m_PlayerAttri = CPlayerService::GetInstance().GetPlayerUpdateData(m_Property.m_School, m_Property.m_Level);
		g_ReloadPersonUpgradeFlag = FALSE;
	}

	if (!m_PlayerAttri)return;
	//0 基础最大生命 1 基础最大内力 2 基础最大体力 3基础攻击 4基础防御 5基础暴击 6 基础躲避
	DWORD *temp = new DWORD[7];
	for (int i = 0; i< 7; i++)
	{
		temp[i] = _L_GetLuaValue("GetPlayerBaseProperties", i, 0);
		if (temp[i] == 0xffffffff){
			rfalse(2, 1, "获取玩家加点属性失败");
			temp[i] = 0;
		}
	}
	m_BaseMaxHp = m_PlayerAttri->m_BaseHp + temp[0];
	m_BaseMaxTp = m_PlayerAttri->m_BaseTp + temp[2];
	m_BaseAtk = m_PlayerAttri->m_GongJi + temp[3];
	m_BaseDefence = m_PlayerAttri->m_FangYu + temp[4];
	m_BaseCtrAtk = m_PlayerAttri->m_BaoJi + temp[5];
	m_BaseEscape = m_PlayerAttri->m_ShanBi + temp[6];

	// 	m_BaseMaxHp		= m_PlayerAttri->m_BaseHp + m_Property.m_JianShen * 40 + m_Property.m_FangYu * 12;
	m_BaseMaxMp		= 100;
	// 	m_BaseMaxTp		= m_PlayerAttri->m_BaseTp + m_Property.m_JianShen * 1;
	// 	m_BaseAtk		= m_PlayerAttri->m_GongJi + m_Property.m_JingGong * 4;
	// 	m_BaseDefence	= m_PlayerAttri->m_FangYu + m_Property.m_FangYu   * 4;
	// 	m_BaseCtrAtk	= m_PlayerAttri->m_BaoJi  + m_Property.m_QingShen * 2;
	// 	m_BaseEscape	= m_PlayerAttri->m_ShanBi + m_Property.m_QingShen * 3;
	//rfalse(2, 1, "m_BaseMaxHp = %d", m_BaseMaxHp);
	MY_ASSERT(m_BaseMaxHp);

	return;
}

void CPlayer::UpdateAllProperties()
{
	if (!m_PlayerAttri)
		return;
	if (m_TemplateMap.size() != 0)
	{
		rfalse(4,1,"进入了m_TemplateMap  %d",m_TemplateMap.size());
		UpdataPlayerCounterpartProperties();
		return;
	}
	int num = lua_gettop(g_Script.ls);
	WORD oldSpeed		= m_OriSpeed;
	m_OriSpeed			= PLAYER_BASIC_SPEED;
	m_AtkSpeed			= PLAYER_BASIC_ATKSPEED;

	m_MaxHp				= m_BaseMaxHp;
	m_MaxMp				= m_BaseMaxMp;
	m_MaxTp				= m_BaseMaxTp;
	m_GongJi			= m_BaseAtk;
	m_FangYu			= m_BaseDefence;
	m_BaoJi				= m_BaseCtrAtk;
	m_ShanBi			= m_BaseEscape;
	m_Hit = m_PlayerAttri->m_Hit;
	m_SorbDamageValue	= 0;
	m_SorbDamagePercent	= 0;

	// 特殊附加属性的处理
	vector<WORD> OldPropertyValue, NewPropertyValue;

	OldPropertyValue.push_back(m_ReduceDamage);
	OldPropertyValue.push_back(m_AbsDamage);
	OldPropertyValue.push_back(m_NonFangyu);
	OldPropertyValue.push_back(m_MultiBaoJi);
	OldPropertyValue.push_back(m_IceDamage);
	OldPropertyValue.push_back(m_IceDefence);
	OldPropertyValue.push_back(m_FireDamage);
	OldPropertyValue.push_back(m_FireDefence);
	OldPropertyValue.push_back(m_XuanDamage);
	OldPropertyValue.push_back(m_XuanDefence);
	OldPropertyValue.push_back(m_PoisonDamage);
	OldPropertyValue.push_back(m_PoisonDefence);

	m_ReduceDamage		= 0;
	m_AbsDamage			= 0;
	m_NonFangyu			= 0;
	m_MultiBaoJi		= 0;

	m_IceDamage			= 0;
	m_IceDefence		= 0;
	m_FireDamage		= 0;
	m_FireDefence		= 0;
	m_XuanDamage		= 0;
	m_XuanDefence		= 0;
	m_PoisonDamage		= 0;
	m_PoisonDefence		= 0;

	//	m_AbsDamageValue	= 0;

	MY_ASSERT(m_MaxHp);
	// 获取体质的附加属性
	m_MaxHp				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_HP];
	m_MaxMp				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_MP];
	m_MaxTp				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_TL];
	m_GongJi			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_AT];
	m_FangYu			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_DF];
	m_BaoJi				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_BJ];
	m_ShanBi			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_SB];
	m_AtkSpeed			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_AS];
	m_SorbDamageValue	+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_MS];
	m_OriSpeed			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_RS];
	//m_AbsDamageValue	+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_SH];

	//m_Property.m_School

	// 获取装备的附加属性
	m_MaxHp				+= m_EquipFactorValue[SEquipDataEx::EEA_MAXHP];
	m_MaxMp				+= m_EquipFactorValue[SEquipDataEx::EEA_MAXMP];
	m_MaxTp				+= m_EquipFactorValue[SEquipDataEx::EEA_MAXTP];
	m_GongJi			+= m_EquipFactorValue[SEquipDataEx::EEA_GONGJI];
	m_FangYu			+= m_EquipFactorValue[SEquipDataEx::EEA_FANGYU];
	m_BaoJi				+= m_EquipFactorValue[SEquipDataEx::EEA_BAOJI];
	m_ShanBi			+= m_EquipFactorValue[SEquipDataEx::EEA_SHANBI];
	m_OriSpeed			+= m_EquipFactorValue[SEquipDataEx::EEA_SPEED];
	m_AtkSpeed			+= m_EquipFactorValue[SEquipDataEx::EEA_ATKSPEED];

	m_ReduceDamage		+= m_EquipFactorValue[SEquipDataEx::EEA_REDUCEDAMAGE];
	m_AbsDamage			+= m_EquipFactorValue[SEquipDataEx::EEA_ABSDAMAGE];
	m_NonFangyu			+= m_EquipFactorValue[SEquipDataEx::EEA_NONFANGYU];
	m_MultiBaoJi		+= m_EquipFactorValue[SEquipDataEx::EEA_MULTIBAOJI];

	m_IceDamage			+= m_EquipFactorValue[SEquipDataEx::EEA_ICE_DAMAGE];			// 冰伤害
	m_IceDefence		+= m_EquipFactorValue[SEquipDataEx::EEA_ICE_DEFENCE];			// 冰抗性
	m_FireDamage		+= m_EquipFactorValue[SEquipDataEx::EEA_FIRE_DAMAGE];			// 火伤害
	m_FireDefence		+= m_EquipFactorValue[SEquipDataEx::EEA_FIRE_DEFENCE];			// 火抗性
	m_XuanDamage		+= m_EquipFactorValue[SEquipDataEx::EEA_XUAN_DAMAGE];			// 玄伤害
	m_XuanDefence		+= m_EquipFactorValue[SEquipDataEx::EEA_XUAN_DEFENCE];			// 玄抗性
	m_PoisonDamage		+= m_EquipFactorValue[SEquipDataEx::EEA_POISON_DAMAGE];			// 毒伤害
	m_PoisonDefence		+= m_EquipFactorValue[SEquipDataEx::EEA_POISON_DEFENCE];		// 毒抗性
	m_Hit += m_EquipFactorValue[SEquipDataEx::EEA_HIT];		// 命中
	m_FightPropertyStatus[XA_HIT] = true;
	//月下夜想曲 新增
	//月夜新属性初始化
	if (m_PlayerAttri)
	{
		m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] += m_PlayerAttri->m_uncrit;
		m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] += m_PlayerAttri->m_wreck;
		m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] += m_PlayerAttri->m_unwreck;
		m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] += m_PlayerAttri->m_puncture;
		m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] += m_PlayerAttri->m_unpuncture;
	}

	for (int i = 0;i<SEquipDataEx::EEA_MAXTP - SEquipDataEx::EEA_UNCRIT;i++)
	{
		m_newAddproperty[i] += m_EquipFactorValue[SEquipDataEx::EEA_UNCRIT + i];
		if (i <SEquipDataEx::EEA_SHANBI)
		{
			//m_AttriRefence[XA_UNCRIT + i] = &m_newAddproperty[i];
			m_FightPropertyStatus[XA_UNCRIT + i] = true;
		}
	}

	// 获取经脉的附加属性
	m_MaxHp				+= m_JingMaiFactor[JMP_MHP];
	m_MaxMp				+= m_JingMaiFactor[JMP_MMP];
	m_MaxTp				+= m_JingMaiFactor[JMP_MTP];
	m_GongJi			+= m_JingMaiFactor[JMP_GONGJI];
	m_FangYu			+= m_JingMaiFactor[JMP_FANGYU];
	m_BaoJi				+= m_JingMaiFactor[JMP_BAOJI];
	m_ShanBi			+= m_JingMaiFactor[JMP_SHANBI];

	//获取信仰的附加属性
	m_MaxHp += m_XinYangFactor[XYP_HP];
	m_MaxJp += m_XinYangFactor[XYP_JP];
	m_MaxTp += m_XinYangFactor[XYP_TP];
	m_Hit += m_XinYangFactor[XYP_HIT];
	m_GongJi += m_XinYangFactor[XYP_GONGJI];
	m_FangYu += m_XinYangFactor[XYP_FANGYU];
	m_BaoJi += m_XinYangFactor[XYP_BAOJI];
	m_ShanBi += m_XinYangFactor[XYP_SHANBI];
	m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] += m_XinYangFactor[XYP_KANGBAO];
	m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] += m_XinYangFactor[XYP_POJI];
	m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] += m_XinYangFactor[XYP_KANGPO];
	m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] += m_XinYangFactor[XYP_CHUANCI];
	m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] += m_XinYangFactor[XYP_KANGCHUAN];

	//称号的附加属性
	m_GongJi += m_TitleFactor[TitleAddType::TITLE_AGONGJI];
	m_MaxHp += m_TitleFactor[TitleAddType::TITLE_AHP];
	m_FangYu += m_TitleFactor[TitleAddType::TITLE_AFANGYU];

	//VIP的附加属性
	m_MaxHp += m_VIPFactor[VipFactorType::VFT_HP];
	m_GongJi += m_VIPFactor[VipFactorType::VFT_GONGJI];
	m_FangYu += m_VIPFactor[VipFactorType::VFT_FANGYU];

	// 获取骑乘的附加属性
	m_MaxHp				+= m_MountAddFactor[SPlayerMounts::SPM_MHP];
	m_MaxMp				+= m_MountAddFactor[SPlayerMounts::SPM_MMP];
	m_MaxTp				+= m_MountAddFactor[SPlayerMounts::SPM_MTP];
	m_GongJi			+= m_MountAddFactor[SPlayerMounts::SPM_GONGJI];
	m_FangYu			+= m_MountAddFactor[SPlayerMounts::SPM_FANGYU];
	m_BaoJi				+= m_MountAddFactor[SPlayerMounts::SPM_BAOJI];
	m_ShanBi			+= m_MountAddFactor[SPlayerMounts::SPM_SHANBI];
	m_OriSpeed			+= (PLAYER_BASIC_SPEED * m_MountAddFactor[SPlayerMounts::SPM_SPEED]) / 100;

	// 获取Buff的附加属性
	m_MaxHp				+= GetBuffModifyValue(BAP_MHP,		m_BaseMaxHp);
	m_MaxMp				+= GetBuffModifyValue(BAP_MMP,		m_BaseMaxMp);
	m_MaxTp				+= GetBuffModifyValue(BAP_MTP,		m_BaseMaxTp);
	m_GongJi			+= GetBuffModifyValue(BAP_GONGJI,	m_BaseAtk);
	m_FangYu			+= GetBuffModifyValue(BAP_FANGYU,	m_BaseDefence);
	m_BaoJi				+= GetBuffModifyValue(BAP_BAOJI,	m_BaseCtrAtk);
	m_ShanBi			+= GetBuffModifyValue(BAP_SHANBI,	m_BaseEscape);
	m_OriSpeed			+= GetBuffModifyValue(BAP_SPEED,	PLAYER_BASIC_SPEED);
	m_AtkSpeed			+= GetBuffModifyValue(BAP_ATKSPEED, PLAYER_BASIC_ATKSPEED);
	m_Hit += GetBuffModifyValue(BAP_HIT, m_PlayerAttri->m_Hit);
	m_SorbDamageValue	+= GetBuffModifyValue(BAP_SORBDAM,		0);
	m_SorbDamagePercent	+= GetBuffModifyValue(BAP_SORBDAM_PER,	0);
	m_ExtraExpPercent += GetBuffModifyValue(BAP_EXTRA_EXP_PER, 100);
	m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNCRIT, m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_WRECK, m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNWRECK, m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_PUNCTURE, m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT]);
	m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] += GetBuffModifyValue(BAP_UNPUNCTURE, m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT]);

	// 获取心法的附加属性
	m_MaxHp				+= m_TelergyFactor[TET_HP];
	m_MaxMp				+= m_TelergyFactor[TET_MP];
	m_MaxTp				+= m_TelergyFactor[TET_TP];
	m_GongJi			+= m_TelergyFactor[TET_ATTACK];
	m_FangYu			+= m_TelergyFactor[TET_DEFENCE];
	m_BaoJi				+= m_TelergyFactor[TET_BAOJI];
	m_ShanBi			+= m_TelergyFactor[TET_SHANBI];

	NewPropertyValue.push_back(m_ReduceDamage);
	NewPropertyValue.push_back(m_AbsDamage);
	NewPropertyValue.push_back(m_NonFangyu);
	NewPropertyValue.push_back(m_MultiBaoJi);
	NewPropertyValue.push_back(m_IceDamage);
	NewPropertyValue.push_back(m_IceDefence);
	NewPropertyValue.push_back(m_FireDamage);
	NewPropertyValue.push_back(m_FireDefence);
	NewPropertyValue.push_back(m_XuanDamage);
	NewPropertyValue.push_back(m_XuanDefence);
	NewPropertyValue.push_back(m_PoisonDamage);
	NewPropertyValue.push_back(m_PoisonDefence);

	DWORD MoveSpeed = 	_L_GetLuaValue("GetPlayerBaseProperties",7,0);
	DWORD AtkSpeed = 	_L_GetLuaValue("GetPlayerBaseProperties",8,0);

	if (MoveSpeed == 0xffffffff || AtkSpeed == 0xffffffff){
		rfalse(2,1,"从脚本中获取加点后的速度加成失败");
	}else
	{
		m_OriSpeed += MoveSpeed;
		m_AtkSpeed += AtkSpeed;
	}

	// 将速度改为float
	if (oldSpeed != m_OriSpeed)
	{
		m_Speed = m_OriSpeed / 1000.f;
		m_FightPropertyStatus[XA_SPEED] = true;
	}

	if (1)
	{
		m_AtkSpeed = PLAYER_BASIC_ATKSPEED + ( m_AtkSpeed - PLAYER_BASIC_ATKSPEED) / 10;
		// 		if (m_AtkSpeed > 2 * PLAYER_BASIC_ATKSPEED)
		// 		{
		// 			m_AtkSpeed = 2 * PLAYER_BASIC_ATKSPEED;
		// 		}
		m_FightPropertyStatus[XA_ATKSPEED] = true;
	}

	// 标记属性改变
	for (size_t i = 0; i < XA_MAX_EXP; i++)
		m_FightPropertyStatus[i] = true;

// 	// 特殊附加属性的更新判定
// 	for (size_t i = 0; i <= XA_POISON_DEFENCE - XA_REDUCEDAMAGE; ++i)
// 	{
// 		if (OldPropertyValue[i] != NewPropertyValue[i])
// 			m_FightPropertyStatus[i + XA_REDUCEDAMAGE] = true;
// 	}

	//add by ly 2014/5/6 同步玩家战斗力数据
	m_PlayerFightPower = GetPlayerFightPower();	//获取玩家战斗力
	m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;

	// 是否需要修正当前基本属性
	if (m_CurHp > m_MaxHp)	RestoreFullHPDirectly();
	if (m_CurMp > m_MaxMp)  RestoreFullMPDirectly();
	if (m_CurTp > m_MaxTp)	RestoreFullTPDirectly();
}

SPlayerPets::Pets *CPlayer::GetPets(int index)
{
	SPlayerPets *pPets = &m_Property;

	// [2012-7-24 12-04 gw: -] 	if (index>=0 && index<(m_Property.m_petSlots+2))
	// [2012-7-24 12-04 gw: +由于没有设置imaxSize,所以暂时使用数组大小来替代]
	int imaxSize = sizeof(m_Property.pets)/sizeof(m_Property.pets[0]);
	if (index>=0 && index<imaxSize)
		return &pPets->pets[index];
	else
		return 0;

	return 0;
}

SPlayerMounts::Mounts *CPlayer::GetMounts(int index)
{
	SPlayerMounts *pMounts = &m_Property;

	if (index >= 0 && index < SPlayerMounts::MAX_MOUNTS)
		return &pMounts->mount[index];
	else
		return 0;
}

// 坐骑相关消息
void CPlayer::OnMountsMsg(SMountsMsg *pMsg)
{
	switch (pMsg->_protocol)
	{
	case SMountsMsg::EPRO_EQUIP_MOUNTS:		// 装备坐骑
		this->EquipMount(((SQEquipMounts *)pMsg)->index);
		break;
	case SMountsMsg::EPRO_UNEQUIP_MOUNTS:	// 取消装备坐骑
		this->UnEquipMount(((SQUnEquipMounts *)pMsg)->index);
		break;
	case SMountsMsg::EPRO_UP_MOUNTS:		// 上马
		this->PreUpMounts(((SQUpMounts *)pMsg)->index);
		break;
	case SMountsMsg::EPRO_DOWN_MOUNTS:		// 下马
		this->DownMounts();
		break;
	case SMountsMsg::EPRO_DELETE_MOUNTS:	// 放生
		this->DeleteMounts(((SQDeleteMounts *)pMsg)->index);
		break;
	case SMountsMsg::EPRO_DEL_FIGHTPET:		// 删除侠客
		this->DeleteFightPet(((SQDeleteFightPet *)pMsg)->index);
		break;
	case SMountsMsg::EPRO_DELETE_PETS:		
		this->DeletePets(((SQDeletePets *)pMsg)->index);
		break;
	case SMountsMsg::EPRO_USE_SKILL:	
		break;
	case SMountsMsg::EPRO_FEED_MOUNTS:
		this->FeedMounts((SQFeedMounts *)pMsg);	
		break;
	case SMountsMsg::EPRO_ADD_POINT:
		this->AddMountPoint((SQMountAddPoint *)pMsg);
		break;
	case SMountsMsg::EPRO_CALL_OUT_PET:
		this->ShowPet(((SQCallOutPet *)pMsg)->index);
		break;
	case SMountsMsg::EPRO_CALL_BACK_PET:
		this->CallBackPet();
		break;
	case SMountsMsg::EPRO_CALL_OUT_FIGHTPET:
		//this->CallOutFightPet(((SQCallOutFightPet *)pMsg)->index);
		this->CallOutFightPet((SQCallOutFightPet*)pMsg);
		break;
	case SMountsMsg::EPRO_CALL_BACK_FIGHTPET:
		this->CallBackFightPet(false);
		break;
	case SMountsMsg::EPRO_FIGHTPET_MOVETOMSG:
		this->MoveFightPet((SQFightPetMoveto*)pMsg);
		break;
	case SMountsMsg::EPRO_FIGHTPET_IMPEL:	//玩家激励侠客提高心情
		this->ImpelFightPet((SQFightPetImpel*)pMsg);
		break;
	case SMountsMsg::EPRO_FIGHTPET_CHANGEATTTYPE:	//切换侠客攻击模式
		this->_fpChangeAttType((SQfpchangeatttypeMsg*)pMsg);
		break;
	default:
		break;
	}
}

BOOL CPlayer::_UseItem(SPackageItem *pItem, DWORD useType)
{
	g_Script.m_pPlayer = this;
	BOOL rt = CItemUser::_UseItem(pItem, useType);
	g_Script.m_pPlayer = 0;
	return rt;
}

void CPlayer::UseMountSkill(short mountIndex,short skillIdx)
{
	// 	if ( GetCurActionID() == EA_DEAD)return;	//死亡
	// 	if( mountIndex<0 || mountIndex>=SPlayerMounts::MAX_MOUNTS ) return;
	// 	if( skillIdx<0 || skillIdx>=16 ) return;
	// 
	// 	WORD skillId = m_Property.mount[mountIndex].mSkill[skillIdx].ID;
	// 	Mounts::SMountSkill* pSkillInfo = INSTANCE(Mounts::CMountTemplates)->GetMountSkill((int)skillId);
	// 	if( !pSkillInfo ) return ;
	// 
	// 	//判断冷却时间
	// 	int index = mountIndex*Mounts::MaxSkillNum+skillIdx;
	// 
	// 	std::map<int,EventMgr::EventBase*>::iterator i = m_MountSkillColdTimeTbl.find(index);
	// 	Mounts::MountSkillCD* pCD = 0;
	// 	if( i!=m_MountSkillColdTimeTbl.end() )
	// 	{
	// 		pCD = (Mounts::MountSkillCD*)i->second;	
	// 		if (pCD->IsActive())
	// 		{// 已经进行冷却时间.
	// 			//SShowSkillHintInfo_S2C_MsgBody msg;
	// 			//msg.what = what;
	// 			//msg.skillID = skillID;
	// 			//g_StoreMessage(player->m_ClientIndex,&msg,sizeof(SShowSkillHintInfo_S2C_MsgBody));
	// 			return;
	// 		}		
	// 	}
	// 	else
	// 	{
	// 		pCD = new Mounts::MountSkillCD;
	// 		pCD->Init(mountIndex,skillIdx);
	// 		int index = mountIndex*Mounts::MaxSkillNum + skillIdx;
	// 		m_MountSkillColdTimeTbl[index] = pCD;		
	// 	}
	// 
	// 	//如果是坐骑特殊技能,则调用脚本实现
	// 	if(pSkillInfo->wSkillType == MOUNT_SCRIPT_SKILL)	
	// 	{
	// 		lite::Variant result;
	// 		g_Script.SetCondition( NULL, this, NULL );
	// 		LuaFunctor( g_Script, "UseMountSkill" )[ skillId ]( &result );
	// 		g_Script.CleanCondition();
	// 		BOOL rt = (__int32)result ? TRUE : FALSE;
	// 		//使用成功启动冷却时间
	// 		if(rt)
	// 			pCD->Start( this,(DWORD)pSkillInfo->wSkillCDTime );
	// 		return;
	// 	}
}

void CPlayer::SkillNumUpdate()
{

}

void CPlayer::AddMountPoint(SQMountAddPoint *msg)
{
	if (0 == m_CurHp)
		return;

	// 	SPlayerMounts::Mounts *pMount = GetMounts(msg->mountIndex);
	// 	if (!pMount || 0 == pMount->ID)
	// 		return;
	// 	
	// 	DWORD totalPoint = msg->attri[0] + msg->attri[1] + msg->attri[2] + msg->attri[3] + msg->attri[4];
	// 	if (totalPoint > pMount->RemainPoint)
	// 		return;
	// 
	// 	pMount->RemainPoint -= totalPoint;
	// 
	// 	for (size_t i=0; i<SPlayerMounts::SPM_SPEED; ++i)
	// 		pMount->Points[i] += msg->attri[i];
	// 	
	// 	// 如果装备了这匹马，那么及时生效
	// 	int equipIndex = GetEquipMount();	
	// 	if (-1 != equipIndex && msg->mountIndex == equipIndex)
	// 	{
	// 		ChangeMountFactor(pMount, 1);
	// 		UpdateAllProperties();
	// 	}
	// 
	// 	SAMountAddPoint backMsg;
	// 	backMsg.mountIndex	= msg->mountIndex;
	// 	backMsg.remainPoint	= pMount->RemainPoint;
	// 	memcpy(backMsg.value, pMount->Points, sizeof(pMount->Points));
	// 	
	// 	g_StoreMessage(m_ClientIndex, &backMsg, sizeof(backMsg));

	return;
}

void CPlayer::FeedMounts(SQFeedMounts *msg)
{
	if (0 == m_CurHp)
		return;

	SPlayerMounts::Mounts *pMount = GetMounts(msg->mountIndex);
	if (!pMount || 0 == pMount->ID)
		return;

	// 满级不升级
	if (MAX_MOUNT_LEVEL == pMount->Level)
		return;

	const SMountBaseData *pMountData = CMountService::GetInstance().GetMountBaseData(pMount->ID, pMount->Level);
	if (!pMountData)
		return;

	// 有的骑乘无法升级或已满级
	if (!pMountData->m_CanUpdate)
		return;

	SPackageItem *pItem = FindItemByPos(msg->foodIndex, XYD_FT_ONLYUNLOCK);
	if (!pItem)
		return;

	// 扣除饲料
	DelItem(*pItem);

	// 回馈
	SAFeedMounts backmsg;
	backmsg.mountIndex = msg->mountIndex;

	// 计算得到所要加的经验
	DWORD AddExp = 1000;

	// 升级
	pMount->GrowPoint += (0xffffffff - pMount->GrowPoint) > AddExp ? AddExp : (0xffffffff - pMount->GrowPoint);

	WORD oldLevel = pMount->Level;

	// 判断是否能升级
	do
	{
		if (MAX_MOUNT_LEVEL == pMount->Level)
		{
			pMount->GrowPoint = 0;
			break;
		}

		if (pMount->GrowPoint < pMountData->m_NeedExp)
			break;

		pMount->GrowPoint -= pMountData->m_NeedExp;
		pMount->Level += 1;
		backmsg.Attri[SPlayerMounts::SPM_SPEED] = pMountData->m_SpeedUp;

		pMountData = CMountService::GetInstance().GetMountBaseData(pMountData->m_ID, pMountData->m_Level);

	} while (pMountData);

	// 如果升级了，那么属性变更
	if (oldLevel != pMount->Level && pMountData)
	{
		for (size_t i = 0; i < EMA_MAX; i++)
			pMount->Points[i] = CRandom::RandRange(pMountData->m_Attri[i][0], pMountData->m_Attri[i][1]);

		int equipIndex = GetEquipMount();
		if (-1 != equipIndex && msg->mountIndex == equipIndex)
			ChangeMountFactor(pMount, 1);

		int rideIndex = GetRidingMount();	
		if (-1 != rideIndex && msg->mountIndex == rideIndex)
			ChangeMountFactor(pMount, 2);

		UpdateAllProperties();
	}

	backmsg.mountLevel	= pMount->Level;
	backmsg.curExp		= pMount->GrowPoint;
	memcpy(backmsg.Attri, pMount->Points, sizeof(pMount->Points));
	g_StoreMessage(m_ClientIndex, &backmsg, sizeof(backmsg));

	return;
}

BOOL CPlayer::CanUpMount(BYTE mountIndex)
{
	if (0 == m_CurHp)
		return FALSE;

	if (m_IsInFight)
	{
		TalkToDnid(m_ClientIndex,"你当前正在战斗状态，无法上马");
		return FALSE;
	}

	if (EA_STAND != GetCurActionID() && EA_RUN != GetCurActionID() && EA_WALK != GetCurActionID())
	{
		TalkToDnid(m_ClientIndex,"你当前状态下无法上马");
		return FALSE;
	}

	if (m_InUseItemevens)
	{
		TalkToDnid(m_ClientIndex,"你当前状态下无法上马");
		return FALSE;
	}


	if (IsInStall() || InExchange())
		return FALSE;

	SPlayerMounts::Mounts *pMount = GetMounts(mountIndex);
	if (!pMount || 0 == pMount->ID)
		return FALSE;

	int indexRide = GetRidingMount();
	if (-1 != indexRide)
		return FALSE;

	if(m_InRidingevens)
	{
		TalkToDnid(m_ClientIndex,"你正在上马过程中，操作失败");
		return FALSE;
	}

	return TRUE;
}

void CPlayer::PreUpMounts(BYTE mountIndex)
{
	if (!CanUpMount(mountIndex))
		return;

	/*
	// 设置骑乘读条事件，暂定为5秒.
	Event::ReadyEvent *pEvent = m_BatchReadyEvent.SetReadyEvent(new Mounts::ReadyMountEvent(this, mountIndex, Event::EEventInterrupt_Combinate_Base), 2000);
	if (pEvent){
	pEvent->SetEventName("UpMountEvent");
	m_InRidingevens = 1;		//设置为骑马读条状态
	}
	*/

	UpMounts(mountIndex);
}

// 上马，加速
void CPlayer::UpMounts(int mountIndex)
{
	SPlayerMounts::Mounts *pMount = GetMounts(mountIndex);
	if (!pMount || 0 == pMount->ID)
		return;

	MY_ASSERT(-1 == GetRidingMount());

	SAUpMount upmsg;
	upmsg.index  = mountIndex;
	upmsg.result = 1;
	g_StoreMessage(m_ClientIndex, &upmsg, sizeof(upmsg));

	m_InRidingevens = 0;
	pMount->State |= SPlayerMounts::State_Riding;
	m_RidingStatus = pMount->ID;
	m_RidingStatus = (m_RidingStatus << 16) | pMount->Level;

	// 同步动作
	SendSynPlayMount(pMount->ID, pMount->Level, SAMountAction::MA_UP);

	ChangeMountFactor(pMount, 2);

	UpdateAllProperties();
}

// 下马，减速
void CPlayer::DownMounts()
{
	int indexRide = GetRidingMount();
	if (-1 == indexRide)
		return;

	SPlayerMounts::Mounts *pMount = GetMounts(indexRide);
	if (!pMount || 0 == pMount->ID)
		return;

	SADownMounts downmsg;
	downmsg.index  = indexRide;
	downmsg.result = 1;

	g_StoreMessage(m_ClientIndex, &downmsg, sizeof(downmsg));

	pMount->State &= ~SPlayerMounts::State_Riding;
	m_RidingStatus = 0;

	SendSynPlayMount(pMount->ID, pMount->Level, SAMountAction::MA_DOWN);

	ChangeMountFactor(pMount, 4);

	UpdateAllProperties();
}

void CPlayer::StartCollect(DWORD npcid, DWORD time, int x, int y, const string& sFinishFunc)
{
	if (EA_STAND != GetCurActionID())
		return;

	// 查看骑乘/摆摊状态
	if (IsInStall())
		return;

	///如果在骑乘自动下马
	if(0 != m_RidingStatus)
	{
		DownMounts();
	}

	m_nCollectTimeLen	= time;
	m_nCollectBeginTime = timeGetTime();
	m_nCollectNpcId		= npcid;
	m_sCollectFunc		= sFinishFunc;

	SendCollectMsg(m_nCollectNpcId, time, x, y);

	SetCurActionID(EA_COLLECT);
	SetBackupActionID(EA_COLLECT);
}

void CPlayer::SendCollectMsg(DWORD npcid, DWORD time, int x, int y)
{
	SASetCollectionMsg msg;
	msg.dwGID			= GetGID();
	msg.dwNpcId			= npcid;
	msg.dwTimeRemain	= time;
	msg.x				= x;
	msg.y				= y;

	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea && DynamicCast(IID_PLAYER))
		pArea->SendAdj(&msg, sizeof(SASetCollectionMsg), -1);
}

void CPlayer::SendCancelCollectMsg()
{
	SACancelCollectMsg msg;
	msg.dwGID = GetGID();

	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea && DynamicCast(IID_PLAYER))
		pArea->SendAdj(&msg, sizeof(SACancelCollectMsg), -1);
}

void CPlayer::CancelCollect()
{
	if (0 == m_nCollectTimeLen)
		return;

	m_nCollectTimeLen = 0;

	SendCancelCollectMsg();

	SetCurActionID(EA_STAND);
	SetBackupActionID(EA_STAND);
}

void CPlayer::OnFinishCollect()
{
	Stand();

	if (m_ParentRegion)
	{
		LPIObject pObject = m_ParentRegion->GetObjectByControlId(m_nCollectNpcId);
		CNpc *pNpc = (CNpc*)pObject->DynamicCast(IID_NPC);
		if (!pNpc)
			return;

		if (!pNpc->m_IsDisappear)
		{
			// 采集完成回调
			g_Script.m_pPlayer = (CPlayer*)this;
			LuaFunctor(g_Script, m_sCollectFunc.c_str())[m_nCollectNpcId]();
			g_Script.m_pPlayer = NULL;
		}
	}
}

bool CPlayer::UpdateCollect()
{
	MY_ASSERT(0 != m_nCollectTimeLen);

	if ((timeGetTime()-m_nCollectBeginTime) >= m_nCollectTimeLen)
	{
		OnFinishCollect();
		m_nCollectTimeLen	= 0;
		return true;
	}

	return false;
}

bool CPlayer::CheckAddBuff(const SBuffBaseData *pBuff)
{
	return true;			
}

void CPlayer::SetBuffSaveData()
{
}

void CPlayer::InitBeginBuffData()
{
	// 加被动技能的永久Buff
	for (size_t i = 0; i < MAX_SKILLCOUNT; i++)
	{
		if (0 == m_Property.m_pSkills[i].wTypeID)
		{
			MY_ASSERT(0 == m_Property.m_pSkills[i].byLevel);
			break;
		}

		const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(m_Property.m_pSkills[i].wTypeID);
		MY_ASSERT(pData);

		if (SKT_PASSITIVE == pData->m_SkillType)
		{
			g_Script.SetCondition(0, this, 0);
			LuaFunctor(g_Script, FormatString("AttachPasstiveSkillBuff%d", pData->m_ID))[m_Property.m_pSkills[i].byLevel]();
			g_Script.CleanCondition();
		}
	}

	BYTE bySaveBuffNum = 0;
	for (size_t i = 0; i < m_Property.msc_byMaxSaveBuffCount; ++i)
	{
		if (m_Property.m_tSaveBuffData[i].m_dwBuffID == 0)
			break;
		++bySaveBuffNum;
	}
	if (bySaveBuffNum > 0)
	{
		const typedef_v4::TSaveBuffData *pSaveBuff = &m_Property.m_tSaveBuffData[0];
		m_buffMgr.RecoverSaveBuffList(pSaveBuff, bySaveBuffNum);
	}
	int num = lua_gettop(g_Script.ls);
	//重新启动BUFF
	//	if (m_Property.stBuffData[0].buffId == 0)
	//		return;

	//	DWORD curTime = timeGetTime();
	//	DWORD BeginTime = 0;
	//	DWORD LeaveTime = 0;
	//	DWORD MaxTime = 0;
	//	WORD  level   = 0;
	//	int   fightType = 0;
	//	for (int i = 0;i < BUFF_SAVE_MAX;++i)
	//	{
	// 		WORD buffId = m_Property.stBuffData[i].buffId;
	// 		if(buffId != 0)
	// 		{
	// 			MaxTime	  = m_Property.stBuffData[i].MaxTime * 1000;
	// 			BeginTime = m_Property.stBuffData[i].BeginTime;
	// 			LeaveTime = m_Property.m_dwLeaveTime;
	// 			level	  = m_Property.stBuffData[i].level;
	// 			fightType = m_Property.stBuffData[i].fightType;
	// 
	// 			if(m_Property.stBuffData[i].buffType < 2) 
	// 				continue;
	// 			else if(m_Property.stBuffData[i].buffType == 2 &&
	// 			  ( curTime > (MaxTime + LeaveTime) )   )
	// 				continue;
	// 			
	// 			//剩余的就是可以重新启动的BUFF
	// 			else if (m_Property.stBuffData[i].buffType == 2)
	// 			{
	// 				buffManager.RenewBuff(buffId,false, MaxTime - (curTime - LeaveTime),level,fightType);
	// 			}
	// 			else if (m_Property.stBuffData[i].buffType == 3)
	// 			{
	// 				buffManager.RenewBuff(buffId,false,MaxTime,level,fightType);
	// 			}
	// 		}
	//	}
}

void CPlayer::DropRegionBuffList( WORD regionId )
{
	// 	extern LPIObject FindRegionByID( DWORD ID );
	// 	CRegion *pRegion = (CRegion*)FindRegionByID(regionId)->DynamicCast(IID_REGION);
	// 	if (pRegion == NULL)
	// 		return;
	// 	size_t len = pRegion->m_vTrigerBuffPlayer.size();
	// 	for (size_t i = 0;i < len;++i)
	// 	{
	// 		BYTE buffId = pRegion->m_vTrigerBuffPlayer[i];
	// 		buffManager.DropBuff(buffId);
	// 	}
}

// 装备属性添加
void CPlayer::InitEquipAttriData(int EquipIndex)
{
	// 装备的升阶属性
	for (size_t index = 0; index < SEquipment::MAX_BORN_ATTRI; ++index)
	{
		BYTE bIndex = m_Property.m_Equip[EquipIndex].attribute.bornAttri[index].type;
		WORD nvalue = m_Property.m_Equip[EquipIndex].attribute.bornAttri[index].value;

		if (0 == bIndex && nvalue == 0)
		{
			continue;
		}
		m_EquipFactorValue[bIndex] += nvalue;
	}

	// 装备的升星的属性
	for (size_t j = 0; j < SEquipment::MAX_BORN_ATTRI; ++j)
	{
		BYTE StarIndex = m_Property.m_Equip[EquipIndex].attribute.starAttri[j].type;
		BYTE StarValue = m_Property.m_Equip[EquipIndex].attribute.starAttri[j].value;

// 		if (0 != bValue)
// 		{
// 			if (SEquipDataEx::EEA_SPEED == bIndex)
// 				m_EquipFactorValue[bIndex] += m_OriSpeed * (bValue / 200 * 0.01);
// 			else
// 				m_EquipFactorValue[bIndex] += bValue;
// 		}

		if (0 != StarValue)
		{
			m_EquipFactorValue[StarIndex] += StarValue;
// 			if (SEquipDataEx::EEA_SPEED == StarIndex)
// 				m_EquipFactorValue[StarIndex] += m_OriSpeed * (bValue / 200 * 0.01);
// 			else
// 				m_EquipFactorValue[StarIndex] += StarValue;
		}
	}

	return;
}

void CPlayer::InitEquipJewelData(int EquipIndex)
{
	for (int m = 0; m < SEquipment::MAX_SLOTS; m++)
	{
		if (m_Property.m_Equip[EquipIndex].slots[m].isJewel())
		{
			WORD bType  = m_Property.m_Equip[EquipIndex].slots[m].type;
			WORD bValue = m_Property.m_Equip[EquipIndex].slots[m].value;

			// 得到宝石ID，新公式
			DWORD itemID = 4004001 + (bType - 1) * 100 + (bValue - 1);
			const SJewelAttribute *pJwlAttri = CItemService::GetInstance().GetJewelAttribute(itemID);

			if (!pJwlAttri)
				continue;

			SItemFactorData ItemFactorData;

			// 先置零，然后复制属性。
			//memset(pItemFactorData, 0, sizeof(int) * SEquipDataEx::EEA_MAX);
			memcpy(&ItemFactorData.m_ValueFactor[0], pJwlAttri->ExtraData, sizeof(int) * SEquipDataEx::EEA_MAX);

			InitEquipAttribute(&ItemFactorData);


		}
		else
			continue;
	}
}

void CPlayer::InitEquipAttribute(const SItemFactorData *pItemFactorData)
{
	if (!m_PlayerAttri)
		return;

	for (int j = 0; j < (int)SEquipDataEx::EEA_MAX; ++j)
	{		
		m_EquipFactorValue[j] += pItemFactorData->m_ValueFactor[j];

		int PreValue = pItemFactorData->m_PrecentFactor[j];
		const WORD PreMode = 10000;

		if (0 == pItemFactorData->m_PrecentFactor[j])
			continue;

		// 计算百分比
		switch (j)
		{
		case SEquipDataEx::EEA_MAXHP:
			m_EquipFactorValue[j] += m_BaseMaxHp * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_MAXMP:
			m_EquipFactorValue[j] += m_BaseMaxMp * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_MAXTP:
			m_EquipFactorValue[j] += m_BaseMaxTp * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_GONGJI:
			m_EquipFactorValue[j] += m_BaseAtk * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_FANGYU:
			m_EquipFactorValue[j] += m_BaseDefence * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_BAOJI:
			m_EquipFactorValue[j] += m_BaseCtrAtk * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_SHANBI:
			m_EquipFactorValue[j] += m_BaseEscape * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_SPEED:
			m_EquipFactorValue[j] += PLAYER_BASIC_SPEED * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_ATKSPEED:
			m_EquipFactorValue[j] += 100 * PreValue/PreMode;
			break;

		default:
			break;
		}
	}
}

void CPlayer::CallOutFightPet(BYTE index)
{
	if (index >= MAX_NEWPET_NUM)
		return;

	if (0 == m_Property.m_NewPetData[index].m_PetID)
		return;

	/////侠客的等级不能大于玩家6级
	//if (m_Property.m_NewPetData[index].m_PetLevel-m_Property.m_Level>5)
	//{
	//	TalkToDnid(m_ClientIndex,"你所携带的侠客等级太高了，你无法召唤");
	//	return ;
	//}

	if (m_pFightPet)
		return;

	CFightPet::SParameter param;
	param.owner = this;
	param.index	= index;
	param.fX	= m_curX/*+100.0f*/;
	param.fY	= m_curY/*+100.0f*/;

	CFightPet *pFightPet = m_ParentRegion->CreateFightPet(&param);
	if (!pFightPet)
		return;

	m_pFightPet = pFightPet;

	SACallOutFightPet outMsg;
	outMsg.index = index;
	outMsg.fpGID = pFightPet->GetGID();

	g_StoreMessage(m_ClientIndex, &outMsg, sizeof(SACallOutFightPet));

	return;
}

void CPlayer::CallOutFightPet( SQCallOutFightPet *pmsg )
{
	if (pmsg->index >= MAX_NEWPET_NUM)
		return;

	if (!m_ParentRegion)
		return;

	{
		CDynamicRegion *pRegionD = (CDynamicRegion *)m_ParentRegion->DynamicCast(IID_DYNAMICREGION);
		if (pRegionD)
		{
			int Flag = 0;
			lite::Variant ret;
			LuaFunctor(g_Script,"IsCanUsePet")[pRegionD->m_DynamicIndex](&ret);
			if (lite::Variant::VT_INTEGER == ret.dataType)
			{
				Flag = ret;
				if (Flag == 0)
				{
					TalkToDnid(m_ClientIndex,"这个副本中侠客不能出战!");
					return;
				}
			}
		}

	}

	if (0 == m_Property.m_NewPetData[pmsg->index].m_PetID)
		return;
	if (m_Property.m_NewPetData[pmsg->index].m_CurPetDurable == 0)
	{
		TalkToDnid(m_ClientIndex, "宠物魂力为0，不能召唤宠物");
		return;
	}

	if (m_Property.m_Equip[EQUIP_P_SHIZHUANG_HAT1].details.wIndex == 0)
	{
		TalkToDnid(m_ClientIndex, "玩家没有武器3，不能让该宠物跟随");
		return ;
	}

	/////侠客的携带等级不能大于玩家6级
	//if (m_Property.m_NewPetData[pmsg->index].m_PetLevel-m_Property.m_Level>5)
	//{
	//	TalkToDnid(m_ClientIndex,"你所携带的侠客等级太高了，你无法召唤");
	//	return ;
	//}

	if (m_pFightPet /*|| timeGetTime() < m_FightPetTimeLimit*/)
	{
		CallBackFightPet(false);
	}
	////出战开始计技能学习时间
	//if(m_Property.m_FightPets[pmsg->index].m_fpLearnSkill)m_nfpLearnSkillTime = timeGetTime();

	CFightPet::SParameter param;
	param.owner = this;
	param.index	= pmsg->index;
	param.fX	= pmsg->x;
	param.fY	= pmsg->y;

	CFightPet *pFightPet = m_ParentRegion->CreateFightPet(&param);
	if (!pFightPet)
		return;

	m_pFightPet = pFightPet;



	SACallOutFightPet outMsg;
	outMsg.index = pmsg->index;
	outMsg.fpGID = pFightPet->GetGID();
	outMsg.x=pmsg->x;
	outMsg.y=pmsg->y;
	g_StoreMessage(m_ClientIndex, &outMsg, sizeof(SACallBackFightPet));

	//CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	//if (pArea)
	//	pArea->AddObject(pFightPet->self.lock());

	SQfpchangeatttypeMsg fpatttype;
	fpatttype.atttype = FP_PASSIVE;
	_fpChangeAttType(&fpatttype);


	return;
}

void CPlayer::CallBackFightPet(bool IsTemp)
{
	if (!m_pFightPet)
		return;

	int fpindex = m_pFightPet->m_index;

	SACallBackFightPet backMsg;
	backMsg.index = m_pFightPet->m_index;
	backMsg.fpGID = m_pFightPet->GetGID();
	g_StoreMessage(m_ClientIndex, &backMsg, sizeof(SACallBackFightPet));

	m_pFightPet->OnCallBackMe();

	m_FightPetTimeLimit = IsTemp ? m_FightPetTimeLimit : (timeGetTime() + MakeLifeTime(1));
	m_nfpLearnSkillTime = IsTemp ? m_nfpLearnSkillTime : 0xffffffff ;
	m_pFightPet->m_ParentRegion->DelFightPet(m_pFightPet->GetGID());
	m_pFightPet->m_owner->m_PreFPIndex = IsTemp ? m_pFightPet->m_index : 0xff;
	m_pFightPet = 0;

	//if(!IsTemp)_fpUpdateAllProperties(fpindex);
}

void CPlayer::ShowPet(BYTE index)
{
	static DWORD s_CurGlobalID = 1000;

	// [2012-7-27 16-37 gw: +编号合法性验证]
	int iSize = sizeof(m_Property.pets) / sizeof(m_Property.pets[0]);
	if (index < 0 || index >= iSize)
		return ;

	if (!m_ParentRegion || !m_ParentArea)
		return;

	if (m_PetStatus)
	{
		TalkToDnid(m_ClientIndex, "你已经有了宠物，还不满足嗦。。。。");
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// [2012-7-27 16-36 gw: * 目前把宠物的IC赋值给宠物状态] m_PetStatus = 1;
	m_PetStatus = m_Property.pets[index].ID;

	if (!reUseID.empty())
	{
		m_PetGID = reUseID[0];
		reUseID.erase(reUseID.begin());
	}
	else
	{
		int i = 0;
		for (; i<0xffff; i++) 
		{
			m_PetGID = 0xf0000000 | (0x0fffffff & (s_CurGlobalID++));
			if (inUseID.find(m_PetGID) == inUseID.end())
				break;
		}

		if (0xffff == i)
			return;

		inUseID.insert(m_PetGID);
	}

	// 向周围的玩家发送宠物消息
	SACallOutPet msg;
	msg.dwOwnerID	  = GetGID();
	msg.wPetID		  = m_PetStatus;
	msg.dwPetGlobalID = m_PetGID;

	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea)
		pArea->SendAdj(&msg, sizeof(SACallOutPet), -1);

	return;
}

void CPlayer::CallBackPet()
{
	if (0 == m_PetStatus)
		return;

	SACallBackPet msg;
	msg.dwOwnerID	  = GetGID();
	msg.wPetID		  = m_PetStatus;
	msg.dwPetGlobalID = m_PetGID;

	inUseID.erase(m_PetGID);
	reUseID.push_back(m_PetGID);

	m_PetStatus = 0;
	m_PetGID	= 0;

	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea)
		pArea->SendAdj(&msg, sizeof(SACallOutPet), -1);
}

void CPlayer::AddToSaledItemList(SPackageItem &item, DWORD value)
{
	// 加入回购列表
	if (CONST_BUYBACK_COUNT == m_curSaledItemCount)
		memcpy(m_buyBackItem, &m_buyBackItem[1], sizeof(SaledItemType)*(CONST_BUYBACK_COUNT-1));
	else
		m_curSaledItemCount++;

	m_buyBackItem[m_curSaledItemCount-1].dwValue	= value;
	m_buyBackItem[m_curSaledItemCount-1].sItem		= item;

	return;
}

void CPlayer::BuyBackSaledItem(SQBuySaledItem *pMsg)
{
	if (0 == m_curSaledItemCount)
		return;

	if (0xff != pMsg->bIndex && pMsg->bIndex >= m_curSaledItemCount)
		return;

	QWORD needMoney = 0;
	std::list<SAddItemInfo> itemList;

	if (0xff != pMsg->bIndex)
	{
		needMoney = m_buyBackItem[pMsg->bIndex].dwValue;
		itemList.push_back(SAddItemInfo(m_buyBackItem[pMsg->bIndex].sItem.wIndex, m_buyBackItem[pMsg->bIndex].sItem.overlap));
	}
	else
	{
		for (size_t i = 0; i < m_curSaledItemCount; i++)
		{
			needMoney += m_buyBackItem[i].dwValue;
			itemList.push_back(SAddItemInfo(m_buyBackItem[i].sItem.wIndex, m_buyBackItem[i].sItem.overlap));
		}
	}

	//QWORD totalMoney = m_Property.m_BindMoney + m_Property.m_Money;
	QWORD totalMoney = m_Property.m_Money;

	if (totalMoney < needMoney)
		return;

	if (!CanAddItems(itemList))
		return;

	if (0xff != pMsg->bIndex)
	{
		const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(m_buyBackItem[pMsg->bIndex].sItem.wIndex);
		if (!itemData)
			return;

		WORD pos = FindBlankPos(XYD_PT_BASE);
		WORD willCreate = TryPossibleMerge(m_buyBackItem[pMsg->bIndex].sItem.wIndex, m_buyBackItem[pMsg->bIndex].sItem.overlap, itemData->m_Overlay);
		InsertItem(m_buyBackItem[pMsg->bIndex].sItem.wIndex, willCreate, itemData, m_buyBackItem[pMsg->bIndex].sItem, false);

		if (pMsg->bIndex != (m_curSaledItemCount-1))
			memcpy(&m_buyBackItem[pMsg->bIndex], &m_buyBackItem[pMsg->bIndex+1], m_curSaledItemCount-pMsg->bIndex-1);
		memset(&m_buyBackItem[m_curSaledItemCount-1], 0 ,sizeof(SaledItemType));
		m_curSaledItemCount--;
	}
	else
	{
		for (size_t i = 0; i < m_curSaledItemCount; i++)
		{
			const SItemBaseData *itemData = CItemService::GetInstance().GetItemBaseData(m_buyBackItem[i].sItem.wIndex);
			if (!itemData)
				return;

			WORD pos = FindBlankPos(XYD_PT_BASE);
			WORD willCreate = TryPossibleMerge(m_buyBackItem[i].sItem.wIndex, m_buyBackItem[i].sItem.overlap, itemData->m_Overlay);
			InsertItem(m_buyBackItem[i].sItem.wIndex, willCreate, itemData, m_buyBackItem[i].sItem, false);
		}

		memset(m_buyBackItem, 0, sizeof(m_buyBackItem));
		m_curSaledItemCount = 0;
	}

	//if (m_Property.m_BindMoney >= needMoney)
	//{
	//	m_Property.m_BindMoney -= needMoney;
	//}
	//else
	//{
	//	m_Property.m_Money -= (needMoney - m_Property.m_BindMoney);
	//	m_Property.m_BindMoney = 0;
	//	m_PlayerPropertyStatus[XA_UNBIND_MONEY-XA_MAX_EXP] = true;
	//}

	//m_PlayerPropertyStatus[XA_BIND_MONEY-XA_MAX_EXP] = true;
	if (m_Property.m_Money >= needMoney)
	{
		m_Property.m_Money -= needMoney;
	}
	else
		return;

	m_PlayerPropertyStatus[XA_UNBIND_MONEY - XA_MAX_EXP] = true;

	// 发送删除消息
	SABuySaledItem buyItem;
	buyItem.bIndex = pMsg->bIndex;
	g_StoreMessage(m_ClientIndex, &buyItem, sizeof(SABuySaledItem));
}


/// 当经脉充穴成功的经验分享
void CPlayer::OnShareExpWhenJingMaiOpenVenSuccess(WORD wXueID)
{
	BYTE byShareTimes = m_Property.GetJingMaiShareExpTimes();
	if (0 == byShareTimes)
	{
		rfalse(1, 2, "您今天的经脉充穴经验分享次数已经使用完了。"); 
		return ;
	}
	if (NULL == m_ParentRegion)
	{
		return ;
	}
	check_list<LPIObject> *pPlayerList ;
	pPlayerList = m_ParentRegion->GetPlayerListByAreaIndex(m_ParentArea->m_X, m_ParentArea->m_Y);
	if (pPlayerList)
	{
		DWORD dwAddExp = 1000; /// 分享多少经验，有待调整
		for (check_list<LPIObject>::iterator it = pPlayerList->begin(); it != pPlayerList->end(); ++it)
		{
			LPIObject target = *it;
			CPlayer *pPlayer = (CPlayer*)target->DynamicCast(IID_PLAYER);
			if (pPlayer && 0 != pPlayer->m_CurHp)
			{				
				pPlayer->SendAddPlayerExp(dwAddExp, SAExpChangeMsg::JINGMAIOPENVENSHARE, "JinMai Share");
			}
		}
	}
	//m_ParentRegion
}

BOOL CPlayer::RecvOpenVenMsg(SQOpenVenMsg *pMsg)
{
	const SJingMaiBaseData *pVenData = CJingMaiService::GetInstance().GetJingMaiBaseData(pMsg->wVenID);
	if (!pVenData)
		return FALSE;


	if (!IsItemPosValid(pMsg->wItemPos))
		return FALSE;


	/* 没有这个东西 屏蔽掉 以后做真龙脉的时候用
	// 如果是经外奇脉，那么看看之前的是否已经充通
	// 此处的做法逻辑正确，但效率极低，可以通过在玩家进入游戏时候做缓存
	// 以及在游戏过程中做更新来将判断的负担分摊！
	if (MAX_MAI_NUMBER == pVenData->m_MaiID)
	{
	// 看看之前的脉线都已经打通
	for (size_t i = 1; i < MAX_MAI_NUMBER; i++)
	{
	if (!IsAllVenOpened(i))
	{
	TalkToDnid(m_ClientIndex,"请先打通之前的穴位");
	return FALSE;
	} 
	}

	}*/

	if (m_Property.IsVenMarked(pMsg->wVenID))
	{
		TalkToDnid(m_ClientIndex,"穴位已经打通");
		return FALSE;
	}

	if(!CheckFrontOpened(pVenData->m_NeedFrontXueID))
	{
		TalkToDnid(m_ClientIndex,"需要打开前置穴位");
		return FALSE;
	}


	// 验证真气是否足够
	if (!CheckPlayerSp(pVenData->m_NeedSp, true))
	{
		TalkToDnid(m_ClientIndex,"你的真气不足");
		return FALSE;
	}


	// 先扣除充穴铜人 以前是需要铜人材料 现在是作为幸运道具

	if(pMsg->bNum > 0)
	{
		//判断铜人使用个数显示
		if (pMsg->bNum > 9)
		{
			TalkToDnid(m_ClientIndex,"使用的铜人个数过多");
			return FALSE;
		}

		if (!IsItemPosValid(pMsg->wItemPos))
			return FALSE;

		SPackageItem *pSrcItem = FindItemByPos(pMsg->wItemPos, XYD_FT_ONLYUNLOCK);
		if (!pSrcItem)
			return FALSE;

		if (pVenData->m_NeedItem != pSrcItem->wIndex)
			return FALSE;


		if (pSrcItem->overlap > pMsg->bNum)
		{
			ChangeOverlap(pSrcItem, pMsg->bNum, false, true);
		}
		else
		{
			DelItem(*pSrcItem, "充穴扣除");
		}

	}



	// 扣除真气
	CheckPlayerSp(pVenData->m_NeedSp, false);

	//WORD emptyPos = pSrcItem->wCellPos;





	//MY_ASSERT(pSrcItem->overlap == 1);
	//DelItem(*pSrcItem, "充穴扣除");

	SAOpenVenMsg resultMsg;
	resultMsg.wVenID  = pMsg->wVenID;
	resultMsg.bResult = SAOpenVenMsg::SOV_SUCCESS;

	// 开始充穴
	DWORD OpenRate = CRandom::RandRange(1, 100);
	if (OpenRate <= (pVenData->m_Rate + pMsg->bNum * 5))
	{
		OpenVenPoint(pMsg->wVenID);
		IsAllVenOpened(pVenData->m_MaiID, true);

		rfalse(2, 1, "穴位%d被充通了", pMsg->wVenID);
		// 充穴成功分享经验
		//OnShareExpWhenJingMaiOpenVenSuccess(pMsg->wVenID);		
		JingMaiShareExp::ReadyJingMaiEvent *pEvent = new JingMaiShareExp::ReadyJingMaiEvent(this,pMsg->wVenID,Event::EEventInterrupt_Logout);
		if(pEvent)
		{
			DWORD dwDelayTime = 5000; 
			pEvent->SetEventName("OpenVenShareExpEvent");
			m_BatchReadyEvent.SetReadyEvent(pEvent,dwDelayTime);			
		}

		// 更新属性哦
		UpdateAllProperties();
	}
	else
	{
		// 充穴失败，那么产生一个碎片（就在原位置）
		//SRawItemBuffer item;
		//GenerateNewItem(item, SAddItemInfo(3510003, 1));
		//AddExistingItem(item, emptyPos, true);
		//[2011-9-30 xj]修改，充穴失败道具直接消失，不再产生碎片，
		resultMsg.bResult = SAOpenVenMsg::SOV_FAILED;
	}

	g_StoreMessage(m_ClientIndex, &resultMsg, sizeof(resultMsg));

	return TRUE;
}
///判断前置穴位是否打开///
bool CPlayer::CheckFrontOpened(WORD wVenID)
{
	if(wVenID <= 0)
		return true;

	return m_Property.IsVenMarked(wVenID);

}

void CPlayer::OpenVenPoint(WORD VenId)
{
	if (0 == VenId || VenId > MAX_JM_XUE)
		return;

	const SJingMaiBaseData *pVenData = CJingMaiService::GetInstance().GetJingMaiBaseData(VenId);
	if (!pVenData)
		return;

	for (size_t i = 0; i < JMP_MAX; ++i)
		m_JingMaiFactor[i] += pVenData->m_Factor[i];

	// 标记为已打开状态
	m_Property.MarkVenPoint(VenId);

	return;
}

BOOL CPlayer::IsAllVenOpened(WORD MaiID, bool AddExtra)
{
	if (0 == MaiID)
		return FALSE;

	const std::list<WORD> *pList = CJingMaiService::GetInstance().GetXueList(MaiID);
	if (!pList)
		return FALSE;

	for (std::list<WORD>::const_iterator it = pList->begin(); it != pList->end(); ++it)
	{
		MY_ASSERT(-1 != m_Property.IsVenMarked(*it));

		if (0 == m_Property.IsVenMarked(*it))
			return FALSE;
	}

	if (AddExtra)
	{
		m_iPassVenationCount++;
		// 全部穴位打通，增加额外属性
		const SJingMaiBaseData *pExtraVen = CJingMaiService::GetInstance().GetJingMaiAwardData(MaiID);
		if (!pExtraVen)
			return FALSE;

		for (size_t i = 0; i < JMP_MAX; ++i)
			m_JingMaiFactor[i] += pExtraVen->m_Factor[i];
	}
	if(OnCheckNameColorByVen())
		SendPlayerNameColor();

	return TRUE;
}

//update by ly 2014/2/18  该函数用于信仰的属性加成

void CPlayer::InitJingMaiData()
{
	//memset(m_JingMaiFactor, 0, sizeof(m_JingMaiFactor));

	//// 遍历经脉
	//for (size_t i = 1; i <= MAX_MAI_NUMBER; i++)
	//{
	//	bool AllOpened = true;
	//	const SJingMaiBaseData *pVenData = 0;

	//	const std::list<WORD> *pList = CJingMaiService::GetInstance().GetXueList(i);
	//	if (pList)
	//	{
	//		for (std::list<WORD>::const_iterator it = pList->begin(); it != pList->end(); ++it)
	//		{
	//			WORD XueID = *it;
	//			MY_ASSERT(-1 != m_Property.IsVenMarked(*it));

	//			if (0 == m_Property.IsVenMarked(*it))
	//				AllOpened = false;
	//			else
	//			{
	//				// 增加已冲开经脉的属性
	//				pVenData = CJingMaiService::GetInstance().GetJingMaiBaseData(*it);
	//				if (pVenData)
	//					for (size_t j = 0; j < JMP_MAX; ++j) m_JingMaiFactor[j] += pVenData->m_Factor[j];
	//			}
	//		}

	//		// 看看是否都冲开了
	//		if (AllOpened)
	//		{
	//			m_iPassVenationCount ++;
	//			pVenData = CJingMaiService::GetInstance().GetJingMaiAwardData(i);
	//			if (pVenData)
	//				for (size_t j = 0; j < JMP_MAX; ++j) m_JingMaiFactor[j] += pVenData->m_Factor[j];
	//		}
	//	}
	//}

	memset(m_XinYangFactor, 0, sizeof(m_XinYangFactor));
	for (size_t i = 0; i < MAX_MAI_NUMBER; i++)
	{
		for (int k = 1; k <= m_Property.m_JingMai[i]; k++)
		{
			const SXinYangBaseData *lpXinYangBaseData = CXinYangService::GetInstance().GetXingXiuDataByID(k);
			if (lpXinYangBaseData != NULL)
			for (int j = 0; j < XYP_MAX; j++)
				m_XinYangFactor[j] += lpXinYangBaseData->m_Factor[j];
		}
	}
	return;
}

//add by 2014/3/25 初始化称号附加属性
void CPlayer::InitTitleData()
{
	memset(m_TitleFactor, 0, sizeof(m_TitleFactor));
	BYTE FirstState = (BYTE)(*g_Script.m_pPlayer->m_Property.m_Title[0]) >> 7;	//获取第一个元素的状态
	BYTE FirstTitleID = ((BYTE)((*g_Script.m_pPlayer->m_Property.m_Title[0]) << 1)) >> 1;
	const STitleBaseData *pTitleBaseData = CGloryAndTitleServer::GetInstance().GetTitleBaseDataByID(FirstTitleID);	//默认第一个为当前使用的称号，如果当前没有使用称号，则第一个元素为空字符串
	if (pTitleBaseData == NULL)
		return;
	if (FirstState == 1)//附加属性加成
		memcpy(m_TitleFactor, pTitleBaseData->m_AddFactor, sizeof(m_TitleFactor));
	return;
}


void CPlayer::MoveFightPet(SQFightPetMoveto *pmsg)
{
	if (this->GetGID()!=pmsg->dwGID||!m_pFightPet)return;
	if(!m_ParentRegion)return;


	// 	float dis=pow(m_curX-pmsg->PosX,2)+pow(m_curY-pmsg->PosY,2);
	// 
	// 	dis=sqrtf(dis);
	// 
	// 	if (dis<50.0f)return;

	m_pFightPet->MoveToPostion(m_ParentRegion->GetIID(),pmsg->PosX,pmsg->PosY);

}

bool CPlayer::CheckPkRuleAttk( DWORD gid,BYTE AtkType )
{
	// [2012-5-15 17-21 gw: +暂时打开PK模式，测试完需要删除]
	//return true;

	///如果不是单攻技能则返回
	if (AtkType>SAST_SINGLE)return true;

	///判断为怪物直接返回
	LPIObject pObject = m_ParentRegion->SearchObjectListInAreas(gid, m_ParentArea->m_X, m_ParentArea->m_Y);
	CMonster *pmonster=(CMonster*)pObject->DynamicCast(IID_MONSTER);
	if (pmonster)
	{
		return true;
	}

	///地图PK模式判断返回
	if (m_ParentRegion&&m_ParentRegion->m_dwPKAvailable == CRegion::PK_SAFE)
	{
		TalkToDnid(m_ClientIndex,"在当前地图中无法攻击");
		return false;
	}

	///仇恨列表判断
	if (IsInHatelist(gid)){
		//TalkToDnid(m_ClientIndex,"仇恨列表中可以攻击");
		return true;
	}

	///等级判断
	CPlayer *player=(CPlayer*)pObject->DynamicCast(IID_PLAYER);
// 	if (player&&player->m_Level<30)
// 	{
// 		TalkToDnid(m_ClientIndex,"玩家等级小于30无法攻击");
// 		return false;
// 	}

	CFightPet *pet=(CFightPet*)pObject->DynamicCast(IID_FIGHT_PET);
	if (pet&&pet->m_owner)
	{
		if (pet->m_owner->m_Level<30)
		{
			TalkToDnid(m_ClientIndex,"侠客的主人等级小于30无法攻击");
			return false;
		}
	}

	if (!player&&!pet)
	{
		return true;
	}

	if (!player)
	{
		player=pet->m_owner;
	}

	//看目标是否红名，红名则无需验证
	if (player->m_wPKValue)return true;


	///模式判断
// 	switch(m_nPkRule)
// 	{
// 	case 0:///和平模式
// 		{
// 			TalkToDnid(m_ClientIndex,"和平模式无法攻击");
// 			return false;
// 		}
// 		break;
// 	case 1:///帮派模式 
// 		{
// 			///TalkToDnid(m_ClientIndex,"帮派模式");
// 			return (m_Property.m_TongID==0)?true:(m_Property.m_TongID!=player->m_Property.m_TongID);
// 		}
// 		break;
// 	case 2:///队伍模式
// 		{
// 			///TalkToDnid(m_ClientIndex,"队伍模式");
// 			return (m_dwTeamID==0)?true:(m_dwTeamID!=player->m_dwTeamID);
// 		}
// 		break;
// 	case 3:///门派模式
// 		{
// 			///TalkToDnid(m_ClientIndex,"门派模式");
// 			return (m_Property.m_School==0)?true:(m_Property.m_School!=player->m_Property.m_School);
// 		}
// 		break;
// 	case 4:///全体模式
// 		{
// 			///	TalkToDnid(m_ClientIndex,"全体模式可以攻击");
// 			return true;
// 		}
// 		break;
// 	default:
// 		break;
// 	}
	return true;
}

WORD CPlayer::GetPlayerReginID()
{
	return (m_ParentRegion)?m_ParentRegion->m_wRegionID:0;
}

// DWORD CPlayer::GetDynamicRegionID()
// {
// 	CDynamicRegion *dyregion=(CDynamicRegion*)m_ParentRegion->DynamicCast(IID_DYNAMICREGION);
// 	return (dyregion)?dyregion->GetGID():0;
// }

WORD CPlayer::GetTelerLevelByIndex(WORD index)
{
	for (int i = 0; i < MAX_TELEGRYNUM; ++i)
	{
		if (0 == m_Property.m_Xyd3Telergy[i].m_TelergyID && 0 == m_Property.m_Xyd3Telergy[i].m_TelergyLevel)
		{
			return 0;
		}

		if (m_Property.m_Xyd3Telergy[i].m_TelergyID == index)
		{
			return m_Property.m_Xyd3Telergy[i].m_TelergyLevel;
		}
	}

	return 0;
}

void CPlayer::ListenProperty()
{
	//if (m_LastXinFaTime - m_StartXinFaTime > TP_INTERVAL_TIME)
	//{
	//	m_GongJi			+= m_AddAttack;
	//	m_AddAttack			 = 0;
	//	m_IsInDeductAttack	 = false;
	//}

	//if (m_LastXinFaDefenceTime - m_StartXinFaDefenceTime > TP_INTERVAL_TIME)
	//{
	//	m_FangYu		   += m_AddDefence;
	//	m_AddDefence		= 0;
	//	m_IsInDeductDefence	= false;
	//}
	return;
}

LPCSTR CPlayer::GetTeamLeaderName( BYTE index )
{
	if(!m_dwTeamID)return "";
	if(index<0)return "";
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(m_dwTeamID);
	if (iter == teamManagerMap.end()){
		return "";
	}
	Team &team = iter->second;
	if(index<team.byMemberNum)return team.stTeamPlayer[index].szName;
	return "";
}
DWORD CPlayer::GetTeamLeaderRegionGID()
{
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(m_dwTeamID);
	if (iter != teamManagerMap.end())
	{
		Team stTeamInfo = iter->second;
		extern LPIObject FindRegionByGID(DWORD GID);
		CDynamicRegion *dynamicRegion = (CDynamicRegion *)FindRegionByGID(stTeamInfo.ParentReginID)->DynamicCast(IID_DYNAMICREGION);
		if (dynamicRegion)
		{
			return stTeamInfo.ParentReginID;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}
DWORD CPlayer::GetTeamCurPartRegionGID()
{
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(m_dwTeamID);
	if (iter != teamManagerMap.end())
	{
		Team stTeamInfo = iter->second;
		return stTeamInfo.PartregionID;
	}
	return 0;
}
void CPlayer::OnCheckFriendNum( DWORD dwCurTick )
{
	WORD  nFriendNum = GetOnlineFriendsNum();
	if (nFriendNum == 0 ){
		m_nOnlineFriendTime = 0;
		return;
	}

	//可以开始计算时间
	if (m_nOnlineFriendTime == 0){
		m_nOnlineFriendTime = dwCurTick;
		return;
	}

	//得到奖励的现在时间
	DWORD nRewardtime=CFriendService::GetInstance().GetFriendRewardTime(m_Level);
	if (dwCurTick<m_nOnlineFriendTime+nRewardtime * 1000)return;

	//得到奖励限制的累积次数
	WORD nRewardCount = CFriendService::GetInstance().GetFriendRewardCount(m_Level);
	if (m_nOnlineExpCount> nRewardCount-1)return;

	//得到奖励的经验
	DWORD nRewardexp =CFriendService::GetInstance().GetFriendRewardExp(m_Level);
	nRewardexp = nRewardexp * nFriendNum ;

	m_nOnlineFriendTime = dwCurTick;

	///根据时间间隔以及好友数量送经验
// 	SFriendOnlineMsg msg;
// 	msg.eState = SFriendOnlineMsg::FRIEND_STATE_TIMEOUTWITHEXP;
// 	memcpy(msg.cName, GetName(), CONST_USERNAME);
// 	msg.dwgiveexp=nRewardexp;
// 	msg.wFriendCount = nFriendNum;
// 	SYSTEMTIME stime;
// 	GetLocalTime(&stime);
// 	msg.giveTime = stime;
// 	g_StoreMessage(m_ClientIndex,&msg,sizeof(SFriendOnlineMsg));
	//char str[200];
	//sprintf(str,"送经验%d,在线好友个数%d,于%d年%d月%d日",nRewardexp,nFriendNum,stime.wYear,stime.wMonth,stime.wDay);
	//TalkToDnid(m_ClientIndex,str);

	m_nOnlineExpCount++;//记录已经赠送的次数
	m_nOnlineFriendExp+=nRewardexp; //记录总共赠送的经验
	m_nOnlineFriendLastExp = nRewardexp;	//记录最后一次赠送的经验
}

void CPlayer::ImpelFightPet( SQFightPetImpel *PetImpelmsg )
{
	BYTE index = PetImpelmsg->m_index;
	WORD Impelvalue = PetImpelmsg->m_impelValue;

	if (m_Property.m_EngValue < Impelvalue)
	{
		TalkToDnid(m_ClientIndex,"激励值不足，激励失败");
		return;
	}

	SFightPetExt &pPet = m_Property.m_FightPets[index];
	if (0 == pPet.m_fpID)
		return;
	if(Impelvalue == 0)return;

	//心情最大值是不减少激励
	if (pPet.m_fpMood == CFightPet::MAX_Mood)
	{
		TalkToDnid(m_ClientIndex,"侠客心情已达到最大。无需激励");
		return;
	}


	//计算心情
	WORD tempEng = pPet.m_fpMood +Impelvalue * 10;
	WORD tempMood = pPet.m_fpMood;
	WORD tempEngValue = m_Property.m_EngValue ;

	m_Property.m_EngValue = m_Property.m_EngValue >= Impelvalue ?m_Property.m_EngValue - Impelvalue:m_Property.m_EngValue;
	pPet.m_fpMood = tempEng>=CFightPet::MAX_Mood?CFightPet::MAX_Mood:tempEng;

	//通知客户端更改心情
	if (tempMood != pPet.m_fpMood)
	{
		SAFightPetExtraChange PropertyMsg;
		PropertyMsg.changeType =	SAFightPetExtraChange::CHANGE_MOOD;
		PropertyMsg.wchangeValue = pPet.m_fpMood;
		PropertyMsg.index = index;
		g_StoreMessage(m_ClientIndex, &PropertyMsg, sizeof(SAFightPetExtraChange));
	}

	//通知客户端更改激励
	if (tempEngValue !=	m_Property.m_EngValue )
	{
		SAFightPetImpel Impelmsg;
		Impelmsg.m_impelValue = m_Property.m_EngValue;
		g_StoreMessage(m_ClientIndex, &Impelmsg, sizeof(SAFightPetImpel));
	}

}

void CPlayer::GetImpelValueOntime( DWORD dwCurTick )
{
	//等到激励值，每2小时得到1点。测试改为1分钟
	if (dwCurTick - m_nImpelFightPetTime > MINUTESTIME(1))
	{
		m_nImpelFightPetTime = dwCurTick;
		WORD tempEngValue =	m_Property.m_EngValue;
		m_Property.m_EngValue = m_Property.m_EngValue+1 <= CFightPet::MAX_IMPEL ?m_Property.m_EngValue +1:m_Property.m_EngValue;

		//通知客户端更改激励
		if (tempEngValue !=	m_Property.m_EngValue )
		{
			SAFightPetImpel Impelmsg;
			Impelmsg.m_impelValue = m_Property.m_EngValue;
			g_StoreMessage(m_ClientIndex, &Impelmsg, sizeof(SAFightPetImpel));
		}
	}

	for(int index= 0;index<MAX_FIGHTPET_NUM;index++)//侠客回复疲劳时间
	{
		if (0 == m_Property.m_FightPets[index].m_fpID)
			continue;

		//如果正在学习技能并且时间足够
		if (_fpIsInfightPet(index)&&m_Property.m_FightPets[index].m_fpLearnSkill>0)
		{
			if(m_nfpLearnSkillTime != 0xffffffff && dwCurTick - m_nfpLearnSkillTime > MINUTESTIME(1))
			{
				//让客户端卸载装备的技能书
				SAfpSkillUpdate askillmeg;
				askillmeg.byWhat = SAfpSkillUpdate::SSU_UNEQUIPSKILL;
				askillmeg.itemID = 0;
				askillmeg.index = index;
				g_StoreMessage(m_ClientIndex,&askillmeg,sizeof(SAfpSkillUpdate));

				const  SItemBaseData *itemdata = CItemService::GetInstance().GetItemBaseData(m_Property.m_FightPets[index].m_fpLearnSkill);
				if(!itemdata) return;
				_fpActivaSkillByItem(itemdata->m_ID,index,"技能");
				m_nfpLearnSkillTime = 0xffffffff ;
				m_Property.m_FightPets[index].m_fpLearnSkill = 0;

			}
		}
		//在出战中德侠客不回复疲劳
		if (m_pFightPet&&m_pFightPet->m_index == index)continue;

		///侠客每5分钟恢复1点疲劳,测试改为1分钟
		if (dwCurTick-m_nRestoreTiredTime[index]>MINUTESTIME(1))
		{
			m_nRestoreTiredTime[index] = dwCurTick;
			_fpChangeExtraProperties(SAFightPetExtraChange::CHANGE_TIRED,-1,index);
		}

	}

}

BOOL CPlayer::_fpSetEquipFightPet( BYTE index )
{
	m_nEquipfpIndex = m_nEquipfpIndex<0?index:m_nEquipfpIndex;
	return m_nEquipfpIndex==index ;
}

void CPlayer::_fpInitEquipAttribute( const SItemFactorData* pItemFactorData )
{
	if (m_nEquipfpIndex<0)
		return;

	if(m_Property.m_FightPets[m_nEquipfpIndex].m_fpID == 0)return;

	for (int j = 0; j < (int)SEquipDataEx::EEA_MAX; ++j)
	{
		m_EquipFactorValue[j] += pItemFactorData->m_ValueFactor[j];

		int PreValue = pItemFactorData->m_PrecentFactor[j];
		const WORD PreMode = 10000;

		if (0 == pItemFactorData->m_PrecentFactor[j])
			continue;

		DWORD BaseValue = 0;
		// 计算百分比
		switch (j)
		{
		case SEquipDataEx::EEA_MAXHP:
			BaseValue = m_Property.m_FightPets[m_nEquipfpIndex].m_fpHpIncRadio;
			m_EquipFactorValue[j] += BaseValue * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_MAXMP:
			BaseValue = m_Property.m_FightPets[m_nEquipfpIndex].m_fpMpIncRadio;
			m_EquipFactorValue[j] += BaseValue * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_MAXTP:
			//m_EquipFactorValue[j] += m_BaseMaxTp * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_GONGJI:
			BaseValue = m_Property.m_FightPets[m_nEquipfpIndex].m_fpGongJi;
			m_EquipFactorValue[j] += BaseValue * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_FANGYU:
			BaseValue = m_Property.m_FightPets[m_nEquipfpIndex].m_fpFangYu;
			m_EquipFactorValue[j] += BaseValue * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_BAOJI:
			BaseValue = m_Property.m_FightPets[m_nEquipfpIndex].m_fpBaoJi;
			m_EquipFactorValue[j] += BaseValue * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_SHANBI:
			BaseValue = m_Property.m_FightPets[m_nEquipfpIndex].m_fpShanBi;
			m_EquipFactorValue[j] += BaseValue * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_SPEED:
			m_EquipFactorValue[j] += BASIC_RUN_SPEED * PreValue/PreMode;
			break;

		case SEquipDataEx::EEA_ATKSPEED:
			m_EquipFactorValue[j] += 100 * PreValue/PreMode;
			break;

		default:
			break;
		}
	}
}

void CPlayer::_fpInitEquipJewelData( int EquipIndex )
{
	if(m_nEquipfpIndex < 0 )return;
	//m_Property.m_FightPets[m_nEquipfpIndex].m_Equip
	for (int i=0; i < SEquipment::MAX_SLOTS; i++)
	{
		if (m_Property.m_FightPets[m_nEquipfpIndex].m_Equip[EquipIndex].slots[i].isJewel())
		{
			WORD bType  = m_Property.m_FightPets[m_nEquipfpIndex].m_Equip[EquipIndex].slots[i].type;
			WORD bValue = m_Property.m_FightPets[m_nEquipfpIndex].m_Equip[EquipIndex].slots[i].value;
			DWORD itemID = 4010001 + (bType - 1) * 15 + (bValue - 1);
			const SItemFactorData *pItemFactor = CItemService::GetInstance().GetItemFactorData(itemID);
			if (!pItemFactor)
				continue;

			_fpInitEquipAttribute(pItemFactor);
		}
		else
			continue;
	}
}

void CPlayer::_fpInitEquipRandAttriData( int EquipIndex )
{
	if(m_nEquipfpIndex < 0)return;

	// 装备产生后的随机属性
	for (size_t i = 0; i < SEquipment::MAX_BORN_ATTRI; ++i)
	{
		BYTE bIndex = m_Property.m_FightPets[m_nEquipfpIndex].m_Equip[EquipIndex].attribute.bornAttri[i].type;
// 		WORD bValue = m_Property.m_FightPets[m_nEquipfpIndex].m_Equip[EquipIndex].attribute.bornAttri[i].value
// 			+ m_Property.m_FightPets[m_nEquipfpIndex].m_Equip[EquipIndex].attribute.refineExtraAttri[i].value;

		if (0xff == bIndex)
		{
			continue;
		}

		m_EquipFactorValue[bIndex] += 0;		
	}

	// 装备的升阶后的属性
	for (size_t j = 0; j < SEquipment::MAX_GRADE_ATTRI; ++j)
	{
		//BYTE bIndex = m_Property.m_FightPets[m_nEquipfpIndex].m_Equip[EquipIndex].attribute.gradeAttri[j].type;
//		WORD bValue = m_Property.m_FightPets[m_nEquipfpIndex].m_Equip[EquipIndex].attribute.gradeAttri[j].value;

// 		if (0xff == bIndex)
// 		{
// 			continue;
// 		}
// 
// 		m_EquipFactorValue[bIndex] += bValue;
	}

	return;
}

void CPlayer::_fpInitEquipmentData()
{
	if(m_nEquipfpIndex < 0)return;
	memset(m_EquipFactorValue, 0, sizeof(m_EquipFactorValue));

	// 计算随机属性
	for (int i = 0; i < EQUIP_P_MAX; ++i)
	{
		if (0 == m_Property.m_FightPets[m_nEquipfpIndex].m_Equip[i].wIndex)
			continue;

		// 耐久度为0，则不加属性
		if (!m_Property.m_FightPets[m_nEquipfpIndex].m_Equip[i].attribute.currWear)
			continue;

		const SItemFactorData *pItemFactor = CItemService::GetInstance().GetItemFactorData(m_Property.m_FightPets[m_nEquipfpIndex].m_Equip[i].wIndex);
		if (!pItemFactor)
			continue;

		// 装备本身的属性加成
		_fpInitEquipAttribute(pItemFactor);

		// 装备宝石的属性加成
		_fpInitEquipJewelData(i);

		// 装备的其他加成属性
		_fpInitEquipRandAttriData(i);
	}
}

void CPlayer::_fpUpdateAllProperties(int index)
{
	if(m_nEquipfpIndex < 0 && index < 0)return;

	if(m_nEquipfpIndex < 0 ) m_nEquipfpIndex = index;

	const SFightPetBaseData *pBaseData = CFightPetService::GetInstance().GetFightPetBaseData(m_Property.m_FightPets[m_nEquipfpIndex].m_fpID);
	if (!pBaseData)
		return ;

	SFightPetExt petProperty = m_Property.m_FightPets[m_nEquipfpIndex];

	WORD tempLevel  = petProperty.m_fpLevel - 1;
	// 	petProperty.m_nMaxHp			= petProperty.m_BaseMaxHp		+ (tempLevel) * petProperty.m_fpHpIncRadio;
	// 	petProperty.m_nMaxMp			= petProperty.m_BaseMaxMp		+ (tempLevel) * petProperty.m_fpMpIncRadio;
	// 	petProperty.m_nGongJi				= petProperty.m_BaseAtk				+ (tempLevel) * petProperty.m_fpGongJi;
	// 	petProperty.m_nFangYu			= petProperty.m_BaseDefence		+ (tempLevel) * petProperty.m_fpFangYu;


	petProperty.m_nMaxHp			= pBaseData->m_Attri[FPA_MAXHP][0]		+ (tempLevel) * petProperty.m_fpHpIncRadio;
	petProperty.m_nMaxMp			= pBaseData->m_Attri[FPA_MAXMP][0]	+ (tempLevel) * petProperty.m_fpMpIncRadio;
	petProperty.m_nGongJi				= pBaseData->m_Attri[FPA_GONGJI][0]		+ (tempLevel) * petProperty.m_fpGongJi;
	petProperty.m_nFangYu			= pBaseData->m_Attri[FPA_FANGYU][0] 	+ (tempLevel) * petProperty.m_fpFangYu;



	petProperty.m_fpBaoJi			=  petProperty.m_BaseCtrAtk;
	petProperty.m_fpShanBi			=  petProperty.m_BaseEscape;

	// 获取装备的附加属性
	petProperty.m_nMaxHp			+= m_EquipFactorValue[SEquipDataEx::EEA_MAXHP];
	petProperty.m_nMaxMp			+= m_EquipFactorValue[SEquipDataEx::EEA_MAXMP];
	petProperty.m_nGongJi			+= m_EquipFactorValue[SEquipDataEx::EEA_GONGJI];
	petProperty.m_nFangYu			+= m_EquipFactorValue[SEquipDataEx::EEA_FANGYU];
	petProperty.m_fpBaoJi			+= m_EquipFactorValue[SEquipDataEx::EEA_BAOJI];
	petProperty.m_fpShanBi			+= m_EquipFactorValue[SEquipDataEx::EEA_SHANBI];


	m_Property.m_FightPets[m_nEquipfpIndex] = petProperty;
	//发送侠客属性变化消息
	SAFightPetPropertyChange PropertyMsg;
	PropertyMsg.m_MaxHp=petProperty.m_nMaxHp	;
	PropertyMsg.m_MaxMp=petProperty.m_nMaxMp;	
	PropertyMsg.m_GongJi=petProperty.m_nGongJi	;
	PropertyMsg.m_FangYu=petProperty.m_nFangYu;
	PropertyMsg.m_BaoJi=petProperty.m_fpBaoJi	;
	PropertyMsg.m_ShanBi=petProperty.m_fpShanBi;
	PropertyMsg.m_CurHp=petProperty.m_nCurHp;
	PropertyMsg.m_CurMp=petProperty.m_nCurMp;
	PropertyMsg.m_index=m_nEquipfpIndex;
	g_StoreMessage(m_ClientIndex, &PropertyMsg, sizeof(SAFightPetPropertyChange));

	// 	m_OriSpeed			+= m_EquipFactorValue[SEquipDataEx::EEA_SPEED];
	// 	m_AtkSpeed			+= m_EquipFactorValue[SEquipDataEx::EEA_ATKSPEED];
}

void CPlayer::_fpUpdateExtraProperties( BYTE changetype,BYTE index )
{
	SFightPetExt &pPet = m_Property.m_FightPets[index];
	if (0 == pPet.m_fpID)
		return;

	bool change = false;
	SAFightPetExtraChange PropertyMsg;
	PropertyMsg.index = index;
	PropertyMsg.changeType = changetype;
	char str[200]={};

	if (changetype == SAFightPetExtraChange::CHANGE_TIRED)
	{
		PropertyMsg.wchangeValue = pPet.m_fpTired;
		change = true;
		sprintf(str,"疲劳度改变为%d",pPet.m_fpTired);
	}
	else if (changetype == SAFightPetExtraChange::CHANGE_RELATION)
	{
		PropertyMsg.wchangeValue = pPet.m_fpRelation;
		change = true;
		sprintf(str,"亲密度改变为%d",pPet.m_fpRelation);
	}
	else if (changetype == SAFightPetExtraChange::CHANGE_MOOD)
	{
		PropertyMsg.wchangeValue = pPet.m_fpMood;
		change = true;
		sprintf(str,"心情改变为%d",pPet.m_fpMood);
	}

	if(change)
	{
		g_StoreMessage(m_ClientIndex, &PropertyMsg, sizeof(SAFightPetExtraChange));
		TalkToDnid(m_ClientIndex,str);
	}
}

void CPlayer::_fpChangeExtraProperties( BYTE changetype,int changevalue,BYTE index )
{
	SFightPetExt &pPet = m_Property.m_FightPets[index];
	if (0 == pPet.m_fpID)
		return;

	bool Ischange = false;

	if (changetype == SAFightPetExtraChange::CHANGE_TIRED)
	{
		BYTE tempTired = pPet.m_fpTired+changevalue ;
		pPet.m_fpTired = (tempTired<=CFightPet::MAX_TIRED&&tempTired>=0)?tempTired:pPet.m_fpTired;
		if (pPet.m_fpTired == tempTired)Ischange = true;


	}
	else if (changetype == SAFightPetExtraChange::CHANGE_RELATION)
	{
		WORD tempRelation = pPet.m_fpRelation+changevalue;
		pPet.m_fpRelation = (tempRelation<=CFightPet::MAX_RELATION&&tempRelation>=0)?tempRelation:pPet.m_fpRelation;
		if(tempRelation == pPet.m_fpRelation)Ischange = true;

	}
	else if (changetype == SAFightPetExtraChange::CHANGE_MOOD)
	{
		BYTE tempmood = pPet.m_fpMood +changevalue;
		pPet.m_fpMood = (tempmood<=CFightPet::MAX_Mood&&tempmood>=0)?tempmood:pPet.m_fpMood;
		if(tempmood == pPet.m_fpMood)Ischange = true;

	}
	if(Ischange)_fpUpdateExtraProperties(changetype,index);
}

SFightPetExt * CPlayer::_fpGetFightPetByIndex( BYTE index )
{
	if (index>=MAX_FIGHTPET_NUM)return 0;
	return m_Property.m_FightPets[index].m_fpID == 0?0:&m_Property.m_FightPets[index];

}

bool CPlayer::_fpIsEquipSkillItem( DWORD itemid )
{
	const SItemBaseData *itemdata = CItemService::GetInstance().GetItemBaseData(itemid);
	return itemdata?(itemdata->m_School==SS_XIAKE&&ITEM_IS_BOOK(itemdata->m_Type)):false;
}

int CPlayer::lua_fpGetItemToSkillID( DWORD itemid )
{
	lite::Variant ret;
	g_Script.SetCondition(NULL, NULL, NULL);
	LuaFunctor(g_Script, "GetItemTofpSkillID")[itemid](&ret);
	g_Script.CleanCondition();

	if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
		return 0;
	return ( __int32 )ret;
}

int CPlayer::_fpGetSkillSlot( WORD wSkillID,BYTE index )
{
	SFightPetExt *pet = _fpGetFightPetByIndex(index);
	if(!pet)return -2;
	for (int i = 0; i < SFightPetBase::FP_SKILL_MAX; i++)
	{
		if (wSkillID == pet->m_fpSkill[i].wTypeID && pet->m_fpSkill[i].byLevel > 0)
			return i;
	}
	return -1;
}

int CPlayer::_fpLearnSkillByfp( SQfpSkillUpdate *pMsg )
{
	if(pMsg->byWhat != SQfpSkillUpdate::SSU_LEARNEDBYANOTHER)return -1;
	SFightPetExt *pet = _fpGetFightPetByIndex(pMsg->index);
	SFightPetExt *anotherpet = _fpGetFightPetByIndex(pMsg->anotherindex);
	if(!pet||!anotherpet)return -1;
	if(m_pFightPet&&m_pFightPet->m_index == pMsg->anotherindex){
		TalkToDnid(m_ClientIndex,"不能是正在出战的侠客");
		return -1;
	}
	if(pMsg->index == pMsg->anotherindex)
	{
		TalkToDnid(m_ClientIndex,"不能是同一个侠客");
		return -1;
	}

	//从1开始。0是侠客的普通攻击
	for (int i = 1; i < SFightPetBase::FP_SKILL_MAX; i++)
	{
		if (anotherpet->m_fpSkill[i].wTypeID && anotherpet->m_fpSkill[i].byLevel )
		{
			int skillnum = _fpGetSkillNum(pMsg->index);
			if(pet->m_fpActivedSkill<skillnum+1)
			{
				TalkToDnid(m_ClientIndex,"侠客技能栏已满，不能学习新的技能！");
				return 0;
			}
			_fpActivaSkill(anotherpet->m_fpSkill[i].wTypeID ,anotherpet->m_fpSkill[i].byLevel ,pMsg->index,"从其他侠客身上学习技能",pMsg->byWhat);

		}
	}
	//删除这个侠客
	DeleteFightPet(pMsg->anotherindex);
	return 1;
}
//接收闭关领取消息
void CPlayer::OnRecvBiGuanMsg( SBiguanMsg* pMsg )
{
	switch(pMsg->_protocol)
	{
	case SBiguanMsg::EPRO_BIGUAN_LINGQU:
		OnRecvBiGuanLingquMsg((SQBiguanLingquMsg*)pMsg);
		break;
	}
}
//接收闭关领取消息
void CPlayer::OnRecvBiGuanLingquMsg( SQBiguanLingquMsg* pMsg )
{
	SABiguanLingquMsg blMsg;
	INT64 now;
	_time64(&now);
	bool issuccess = _L_calculateSpAndExpByTime(now - m_Property.m_lastBiguanTime,blMsg.sp,blMsg.exp);
	//calculateSpAndExpByTime(now - m_Property.m_lastBiguanTime,blMsg.sp,blMsg.exp);
	if (pMsg->type == SQBiguanLingquMsg::BIGUAN_LINGQU_SINGLE)	//单倍领取
	{
		blMsg.result = SABiguanLingquMsg::SUCCESS;
		blMsg.type = SABiguanLingquMsg::SINGLE_LINGQU;

	}
	else if (pMsg->type == SQBiguanLingquMsg::BIGUAN_LINGQU_DOUBLE)	//双倍领取，需要鼠儿果
	{
		int index = -1;
		for(int i = 0; i < PackageAllCells;i++)
		{
			if (m_PackageItems[i].m_Item.wIndex == ITEM_SHUERGUO_ID)
			{
				index = i;
				break;
			}
		}
		if (index != -1)
		{
			blMsg.sp = blMsg.sp*2;
			blMsg.exp = blMsg.exp*2;
			blMsg.result = SABiguanLingquMsg::SUCCESS;
			blMsg.type = SABiguanLingquMsg::DOUBLE_LINGQU;
			if (m_PackageItems[index].m_Item.overlap > 1)
			{
				m_PackageItems[index].m_Item.overlap--;
				SendItemSynMsg(&m_PackageItems[index].m_Item);
			}
			else
			{
				DelItem(m_PackageItems[index].m_Item,"双倍领取闭关奖励");
			}			
		}
		else
		{
			blMsg.sp = 0;
			blMsg.exp = 0;
			blMsg.result = SABiguanLingquMsg::FAILED;
			blMsg.type = SABiguanLingquMsg::DOUBLE_LINGQU;
		}
	}
	else if (pMsg->type == SQBiguanLingquMsg::BIGUAN_LINGQU_GOUMAI)		//购买鼠儿果，并领取双倍奖励
	{

	}
	blMsg.result = !issuccess;
	if (blMsg.result == SABiguanLingquMsg::SUCCESS)
	{
		AddPlayerSp(blMsg.sp);
		SendAddPlayerExp(blMsg.exp,SAExpChangeMsg::BIGUANJIANGLI);
		//m_Property.m_Exp += blMsg.exp;
		//m_PlayerPropertyStatus[XA_MAX_EXP-XA_MAX_EXP] = true;
		m_Property.m_lastBiguanTime = now;		
	}	
	g_StoreMessage(m_ClientIndex,&blMsg,sizeof(blMsg));
}
int CPlayer::lua_fpGetItemToSkillLevel( DWORD itemid )
{
	lite::Variant ret;
	g_Script.SetCondition(NULL, NULL, NULL);
	LuaFunctor(g_Script, "GetItemTofpSkillLevel")[itemid](&ret);
	g_Script.CleanCondition();

	if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
		return 0;
	return ( __int32 )ret;
}
void CPlayer::calculateSpAndExpByTime( INT64 sec ,DWORD &sp,DWORD &exp)
{
	sp = 0;
	exp = 0;
	INT64 t = 0;
	int i = 3;		//3小时为一阶段
	int j = 90;		//第一阶段为90秒加一次，每阶段减10秒
	int k = 20;		//第一阶段加20点经验,每阶段多加20点
	while(true)
	{
		if (i > 12)		//大于12小时不再累积
		{
			break;
		}
		int temp = i * 3600;
		int temp1 = (i-3) * 3600;
		if (sec > temp)
		{
			int temp2 = (temp - temp1)/j;
			sp += temp2;
			exp += temp2*k;
		}
		else
		{
			int temp2 = (sec - temp1)/j;
			sp += temp2;
			exp += temp2*k;
			break;
		}
		i += 3;
		j -= 10;
		k += 20;
	}	
}

// 默认打开第一 个
BOOL CPlayer::QuestDB2GetRankList(struct SQClientQuestRankFromDB *pMsg)
{
	if (CSingleRank::GetInstance().GetRankCondition())
	{
		CSingleRank::GetInstance().GetTheRankList(this);
		CSingleRank::GetInstance().SetRankCondition(false);

		// 		SAClientQuestRankFromDB msg;
		// 		msg.type = SAClientQuestRankFromDB::IU_UPDATE;
		// 		
		// 		memcpy(msg.RankNum, m_RankNum, sizeof(BYTE) * RT_MAX);
		// 		memcpy(msg.RankList, m_RankList, sizeof(Rank4Client) * RT_MAX * MAX_RANKLIST_NUM);
		// 
		// 		SectionMessageManager::getInstance().sendMessageWithSection(m_ClientIndex,&msg,sizeof(msg));
		// 	}
		// 	else
		// 	{
		// 		SAClientQuestRankFromDB msg;
		// 		msg.type = SAClientQuestRankFromDB::IU_UNUPDATE;
		// 
		// 		g_StoreMessage(m_ClientIndex, &msg, sizeof(msg.type));
	}

	// 默认发送等级排行榜的数据
	SARankListMsg msg;
	msg.bMyRank = m_RankNum[RT_LEVEL];
	memcpy(msg.ranklist, m_RankList[RT_LEVEL], sizeof(Rank4Client) * MAX_RANKLIST_NUM);
	g_StoreMessage(m_ClientIndex, &msg, sizeof(SARankListMsg));

	return TRUE;
}

// XYD3-查看排行榜信息
BOOL CPlayer::GetRankListInfo(struct SQRankListMsg *pMsg)
{
	if (pMsg->bType < RT_LEVEL || pMsg->bType >= RT_MAX)
	{
		rfalse("The type of Rank List which Client requested is exceptional!!");
		return FALSE;
	}

	SARankListMsg msg;

	msg.bMyRank = m_RankNum[pMsg->bType];

	memcpy(msg.ranklist, m_RankList[pMsg->bType], sizeof(Rank4Client) * MAX_RANKLIST_NUM);

	g_StoreMessage(m_ClientIndex, &msg, sizeof(SARankListMsg));

	return TRUE;
}

bool CPlayer::tizhiGrow(DWORD itemId)
{
	const CTiZhiService::TiZhiData &tzData = CTiZhiService::GetInstance().getTiZhiData(m_Property.byTiZhiLevel);
	if (itemId != tzData.dwGrowItem)		//错误的道具
	{
		TalkToDnid(m_ClientIndex,"当前体质等级无法使用此道具！");
		return false;
	}
	bool ret = false;
	for (int i = 0;i < SPlayerTiZhi::TZ_MAX;i++)
	{
		DWORD temp = m_Property.dwTiZhiAttrs[i];
		if (tzData.dwMaxAttrs[i]&&temp < (tzData.dwPrevTotalMaxAttrs[i] + tzData.dwMaxAttrs[i]))
		{
			if(!tzData.dwIncrements[i]) continue;
			ret = true;
			//temp += CRandom::RandRange(1,tzData.dwIncrements[i]);
			temp += tzData.dwIncrements[i];
			if (temp > (tzData.dwPrevTotalMaxAttrs[i] + tzData.dwMaxAttrs[i]))
			{
				temp = tzData.dwPrevTotalMaxAttrs[i] + tzData.dwMaxAttrs[i];
			}
			m_Property.dwTiZhiAttrs[i] = temp;
		}

	}
	if (!ret)
	{
		TalkToDnid(m_ClientIndex,"当前体质等级全部属性已达到上限，无法使用此道具，请提升体质等级！");
		return false;
	}
	SendTiZhiData();
	UpdateAllProperties();
	return true;
}
void CPlayer::_fpMakeItem( BYTE index )
{
	SFightPetExt *pet = _fpGetFightPetByIndex(index);
	if(!pet)return;
	if(m_pFightPet&&m_pFightPet->m_index == index)CallBackFightPet(false);

	SFightPetExt pPet = m_Property.m_FightPets[index];
	if (0 == pPet.m_fpID)return;

	//必须卸载身上的装备
	for(size_t i = 0;i < EQUIP_P_MAX;i++)
	{
		if(m_Property.m_FightPets[index].m_Equip[i].wIndex)
		{
			TalkToDnid(m_ClientIndex,"请先将侠客的装备放进背包内才能进行此操作");
			return;
		}
	}

	// 给玩家添加一个侠客道具
	SRawItemBuffer itemBuffer;

	memset(&itemBuffer, 0, sizeof(itemBuffer));
	extern BOOL GenerateNewUniqueId(SItemBase &item);
	if (!GenerateNewUniqueId(itemBuffer))
		return ;

	SItemBase *item = &itemBuffer;
	item->wIndex	= m_Property.m_FightPets[index].m_niconID - 10000 ; //侠客道具ID
	item->usetimes	= 1;
	item->flags		= 0;
	item->overlap	= 1;

	const SItemBaseData *itemdata = CItemService::GetInstance().GetItemBaseData(item->wIndex);
	if(!itemdata)
	{
		TalkToDnid(m_ClientIndex,FormatString("没有找到侠客道具%d,生成道具失败",item->wIndex));
		return;
	}

	SFightPetItem *petitem = reinterpret_cast<SFightPetItem *>(item);
	memcpy(&(petitem->fpInfo),&pPet,sizeof(SFightPetBase));
	int a = sizeof(SFightPetBase);

	WORD pos = FindBlankPos(XYD_PT_BASE);
	if (0xffff == pos)
	{
		TalkToDnid(m_ClientIndex,"背包已满，无法取下");
		return;
	}

	AddExistingItem(itemBuffer, pos, true);

	DeleteFightPet(index);

}

BOOL CPlayer::_fpItemTofp( SPackageItem *pItem )
{
	if (!pItem || 0 == pItem->wIndex || 0 == pItem->overlap)
		return FALSE;

	const SItemBaseData *itemfp = CItemService::GetInstance().GetItemBaseData(pItem->wIndex);
	if (!itemfp)
		return FALSE;


	MY_ASSERT(ITEM_IS_FIGHTPET(itemfp->m_Type) && 1 == itemfp->m_Overlay && 1 == pItem->overlap);
	SItemBase *item = pItem;
	SRawItemBuffer *itembuffer = pItem;
	SFightPetItem petbase;
	memcpy(&(petbase.fpInfo),&(itembuffer->buffer),sizeof(SFightPetBase));


	WORD fpID = petbase.fpInfo.m_fpID;
	if(fpID == 0)return FALSE;

	size_t index = 0;
	for (; index < MAX_FIGHTPET_NUM; ++index)
	{
		if (0 == m_Property.m_FightPets[index].m_fpID)
			break;
	}
	if (index == MAX_FIGHTPET_NUM)
		return FALSE;

	if (index >= m_Property.m_FightPetActived )
	{
		TalkToDnid(m_ClientIndex,"你所携带的侠客数量太多了，必须先激活格子");
		return FALSE;
	}
	const SFightPetBaseData *pFPData = CFightPetService::GetInstance().GetFightPetBaseData(fpID);
	if (!pFPData)
		return FALSE;

	///玩家等级必须大于等于侠客的携带等级
	if (m_Property.m_Level<pFPData->m_nLimitLevel)
	{
		TalkToDnid(m_ClientIndex,"你所携带的侠客等级太高了，你无法装备");
		return FALSE;
	}
	memset(&m_Property.m_FightPets[index], 0, sizeof(SFightPetExt));
	memcpy(&m_Property.m_FightPets[index],&(petbase.fpInfo),sizeof(SFightPetBase));

	//检测一下主人的姓名以便清除亲密度

	if (dwt::strcmp(m_Property.m_FightPets[index].m_owername, m_Property.m_Name, CONST_USERNAME))
	{
		m_Property.m_FightPets[index].m_fpRelation = 0;
		dwt::strcpy(m_Property.m_FightPets[index].m_owername, m_Property.m_Name, CONST_USERNAME);
	}

	SAAddFightPet addMsg;
	memcpy(&addMsg.fightpet, &m_Property.m_FightPets[index], sizeof(SFightPetExt));
	g_StoreMessage(m_ClientIndex, &addMsg, sizeof(SAAddFightPet));

	return DelItem(*pItem);
}

void CPlayer::_fpChangeAttType( SQfpchangeatttypeMsg *msg )
{
	if(!m_pFightPet)return;
	BYTE _fpAtkType = m_pFightPet->m_bAttType;
	m_pFightPet->ChangeAttType(msg->atttype);
	if (_fpAtkType != m_pFightPet->m_bAttType)
	{
		SAfpchangeatttypeMsg fpchangeatttypeMsg;
		fpchangeatttypeMsg.atttype = m_pFightPet->m_bAttType;
		g_StoreMessage(m_ClientIndex,&fpchangeatttypeMsg,sizeof(SAfpchangeatttypeMsg));
	}
}

BOOL CPlayer::_fpRecvEquipSkillBookMsg( BYTE index,SPackageItem *pItem )
{
	SFightPetExt *pet = _fpGetFightPetByIndex(index);
	if(!pet)return FALSE;
	if (!_fpIsEquipSkillItem(pItem->wIndex))return FALSE;

	if(pItem->overlap<1)return FALSE;

	//判断当前技能等级是学习还是升级技能
	int skillid = lua_fpGetItemToSkillID(pItem->wIndex);
	if(skillid == 0 )return FALSE;

	//得到当前技能书对应技能等级
	int skilllevel = lua_fpGetItemToSkillLevel(pItem->wIndex);
	//得到技能孔
	int lastskillSlot= _fpGetSkillSlot(skillid,index);
	if (skilllevel>1)
	{
		if(lastskillSlot<0)
		{
			TalkToDnid(m_ClientIndex,"你还没有学会上一个技能哈！");
			return FALSE;
		}
		//得到当前已经学习的技能等级
		int currentskilllevel = pet->m_fpSkill[lastskillSlot].byLevel;
		if (skilllevel<=currentskilllevel)
		{
			TalkToDnid(m_ClientIndex,"你已经学会了技能！");
			return FALSE;
		}

		if (skilllevel>currentskilllevel+1)
		{
			TalkToDnid(m_ClientIndex,"你必须先学会上一个等级技能！");
			return FALSE;
		}
	}
	else
	{//是否已经学习过
		if(lastskillSlot>=0)
		{
			TalkToDnid(m_ClientIndex,"你的侠客已经学习了这个技能！");
			return false;
		}
		//技能栏是否已经满
		int skillnum = _fpGetSkillNum(index);
		if (pet->m_fpActivedSkill<skillnum+1)
		{
			TalkToDnid(m_ClientIndex,"侠客技能栏已满，不能学习新的技能！");
			return false;
		}
	}

	//至此学习技能判断完毕，应该可以学习了
	//如果当前正在学习，返回正在学习的技能书。装备新技能
	if(pet->m_fpLearnSkill>0)
	{
		if(pet->m_fpLearnSkill==pItem->wIndex)
		{
			TalkToDnid(m_ClientIndex,"你正在学习这个技能！");
			return FALSE;
		}

		std::list<SAddItemInfo> itemList;
		itemList.push_back(SAddItemInfo(pet->m_fpLearnSkill, 1));
		if (!CanAddItems(itemList))
		{
			TalkToDnid(m_ClientIndex, "背包已满！无法拾取！");
			return FALSE;
		}
		StartAddItems(itemList);

		//让客户端卸载装备的技能书
		SAfpSkillUpdate askillmeg;
		askillmeg.byWhat = SAfpSkillUpdate::SSU_UNEQUIPSKILL;
		askillmeg.itemID = 0;
		askillmeg.index = index;
		g_StoreMessage(m_ClientIndex,&askillmeg,sizeof(SAfpSkillUpdate));

		//清空当前学习的技能
		m_Property.m_FightPets[index].m_fpLearnSkill = 0;
	}


	//开始学习技能
	DWORD itemid = pItem->wIndex;
	m_Property.m_FightPets[index].m_fpLearnSkill = pItem->wIndex;

	if(_fpIsInfightPet(index))m_nfpLearnSkillTime = timeGetTime(); //如果是出战的侠客,重新计算时间

	if(pItem->overlap>1)ChangeOverlap(pItem, 1, false);
	else DelItem(*pItem, "侠客装备技能书道具删除！");


	SAfpSkillUpdate aSkillUpmsg;
	aSkillUpmsg.byWhat = SAfpSkillUpdate::SSU_EQUIPSKILL;
	aSkillUpmsg.index = index;
	aSkillUpmsg.itemID = itemid;
	g_StoreMessage(m_ClientIndex,&aSkillUpmsg,sizeof(SAfpSkillUpdate));
	return TRUE;
}


bool CPlayer::tizhiUpgrade( DWORD itemId )
{
	SATiZhiTupoMsg msg;
	msg.byState = 0;
	if (m_Property.byTiZhiLevel>=SPlayerTiZhi::MaxLevel)		//已经达到最大等级，不能再升了
	{
		msg.byState = SATiZhiTupoMsg::MAXLEVEL;
		g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
		return false;
	}
	const CTiZhiService::TiZhiData &tzData = CTiZhiService::GetInstance().getTiZhiData(m_Property.byTiZhiLevel);
	if (m_Property.m_Level < tzData.dwNeedPlayerLevel)		//玩家等级不够
	{
		msg.byState = SATiZhiTupoMsg::NOTLEVEL;
		g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
		return false;
	}
	if (m_Property.m_CurSp < tzData.dwSpConsume)	//真气不足
	{
		msg.byState = SATiZhiTupoMsg::NOTSP;
		g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
		return false;
	}
	if (itemId)
	{
		if(itemId != tzData.dwUpgradeItem)		//错误的道具
		{
			msg.byState = SATiZhiTupoMsg::NOITEM;
			g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
			return false;
		}
	}
	else
	{
		int index = -1;
		for(int i = 0; i < PackageAllCells;i++)
		{
			if (m_PackageItems[i].m_Item.wIndex == tzData.dwUpgradeItem)
			{
				index = i;
				break;
			}
		}
		if (index != -1)
		{
			if (m_PackageItems[index].m_Item.overlap > 1)
			{
				m_PackageItems[index].m_Item.overlap--;
				SendItemSynMsg(&m_PackageItems[index].m_Item);
			}
			else
			{
				DelItem(m_PackageItems[index].m_Item,"体质突破");
			}	
			if (CRandom::RandRange(0,10000) > tzData.dwUpgradeRate)		//有一定几率失败
			{
				msg.byState = SATiZhiTupoMsg::FAILED;
				g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
				return false;
			}
			m_Property.m_CurSp -= tzData.dwSpConsume;
			m_PlayerPropertyStatus[XA_CUR_SP-XA_MAX_EXP] = true;		
			m_Property.byTiZhiLevel++;
			msg.byState = SATiZhiTupoMsg::SUCCESS;
			g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
			return true;
		}
		msg.byState = SATiZhiTupoMsg::NOITEM;
		g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
		return false;
	}
}

void CPlayer::SendTiZhiData()
{
	SATiZhiUpdateMsg tizhiMsg;
	tizhiMsg.tizhiData = (SPlayerTiZhi)m_Property;
	g_StoreMessage(m_ClientIndex,&tizhiMsg,sizeof(tizhiMsg));
}

void CPlayer::OnRecvTiZhiUpdateMsg(SQTiZhiUpdateMsg *pMsg)
{
	SendTiZhiData();
}

void CPlayer::OnRecvTizhiTupoMsg( SQTiZhiTupoMsg *pMsg )
{
	//_UseItem()
	if (tizhiUpgrade(0))
	{
		SendTiZhiData();
	}

}

bool CPlayer::_fpIsInfightPet( BYTE index )
{
	return m_pFightPet&&m_pFightPet->m_index==index ;
}

void CPlayer::_fpUpdateProperties( BYTE type,WORD propertyvalue ,BYTE index)
{
	SAfpfightPropertyChange fppropertychange;
	fppropertychange.m_index = index;
	fppropertychange.m_type = type;
	fppropertychange.value = propertyvalue;
	g_StoreMessage(m_ClientIndex,&fppropertychange,sizeof(SAfpfightPropertyChange));
}

void CPlayer::_fpUpdateProperties()
{
	return;
}

int CPlayer::_fpGetSkillNum( BYTE index )
{
	SFightPetExt *pet = _fpGetFightPetByIndex(index);
	if(!pet)return -1;
	int num = 0;
	for (int i = 0; i < SFightPetBase::FP_SKILL_MAX; i++)
	{
		if (pet->m_fpSkill[i].wTypeID && pet->m_fpSkill[i].byLevel > 0)
			num++;
	}
	return num;
}

bool CPlayer::activeKylinArm()
{
	m_Property.byKylinArmLevel = 1;
	m_Property.wYuanqi = 0;
	return true;
}

bool CPlayer::kylinArmUpgrade()
{
	const CKylinArmService::KylinArmData &data = CKylinArmService::GetInstance().getKylinArmData(m_Property.byKylinArmLevel);
	SAKylinArmUpgradeMsg msg;
	if (m_Property.wYuanqi < data.wYuanqiMax)		//元气
	{
		msg.byState = SAKylinArmUpgradeMsg::NOTYUANQI;
		g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
		return false;
	}
	if (m_Property.m_Level < data.wNeedLevel)		//玩家等级
	{
		msg.byState = SAKylinArmUpgradeMsg::NOTLEVEL;
		g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
		return false;
	}
	if (m_Property.m_Money < data.wNeedMoney)		//金钱
	{
		msg.byState = SAKylinArmUpgradeMsg::NOTMONEY;
		g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
		return false;
	}
	if (m_Property.m_CurSp < data.wNeedSP)
	{
		msg.byState = SAKylinArmUpgradeMsg::NOTSP;
		g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
		return false;
	}
	bool ret = false;
	for(int i = 0; i < PackageAllCells;i++)
	{
		int num = 0;
		if (m_PackageItems[i].m_Item.wIndex == data.wNeedItemID)
		{
			num += m_PackageItems[i].m_Item.overlap;
			if (num >= data.wNeedItemNum)
			{
				ret = true;
				break;
			}

		}
	}
	if (!ret)
	{
		msg.byState = SAKylinArmUpgradeMsg::NOTITEM;
		g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
		return false;
	}
	m_Property.m_Money -= data.wNeedMoney;
	m_PlayerPropertyStatus[XA_UNBIND_MONEY-XA_MAX_EXP] = true;
	m_Property.m_CurSp -= data.wNeedSP;
	m_PlayerPropertyStatus[XA_CUR_SP-XA_MAX_EXP] = true;
	int num = data.wNeedItemNum;
	for(int i = 0; i < PackageAllCells;i++)
	{		
		if (m_PackageItems[i].m_Item.wIndex == data.wNeedItemID)
		{
			if (num < m_PackageItems[i].m_Item.overlap)
			{
				m_PackageItems[i].m_Item.overlap -= num;
				num = 0;
				break;
			}
			else if(num == m_PackageItems[i].m_Item.overlap)
			{
				DelItem(m_PackageItems[i].m_Item);
				num = 0;
				break;
			}
			else
			{
				num -= m_PackageItems[i].m_Item.overlap;
				DelItem(m_PackageItems[i].m_Item);
			}
		}
	}
	if (CRandom::RandRange(0,10000) > data.wUpgradeRate)
	{
		msg.byState = SAKylinArmUpgradeMsg::FAILED;
		g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
		return false;
	}
	m_Property.byKylinArmLevel++;
	m_Property.wYuanqi = 0;
	msg.byState = SAKylinArmUpgradeMsg::SUCCESS;
	g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
	SendKylinArmData();
	return true;
}

void CPlayer::SendKylinArmData()
{
	SAKylinArmUpdateMsg msg;
	msg.sData = (SPlayerKylinArm)m_Property;
	g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
}

void CPlayer::OnAttack( CFightObject *pFighter )
{

}

void CPlayer::addPlayerYuanqi( WORD value )
{
	const CKylinArmService::KylinArmData &data = CKylinArmService::GetInstance().getKylinArmData(m_Property.byKylinArmLevel);
	m_Property.wYuanqi += value;
	if (m_Property.wYuanqi > data.wYuanqiMax)
	{
		m_Property.wYuanqi = data.wYuanqiMax;
		if (m_Property.byKylinArmLevel < SPlayerKylinArm::MaxLevel)
		{
			SAKylinArmUpgradeMsg msg;
			msg.byState = SAKylinArmUpgradeMsg::NOTICE;
			g_StoreMessage(m_ClientIndex,&msg,sizeof(msg));
		}
	}
	SendKylinArmData();
}
void CPlayer::OnTimeLimitTask()
{
	for (CRoleTaskManager::RTLIST::iterator it = m_TaskManager.m_taskList.begin(); it != m_TaskManager.m_taskList.end(); ++it)
	{
		if (it->m_timelimit != 0xffff)
		{
			QWORD nowtime = timeGetTime();
			if (nowtime - it->m_CreateTime > it->m_timelimit * 1000)
			{
				WORD taskID		 =  it->m_Index;	// 任务ID
				WORD taskStatus  = TS_FAIL;	// 任务状态
				m_TaskInfo.push_back(make_pair(taskID, taskStatus));
				it->m_timelimit = 0xffffffff;

				CRoleTask *roletask = m_TaskManager.GetRoleTask(taskID);
				if(!roletask)return;

				for (CRoleTask::TELIST::iterator ittask = roletask->m_flagList.begin(); ittask != roletask->m_flagList.end(); ++ittask)
				{
					if ( ittask->m_Complete == 1 ||  ittask->m_Complete == 2 )
					{

						ittask->m_Complete = 0;

						// 发送更新旗标的消息
						SendUpdateTaskFlagMsg(*ittask, true);
						break;
					}
				}	
			}
		}
	}
}

void CPlayer::SendPlayerPkValue()
{
	SAPkvalueChangeMsg spvaluemsg;
	spvaluemsg.nPkvalue = m_wPKValue;
	g_StoreMessage(m_ClientIndex,&spvaluemsg,sizeof(SAPkvalueChangeMsg));
	char colorstr[128];
	sprintf(colorstr,"同步玩家杀孽值为%d",m_wPKValue);
	TalkToDnid(m_ClientIndex,colorstr);

}

void CPlayer::SendPlayerNameColor( )
{
	//当玩家没有红名时才计算称号颜色
	if(m_bNameColor == SAPNameColorChangeMsg::COLOR_NORMAL)
		OnCheckNameColorByVen();

	MY_ASSERT(m_bNameColor<=SAPNameColorChangeMsg::COLOR_RED);
	SAPNameColorChangeMsg snamemsg;
	snamemsg.bNameColor = m_bNameColor;
	snamemsg.m_gid = GetGID();

	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea && DynamicCast(IID_PLAYER))
		pArea->SendAdj(&snamemsg, sizeof(SAPNameColorChangeMsg), -1);
	/*
	char colorstr[128];
	switch (m_bNameColor)
	{
	case SAPNameColorChangeMsg::COLOR_NORMAL:
		sprintf(colorstr,"同步玩家名称颜色为%s色","正常");
		break;
	case SAPNameColorChangeMsg::COLOR_YELLOW:
		sprintf(colorstr,"同步玩家名称颜色为%s色","黄色");
		break;
	case SAPNameColorChangeMsg::COLOR_LIGHTRED:
		sprintf(colorstr,"同步玩家名称颜色为%s色","浅红");
		break;
	case SAPNameColorChangeMsg::COLOR_RED:
		sprintf(colorstr,"同步玩家名称颜色为%s色","深红");
		break;
	default:
		break;
	}
	*/
//	TalkToDnid(m_ClientIndex,colorstr);
}



BYTE CPlayer::GetPlayerNameColor()
{
	if(m_wPKValue < 1){
		return SAPNameColorChangeMsg::COLOR_NORMAL;
	}
	else if (m_wPKValue>1 && m_wPKValue< 299){
		return SAPNameColorChangeMsg::COLOR_LIGHTRED;
	}

	return  SAPNameColorChangeMsg::COLOR_RED;
}

DWORD CPlayer::CalculatePkValueDecrease()
{
	return m_wPKValue == 0 ? 0 : (m_wPKValue +100)/100 * 2 *1000;
}

void CPlayer::CheckPkPlayerName( CFightObject *fightobj )
{
	if(!fightobj)return;
	CPlayer *player = GetFightObjToPlayer(fightobj);
	if(!player)return;

	if(m_wPKValue > 0)return; //大于0时应该是红名玩家了，不在这里计算
	if(player->m_wPKValue )return; // 如果大于0，目标应该是红名玩家了，不计算黄名

	m_dChangeColorTime = timeGetTime();	//这里应该是黄名玩家的计算
	BYTE NameColor = SAPNameColorChangeMsg::COLOR_YELLOW;
	if (NameColor != m_bNameColor)
	{
		m_bNameColor = NameColor;
		SendPlayerNameColor();
	}
}

bool CPlayer::OnCheckNameColorByVen()
{
	bool changed = (m_bNameColor !=  (m_iPassVenationCount / 2));
	m_bNameColor = m_iPassVenationCount / 2;

	return changed;
}


void CPlayer::OnCheckNameColor()
{
	if (m_wPKValue == 0)
	{
		//这里应该是黄名
		if (m_bNameColor == SAPNameColorChangeMsg::COLOR_YELLOW)
		{
			if (timeGetTime() - m_dChangeColorTime > 30 * 1000)
			{
				m_bNameColor = SAPNameColorChangeMsg::COLOR_NORMAL;
				SendPlayerNameColor();
				m_dChangeColorTime = 0xffffffff;
			}
		}
		return;
	}

	//这里应该是红名
	if (m_wPKValue)
	{
		if (timeGetTime() - m_dChangeColorTime > CalculatePkValueDecrease())
		{
			m_wPKValue -- ;
			m_dChangeColorTime = timeGetTime();
			BYTE NameColor = GetPlayerNameColor();
			if (NameColor != m_bNameColor)
			{
				m_bNameColor = NameColor;
				SendPlayerNameColor();
			}
			SendPlayerPkValue();
		}
	}


}

void CPlayer::ShowChangeProperty(BYTE type, WORD value)
{
	// 当没有任何属性的时候
	if (0xff == type)
		return;

	MY_ASSERT(type >= SEquipDataEx::EEA_MAXHP && type < SEquipDataEx::EEA_MAX);

	switch(type)
	{
	case SEquipDataEx::EEA_MAXHP:
		TalkToDnid(m_ClientIndex, FormatString("生命上限: %d", value));
		break;

	case SEquipDataEx::EEA_MAXMP:
		TalkToDnid(m_ClientIndex, FormatString("内力上限: %d", value));
		break;

	case SEquipDataEx::EEA_MAXTP:
		TalkToDnid(m_ClientIndex, FormatString("体力上限: %d", value));
		break;

	case SEquipDataEx::EEA_GONGJI:
		TalkToDnid(m_ClientIndex, FormatString("攻击: %d", value));
		break;

	case SEquipDataEx::EEA_FANGYU:
		TalkToDnid(m_ClientIndex, FormatString("防御: %d", value));
		break;

	case SEquipDataEx::EEA_BAOJI:
		TalkToDnid(m_ClientIndex, FormatString("暴击: %d", value));
		break;

	case SEquipDataEx::EEA_SHANBI:
		TalkToDnid(m_ClientIndex, FormatString("闪避: %d", value));
		break;

	case SEquipDataEx::EEA_SPEED:
		TalkToDnid(m_ClientIndex, FormatString("移动速度: %d", value));
		break;

	case SEquipDataEx::EEA_ATKSPEED:
		TalkToDnid(m_ClientIndex, FormatString("攻击速度: %d", value));
		break;

	case SEquipDataEx::EEA_REDUCEDAMAGE:
		TalkToDnid(m_ClientIndex, FormatString("伤害减免: %d", value));
		break;

	case SEquipDataEx::EEA_ABSDAMAGE:
		TalkToDnid(m_ClientIndex, FormatString("绝对伤害: %d", value));
		break;

	case SEquipDataEx::EEA_NONFANGYU:
		TalkToDnid(m_ClientIndex, FormatString("无视防御: %d", value));
		break;

	case SEquipDataEx::EEA_MULTIBAOJI:
		TalkToDnid(m_ClientIndex, FormatString("多倍暴击: %d", value));
		break;

	default:
		return;
	}

	return;
}

void CPlayer::OnDropItem()
{
	//杀孽值100以上才掉落第
	if (m_wPKValue < 100)return;
	BYTE dropValue = m_wPKValue/100 > 50 ? 50 : m_wPKValue/100;
	BYTE dropMountRate = CRandom::SimpleRandom()%100;	//掉落装备机率
	if(dropValue < dropMountRate) return;

	WORD dropItemRate =   CRandom::SimpleRandom()%100;	//掉落装备机率表
	const PlayerDropTable *PDTable = CItemDropService::GetInstance().GetPlayerDrop();
	if (!PDTable)return;

	if(!IsMoneyEnough(XYD_UM_ONLYUNBIND,20)&&GetAllItemNum()==0&&GetEquipmentNum()==0)
	{
		return;
	}

	CItem::SParameter args;
	std::vector<SRawItemBuffer> item;

	BYTE dropstate = PlayerDropTable::STATE_DROPITEM_BODY;

	if (dropItemRate <PDTable->m_wDropItem_body){
		dropstate =  PlayerDropTable::STATE_DROPITEM_BODY;
	}else if(dropItemRate <PDTable->m_wDropItem_bag){
		dropstate =  PlayerDropTable::STATE_DROPITEM_BAG;
	}else{
		dropstate =  PlayerDropTable::STATE_DROPITEM_MONEY;
	}

	TalkToDnid(m_ClientIndex,FormatString("装备掉落几率%d",dropItemRate));
	switch (dropstate)
	{
	case PlayerDropTable::STATE_DROPITEM_BODY:
		{
			if(GetEquipmentNum() == 0)
			{
				if(GetAllItemNum()){
					dropstate = PlayerDropTable::STATE_DROPITEM_BAG;
				}
				else{
					dropstate = PlayerDropTable::STATE_DROPITEM_MONEY;
				}
			}
		}
		break;
	case PlayerDropTable::STATE_DROPITEM_BAG:
		{
			if(GetAllItemNum() == 0)
			{
				if(GetEquipmentNum()){
					dropstate = PlayerDropTable::STATE_DROPITEM_BODY;
				}
				else{
					dropstate = PlayerDropTable::STATE_DROPITEM_MONEY;
				}
			}
		}
		break;
	case PlayerDropTable::STATE_DROPITEM_MONEY:
		{
			if (!IsMoneyEnough(XYD_UM_ONLYUNBIND,20))
			{
				if (GetAllItemNum()){
					dropstate = PlayerDropTable::STATE_DROPITEM_BAG;
				}
				else{
					dropstate = PlayerDropTable::STATE_DROPITEM_BODY;
				}
			}
		}
		break;
	default:
		break;
	}

	if (dropstate < PlayerDropTable::STATE_DROPITEM_MONEY){
		RandomDropItemToGroundBag(item,dropstate);
	}else
	{
		args.Money = m_Property.m_Money * 5 / 100;
		if (args.Money)
		{
			CheckPlayerMoney(XYD_UM_ONLYUNBIND,args.Money,false);
			args.MoneyType = XYD_UM_ONLYUNBIND;
		}
	}

	if (item.empty() && 0 == args.Money)	// 没东西可掉
		return ;

	args.PackageModel	= IPM_BUDAI;
	args.dwLife			= MakeLifeTime(5);
	args.xTile			= m_curTileX;
	args.yTile			= m_curTileY;
	args.ItemsInPack	= item.size();
	args.ProtectedGID=GetGID();
	args.ProtectTeamID	= 0; //这里忽略队伍ID，要不然被队友杀了就能直接拣装备了

	if (!CItem::GenerateNewGroundItem(m_ParentRegion, 3, args, item, LogInfo(0, "玩家死亡掉落")))
		return ; 

}

int CPlayer::GetRandomEquipmentIndex()
{
	std::vector<int> EquipIndex;
	for (int i = 0; i < EQUIP_P_MAX; ++i)
	{
		if (0 == m_Property.m_Equip[i].wIndex)
			continue;
		EquipIndex.push_back(i);
	}
	if (EquipIndex.size() == 0)return -1;
	BYTE randindex = CRandom::RandRange(0,EquipIndex.size()-1);
	return EquipIndex[randindex];
}



void CPlayer::OnChangePKValue( int pvalue )
{
	if(pvalue == 0 )return;
	int temp = m_wPKValue;
	temp = temp + pvalue > 0 ? temp + pvalue: 0;
	if (temp != m_wPKValue)
	{
		m_wPKValue = temp;
		SendPlayerPkValue();
		BYTE NameColor = GetPlayerNameColor();
		if (NameColor != m_bNameColor)
		{
			m_bNameColor = NameColor;
			SendPlayerNameColor();
		}
	}

}

int CPlayer::GetEquipmentNum()
{
	int num = 0;
	for (int i = 0; i < EQUIP_P_MAX; ++i)
	{
		if (m_Property.m_Equip[i].wIndex)
			num++;
	}
	return num;
}

int CPlayer::RandomDropItemToGroundBag( std::vector<SRawItemBuffer> &itembag,BYTE dropType )
{
	if (dropType == 0)
	{
		int dropcrang = CRandom::SimpleRandom()%100;
		int dropcount = 1;			//掉落一件装备
		if (dropcrang<35)dropcount++;	//如果在35以下，掉落2件装备
		while(dropcount--)
		{
			int EquipmentIndex = GetRandomEquipmentIndex();
			if(EquipmentIndex == -1)break;
			DWORD wIndex = m_Property.m_Equip[EquipmentIndex].wIndex;
			MY_ASSERT(wIndex);
			SRawItemBuffer temp;
			memcpy(&temp, &m_Property.m_Equip[EquipmentIndex], sizeof(SRawItemBuffer));
			itembag.push_back(temp);
			ToPackage(EquipmentIndex,TRUE);
		}
	}
	else if (dropType == 1)
	{
		int dropcrang = CRandom::SimpleRandom()%100;
		int dropcount = 1;			//掉落一件装备
		if (dropcrang<35)dropcount=4;	//如果在35以下，掉落4件装备

		while(dropcount--)
		{
			int itempos= GetRadomItemPos();
			if(itempos == -1)break;

			TalkToDnid(m_ClientIndex,FormatString("当前背包中道具数量%d",GetAllItemNum()));
			SPackageItem *item = FindItemByPos(itempos,XYD_FT_ONLYUNLOCK);
			if (!item)break;

			SRawItemBuffer itemBuffer;
			memcpy(&itemBuffer,&(item->wIndex),sizeof(SRawItemBuffer));

			itembag.push_back(itemBuffer);

			CheckGoods(itemBuffer.wIndex,itemBuffer.overlap,FALSE);
		}
	}
	return itembag.size();
}

void CPlayer::OnRecvKylinArmUpgradeMsg( SQKylinArmUpgradeMsg* pMsg )
{
	kylinArmUpgrade();
}

void CPlayer::DeleteCycData(WORD taskID,BYTE flag)
{//SPlayerCycTask
	if (flag == 1)//
	{
		m_Property.ClearCompleteMark(taskID);
	}
	else
	{
		//
		//m_Property.ClearCompleteMark(taskID);
		lite::Variant ret;
		std::string str;
		LuaFunctor(g_Script,"GetClearCycTaskData")[taskID](&ret);
		if (lite::Variant::VT_STRING == ret.dataType)
		{
			str = ret;
			std::vector<long> vec;
			ParaseTOstring(str,vec);
			long len = vec.size();
			for (long i = 0; i < len; ++i)
			{
				m_Property.ClearCompleteMark(vec[i]);
			}

		}
		else
		{
			rfalse("没有取到数据!!taskID: %ld,DeleteCycData",taskID);
		}

	}
}

void CPlayer::UpDataCycData()
{
	std::vector<long> indexVec;
	for (long i = 0; i < 20; ++i)
	{
		WORD	m_TaskID = m_Property.m_PlayerCyc[i].m_TaskID;				//任务ID，最后一个完成的日常任务索引ID
		std::string m_CompleteStrTime = m_Property.m_PlayerCyc[i].m_CompleteTime;			//交最后一个任务完成时间
		BYTE	m_type = m_Property.m_PlayerCyc[i].m_type;//任务类型1,2taskType = 1(单个任务)  taskType = 2(多个任务)
		BYTE    m_cyctype = m_Property.m_PlayerCyc[i].m_cyctype;//任务时间类型cyctype  = 1(日常任务),	2(跑环(即时)任务),	3(周常任务)

		if (m_cyctype == 1)//
		{
			bool flag = IsCompleteDayTime(m_CompleteStrTime);//判定日常
			if (false == flag)
			{
				continue;
			}
			indexVec.push_back(i);
		}
		else if (m_cyctype == 3)
		{
			bool flag = IsCompleteDayTime(m_CompleteStrTime);//判定周常
			if (false == flag)
			{
				continue;
			}
			indexVec.push_back(i);
		}
	}
	if (indexVec.size() != 0)
	{
		ClearCycData(indexVec);
	}
}

void CPlayer::ClearCycData(std::vector<long> indexVec,BYTE flag)
{
	std::string DataStream;
	for (int i = 0; i < indexVec.size(); ++i)
	{
		long index = indexVec[i];
		WORD	m_TaskID = m_Property.m_PlayerCyc[index].m_TaskID;
		lite::Variant ret;
		std::string str;
		LuaFunctor(g_Script,"GetClearCycTaskData")[m_TaskID](&ret);
		if (lite::Variant::VT_STRING == ret.dataType)
		{
			str = ret;
			if (str == "faile")//说明只有一个任务
			{
				m_Property.ClearCompleteMark(m_TaskID);//清空数据
				char ka[20];
				itoa(m_TaskID,ka,10);
				DataStream.append(ka);
				if (flag == 0)//说明是日常任务
				{
					RecvRdData(m_TaskID);
				}
				//发送数据
			}
			else
			{
				std::vector<long> vec;
				ParaseTOstring(str,vec);
				long len = vec.size();//任务链
				for (long i = 0; i < len; ++i)
				{
					m_Property.ClearCompleteMark(vec[i]);//清空数据
					char ka[20];
					itoa(vec[i],ka,10);
					DataStream.append(ka);
					DataStream.append(1,':');
					if (flag == 0)//说明是日常任务
					{

					}
					//发送数据
				}
				if (vec.size() > 0)
				{
					WORD id = vec[0];
					RecvRdData(id);
				}
			}
			m_Property.m_PlayerCyc[index] = SPlayerCycTask();//清空数据
		}
		else
		{
			rfalse("没有取到数据!!,ClearCycData");
		}
	}
	//if (m_type == 1 )//单个
	//{

	//}
	//else if (m_type == 2)//多个
	//{
	//	lite::Variant ret;
	//	std::string str;
	//	LuaFunctor(g_Script,"GetClearCycTaskData")[m_TaskID](&ret);
	//	std::vector<long> vec;
	//	if (lite::Variant::VT_STRING == ret.dataType)
	//	{
	//		ParaseTOstring(str,vec);
	//	}
	//}
	//else
	//{
	//	rfalse("没有取到数据!!taskIndex: %ld,UpDataCycData",i);
	//}
}

void CPlayer::SendCycData(WORD taskID)
{

}
void CPlayer::InsertPlayerRdTaskInfo(WORD NpcID,WORD taskID)
{
	long k = m_Property.m_SPlayerLimtTime.m_RdNum;
	if (k <= 20 && k >= 1)
	{
		long index = 0;
		bool Flag = m_Property.m_SPlayerLimtTime.IsExitNpcIDWithIndex(NpcID,index);	
		if (Flag )//说明这个任务是存在的可以更新数据
		{
			//m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[index].m_RdTaskNum++;
			m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[index].m_TaskID = taskID;
		} 
		else
		{
			if (k == 80)
			{
				return;
			}
			m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[k].m_NpcID = NpcID;
			m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[k].m_TaskID = taskID;
			//m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[k].m_RdTaskNum++;
			m_Property.m_SPlayerLimtTime.m_RdNum++;
		}
	}
	else if (k == 0)
	{
		m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[k].m_NpcID = NpcID;
		m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[k].m_TaskID = taskID;
		//m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[k].m_RdTaskNum++;
		m_Property.m_SPlayerLimtTime.m_RdNum++;
	}
	else
	{
		rfalse("你的任务数据异常大于20: %d",k);
	}
}
void CPlayer::UpDataPlayerRdTaskInfo()
{//暂时没有用
	if ( m_Property.m_SPlayerLimtTime.flag == 0 )
		return;
	bool Flag = IsCompleteDayTime(m_Property.m_SPlayerLimtTime.m_CompleteTime);
	if (Flag)
	{
		m_Property.m_SPlayerLimtTime.UpdataLimtTimeData();
		std::string currtimestr;
		GetSysTimeTick(currtimestr);
		memcpy(m_Property.m_SPlayerLimtTime.m_CompleteTime,currtimestr.c_str(),currtimestr.size());
	}
}
WORD CPlayer::GetRdTasksNum(WORD NpcID)
{
	for (long i = 0; i < m_Property.m_SPlayerLimtTime.m_RdNum; ++i)
	{
		if (NpcID == m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[i].m_NpcID)
		{
			return m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[i].m_RdTaskNum; 
		}
	}
	return 0;
}
void CPlayer::RecvRdData(WORD taskID)
{
	lite::Variant ret;
	int npcid = 0;
	LuaFunctor(g_Script,"GetNpcIDByTaskID")[taskID](&ret);
	if (lite::Variant::VT_INTEGER == ret.dataType)
	{
		npcid = ret;
		for (long i = 0; i <  m_Property.m_SPlayerLimtTime.m_RdNum; ++i )
		{
			WORD id = m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[i].m_NpcID;
			if (id == npcid)
			{
				if (m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[i].m_RdTaskNum <= 0)
				{
					return;
				}
				m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[i].m_RdTaskNum--;
				return;
			}
		}
	}
	else
	{
		rfalse("NPCTABLE,没有找到数据");
	}
}
WORD CPlayer::GetCurrentNpcTaskID(WORD NpcID)
{
	for (long i = 0; i < m_Property.m_SPlayerLimtTime.m_RdNum; ++i)
	{
		if (NpcID == m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[i].m_NpcID)
		{
			return m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[i].m_TaskID; 
		}
	}
	return 0;
}
WORD CPlayer::GetRdNpcNum()
{
	return m_Property.m_SPlayerLimtTime.m_RdNum;
}
void CPlayer::UpDataPlayerXKL()
{
	g_Script.SetCondition(NULL,this,NULL,NULL);
	g_Script.CallFunc("UpDataPlayerXKL", m_Property.m_Level);
	g_Script.CleanCondition();
}
long CPlayer::OnXKLProcess()
{
	//char  GKLtime[30];  //记录时间
	//short m_GKLUseNum;	//次数
	//BYTE  m_GKLFlag;
	if (m_Property.m_GKLFlag == 0)
	{
		m_Property.m_GKLFlag = 1;
		m_Property.m_GKLUseNum = 1;
		std::string currtimestr;
		GetSysTimeTick(currtimestr);
		memcpy(m_Property.GKLtime,currtimestr.c_str(),currtimestr.size());
		return 0;
	} 
	else
	{
		long tk = GetSpaceDays(m_Property.GKLtime);//相差的天数
		return tk;
		//if (tk >= 1)
		//{
		//	m_Property.AddUseNum(1);
		//	std::string currtimestr;
		//	GetSysTimeTick(currtimestr);
		//	memset(m_Property.GKLtime,0,30);
		//	memcpy(m_Property.GKLtime,currtimestr.c_str(),currtimestr.size());
		//}
		//else if (tk >= 2)
		//{
		//	m_Property.AddUseNum(2);
		//	std::string currtimestr;
		//	GetSysTimeTick(currtimestr);
		//	memset(m_Property.GKLtime,0,30);
		//	memcpy(m_Property.GKLtime,currtimestr.c_str(),currtimestr.size());
		//}
		//else if (tk >= 0)//
		//{
		//	//int t = 0;//不做任何事情
		//	rfalse("不做任何事情1");
		//}
		//else
		//{
		//	rfalse("不做任何事情2");
		//}
	}
	return 0;
}
void CPlayer::AddXKLUseNum(short num)
{
	m_Property.AddUseNum(num);
}
short CPlayer::GetXKLUseNum()
{
	return m_Property.m_GKLUseNum;
}
void CPlayer::OnHandleCycTask(WORD npcID)
{
	for (long i = 0; i < m_Property.m_SPlayerLimtTime.m_RdNum; ++i)
	{
		WORD id = m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[i].m_NpcID;
		if (npcID == id)
		{
			m_Property.m_SPlayerLimtTime.m_SPlayerRdTaskInfo[i].m_RdTaskNum++;
			break;
		}
	}
}
WORD CPlayer::GetHeadIDByStr( std::string  headHail)
{
	std::vector<long> vec;
	ParaseTOstring(headHail,vec);
	if (vec.size() > 0)
	{
		return (WORD)vec[0];
	}
	else
	{
		rfalse("HeadHail 填错了");
		return 0;
	}
}
void CPlayer::PrintXKLLOG(WORD num,long ItemID)
{
	m_ServerRelationName;//服务器
	m_szAccount;//账号ID
	std::string NickName = m_Property.m_Name;
	std::string year;
	std::string time;
	GetSysYearMd(year);
	GetSysHourMm(time);
	m_Property.m_Level;
	//num;
	//ItemID;
	char temp[100];
	memset(temp,0,100);
	sprintf(temp,"%s\t%s\t%s\t%s\t%s\t%ld\t%ld\t%ld\t",m_ServerRelationName.c_str(),m_szAccount.c_str(),
		NickName.c_str(),year.c_str(),time.c_str(),m_Property.m_Level,num,ItemID);
	rfalse(5,1,temp);
	/*memset(temp1,0,100);
	sprintf(temp1,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t","aaa","xjz","第一次","2011-12-31","00:29:46","85","4","260001");
	rfalse(5,1,temp1)*/;
}
void  CPlayer::SendTemplateSkill(WORD bySkillPosID,WORD Flag)
{
	SAPlayerTemplateSkillmsg msg;
	msg.Flag = Flag;
	msg.m_num =	m_TemplateSkillInfo.size();
	for (int i = 0; i < m_TemplateSkillInfo.size(); ++i)
	{
		msg.temp[i] = m_TemplateSkillInfo[i];
	}
	g_StoreMessage(m_ClientIndex, &msg, sizeof(SAPlayerTemplateSkillmsg));
}

void CPlayer::UpdatePlayerXwzMsg()
{
	// 从脚本中计算修为值
	CPlayer *player = g_Script.m_pPlayer;
	if (player)
	{
		if (player->GetGID() != this->GetGID()){
			return;
		}
		lite::Variant ret;
		LuaFunctor(g_Script, "GetPlayerXwZ")(&ret);

		if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
			return ;
		if (0 == ( __int32 )ret)
			return;

		SAXwzUpdateMsg xwzmsg;
		xwzmsg.xwzValue = ( __int32 )ret;
		m_XwzValue = ( __int32 )ret;
		g_StoreMessage(m_ClientIndex,&xwzmsg,sizeof(SAXwzUpdateMsg));
		return;
	}

	g_Script.SetPlayer(this);
	lite::Variant ret;
	LuaFunctor(g_Script, "GetPlayerXwZ")(&ret);
	g_Script.CleanPlayer();

	if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
		return ;

	if (0 == ( __int32 )ret)
		return;
	SAXwzUpdateMsg xwzmsg;
	xwzmsg.xwzValue = ( __int32 )ret;
	m_XwzValue = ( __int32 )ret;
	g_StoreMessage(m_ClientIndex,&xwzmsg,sizeof(SAXwzUpdateMsg));
}

void CPlayer::OnRecvQuestUpdateBossMsg( SQBossDeadUpdatemsg* pMsg )
{
	GetGW()->RecvUpdateBossData(pMsg,this);
}

void CPlayer::_L_CheckOnRunTime()
{
	// 	INT64 nowtime = 0;
	// 	_time64(&nowtime);
	// 	if (m_Property.m_bStopTime == 0){
	// 		DWORD onlinetime = nowtime - m_dwOnlineTime;
	// 		m_Property.m_firstLoginTime += onlinetime ;
	// 	}

	//if(m_Property.m_bStopTime == 1)return;
	CPlayer *player = g_Script.m_pPlayer;
	if (player)
	{
		if (player->GetGID() != this->GetGID()){
			return;
		}
		g_Script.CallFunc("CheckOnRunTime",m_Property.m_dayOnlineTime );
		return;
	}
	g_Script.SetPlayer(this);
	g_Script.CallFunc("CheckOnRunTime",m_Property.m_dayOnlineTime );
	g_Script.CleanPlayer();

}

//void CPlayer::_L_StartTime()
//{
//	_time64(&m_dwOnlineTime);
//	m_Property.m_bStopTime = 0;
//	m_Property.m_firstLoginTime = m_dwOnlineTime;
//}
//
//void CPlayer::_L_StopTime()
//{
//	m_Property.m_bStopTime = 1;
//}
//
//void CPlayer::_L_CleanTime()
//{
//	m_dwOnlineTime = m_Property.m_firstLoginTime = 0;
//	//m_dwOnlineTime = 0;
//}
//
//bool CPlayer::_L_IsStopTime()
//{
//	return m_Property.m_bStopTime ;
//}


BYTE CPlayer::_L_GetOnlineGiftState(int index)
{
	assert(index>=0 && index<MAX_ONLINEGIFT_NUM);
	return m_Property.m_onlineGiftStates[index];
}

void CPlayer::_L_UpdateOnlineGiftState(int index, BYTE state)
{
	assert(index>=0 && index<MAX_ONLINEGIFT_NUM);
	m_Property.m_onlineGiftStates[index] = state;
}

int CPlayer::_L_GetCountDownGiftState()
{
	return m_Property.m_dTimeCountDownGiftState +1 ;
}

void CPlayer::_L_UpdateCountDownGiftState()
{
	m_Property.m_dTimeCountDownGiftState ++;
}

INT64 CPlayer::_L_GetOnlineBeginTime()
{
	return m_dwOnlineBeginTime;
}

void CPlayer::OnRecvGetGiftMsg( SQGetOnlienGiftMsg *pMsg )
{
	if (pMsg->PackType == SQGetOnlienGiftMsg::PACK_COUNTDOWNGIFT){
		_L_SafeCallLuaFunction("onRecvGetCountDownGift");
	}
	else if (pMsg->PackType == SQGetOnlienGiftMsg::PACK_ONLINEGIFT){
		g_Script.SetPlayer(this);
		g_Script.CallFunc("onRecvGetOnlineGift",pMsg->index);
		g_Script.CleanPlayer();
	}
	else if (pMsg->PackType == SQGetOnlienGiftMsg::PACK_LEVELGIFT){
		_L_SafeCallLuaFunction("OnrecvLevelGiftList");
	}
	else if (pMsg->PackType == SQGetOnlienGiftMsg::PACK_LOGINGIFT){
		_L_SafeCallLuaFunction("OnrecvLoginGiftList");
	}
}

void CPlayer::_L_SafeCallLuaFunction( const char *funcnam,... )
{
	CPlayer *player = g_Script.m_pPlayer;
	if (player)
	{
		if (player->GetGID() != this->GetGID()){
			return;
		}
		g_Script.CallFunc(funcnam);
		return;
	}
	g_Script.SetPlayer(this);
	g_Script.CallFunc(funcnam);
	g_Script.CleanPlayer();
}

void CPlayer::OnRecvQuestChangeFashion( SQFashionChangeMsg *pmsg )
{
	if (pmsg->mode == m_bFashionMode)return;
	if (pmsg->mode > SQFashionChangeMsg::FASHIONMODE)return;
	m_bFashionMode = pmsg->mode;
	SAFashionChangeMsg msg;
	msg.mode = m_bFashionMode;
	msg.playergid = GetGID();

	//同步给周围玩家
	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea && DynamicCast(IID_PLAYER))
		pArea->SendAdj(&msg, sizeof(SAFashionChangeMsg), -1);

}
void CPlayer::OnClinetReady()
{
	//初始化时调用脚本，发送一些初始化消息
	CScriptState s(0, this, 0, 0);
	LuaFunctor(g_Script, "OnPlayerSendInitMsg")();

	if ( limitedState )
	{
		// ################防沉迷系统################
		char tempStr[256];
#ifdef TimeCompress
		sprintf( tempStr, "累计在线时间[%d分]离线时间[%d分]！", m_OnlineTime, offlineTime );
#else
		sprintf( tempStr, "累计在线时间[%d分]离线时间[%d分]！", m_OnlineTime/60, offlineTime/60 );
#endif
		TalkToDnid( m_ClientIndex, "您已经被纳入防沉迷系统中！" );
		TalkToDnid( m_ClientIndex, tempStr );

		if ( m_OnlineTime >= 60*60*5 )
			showStep = 7;
		else if ( m_OnlineTime >= 60*60*3 )
			showStep = 3;
		else
			showStep = 0;
		// ##########################################
	}

}
INT64 CPlayer::_L_GetNowTime()
{
	INT64 nowtime = 0;
	_time64(&nowtime);
	return nowtime;
}

// 生成套装新属性
BOOL CPlayer::InitEquipSuitAttributes_New()
{
	std::map<WORD, BYTE> mapSuit;
	for (int i = 0; i < EQUIP_P_MAX; ++i)
	{
		if (0 == m_Property.m_Equip[i].wIndex)
		{
			continue ;
		}		
// 		if (0 == m_Property.m_Equip[i].attribute.wSuitEquipID)
// 		{
// 			continue;
// 		}
// 		std::map<WORD, BYTE>::iterator itr = mapSuit.find(m_Property.m_Equip[i].attribute.wSuitEquipID);
// 		if (itr != mapSuit.end())
// 		{
// 			++(itr->second);
// 		}
// 		else 
// 		{
// 			mapSuit.insert(std::make_pair(m_Property.m_Equip[i].attribute.wSuitEquipID, static_cast<BYTE>(1)));
// 		}
	}

	g_Script.SetPlayer(this);
	for (std::map<WORD, BYTE>::const_iterator citr = mapSuit.begin(); citr != mapSuit.end(); ++citr)
	{
		lite::Variant ret;	
		LuaFunctor(g_Script, "CalclulateSuitExtraAttribute") [citr->first] [citr->second] (&ret);
	}
	g_Script.CleanPlayer();		

	return TRUE;
}

// 套装属性加成
BOOL CPlayer::InitEquipSuitAttributes()
{
	return InitEquipSuitAttributes_New();
	// 第一次循环，判断是否是全装备，只判断13件，如果不是，则不再进行下面的计算
	for (size_t i = EQUIP_P_WEAPON; i <= EQUIP_P_RING2; ++i)
	{
		if (!m_Property.m_Equip[i].wIndex)
		{
			m_ModifyTypeMark = 0;
			return FALSE;
		}
	}

	// 到了这里，装备齐全
	WORD LevelValue = 0;
	WORD GradeValue = 0;
	WORD ColorValue = 0;

	// 存储一次原始值，用于以后属性更改判定。
	WORD wTempModifyMark = m_ModifyTypeMark;
	// 这里用bitset完全是为了以后做条件判断，效率有些低，但是比直接用位操作方便。
	bitset<16> modify_bit_suitAttriType(m_ModifyTypeMark);

	// 判定套装属性 各种属性 加成条件 是否满足,目前只用判断13件装备
	for (size_t i = EQUIP_P_WEAPON; i <= EQUIP_P_RING2; ++i)
	{
		const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(m_Property.m_Equip[i].wIndex);
		if (!pData)
		{
			// 如果在这里退出了，完全是配置表的问题。与程序无关...
			m_ModifyTypeMark = 0;
			return FALSE;
		}

		// 第一次循环，把以后进行比较的基础值提取出来
		if (EQUIP_P_WEAPON == i)
		{
			modify_bit_suitAttriType.set(SAT_LEVEL);
			modify_bit_suitAttriType.set(SAT_GRADE);
			modify_bit_suitAttriType.set(SAT_COLOR);

			LevelValue = pData->m_Level;
			GradeValue = m_Property.m_Equip[i].attribute.grade;
			ColorValue = pData->m_Color;

			// 星星全满
// 			if (SEquipment::SLOT_IS_EMPTY != m_Property.m_Equip[i].attribute.starNum[SEquipment::MAX_STAR_NUM - 1])
// 				modify_bit_suitAttriType.set(SAT_STAR);
// 			else
// 				modify_bit_suitAttriType.reset(SAT_STAR);

			// 宝石全满，并且还要10级
			if (m_Property.m_Equip[i].slots[SEquipment::MAX_SLOTS - 1].isJewel())
			{
				BYTE bType = m_Property.m_Equip[i].slots[SEquipment::MAX_SLOTS - 1].type;
				BYTE bValue = m_Property.m_Equip[i].slots[SEquipment::MAX_SLOTS - 1].value;

				for (size_t i = 0; i < SEquipment::MAX_SLOTS; ++i)
				{
					// 10级以下
					if (((4004001 + (bType - 1) * 100 + (bValue - 1)) % 100) < 10)
					{
						modify_bit_suitAttriType.reset(SAT_JEWEL);
						break;
					}

					if (SEquipment::MAX_SLOTS - 1 == i)
						modify_bit_suitAttriType.set(SAT_JEWEL);
				}
			}
			else
				modify_bit_suitAttriType.reset(SAT_JEWEL);

			continue;
		}

		// 第二次循环以后，根据基础值进行比较是否满足套装加成条件
		if (modify_bit_suitAttriType[SAT_LEVEL] && LevelValue != pData->m_Level)
			modify_bit_suitAttriType.reset(SAT_LEVEL);

		if (modify_bit_suitAttriType[SAT_GRADE] && GradeValue != m_Property.m_Equip[i].attribute.grade)
			modify_bit_suitAttriType.reset(SAT_GRADE);

		if (modify_bit_suitAttriType[SAT_COLOR] && ColorValue != pData->m_Color)
			modify_bit_suitAttriType.reset(SAT_COLOR);

//		if (modify_bit_suitAttriType[SAT_STAR] && SEquipment::SLOT_IS_EMPTY != m_Property.m_Equip[i].attribute.starNum[SEquipment::MAX_STAR_NUM - 1])
			modify_bit_suitAttriType.reset(SAT_STAR);

		if (modify_bit_suitAttriType[SAT_JEWEL] && m_Property.m_Equip[i].slots[SEquipment::MAX_SLOTS - 1].isJewel())
		{
			BYTE bType = m_Property.m_Equip[i].slots[SEquipment::MAX_SLOTS - 1].type;
			BYTE bValue = m_Property.m_Equip[i].slots[SEquipment::MAX_SLOTS - 1].value;

			for (size_t i = 0; i < SEquipment::MAX_SLOTS; ++i)
			{
				// 10级以下
				if (((4004001 + (bType - 1) * 100 + (bValue - 1)) % 100) < 10)
				{
					modify_bit_suitAttriType.reset(SAT_JEWEL);
					break;
				}
			}
		}
	}

	m_ModifyTypeMark = modify_bit_suitAttriType.to_ulong();

	// 根据最终的判定结果添加套装附加属性值
	ModifySuitAttributes(wTempModifyMark);

	// ----test------
	// ModifySuitAttriValue(wTempModifyMark);
	// ---------------

	return TRUE;
}

// 修改套装属性值
BOOL CPlayer::ModifySuitAttributes(WORD wMark)
{
	// 一个原始值，还有一个当前值，用于比较
	bitset<16> bit_temp_mark(m_ModifyTypeMark);
	bitset<16> Original_bit_Mark(wMark);

	for (size_t i = 0; i < SAT_MAX; ++i)
	{
		if (bit_temp_mark.test(i))
		{
			switch(i)
			{
			case SAT_LEVEL:
				// 如果以前是没有该项加成的话，则需要计算加成值，如果有的话，直接用以前的值。
				if (!Original_bit_Mark[i])
				{
					m_SaveTheSuitAttriValue[SEquipDataEx::EEA_MAXHP] = m_MaxHp * 0.05;
					m_SaveTheSuitAttriValue[SEquipDataEx::EEA_MAXMP] = m_MaxMp * 0.05;
					m_SaveTheSuitAttriValue[SEquipDataEx::EEA_MAXTP] = m_MaxTp * 0.05;
				}

				m_EquipFactorValue[SEquipDataEx::EEA_MAXHP] += m_SaveTheSuitAttriValue[SEquipDataEx::EEA_MAXHP];
				m_EquipFactorValue[SEquipDataEx::EEA_MAXMP] += m_SaveTheSuitAttriValue[SEquipDataEx::EEA_MAXMP];
				m_EquipFactorValue[SEquipDataEx::EEA_MAXTP] += m_SaveTheSuitAttriValue[SEquipDataEx::EEA_MAXTP];

				break;

			case SAT_GRADE:
				if (!Original_bit_Mark[i])
				{
					m_SaveTheSuitAttriValue[SEquipDataEx::EEA_SHANBI] = m_ShanBi * 0.05;
					m_SaveTheSuitAttriValue[SEquipDataEx::EEA_BAOJI] = m_BaoJi * 0.05;
				}

				m_EquipFactorValue[SEquipDataEx::EEA_SHANBI] += m_SaveTheSuitAttriValue[SEquipDataEx::EEA_SHANBI];
				m_EquipFactorValue[SEquipDataEx::EEA_BAOJI]	 += m_SaveTheSuitAttriValue[SEquipDataEx::EEA_BAOJI];

				break;

			case SAT_COLOR:
				if (!Original_bit_Mark[i])
				{
					m_SaveTheSuitAttriValue[SEquipDataEx::EEA_GONGJI] = m_GongJi * 0.05;
					m_SaveTheSuitAttriValue[SEquipDataEx::EEA_FANGYU] = m_FangYu * 0.05;
				}

				m_EquipFactorValue[SEquipDataEx::EEA_GONGJI] += m_SaveTheSuitAttriValue[SEquipDataEx::EEA_GONGJI];
				m_EquipFactorValue[SEquipDataEx::EEA_FANGYU] += m_SaveTheSuitAttriValue[SEquipDataEx::EEA_FANGYU];

				break;

			case SAT_JEWEL:
				if (!Original_bit_Mark[i])
				{
					m_SaveTheSuitAttriValue[SEquipDataEx::EEA_SPEED] = m_OriSpeed * 0.05;
					m_SaveTheSuitAttriValue[SEquipDataEx::EEA_ATKSPEED] = m_AtkSpeed * 0.05;
				}

				m_EquipFactorValue[SEquipDataEx::EEA_SPEED] += m_SaveTheSuitAttriValue[SEquipDataEx::EEA_SPEED];
				m_EquipFactorValue[SEquipDataEx::EEA_ATKSPEED] += m_SaveTheSuitAttriValue[SEquipDataEx::EEA_ATKSPEED];

				break;

			case SAT_STAR:
				if (!Original_bit_Mark[i])
				{
					m_SaveTheSuitAttriValue[SEquipDataEx::EEA_REDUCEDAMAGE] = m_ReduceDamage * 0.05;
					m_SaveTheSuitAttriValue[SEquipDataEx::EEA_ABSDAMAGE] = m_AbsDamage * 0.05;
				}

				m_EquipFactorValue[SEquipDataEx::EEA_REDUCEDAMAGE] += m_SaveTheSuitAttriValue[SEquipDataEx::EEA_REDUCEDAMAGE];
				m_EquipFactorValue[SEquipDataEx::EEA_ABSDAMAGE] += m_SaveTheSuitAttriValue[SEquipDataEx::EEA_ABSDAMAGE];

				break;

			default:
				continue;
			}
		}

	}

	return TRUE;
}

BOOL CPlayer::ModifySuitAttriValue(WORD wMark)
{
	// 一个原始值，还有一个当前值，用于比较
	bitset<16> bit_temp_mark(m_ModifyTypeMark);
	bitset<16> Original_bit_Mark(wMark);

	for (size_t i = 0; i < SAT_MAX; ++i)
	{
		if (bit_temp_mark.test(i))
		{
			BYTE IfConsume = (BYTE)Original_bit_Mark[i];

			g_Script.SetCondition(NULL, this, NULL);
			LuaFunctor(g_Script, "SuitAttributes")[i, IfConsume]();
			g_Script.CleanCondition();
		}
	}

	return TRUE;
}

int CPlayer::_L_GetLuaValue( const char *funcnam )
{
	lite::Variant ret;
	CPlayer *player = g_Script.m_pPlayer;
	if (player)
	{
		if (player->GetGID() != GetGID()){
			return 0xffffffff;
		}
		LuaFunctor(g_Script, FormatString(funcnam))(&ret);
		if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
			return 0xffffffff;
		return ( __int32 )ret;
	}
	g_Script.SetPlayer(this);
	LuaFunctor(g_Script, FormatString(funcnam))(&ret);
	g_Script.CleanPlayer();

	if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
		return 0xffffffff;

	return ( __int32 )ret;
}

int CPlayer::_L_GetLuaValue( const char *funcnam,lite::Variant const &lvt,lite::Variant const &lvt2 )
{
	lite::Variant ret;
	CPlayer *player = g_Script.m_pPlayer;
	if (player)
	{
		if (player->GetGID() != GetGID()){
			return 0xffffffff;
		}
		LuaFunctor(g_Script, FormatString(funcnam))[lvt][lvt2](&ret);
		if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
			return 0xffffffff;
		return ( __int32 )ret;
	}
	g_Script.SetPlayer(this);
	LuaFunctor(g_Script, FormatString(funcnam))[lvt][lvt2](&ret);
	g_Script.CleanPlayer();

	if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
		return 0xffffffff;

	return ( __int32 )ret;
}

int CPlayer::_L_GetLuaValue( const char *funcnam,lite::Variant const &lvt,lite::Variant const &lvt2,lite::Variant const &lvt3 )
{
	lite::Variant ret;
	CPlayer *player = g_Script.m_pPlayer;
	if (player)
	{
		if (player->GetGID() != GetGID()){
			return 0xffffffff;
		}
		LuaFunctor(g_Script, FormatString(funcnam))[lvt][lvt2][lvt3](&ret);
		if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
			return 0xffffffff;
		return ( __int32 )ret;
	}
	g_Script.SetPlayer(this);
	LuaFunctor(g_Script, FormatString(funcnam))[lvt][lvt2][lvt3](&ret);
	g_Script.CleanPlayer();

	if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
		return 0xffffffff;

	return ( __int32 )ret;
}

int CPlayer::_L_GetLuaValueParmer( const char *funcnam,WORD parmernum,... )
{
	lite::Variant ret;
	CPlayer *player = g_Script.m_pPlayer;
	if (player)
	{
		if (player->GetGID() != GetGID()){
			return 0xffffffff;
		}
	}
	g_Script.SetPlayer(this);
	if (parmernum == 0)return 0;
	WORD *pavle = &parmernum;
	pavle += sizeof(WORD);
	lite::Variant  vecVariant;
	g_Script.PrepareFunction( funcnam );

	lite::Variant  const *pVariant = reinterpret_cast<lite::Variant const*>(pavle);
	for(int i = 0;i<parmernum;i++)
	{
		g_Script.PushParameter( *pVariant);
		pVariant += sizeof(lite::Variant);
	}
	g_Script.Execute(&ret);
	g_Script.CleanPlayer();

	if (lite::Variant::VT_EMPTY == ret.dataType || lite::Variant::VT_NULL == ret.dataType)
		return 0xffffffff;

	return ( __int32 )ret;

}

void CPlayer::UpdataPlayerCounterpartProperties()
{
	if (!m_PlayerAttri)
		return;

	long BV = 0;
	long TS = 310208;
	WORD oldSpeed		= m_OriSpeed;	
	if ( ( (BV = FindpartData(XA_SPEED) ) == TS )? true : false ) //说明没有找到数据
	{
		m_OriSpeed			= PLAYER_BASIC_SPEED;
		m_OriSpeed			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_RS];
		m_OriSpeed			+= m_EquipFactorValue[SEquipDataEx::EEA_SPEED];
		m_OriSpeed			+= (PLAYER_BASIC_SPEED * m_MountAddFactor[SPlayerMounts::SPM_SPEED]) / 100;
		m_OriSpeed			+= GetBuffModifyValue(BAP_SPEED,	PLAYER_BASIC_SPEED);
	}
	else
	{
		m_OriSpeed			= BV;
		m_OriSpeed			+= GetBuffModifyValue(BAP_SPEED,	BV);
	}
	float oldAtkSpeed	= m_AtkSpeed;
	if ( ( (BV = FindpartData(XA_ATKSPEED) ) == TS )? true : false ) //说明没有找到数据
	{
		m_AtkSpeed			= PLAYER_BASIC_ATKSPEED;
		m_AtkSpeed			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_AS];
		m_AtkSpeed			+= m_EquipFactorValue[SEquipDataEx::EEA_ATKSPEED];
		m_AtkSpeed			+= GetBuffModifyValue(BAP_ATKSPEED, PLAYER_BASIC_ATKSPEED);
	}
	else
	{
		m_AtkSpeed			= BV;
		m_AtkSpeed			+= GetBuffModifyValue(BAP_ATKSPEED, BV);
	}
	if ( ( (BV = FindpartData(XA_MAX_HP) ) == TS )? true : false ) //说明没有找到数据
	{
		m_MaxHp				= m_BaseMaxHp;
		m_MaxHp				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_HP];
		m_MaxHp				+= m_EquipFactorValue[SEquipDataEx::EEA_MAXHP];
		m_MaxHp				+= m_JingMaiFactor[JMP_MHP];
		m_MaxHp				+= m_MountAddFactor[SPlayerMounts::SPM_MHP];
		m_MaxHp				+= GetBuffModifyValue(BAP_MHP,		m_BaseMaxHp);
		m_MaxHp				+= m_TelergyFactor[TET_HP];
	}
	else
	{
		m_MaxHp				= BV;
		m_MaxHp				+= GetBuffModifyValue(BAP_MHP,		BV);
	}
	if ( ( (BV = FindpartData(16) ) == TS )? true : false )
	{
		m_MaxMp				= m_BaseMaxMp;
		m_MaxMp				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_MP];
		m_MaxMp				+= m_EquipFactorValue[SEquipDataEx::EEA_MAXMP];
		m_MaxMp				+= m_JingMaiFactor[JMP_MMP];
		m_MaxMp				+= m_MountAddFactor[SPlayerMounts::SPM_MMP];
		m_MaxMp				+= GetBuffModifyValue(BAP_MMP,		m_BaseMaxMp);
		m_MaxMp				+= m_TelergyFactor[TET_MP];
	}
	else
	{
		m_MaxMp				= BV;
		m_MaxMp				+= GetBuffModifyValue(BAP_MMP,		BV);
	}
	if ( ( (BV = FindpartData(17) ) == TS )? true : false )
	{
		m_MaxTp				= m_BaseMaxTp;
		m_MaxTp				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_TL];
		m_MaxTp				+= m_EquipFactorValue[SEquipDataEx::EEA_MAXTP];
		m_MaxTp				+= m_JingMaiFactor[JMP_MTP];
		m_MaxTp				+= m_MountAddFactor[SPlayerMounts::SPM_MTP];
		m_MaxTp				+= GetBuffModifyValue(BAP_MTP,		m_BaseMaxTp);
		m_MaxTp				+= m_TelergyFactor[TET_TP];
	}
	else
	{
		m_MaxTp				= BV;
		m_MaxTp				+= GetBuffModifyValue(BAP_MTP,		BV);
	}
	if ( ( (BV = FindpartData(XA_GONGJI) ) == TS )? true : false )
	{
		m_GongJi			= m_BaseAtk;
		m_GongJi			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_AT];
		m_GongJi			+= m_EquipFactorValue[SEquipDataEx::EEA_GONGJI];
		m_GongJi			+= m_JingMaiFactor[JMP_GONGJI];
		m_GongJi			+= m_MountAddFactor[SPlayerMounts::SPM_GONGJI];
		m_GongJi			+= GetBuffModifyValue(BAP_GONGJI,	m_BaseAtk);
		m_GongJi			+= m_TelergyFactor[TET_ATTACK];
	}
	else
	{
		m_GongJi			= BV;
		m_GongJi			+= GetBuffModifyValue(BAP_GONGJI,	BV);
	}
	if ( ( (BV = FindpartData(XA_FANGYU) ) == TS )? true : false )
	{
		m_FangYu			= m_BaseDefence;
		m_FangYu			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_DF];
		m_FangYu			+= m_EquipFactorValue[SEquipDataEx::EEA_FANGYU];
		m_FangYu			+= m_JingMaiFactor[JMP_FANGYU];
		m_FangYu			+= m_MountAddFactor[SPlayerMounts::SPM_FANGYU];
		m_FangYu			+= GetBuffModifyValue(BAP_FANGYU,	m_BaseDefence);
		m_FangYu			+= m_TelergyFactor[TET_DEFENCE];
	}
	else
	{
		m_FangYu			= BV;
		m_FangYu			+= GetBuffModifyValue(BAP_FANGYU,	BV);
	}
	if ( ( (BV = FindpartData(XA_BAOJI) ) == TS )? true : false )
	{
		m_BaoJi				= m_BaseCtrAtk;
		m_BaoJi				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_BJ];
		m_BaoJi				+= m_EquipFactorValue[SEquipDataEx::EEA_BAOJI];
		m_BaoJi				+= m_JingMaiFactor[JMP_BAOJI];
		m_BaoJi				+= m_MountAddFactor[SPlayerMounts::SPM_BAOJI];
		m_BaoJi				+= GetBuffModifyValue(BAP_BAOJI,	m_BaseCtrAtk);
		m_BaoJi				+= m_TelergyFactor[TET_BAOJI];
	}
	else
	{
		m_BaoJi				= BV;
		m_BaoJi				+= GetBuffModifyValue(BAP_BAOJI,	BV);
	}
	if ( ( (BV = FindpartData(XA_SHANBI) ) == TS )? true : false )
	{
		m_ShanBi			= m_BaseEscape;
		m_ShanBi			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_SB];
		m_ShanBi			+= m_EquipFactorValue[SEquipDataEx::EEA_SHANBI];
		m_ShanBi			+= m_JingMaiFactor[JMP_SHANBI];
		m_ShanBi			+= GetBuffModifyValue(BAP_SHANBI,	m_BaseEscape);
		m_ShanBi			+= m_TelergyFactor[TET_SHANBI];
	}
	else
	{
		m_ShanBi			= BV;
		m_ShanBi			+= GetBuffModifyValue(BAP_SHANBI,	BV);
	}
	//m_MaxHp				= m_BaseMaxHp;
	//m_MaxMp				= m_BaseMaxMp;
	//m_MaxTp				= m_BaseMaxTp;
	//m_GongJi			= m_BaseAtk;
	//m_FangYu			= m_BaseDefence;
	//m_BaoJi				= m_BaseCtrAtk;
	//m_ShanBi			= m_BaseEscape;
	m_SorbDamageValue	= 0;
	m_SorbDamagePercent	= 0;
	//	m_AbsDamageValue	= 0;
	MY_ASSERT(m_MaxHp);
	// 获取体质的附加属性
	//m_MaxHp				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_HP];
	//m_MaxMp				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_MP];
	//m_MaxTp				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_TL];
	//m_GongJi			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_AT];
	//m_FangYu			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_DF];
	//m_BaoJi				+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_BJ];
	//m_ShanBi			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_SB];
	//m_AtkSpeed			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_AS];
	m_SorbDamageValue	+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_MS];
	//m_OriSpeed			+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_RS];
	//m_AbsDamageValue	+= m_Property.dwTiZhiAttrs[SPlayerTiZhi::TZ_MAX_SH];

	// 获取装备的附加属性
	//m_MaxHp				+= m_EquipFactorValue[SEquipDataEx::EEA_MAXHP];
	//m_MaxMp				+= m_EquipFactorValue[SEquipDataEx::EEA_MAXMP];
	//m_MaxTp				+= m_EquipFactorValue[SEquipDataEx::EEA_MAXTP];
	//m_GongJi			+= m_EquipFactorValue[SEquipDataEx::EEA_GONGJI];
	//m_FangYu			+= m_EquipFactorValue[SEquipDataEx::EEA_FANGYU];
	//m_BaoJi				+= m_EquipFactorValue[SEquipDataEx::EEA_BAOJI];
	//m_ShanBi			+= m_EquipFactorValue[SEquipDataEx::EEA_SHANBI];
	//m_OriSpeed			+= m_EquipFactorValue[SEquipDataEx::EEA_SPEED];
	//m_AtkSpeed			+= m_EquipFactorValue[SEquipDataEx::EEA_ATKSPEED];
	m_ReduceDamage		+= m_EquipFactorValue[SEquipDataEx::EEA_REDUCEDAMAGE];
	m_AbsDamage			+= m_EquipFactorValue[SEquipDataEx::EEA_ABSDAMAGE];
	m_NonFangyu			+= m_EquipFactorValue[SEquipDataEx::EEA_NONFANGYU];
	m_MultiBaoJi		+= m_EquipFactorValue[SEquipDataEx::EEA_MULTIBAOJI];
	// 获取经脉的附加属性
	//m_MaxHp				+= m_JingMaiFactor[JMP_MHP];
	//m_MaxMp				+= m_JingMaiFactor[JMP_MMP];
	//m_MaxTp				+= m_JingMaiFactor[JMP_MTP];
	//m_GongJi			+= m_JingMaiFactor[JMP_GONGJI];
	//m_FangYu			+= m_JingMaiFactor[JMP_FANGYU];
	//m_BaoJi				+= m_JingMaiFactor[JMP_BAOJI];
	//m_ShanBi			+= m_JingMaiFactor[JMP_SHANBI];

	// 获取骑乘的附加属性
	//m_MaxHp				+= m_MountAddFactor[SPlayerMounts::SPM_MHP];
	//m_MaxMp				+= m_MountAddFactor[SPlayerMounts::SPM_MMP];
	//m_MaxTp				+= m_MountAddFactor[SPlayerMounts::SPM_MTP];
	//m_GongJi			+= m_MountAddFactor[SPlayerMounts::SPM_GONGJI];
	//m_FangYu			+= m_MountAddFactor[SPlayerMounts::SPM_FANGYU];
	//m_BaoJi				+= m_MountAddFactor[SPlayerMounts::SPM_BAOJI];
	//m_ShanBi			+= m_MountAddFactor[SPlayerMounts::SPM_SHANBI];
	//m_OriSpeed			+= (PLAYER_BASIC_SPEED * m_MountAddFactor[SPlayerMounts::SPM_SPEED]) / 100;


	m_SorbDamageValue	+= GetBuffModifyValue(BAP_SORBDAM,		0);
	m_SorbDamagePercent	+= GetBuffModifyValue(BAP_SORBDAM_PER,	0);
	m_ExtraExpPercent	+= GetBuffModifyValue(BAP_EXTRA_EXP_PER, 100);

	// 将速度改为float
	if (oldSpeed != m_OriSpeed)
	{
		m_Speed = m_OriSpeed / 1000.f;
		m_FightPropertyStatus[XA_SPEED] = true;
	}

	if (1/*PLAYER_BASIC_ATKSPEED != m_AtkSpeed*/)
	{
		m_AtkSpeed = PLAYER_BASIC_ATKSPEED + ( m_AtkSpeed - PLAYER_BASIC_ATKSPEED) / 10;
		// 		if (m_AtkSpeed > 2 * PLAYER_BASIC_ATKSPEED)
		// 		{
		// 			m_AtkSpeed = 2 * PLAYER_BASIC_ATKSPEED;
		// 		}
		m_FightPropertyStatus[XA_ATKSPEED] = true;
	}

	// 标记属性改变
	for (size_t i = 0; i < XA_CUR_HP; i++)
		m_FightPropertyStatus[i] = true;

	//add by ly 2014/5/6 同步玩家战斗力数据
	m_PlayerFightPower = GetPlayerFightPower();	//获取玩家战斗力
	m_PlayerPropertyStatus[XA_FIGHTPOWER - XA_MAX_EXP] = true;

	// 是否需要修正当前基本属性
	if (m_CurHp > m_MaxHp)	RestoreFullHPDirectly();
	if (m_CurMp > m_MaxMp)  RestoreFullMPDirectly();
	if (m_CurTp > m_MaxTp)	RestoreFullTPDirectly();
}

long CPlayer::FindpartData(short Index)
{
	long ValueR = 310208; 
	TemplateMap::iterator iter = m_TemplateMap.find(Index);
	if (iter != m_TemplateMap.end())
	{
		ValueR = iter->second;
	}
	return ValueR;
}
void CPlayer::OnPlayerBlessTime()
{
	if (m_Property.m_BlessFlag == 0)
	{
		std::string time;
		GetSysTimeTick(time);
		m_Property.m_BlessFlag = 1;
		memcpy(m_Property.Blesstime,time.c_str(),time.size());
	}
}
long CPlayer::GetPlayerMoney(long type) const
{
	if (type == 1) //银币
	{
		return m_Property.m_Money;
	}
	else if (type == 2)//银两
	{
		return m_Property.m_BindMoney;
	}
	return 0;
}
void CPlayer::ChangeMoney(long type,long value)
{
	if (type == 1)
	{
		long Tmep = m_Property.m_Money;
		Tmep += value;
		if (Tmep <= 0)
		{
			m_Property.m_Money = 0;
		}
		else
		{
			m_Property.m_Money = Tmep;
		}
		m_PlayerPropertyStatus[XA_UNBIND_MONEY - XA_MAX_EXP] = true;
	}
	else if (type == 2)
	{
		long Tmep = m_Property.m_BindMoney;
		Tmep += value;
		if (Tmep <= 0)
		{
			m_Property.m_BindMoney = 0;
		}
		else
		{
			m_Property.m_BindMoney = Tmep;
		}
		m_PlayerPropertyStatus[XA_BIND_MONEY - XA_MAX_EXP] = true;
	}
}

// [2012-8-27 12-39 gw: +获取该玩家所有的钱]
long CPlayer::GetPlayerAllMoney() const
{
	return GetPlayerMoney(EMoneyType_NonBind) + GetPlayerMoney(EMoneyType_Bind);
}
// [gw+Func:通用扣钱-绑定和非绑定钱都可以扣取。注意不能加钱]
// [Param:bDoReduceMoney-是否需要扣钱，true-如果能成功，那将会扣取; false-钱不变]
// [Param:ePriorityReduced-优先扣取钱的类型]
// [Param:lMoneyValue-扣多少钱]
bool CPlayer::ReducedCommonMoney( long lMoneyValue, TPlayerMoneyType ePriorityReduced /* = EMoneyType_Bind*/)
{
	if (lMoneyValue < 0)
	{
		return false;
	}
	long lFirstMoney =  GetPlayerMoney(ePriorityReduced);
	if (lMoneyValue <= lFirstMoney)
	{
		ChangeMoney(ePriorityReduced, -1 * lMoneyValue);
		return true;
	}
	TPlayerMoneyType eSecondType = EMoneyType_NonBind;
	switch (ePriorityReduced)
	{
	case EMoneyType_NonBind:
		eSecondType = EMoneyType_Bind;
		break;
	case EMoneyType_Bind:
		eSecondType = EMoneyType_NonBind;
		break;
	default:
		return false;
	}
	long lSecondMoney = GetPlayerMoney(eSecondType);
	if (lFirstMoney + lSecondMoney >= lMoneyValue)
	{
		ChangeMoney(ePriorityReduced, -1 * lFirstMoney);
		ChangeMoney(eSecondType, -1*(lMoneyValue-lFirstMoney));
		return true;
	}

	return false;
}

void CPlayer::UpdataBless()
{
	if (m_Property.m_BlessFlag != 0)
	{
		bool Flag = IsCompleteDayTime(m_Property.Blesstime);
		if (Flag)
		{
			m_Property.m_BlessFlag = 0;
			g_Script.SetCondition(0, this, 0);
			LuaFunctor(g_Script,"GetBlessInfo")();
			g_Script.CleanCondition();
		}
	}
}

void CPlayer::SentClinetGiftSucceed( BYTE bType,BYTE bResult, BYTE index )
{
	SAGetOnlienGiftMsg GiftMsg;
	GiftMsg.bResult = bResult;
	GiftMsg.PackType = bType;
	GiftMsg.index = index;
	g_StoreMessage(m_ClientIndex,&GiftMsg,sizeof(SAGetOnlienGiftMsg));
}

// 心法登陆初始化
void CPlayer::InitTelergyAndSkill()
{
	m_bFirstLogIn = true;

	for (size_t i = 0; i < MAX_TELEGRYNUM; ++i)
	{
		WORD TelergyID		= m_Property.m_Xyd3Telergy[i].m_TelergyID;
		BYTE TelergyLevel	= m_Property.m_Xyd3Telergy[i].m_TelergyLevel;

		if (TelergyID > 0 && TelergyLevel > 0)
		{
			const SXinFaData *pData = CSkillService::GetInstance().GetXinFaBaseData(TelergyID);
			if (!pData)
			{
				rfalse("心法配置表中找不到该心法ID！");
				continue;
			}

			m_bTempLevel = TelergyLevel;

			// 心法增加属性
			g_Script.SetCondition( NULL, this, NULL );
			//LuaFunctor(g_Script, FormatString("UpdateTelergy%d", TelMsg->wID))[telergyLevel]();
			LuaFunctor(g_Script, FormatString("TelergyProperty%d", TelergyID))[TelergyLevel]();
			g_Script.CleanCondition();
		}
	}

	m_bFirstLogIn = false;

	return;
}

void CPlayer::OnRecvTaskMovemsg( SQtaskmove *pmsg )
{
	if (!IsItemPosValid(pmsg->wCellPos))
		return;

	SPackageItem *pIB = FindItemByPos(pmsg->wCellPos, XYD_FT_ONLYUNLOCK);
	if (!pIB)
	{
		/*		rfalse(4, 1, "[ItemUser] - RecvUseItem~");*/
		return ;
	}

	const SItemBaseData *pData = CItemService::GetInstance().GetItemBaseData(pIB->wIndex);
	if (!pData)return ;

	int itemistransport = _L_GetLuaValue("OnItemIstransport",pData->m_ID,pmsg->taskID);
	if (!itemistransport)return;

	if(pData->m_LocSrvPreTime)
	{
		if (CanUseIt(this,pIB,pData)){
			if(!SetPreTime(pIB,pData->m_LocSrvPreTime))
				return;
		}
	}
	else{
		if (CanUseIt(this,pIB,pData)){
			_UseItem(pIB);
		}
	}
}

void CPlayer::OnProcessDead()
{
	if (0 != m_CurHp)
		return;

	//MY_ASSERT(EA_DEAD == GetCurActionID());
	if (EA_DEAD != GetCurActionID())
	{
		rfalse("OnProcessDead: CurActionID = %d",GetCurActionID());
		SetCurActionID(EA_DEAD);
	}

	if (m_checkTick - m_DeadTime < wReliveTime[1] * 1000)
		return;

	QuestToRelive(SQPlayerDeadMsg::PDM_BACK_CITY);

}

void CPlayer::OnRecvQuestActivityNotice( SQBossDeadUpdatemsg* pMsg )
{
	GetGW()->RecvActivityNotice(pMsg,this);
}
void CPlayer::ClearMetrialRelation()
{
	if (m_vecStrengthenEquip.size() > 0)
	{		
		LockBatchCells(m_vecStrengthenEquip, false);						
		m_vecStrengthenEquip.clear();		
	}
	if (m_vecStrengthenEquipAssist.size() > 0)
	{
		LockBatchCells(m_vecStrengthenEquipAssist, false);
		m_vecStrengthenEquipAssist.clear();
	}
	//m_strengthenEquip = 0;
	m_MetrialItemID = 0;
}

void CPlayer::OnRecvClearPoint( struct SQResetPointMsg* pMsg )
{
	if (!pMsg)return;
	SAResetPointMsg msg;

	WORD totalnum = 0;
	for (int i = 0; i< 4;i++){
		totalnum += pMsg->point[i];
	}
	BOOL CheckOK = TRUE; 
	if (m_Level > 30)
	{
		if (!CheckGoods(dwXiSuiDanID,totalnum,true))
		{
			DWORD itemnum = GetItemNum(dwXiSuiDanID, XYD_FT_ONLYUNLOCK); 
			//DWORD needMoney =  (totalnum - itemnum) * 1000;
			int checkmoney = _L_GetLuaValue("OnReliveCheckPlayerMoney",totalnum - itemnum,0);
			if (!checkmoney)
			{
				TalkToDnid(m_ClientIndex,"金钱不足洗点失败");
				msg.result = SAResetPointMsg::RPM_FAIL;
				g_StoreMessage(m_ClientIndex, &msg, sizeof(SAResetPointMsg));
				return;
			}
			if(itemnum){
				CheckOK = CheckGoods(dwXiSuiDanID,itemnum,false);
			}
		}else
			CheckOK = CheckGoods(dwXiSuiDanID,totalnum,false);
	}

	if (!CheckOK){
		TalkToDnid(m_ClientIndex,"道具不足洗点失败");
		msg.result = SAResetPointMsg::RPM_FAIL;
		g_StoreMessage(m_ClientIndex, &msg, sizeof(SAResetPointMsg));
		return;
	}

	for (int i = 0; i< 4;i++){
		ResetPlayerPoint(i,pMsg->point[i]);
	}
	// 更新剩余点数
	m_Property.m_RemainPoint += totalnum;
	m_PlayerPropertyStatus[XA_REMAINPOINT - XA_MAX_EXP] = true;
	// 更新玩家属性
	UpdateBaseProperties();
	UpdateAllProperties();

	msg.result = SAResetPointMsg::RPM_SUCCESS;
	g_StoreMessage(m_ClientIndex, &msg, sizeof(SAResetPointMsg));

}

void CPlayer::_L_ResetCountDownGiftState()
{
	m_Property.m_dTimeCountDownGiftState = 0;
}

void CPlayer::_L_ResetCountDownGiftTime()
{
	_time64(&m_Property.m_CountDownBeginTime);
}

INT64 CPlayer::_L_GetCountDownGiftBeginTime()
{
	return m_Property.m_CountDownBeginTime;
}

void CPlayer::OnCheckTimeLimit(DWORD dwCurTick)
{
	if ( limitedState && m_OnlineState == OST_NORMAL )
	{
		if( abs( (int)(dwCurTick-m_dwMountTimer)) >= CHECKTIMELIMIT_TIME )
		{
			m_OnlineTime += (dwCurTick-m_dwMountTimer)/1000; // 10秒进一次， 一次加10秒， 正常速度
			m_dwMountTimer = dwCurTick;
			if ( ( m_OnlineTime % ( 15*60 ) ) <= 9 ) // 因为为10的步长, 所以只需要判断是否余数小于等于9就可以了
			{
				// 现在防沉迷系统属于每在线时间每累加满15分钟就向账号服务器发起一个状态检测, 
				// 等返回值确认后, 再设置状态和显示提示
				SQUpdateTimeLimit msg;
				try {
					lite::Serializer slm( msg.streamBuffer, sizeof( msg.streamBuffer ) );
					slm( GetAccount() );
					SendToLoginServer( &msg, long(sizeof( msg ) - slm.EndEdition()) );
				}
				catch ( lite::Xcpt& )
				{
				}
			}
		}
	}

}

void CPlayer::ShowLimitTips()
{
	switch (limitedState)
	{
	case LIMITED_ALREADY:
		TalkToDnid( m_ClientIndex, FormatString( "您累计在线时间已满%d小时。", m_OnlineTime/(60*60) ) );
		break;
	case LIMITED_HALF:
		TalkToDnid( m_ClientIndex, "您已经进入疲劳游戏时间，您的游戏收益将降为正常值的50%。为了您的健康，请尽快下线休息,做适当身体活动，合理安排学习生活。" );
		// 		TalkToDnid( m_ClientIndex, "正常值的50%。为了您的健康，请尽快下线休息，" );
		// 		TalkToDnid( m_ClientIndex, "做适当身体活动，合理安排学习生活。" );
		break;
	case LIMITED_ZERO:
		TalkToDnid( m_ClientIndex, "您已进入不健康游戏时间，为了您的健康，请您立即下线休息。如不下线，您的身体将受到损害,您的收益已降为零，直到您的累计下线时间满5小时后，才能恢复正常" );
		// 		TalkToDnid( m_ClientIndex, "立即下线休息。如不下线，您的身体将受到损害，" );
		// 		TalkToDnid( m_ClientIndex, "您的收益已降为零，直到您的累计下线时间" );
		// 		TalkToDnid( m_ClientIndex, "满5小时后，才能恢复正常。" );
		break;
	default:
		break;
	}
}

void CPlayer::OnSetPlayerLevel( BYTE Level )
{
	if (Level > MAXPLAYERLEVEL || Level < m_Property.m_Level)return;

	WORD DestLevel = Level - m_Property.m_Level ;
	// 	WORD  UpCount = 0 ;
	// 	QWORD needExp = CPlayerService::GetInstance().GetPlayerUpdateExp(m_Property.m_Level);
	// 	QWORD addExp = 0;
	// 
	// 	while(UpCount != DestLevel )
	// 	{
	// 		addExp += needExp, UpCount++;
	// 		needExp = CPlayerService::GetInstance().GetPlayerUpdateExp(m_Property.m_Level+UpCount);
	// 	}
	// 	SendAddPlayerExp(addExp,SAExpChangeMsg::GM,"GMGive");
	SendAddPlayerLevel(DestLevel, SASetEffectMsg::EEFF_LEVELUP, "Notice, plr lvl up");
	if (MAXPLAYERLEVEL == m_Property.m_Level)
		m_Property.m_Exp = 0;

	//升级以后属性改变更新玩家修为值
	UpdatePlayerXwzMsg();

	//玩家升级时脚本回调
	_L_SafeCallLuaFunction("OnPlayerLevelUp");
}

bool CPlayer::_L_calculateSpAndExpByTime( INT64 sec,DWORD &sp,DWORD &exp )
{
	lite::Variant ret,ret2;
	CPlayer *player = g_Script.m_pPlayer;
	if (player)
	{
		if (player->GetGID() != GetGID()){
			return false;
		}
		g_Script.CallFunc("calculateSpAndExpByTime",sec);
		if (g_Script.GetRet(0).GetType() == CScriptValue::VT_NIL ||g_Script.GetRet(1).GetType()  == CScriptValue::VT_NIL  ){
			return false;
		}
		sp = (DWORD)g_Script.GetRet(0);
		exp = (DWORD)g_Script.GetRet(1);
		return true;
	}
	g_Script.SetPlayer(this);
	g_Script.CallFunc("calculateSpAndExpByTime",sec);

	if (g_Script.GetRet(0).GetType() == CScriptValue::VT_NIL ||g_Script.GetRet(1).GetType()  == CScriptValue::VT_NIL  ){
		g_Script.CleanPlayer();
		return false;
	}
	sp = (DWORD)g_Script.GetRet(0);
	exp = (DWORD)g_Script.GetRet(1);
	g_Script.CleanPlayer();
	return true;
}
void CPlayer::SendPlayerBlessOpen()
{
	SABLESSOpenClose msg;
	msg.Flag = m_PlayerOpenClose;
	g_StoreMessage(m_ClientIndex,&msg,sizeof(SABLESSOpenClose));
}
long CPlayer::OnPartEnterNum(DWORD RegionID,BYTE numLimit)
{
	if ( m_Property.m_DayRecordFlag == 0)
	{
		std::string timestr;
		GetSysTimeTick(timestr);
		memcpy(m_Property.m_DayRecordtime,timestr.c_str(),30);
		m_Property.m_DayRecordFlag = 1;
	}
	int index = -1;	
	for (int i = 0; i < 30; ++i)//如果找到了
	{
		if ( m_Property.m_DayRecordArray[i].m_RegionID == RegionID)
		{
			++m_Property.m_DayRecordArray[i].m_EnterNum;
			return 1;
		}
		if ( m_Property.m_DayRecordArray[i].m_RegionID == 0 && index == -1)
		{
			index = i;//找到第一个空位置
		}
	}
	if (index == -1)
	{
		rfalse("为什么会为-1 !!!");
		return 0;
	}
	m_Property.m_DayRecordArray[index].m_RegionID = RegionID;
	m_Property.m_DayRecordArray[index].m_EnterNum = 1;
	m_Property.m_DayRecordArray[index].m_NumLimit = numLimit;
	return 1;
}
long CPlayer::OnTeamPartEnterNum(DWORD RegionID,BYTE numLimit)
{
	if (!m_bIsTeamLeader)
	{
		return 0;
	}
	std::map<DWORD, Team>::iterator iter = teamManagerMap.find(m_dwTeamID);
	if (iter != teamManagerMap.end())
	{
		Team stTeamInfo = iter->second;
		for (int i = 0; i < 5; ++i)
		{
			CPlayer *player = stTeamInfo.stTeamPlayer[i].PlayerRef;
			if (player)
			{
				player->OnPartEnterNum(RegionID,numLimit);
			}
		}
	}
	return 1;
}
long CPlayer::IsPersonalEnterPart(DWORD RegionID)
{
	if (m_Property.m_DayRecordFlag == 0)
	{
		return 1;
	}
	for (int i = 0; i < 30; ++i)
	{
		if ( m_Property.m_DayRecordArray[i].m_RegionID == RegionID)
		{
			if (m_Property.m_DayRecordArray[i].m_EnterNum >= m_Property.m_DayRecordArray[i].m_NumLimit)
			{
				return 0;
			}
		}
	}
	return 1;//没有找到说明成功
}
long CPlayer::IsTeamEnterPart(DWORD RegionID)
{
	if (m_bIsTeamLeader)
	{
		std::map<DWORD, Team>::iterator iter = teamManagerMap.find(m_dwTeamID);
		if (iter != teamManagerMap.end())
		{
			Team stTeamInfo = iter->second;
			for (int i = 0; i < 5; ++i)
			{
				CPlayer *player = stTeamInfo.stTeamPlayer[i].PlayerRef;
				if (player)
				{
					long Flag = player->IsPersonalEnterPart(RegionID);
					if (Flag == 0)//说明有队员的数据不能进入这个副本
					{
						return 0;
					}
				}
			}
		}
	}
	else
	{
		return 0;
	}
	return 1;
}
void CPlayer::UpdataPartEnterNum()
{
	if (m_Property.m_DayRecordFlag == 1)
	{
		std::string strtime = m_Property.m_DayRecordtime;
		if (IsCompleteDayTime(strtime))
		{
			m_Property.InitRecordArray();
		}
	}
}
long CPlayer::IsNonTeamPartEnter(DWORD RegionGid)
{
	extern LPIObject FindRegionByGID(DWORD GID);
	CDynamicRegion *dynamicRegion = (CDynamicRegion *)FindRegionByGID(RegionGid)->DynamicCast(IID_DYNAMICREGION);
	if (dynamicRegion)
	{
		return 1;
	}
	return 0;
}

//检查在线奖励累计时间，当登录时
void CPlayer::CheckTimeWhenLogin()
{
}
void CPlayer::CheckTimeWhenLogout()
{
	INT64 now;
	_time64(&now);//登出时间

	m_Property.m_dLoginTime64 = m_dwLoginTime; //记录本次登录游戏的时间(没有在登录时保存，是因为在游戏过程中m_Property.m_dLoginTime64被用来记录上次登录时间)
	m_Property.m_dLeaveTime64 = now; //记录本次离开游戏的时间

	m_Property.m_OnlineTime += m_Property.m_dLeaveTime64 - m_Property.m_dLoginTime64;

	//更新宠物恢复魂力还需要的时间值
	for (int i = 0; i < MAX_NEWPET_NUM; i++)
	{
		if (m_Property.m_NewPetData[i].m_PetID != 0 && m_Property.m_NewPetData[i].m_CurPetDurable == 0)
		{
			if (m_Property.m_NewPetData[i].m_DurableResumeNeedTime - (m_Property.m_dLeaveTime64 - m_Property.m_dLoginTime64) <= 0)
				m_Property.m_NewPetData[i].m_DurableResumeNeedTime = 0;
			else
				m_Property.m_NewPetData[i].m_DurableResumeNeedTime -= m_Property.m_dLeaveTime64 - m_Property.m_dLoginTime64;
		}
	}
	char logouTimeBuff[8];
	ConverTool::ConvertInt64ToBytes(now, logouTimeBuff);
	memcpy(m_FixData.m_TimeData.m_LogoutTime, logouTimeBuff, sizeof(char) * 8);
}

void CPlayer::_L_ResetOnlineGiftState()
{
	for (int i=0; i<MAX_ONLINEGIFT_NUM; i++)
	{
		m_Property.m_onlineGiftStates[i] = 0;
	}

	m_Property.m_dayOnlineTime = 0;//在线时间
	_time64(&m_dwOnlineBeginTime);//在线计时开始时间
}

INT32 CPlayer::SetCurrentSkillLevel(DWORD dwSkillIndex, BYTE bskillLevel)
{
	SSkill *pSkill = &m_Property.m_pSkills[dwSkillIndex];
	if (pSkill->wTypeID)
	{
		MY_ASSERT(pSkill->byLevel > 0 && pSkill->byLevel <= MaxSkillLevel);
		m_Property.m_pSkills[dwSkillIndex].byLevel = bskillLevel;

		SASkillUpdate msg;

		msg.byWhat = SASkillUpdate::SSU_LEVELUP;
		msg.wPos = dwSkillIndex;
		msg.stSkill.wTypeID = m_Property.m_pSkills[dwSkillIndex].wTypeID;
		msg.stSkill.byLevel = m_Property.m_pSkills[dwSkillIndex].byLevel;
		msg.stSkill.dwProficiency = m_Property.m_pSkills[dwSkillIndex].dwProficiency;

		g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));
		return 1;
	}
	return -1;
}

INT32 CPlayer::GetSkillLevel(DWORD dwSkillIndex)
{
	for (int i = 0; i < MAX_SKILLCOUNT; i++)
	{
		if (m_Property.m_pSkills[i].wTypeID == dwSkillIndex && m_Property.m_pSkills[i].byLevel > 0)
			return m_Property.m_pSkills[i].byLevel;
	}
	return -1;
}

INT32 CPlayer::SetSkillLevel(DWORD dwSkillIndex, BYTE bskillLevel)
{
	for (int i = 0; i < MAX_SKILLCOUNT; i++)
	{
		if (m_Property.m_pSkills[i].wTypeID == dwSkillIndex && m_Property.m_pSkills[i].byLevel > 0)
		{
			m_Property.m_pSkills[i].byLevel = bskillLevel;
			SASkillUpdate msg;

			msg.byWhat = SASkillUpdate::SSU_LEVELUP;
			msg.wPos = i;
			msg.stSkill.wTypeID = m_Property.m_pSkills[i].wTypeID;
			msg.stSkill.byLevel = m_Property.m_pSkills[i].byLevel;
			msg.stSkill.dwProficiency = m_Property.m_pSkills[i].dwProficiency;

			g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));
			return bskillLevel;
		}
	}
	return -1;
}

////add by ly 2014/3/17  玩家升星处理
void CPlayer::OnHandlePlayerRiseStar(SQXinYangRiseStarmsg *pMsg)
{
	g_Script.SetCondition(NULL, this, NULL);
	LuaFunctor(g_Script, FormatString("ConsumeAndAddPlayerProperty"))[pMsg->m_XinXiuID + 1][pMsg->m_num][pMsg->MoneyType][m_Property.m_bVipLevel]();
	g_Script.CleanCondition();
}

bool CPlayer::OnUpdateTask(struct SRoleTaskBaseMsg *pMsg)
{
	if (pMsg->_protocol == SRoleTaskBaseMsg::EPRO_FINISHTADDTASK)
	{
		SQFinishAddTask *sfinishtask = (SQFinishAddTask *)pMsg;
		CPlayer::TASKADD::iterator it = m_AddTask.find(sfinishtask->btype);
		if (it != m_AddTask.end())
		{
			for (std::list<DWORD>::iterator ti = (it->second).begin(); ti != (it->second).end(); ++ti)
			{
				DWORD flagIndex = *ti;
				if (CRoleTask *task = m_TaskManager.GetRoleTask((flagIndex & 0xffff0000) >> 16))
				{
					if (STaskFlag *flag = task->GetFlag(flagIndex))
					{
						if (flag->m_Complete)			// 如果已经完成了，不做任何处理
							continue;

						if (flag->m_TaskDetail.TaskADD.btype == sfinishtask->btype)
						{
							if (flag->m_TaskDetail.TaskADD.btype == flag->m_TaskDetail.TaskADD.TASK_SCENECOMPLETE)
							{
								if (flag->m_TaskDetail.TaskADD.dIndex == sfinishtask->dIndex)
								{
									flag->m_TaskDetail.TaskADD.wCount += sfinishtask->wCount;
									if (flag->m_TaskDetail.TaskADD.wCount >= flag->m_TaskDetail.TaskADD.wAllCount)
									{
										flag->m_Complete = 1;
										flag->m_TaskDetail.TaskADD.wCount = flag->m_TaskDetail.TaskADD.wAllCount;
									}
								}
							}
							else
							{
								flag->m_TaskDetail.TaskADD.wCount += sfinishtask->wCount;
								if (flag->m_TaskDetail.TaskADD.wCount >= flag->m_TaskDetail.TaskADD.wAllCount)
								{
									flag->m_Complete = 1;
									flag->m_TaskDetail.TaskADD.wCount = flag->m_TaskDetail.TaskADD.wAllCount;
								}
							}

						}
						SendUpdateTaskFlagMsg(*flag, true);
						return true;
					}
				}
			}
		}
	}
	return false;
}

//add by ly 2014/3/25	
//发送荣耀的请求应答消息
void CPlayer::SendSAGloryMsg(struct SAGloryMsg *pMsg)
{
	int SendDataSize = pMsg->m_GloryArrSize * sizeof(WORD)+sizeof(SAGloryMsg)-sizeof(void*);
	BYTE *Buf = new BYTE[SendDataSize];
	memset(Buf, 0, SendDataSize);
	memcpy(Buf, pMsg, sizeof(SAGloryMsg)-sizeof(void*));
	memcpy(Buf + sizeof(SAGloryMsg)-sizeof(void*), pMsg->m_pGloryInfArr, pMsg->m_GloryArrSize * sizeof(WORD));
	g_StoreMessage(this->m_ClientIndex, Buf, SendDataSize);
	delete[] Buf;
}

//玩家荣耀请求处理
void CPlayer::GetPlayerGloryInfo(struct SQGloryMsg *pMsg)
{
	g_Script.SetCondition(NULL, this, NULL);
	LuaFunctor(g_Script, FormatString("GetPlayerGloryInfo"))[this->GetGID()]();
	g_Script.CleanCondition();
}

//判断是否满足获取荣耀奖励
BOOL CPlayer::JudgeCanGetGloryAward(const SGloryBaseData *lpGloryBaseData)
{
	if (lpGloryBaseData != NULL)
	{
		BYTE ConditionIndex = lpGloryBaseData->m_GloryConditionIndex;
		switch (ConditionIndex)
		{
		case GLORY_CONLEVELID:	//玩家达到x等级
			if (m_Property.m_Level >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CLEVEL])
				return TRUE;
			break;
		case GLORY_CONONLINEID:	//累积在线时间
			if (m_Property.m_OnlineTime >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CONLINE]);
				return TRUE;
			break;
		case GLORY_CONEQUITINTENSIFYLEVELID:	//拥有x件装备强化到x级
		{
													int EquipIntensifyLevelGtN = 0;		//统计强化等级大于x的装备数量
													for (int i = 0; i < EQUIP_P_MAX; i++)
													{
														if (m_Property.m_Equip[i].attribute.bIntensifyLevel >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CEQUITINTENSIFYLEVEL])
															EquipIntensifyLevelGtN++;
													}
													if (EquipIntensifyLevelGtN >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CEQUITNUM])
														return TRUE;
		}
			break;
		case GLORY_CONEQUITGRADELEVELID:	//拥有x件装备升阶到x级
		{
												int EquipGradeLevelGtN = 0;		//统计升级等级大于x的装备数量
												for (int i = 0; i < EQUIP_P_MAX; i++)
												{
													if (m_Property.m_Equip[i].attribute.bIntensifyLevel >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CEQUITGRADELEVEL])
														EquipGradeLevelGtN++;
												}
												if (EquipGradeLevelGtN >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CEQUITNUM])
													return TRUE;
		}
			break;
		case GLORY_CONEQUITSRARLEVELID:	//拥有x件装备升星到x级
		{
											int EquipStarLevelGtN = 0;		//统计升星等级大于x的装备数量
											for (int i = 0; i < EQUIP_P_MAX; i++)
											{
												if (m_Property.m_Equip[i].attribute.bIntensifyLevel >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CEQUITSRARLEVEL])
													EquipStarLevelGtN++;
											}
											if (EquipStarLevelGtN >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CEQUITNUM])
												return TRUE; 
		}
			break;
		case GLORY_CONEQUITKEYINGLEVELID:	//拥有x件附加x个刻印的装备
		{
												int EquipKeYingLevelGtN = 0;		//统计刻印等级大于x的装备数量
												for (int i = 0; i < EQUIP_P_MAX; i++)
												{
													int MaxKeYingLevel = 0;
													MaxKeYingLevel = max(max(m_Property.m_Equip[i].equipkeyin.MaxKeYinLevel[0], m_Property.m_Equip[i].equipkeyin.MaxKeYinLevel[1]), m_Property.m_Equip[i].equipkeyin.MaxKeYinLevel[2]);
													if (MaxKeYingLevel >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CEQUITKEYINGLEVEL])
														EquipKeYingLevelGtN++;
												}
												if (EquipKeYingLevelGtN >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CEQUITNUM])
													return TRUE;
		}
			break;
		case GLORY_CONYINLIANGANDYUANBAOID:	//拥有游戏币x，拥有元宝x
			if (m_Property.m_BindMoney >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CYINLIANGNUM] &&
				m_Property.m_Money >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CYUANBAONUM])
				return TRUE;
			break;
		case GLORY_CONGONGJIID:	//战斗力达到x点
			if (m_GongJi >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CGONGJI])
				return TRUE;
			break;
		case GLORY_CONBIANSHENGNUMID:	//拥有x个变身形态
		{
											int BianShengNum = 0;	//统计已经激活的变身数量
											for (BYTE bTransformerIndex = 0; bTransformerIndex <= 2; bTransformerIndex++)
											{
												int Level;// = GetTransformersLevel(bTransformerIndex);
												if (Level != 0)
													BianShengNum++;
											}
											if (BianShengNum >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CBIANSHENGNUM])
												return TRUE;
		}
			break;
		case GLORY_CONBELIEFSKILLNUMID:	//获得信仰技能x个
		{
											int XinYangSkillNum = 0;	//统计信仰技能数量
											for (; XinYangSkillNum < MAX_JM_XUE_MEMORY; XinYangSkillNum++)
											{
												if (m_Property.m_JingMai[XinYangSkillNum] != 12)	//只有12颗星都升满才能激活信仰技能
													break;
											}
											if (XinYangSkillNum >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CBELIEFSKILLNUM])
												return TRUE;
		}
			break;
		case GLORY_CONPACKAGENUMID:	//激活x个包裹
		{
										DWORD dwHaveActionCell = BaseActivedPackage + m_Property.m_wExtGoodsActivedNum;		//获取已经激活的背包数
										if (dwHaveActionCell >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CPACKAGENUM])
											return TRUE;
		}
			break;
		case GLORY_CONFINISHGLORYNUMID:	//完成x个荣耀
		{
											int CompleteGloryNum = _L_GetLuaValue("ReturnGloryCompleteNum");	//获取完成的荣耀数量
											if (CompleteGloryNum >= lpGloryBaseData->m_GetConditionFactor[GloryConditionData::GLORY_CFINISHGLORYNUM])
												return TRUE;
		}
			break;
		default:
			break;
		}
		return FALSE;
	}
	return FALSE;
}

//获取荣耀奖励
void CPlayer::GetGloryAward(struct SQGetGloryMsg *pMsg)
{
	g_Script.SetCondition(NULL, this, NULL);
	LuaFunctor(g_Script, FormatString("GetGloryAward"))[pMsg->m_GloryID][this->GetGID()]();
	g_Script.CleanCondition();
}

//处理称号消息
void CPlayer::OnHandleTitleMsg(struct SQTitleMsg *pMsg)
{
	g_Script.SetCondition(NULL, this, NULL);
	LuaFunctor(g_Script, FormatString("HandleTitleMsg"))[pMsg->m_MsgType][pMsg->m_TitleID][this->GetGID()]();
	g_Script.CleanCondition();
}

void CPlayer::RecoverPlayer(CPlayer *pPlayer)
{
	if (pPlayer)
	{
		// 恢复玩家道具
		SPackageItem *pItems = m_Property.m_BaseGoods;
		memset(pItems, 0, PackageAllCells * sizeof(SPackageItem));
		memcpy(pItems, pPlayer->m_Property.m_BaseGoods, PackageAllCells * sizeof(SPackageItem));
		SetPlayerInitItems(this, true);
		//SetPlayerInitItems(this, false);
	}

}

void CPlayer::SaveData()
{
	m_Property.m_X = m_curX;
	m_Property.m_Y = m_curY;
	m_Property.m_dir = m_Direction;
	m_Property.m_CurHp = m_CurHp;
	m_Property.m_CurMp = m_CurMp;
	m_Property.m_CurTp = m_CurTp;
	m_Property.m_PKValue = m_wPKValue;
	m_Property.m_FashionMode = m_bFashionMode;
	m_Property.m_bStopTime = 1;			//更新新玩家数据
	//buff 数据的记录
	//SetBuffSaveData();
	//// 玩家离线时间保存
	//m_Property.m_LeaveTime = timeGetTime();

	// 保存玩家道具
	SPackageItem *pItems = m_Property.m_BaseGoods;
	memset(pItems, 0, PackageAllCells * sizeof(SPackageItem));
	for (DWORD i = 0; i < PackageAllCells; i++, pItems++)
	{
		SPackageItem *curItem = FindItemByPos(i, XYD_FT_WHATEVER);
		if (curItem)
			*pItems = *curItem;
	}

	///保存玩家好友
//	SavePlayerRelations_new(&m_Property);

	//保存玩家送花记录
	//SavePlayerRoseRecod(&m_Property);

	// 任务的填充
	SPlayerTask *pTask = m_Property.m_PlayerTask;
	memset(pTask, 0, sizeof(m_Property.m_PlayerTask));

	int TaskPos = 0;


	for (CRoleTaskManager::RTLIST::iterator it = m_TaskManager.m_taskList.begin(); it != m_TaskManager.m_taskList.end(); ++it)
	{
		pTask[TaskPos].m_TaskID = it->m_Index;
		pTask[TaskPos].m_CreateTime = it->m_CreateTime;
		pTask[TaskPos].m_FlagCount = it->m_flagList.size();

		if (pTask[TaskPos].m_FlagCount > TASK_MAX)
			rfalse("[严重错误]：任务%d的旗标数为0或者大于8，当前值为%d", pTask[TaskPos].m_TaskID, pTask[TaskPos].m_FlagCount);

		int FlagPos = 0;
		for (CRoleTask::TELIST::iterator ti = it->m_flagList.begin(); ti != it->m_flagList.end(); ++ti)
		{
			pTask[TaskPos].m_Flags[FlagPos++] = (*ti);
		}

		TaskPos++;
	}
}


//add by ly 2014/4/16
BOOL CPlayer::DispatchDailyMessage(struct SDailyMsg *pMsg)	//日常活动相关消息
{
	switch (pMsg->_protocol)
	{
	case SDailyMsg::EPRO_DAILY_INIT:
		OnHandleInitDailyMsg((SQInitDailyMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_DAILY_ENTRY:
		OnHandleEntryDaily((SQEntryDailyMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_DAILY_AWARD:
		OnHandleAwardDaily((SQDailyAwardMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_DAILY_LEAVE:
		OnHandleLeaveDaily((SQDailyLeaveMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_FIRSTPAY_AWARD:	//玩家首次充值奖励
		OnHandleGetFirstPayAward((SQFirstPayAwardMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_EVERYDAYPAY_AWARD:	//玩家领取每日充值奖励
		OnHandleGetEveryDayPayAward((SQEveryDayPayAwardMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_EVERYDAYPAY_GETAWARDSTATE:	//获取每日充值奖励状态
		OnEveryDayPayAwardState((SQGetEveryDayAwardStateMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_DAILY_JULONGSHANRISK:		//玩家请求在巨龙山中探险
		OnQuestJuLongShanRisk((SQJuLongShanRiskMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_TAROT_TURNONONECARD:	//玩家请求翻开一张塔罗牌
		OnTurnOnTarotCard((SQTurnOnOneTarotCardMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_TAROT_GETAWARD:	//玩家请求塔罗牌奖
		OnGetTarotAward((SQGetTarotAwardMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_TAROT_UPTPLAYANIMATEFLAG:	//更该塔罗牌播放动画标志
		OnUptPlayAnimateFlag((SQUptTarotPlayAnimateFlagMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_CURDAYONLINE_TIMELONG:		//玩家请求每日在线时长活动玩家当天在线时长
		OnGetCurDayOnlineLong((SQCurDayOnlineLongMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_CURDAYONLINE_GETAWARD:		//获取对应时间段的奖励
		OnCurDayOnlineGetAward((SQCurDayOnlineGetAwardMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_ADDUPLOGIN_ADDUPDAY:	//活动玩家当天累计登陆天数
		OnQuestAddUpLoginDay((SQAddUpLoginDayMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_ADDUPLOGIN_GETAWARD:	//玩家领取累计登陆相应级别ID的奖励
		OnAddUpLoginGetAward((SQAddUpLoginGetAwardMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_RFB_GETAWARD:	//领取充值回馈对应的奖励
		OnGetRFBAward((SQRFBGetAwardMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_LR_GETAWARD:	//获取等级竞赛奖励
		OnGetLRAward((SQLRGetAwardMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_FW_GETOPENRESIDUETIME: //获取打开幸运摩天轮的开放剩余时间数据
		OnGetFWOpenResidueTime((SQFWOpenResidueTimeMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_FW_STARTGAME: //开始幸运摩天轮游戏
		OnStartFWGame((SQFWStartGameMsg*)pMsg);
		break;
	case SDailyMsg::EPRO_FW_GETAWARD:	//获取幸运摩天轮奖励
		OnGetFWAward((SQFWGetAwardMsg*)pMsg);
		break;
	}
	return TRUE;
}


void CPlayer::SendDailyMsgToPlayer(struct SAInitDailyMsg *pMsg)	//发送日常活动列表信息
{
	int AllBufSize = sizeof(SAInitDailyMsg) - 2 * sizeof(void*) + (sizeof(unsigned short) + sizeof(BYTE)) * pMsg->m_DailyArrSize;
	int FirstDataPos = sizeof(SAInitDailyMsg)-2 * sizeof(void*);
	int SecondDataPos = sizeof(SAInitDailyMsg)-2 * sizeof(void*)+sizeof(unsigned short)* pMsg->m_DailyArrSize;
	byte *pBuf = new byte[AllBufSize];
	memset(pBuf, 0, AllBufSize);
	memcpy(pBuf, pMsg, sizeof(SAInitDailyMsg)-2 * sizeof(void*));
	memcpy(pBuf + FirstDataPos, pMsg->m_pDailyDetail, sizeof(unsigned short)* pMsg->m_DailyArrSize);
	memcpy(pBuf + SecondDataPos, pMsg->m_pDailyTotalTimes, sizeof(BYTE)* pMsg->m_DailyArrSize);
	g_StoreMessage(m_ClientIndex, pBuf, AllBufSize);
	delete[] pBuf;
	pBuf = NULL;
}

void CPlayer::OnHandleInitDailyMsg(struct SQInitDailyMsg *pMsg)	//初始化日常活动消息
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnInitDailyListInf"))
	{
		g_Script.PushParameter(0);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnHandleEntryDaily(struct SQEntryDailyMsg *pMsg)	//进入日常活动消息
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnQuestInterDailyScene"))
	{
		g_Script.PushParameter(pMsg->m_DailyID);
		g_Script.PushParameter(pMsg->m_IsFreeDaily);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();

	//g_Script.SetCondition(NULL, this, NULL, NULL);
	//if (pMsg->m_IsFreeDaily == 1)	//付费进入，先结算
	//{
	//	int rs = 0;
	//	lite::Variant ret;
	//	LuaFunctor(g_Script, FormatString("SpendEntryDailyCal"))[pMsg->m_DailyID](&ret);
	//	if (lite::Variant::VT_INTEGER == ret.dataType)
	//		rs = ret;
	//	else
	//		return;
	//	if (rs == 0)	//金币不足
	//		return;
	//}
	//LuaFunctor(g_Script, FormatString("OnQuestInterDailyScene"))[pMsg->m_DailyID][pMsg->m_IsFreeDaily]();
	//g_Script.CleanCondition();
}


void CPlayer::OnHandleAwardDaily(struct SQDailyAwardMsg *pMsg)	//请求获取日常活动的奖励
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("RequestDailyAward"))
	{
		g_Script.PushParameter(pMsg->m_DailyID);
		g_Script.PushParameter(this->GetGID());	//当前玩家的gid
		g_Script.PushParameter(GetSceneRegionID()); //所在地图gid
		g_Script.Execute();
	}
	g_Script.CleanPlayer();

	//g_Script.SetCondition(NULL, this, NULL, NULL);
	//LuaFunctor(g_Script, FormatString("RequestDailyAward"))[pMsg->m_DailyID]();
	//g_Script.CleanCondition();
}

void CPlayer::OnHandleLeaveDaily(struct SQDailyLeaveMsg *pMsg)	//客户端请求离开日常活动场景
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnRecvSQLeaveDailyScene"))
	{
		g_Script.PushParameter(GetGID());
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}


void CPlayer::OnHandleGetFirstPayAward(struct SQFirstPayAwardMsg *pMsg)//玩家首次充值奖励
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetFirstPayAward"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnHandleGetEveryDayPayAward(struct SQEveryDayPayAwardMsg *pMsg) //玩家领取每日充值奖励
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetEveryDayPayAward"))
	{
		g_Script.PushParameter(pMsg->m_AwardIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnEveryDayPayAwardState(struct SQGetEveryDayAwardStateMsg* pMsg)//获取每日充值奖励状态
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetEveryDayPayAwardState"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnQuestJuLongShanRisk(struct SQJuLongShanRiskMsg *pMsg)	//玩家请求在巨龙山中探险
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("JuLongShanRisk"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnTurnOnTarotCard(struct SQTurnOnOneTarotCardMsg* pMsg)	//玩家请求翻开一张塔罗牌
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("TurnOnOneTarotCard"))
	{
		g_Script.PushParameter(pMsg->m_TurnOnTarotPos);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnGetTarotAward(struct SQGetTarotAwardMsg* pMsg)	//玩家请求塔罗牌奖
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetTurnOnTarotAward"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnUptPlayAnimateFlag(struct SQUptTarotPlayAnimateFlagMsg* pMsg)	//更该塔罗牌播放动画标志
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("UptTarotPlayAnimateFlag"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnGetCurDayOnlineLong(struct SQCurDayOnlineLongMsg* pMsg) 		//玩家请求每日在线时长活动玩家当天在线时长
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetCurDayOnlineLong"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnCurDayOnlineGetAward(struct SQCurDayOnlineGetAwardMsg* pMsg)	//获取对应时间段的奖励
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetCurDayOnlineAward"))
	{
		g_Script.PushParameter(pMsg->m_CurDayOnlineLevelID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnQuestAddUpLoginDay(struct SQAddUpLoginDayMsg* pMsg)			//活动玩家当天累计登陆天数
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetAddupLoginTime"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnAddUpLoginGetAward(struct SQAddUpLoginGetAwardMsg* pMsg)	//玩家领取累计登陆相应级别ID的奖励
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetAddUpLoginAward"))
	{
		g_Script.PushParameter(pMsg->m_AddUpLoginLevelID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnGetRFBAward(struct SQRFBGetAwardMsg* pMsg) 	//领取充值回馈对应的奖励
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetSpecRFBAward"))
	{
		g_Script.PushParameter(pMsg->m_AwardID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnGetLRAward(struct SQLRGetAwardMsg* pMsg)	//获取等级竞赛奖励
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("PlayerGetLevelRaceAward"))
	{
		g_Script.PushParameter(pMsg->m_DataID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnGetFWOpenResidueTime(struct SQFWOpenResidueTimeMsg* pMsg) 	//获取打开幸运摩天轮的开放剩余时间数据
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetFerrisWheelCountDownInf"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnStartFWGame(struct SQFWStartGameMsg* pMsg) 	//开始幸运摩天轮游戏
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("PlayerPlayFerrisWheel"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnGetFWAward(struct SQFWGetAwardMsg* pMsg)	//获取幸运摩天轮奖励
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetFerrisWheelAward"))
	{
		g_Script.PushParameter(pMsg->m_SerialID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

//add by ly 2014/4/28
BOOL CPlayer::OnHandleGetSignInAward(struct SQGetSignInAwardMsg *pMsg)	//处理请求获取每日签到奖励消息
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnHandleGetSignInAwardOpt"))
	{
		g_Script.PushParameter(pMsg->m_EveryDaySignInIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
	//g_Script.SetCondition(NULL, this, NULL, NULL);
	//LuaFunctor(g_Script, FormatString("OnHandleGetSignInAwardOpt"))[pMsg->m_EveryDaySignInIndex]();
	//g_Script.CleanCondition();
	return TRUE;
}

//add by ly 2014/5/7
void CPlayer::DispatchShopMessage(SShopMsg* pMsg)	//商城相关信息处理
{
	switch (pMsg->_protocol)
	{
	case SShopMsg::EPRO_REQUEST_SHOP:
		OnHandleQuestShopListOpt((SQShopListMsg*)pMsg);
		break;
	case SShopMsg::EPRO_QUEST_SPECIFYSHOP:
		OnHandleQuestGoodsListOpt((SQShopGoodsListMsg*)pMsg);
		break;
	case SShopMsg::EPRO_BUY_GOODS:
		OnHandleBuyGoods((SQBuyGoodsMsg*)pMsg);
		break;
	case SShopMsg::EPRO_SHOP_COUNTDOWN:
		OnHandleGetShopCountDown((SQGetShopCountDownMsg*)pMsg);
		break;
	case SShopMsg::EPRO_SHOP_FINDGOODSBYITERMID:
		OnHandleFindGoodsByItemID((SQFindGoodsMsg*)pMsg);
		break;
	case SShopMsg::EPRO_PLAYER_PAY:
		OnHandlePlayerPay((SQPlayerPayMsg*)pMsg);
		break;
	default:
		break;
	}
}

void CPlayer::OnHandleQuestShopListOpt(struct SQShopListMsg* pMsg)		//处理玩家请求商城信息
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("QuestShopListInfo"))();
	g_Script.CleanCondition();
}


void CPlayer::OnHandleQuestGoodsListOpt(struct SQShopGoodsListMsg* pMsg)		//处理玩家请求商城中商品列表信息
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("QuestShopGoodsData"))[pMsg->m_ShopType]();
	g_Script.CleanCondition();
}

void CPlayer::OnHandleBuyGoods(struct SQBuyGoodsMsg* pMsg)				//处理玩家购买商品
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("BuyShopGoods"))[pMsg->m_BuyType][pMsg->m_GoodsIndex][pMsg->m_BuyNum]();
	g_Script.CleanCondition();
}

void CPlayer::OnHandleGetShopCountDown(struct SQGetShopCountDownMsg* pMsg)	//获取商城刷新倒计时
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("HandleQuestShopCountDown"))[pMsg->m_ShopType]();
	g_Script.CleanCondition();
}

void CPlayer::SendShopListInfo(struct SAShopListMsg* pData)	//发送商城数据
{
	int HeaderDataSize = sizeof(SAShopListMsg)-sizeof(void*);	//消息头数据大小
	int DataByteSize = sizeof(GoodsData)* pData->m_ShopNum;		//有效数据大小
	int TatolSize = HeaderDataSize + DataByteSize;
	BYTE *pBuf = new BYTE[TatolSize];
	memcpy(pBuf, pData, HeaderDataSize);
	if (pData->m_lpShopData != NULL)	//商城数据
	{
		memcpy(pBuf + HeaderDataSize, pData->m_lpShopData, DataByteSize);
	}
	g_StoreMessage(m_ClientIndex, pBuf, TatolSize);
	delete[] pBuf;
	pBuf = NULL;
}


void CPlayer::SendGoodsListInfo(struct SAShopGoodsListMsg* pData)	//发送指定商城的商品数据
{
	int HeaderDataSize = sizeof(SAShopListMsg)-sizeof(void*);	//消息头数据大小
	int DataByteSize = sizeof(GoodsData)* pData->m_GoodsNum;		//有效数据大小
	int TatolSize = HeaderDataSize + DataByteSize;
	BYTE *pBuf = new BYTE[TatolSize];
	memcpy(pBuf, pData, HeaderDataSize);
	if (pData->m_lpGoodsData != NULL)	//商品数据
	{
		memcpy(pBuf + HeaderDataSize, pData->m_lpGoodsData, DataByteSize);
	}
	g_StoreMessage(m_ClientIndex, pBuf, TatolSize);
	delete[] pBuf;
	pBuf = NULL;
}

void CPlayer::SendBuyGoodsResult(struct SABuyGoodsMsg* pData)		//发送购买操作结果
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SABuyGoodsMsg));
}

void CPlayer::SendNotityGoodsUpdated(struct SANotifySpecialGoodsUpdateMsg* pData)	//通知特价商品已经刷新
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SANotifySpecialGoodsUpdateMsg));
}

void CPlayer::SendShopCountDown(struct SAGetShopCountDownMsg* pData)	//发送商城刷新倒计时
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SAGetShopCountDownMsg));
}


void CPlayer::OnHandleFindGoodsByItemID(struct SQFindGoodsMsg* pMsg)	////请求获取商品信息索引通过道具的ID
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("FindGoodsByItemID"))[pMsg->m_ItemID]();
	g_Script.CleanCondition();
}

void CPlayer::SendSigleGoodsInfo(struct SAFindGoodsMsg* pData)	//发送单个物品的信息
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SAFindGoodsMsg));
}

void CPlayer::OnHandlePlayerPay(struct SQPlayerPayMsg* pMsg)	//处理玩家充值金币消息
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("PlayerPayGlod"))
	{
		g_Script.PushParameter(pMsg->m_PayCount);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

//add by ly 2014/5/17 Vip相关
void CPlayer::DispatchVipMessage(SVIPMsg *pMsg)	//Vip相关消息处理
{
	switch (pMsg->_protocol)
	{
	case SVIPMsg::EPRO_VIP_INITINFO:
		OnHandleInitVipDataOpt((SQInitVipInfMsg*)pMsg);
		break;
	case SVIPMsg::EPRO_VIP_GETGIFTINFO:
		OnHandleGetGiftInfOpt((SQGetVipGiftInfMsg*)pMsg);
		break;
	case SVIPMsg::EPRO_VIP_GETGIFT:
		OnHandleGetGiftOpt((SQGetVipGiftMsg*)pMsg);
		break;
	default:
		break;
	}
}

void CPlayer::OnHandleInitVipDataOpt(struct SQInitVipInfMsg *pMsg)//初始化VIP信息
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("InitVipAndSendState"))[0]();
	g_Script.CleanCondition();
}

void CPlayer::OnHandleGetGiftInfOpt(struct SQGetVipGiftInfMsg *pMsg)//获取礼包信息
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("GetGiftInfOpt"))[pMsg->m_VipLevel][pMsg->m_GetType]();
	g_Script.CleanCondition();
}

void CPlayer::OnHandleGetGiftOpt(struct SQGetVipGiftMsg *pMsg)//获取VIP礼包
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("GetOrBuyGiftOpt"))[pMsg->m_VipLevel][pMsg->m_GetType]();
	g_Script.CleanCondition();
}

void CPlayer::SendVipStateInfOpt(struct SAInitVipInfMsg *pData)		//发送玩家VIP的状态信息
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SAInitVipInfMsg));
}

void CPlayer::SendGiftInfoOpt(struct SAGetVipGiftInfMsg *pData)	//发送礼包数据信息
{
	int HeaderDataSize = sizeof(SAGetVipGiftInfMsg) - 2 * sizeof(void*);	//消息头数据大小
	int DataByteSize1 = sizeof(DWORD) * pData->m_ItemSize;		//有效数据大小
	int DataByteSize2 = sizeof(BYTE) * pData->m_ItemSize;		//有效数据大小
	int TatolSize = HeaderDataSize + DataByteSize1 + DataByteSize2;
	BYTE *pBuf = new BYTE[TatolSize];
	memcpy(pBuf, pData, HeaderDataSize);
	if (pData->m_pItemID != NULL)	//商品数据
	{
		memcpy(pBuf + HeaderDataSize, pData->m_pItemID, DataByteSize1);
	}
	if (pData->m_pItemIDNum != NULL)	//每个商品的数量
	{
		memcpy(pBuf + HeaderDataSize + DataByteSize1, pData->m_pItemIDNum, DataByteSize2);
	}
	g_StoreMessage(m_ClientIndex, pBuf, TatolSize);
	delete[] pBuf;
	pBuf = NULL;
}

void CPlayer::SendGetOrBuyGiftResOpt(struct SAGetVipGiftMsg *pData)		//发送领取或获取VIP礼包结果信息
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SAGetVipGiftMsg));
}


float CPlayer::GetWalkSpeed()
{
	return m_Speed;
}




//add by ly 2014 宠物相关
void CPlayer::DispatchPetMessage(struct SPetMsg *pMsg)		//宠物相关消息处理
{
	switch (pMsg->_protocol)
	{
	case SPetMsg::EPRO_PET_EXTRACT:
		OnHandleCardExtract((SQExtractPetMsg*)pMsg);
		break;
	case SPetMsg::EPRO_PET_CHIPCOMPOSE:
		OnHandleChipCompose((SQChipComposPetMsg*)pMsg);
		break;
	case SPetMsg::EPRO_PET_MERGER:
		OnHandlePetMerger((SQMergerPetMsg*)pMsg);
		break;
	case SPetMsg::EPRO_PET_BREACH:
		OnHandlePetBreach((SQPetBreachStarMsg*)pMsg);
		break;
	case SPetMsg::EPRO_PET_RENAME:
		OnHandlePetRename((SQRenamePetMsg*)pMsg);
		break;
	case SPetMsg::EPRO_PET_SKILLUPGRADE:
		PetSkillUpdate((SQPetSkillUpgradeMsg*)pMsg);
		break;
	case SPetMsg::EPRO_PET_SUISHENFIGHT:
		OnHandlePetFollowFight((SQSuiShenFightMsg*)pMsg);
		break;
	case SPetMsg::EPRO_PET_SHAPESHIFTFIGHT:
		OnHandlePetShapeshiftFight((SQShapeshiftFightMsg*)pMsg);
		break;
	case SPetMsg::EPRO_PET_GETCARDPETINFO:
		OnHandleGetCardPetInfo((SQGetCardPetInf*)pMsg);
		break;
	case SPetMsg::EPRO_PET_USEPETITEM:
		break;
	case SPetMsg::EPRO_PET_GETPETCARDINFO:
		OnHandleGetPetCardInfo((SQGetPetCardInf*)pMsg);
		break;
	case  SPetMsg::EPRO_PET_SKILLCONFIG:
		OnHandleSkillConfig((SQPetSkillCfgInf*)pMsg);
		break;
	case SPetMsg::EPRO_PET_GETEXPINFO:
		OnHandleGetPetExpInf((SQPetExpInfMsg*)pMsg);
		break;
	case SPetMsg::EPRO_PET_GLODBUY:
		OnHandleBuyPet((SQGlodDirectBuyPetMsg*)pMsg);
		break;
	case SPetMsg::EPRO_PET_RESUMEDURABLENEEDGOLD:
		GetResumeDurableNeedGlod((SQResumeNdGoldMsg *)pMsg);	//获取恢复宠物魂力需要的金币数
		break;
	case SPetMsg::EPRO_PET_GOLDRESUMEDURABLE:
		ExcResumePetDurable((SQResumePetDurableMsg *)pMsg);	//花费金币恢复宠物魂力
		break;
	case SPetMsg::EPRO_PET_STUDYSKILL:
		OnHandlePetStudySkill((SQPetStudySkillMsg *)pMsg);
		break;
	//case SPetMsg::EPRO_PET_OPENSTUDYSKILLPLAN:
	//	break;
	default:
		break;
	}
}


void CPlayer::OnHandleCardExtract(struct SQExtractPetMsg *pMsg)		//处理玩家卡牌抽取操作
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("ExtractPet"))[pMsg->m_ExtractMode]();
	g_Script.CleanCondition();
}

void CPlayer::OnHandleChipCompose(struct SQChipComposPetMsg *pMsg)	//处理玩家碎片合成宠物操作
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("ClipComposePet"))[pMsg->m_PetID]();
	g_Script.CleanCondition();
}

void CPlayer::OnHandleGetCardPetInfo(struct SQGetCardPetInf *pMsg)	//处理获取卡牌宠物列表信息
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("GetCardPetInfo"))[pMsg->m_CardType]();
	g_Script.CleanCondition();
}

void CPlayer::OnHandlePetMerger(struct SQMergerPetMsg* pMsg)	//处理宠物吞噬
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("PetSwallowPets"))
	{
		g_Script.PushParameter(pMsg->m_SrcPetIndex);
		for (int i = 0; i < 5; i++)
			g_Script.PushParameter(pMsg->m_DestPetIndex[i]); 
		g_Script.PushParameter(pMsg->m_ExpRateItemID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnHandlePetBreach(struct SQPetBreachStarMsg* pMsg)	//处理宠物突破
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("PetBreach"))
	{
		g_Script.PushParameter(pMsg->m_PetIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnHandlePetRename(struct SQRenamePetMsg* pMsg)	//处理宠物更改名字
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("PetRename"))
	{
		g_Script.PushParameter(pMsg->m_PetIndex);
		g_Script.PushParameter(pMsg->m_NewName);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnHandleSkillConfig(struct SQPetSkillCfgInf* pMsg)	//宠物技能配置
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("PetSkillCfgOpt"))
	{
		g_Script.PushParameter(pMsg->m_PetIndex);
		for (int i = 0; i < PETMAXUSESKILLNUM; i++)
			g_Script.PushParameter(pMsg->m_SkillIndex[i]);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnHandlePetFollowFight(struct SQSuiShenFightMsg *pMsg)	//处理宠物跟随战斗
{
	SASuiShenFightMsg SuiShenFightRes;
	LPSNewPetData pPetData = GetPetByIndex(pMsg->m_PetIndex);
	if (pPetData == NULL)
	{
		m_Property.m_FollowPetIndex = 0xff;
		SuiShenFightRes.m_Res = 0;
		g_StoreMessage(m_ClientIndex, &SuiShenFightRes, sizeof(SASuiShenFightMsg));
		return;
	}
	if (pMsg->m_PetIndex == m_Property.m_TransPetIndex)
		m_Property.m_TransPetIndex = 0xff;
	m_Property.m_FollowPetIndex = pMsg->m_PetIndex;
	SuiShenFightRes.m_Res = 1;
	g_StoreMessage(m_ClientIndex, &SuiShenFightRes, sizeof(SASuiShenFightMsg));
}

void CPlayer::OnHandlePetShapeshiftFight(struct SQShapeshiftFightMsg *pMsg)	//处理变身宠物战斗
{
	SAShapeshiftFightMsg ShapeshiftFightRes;
	LPSNewPetData pPetData = GetPetByIndex(pMsg->m_PetIndex);
	if (pPetData == NULL)
	{
		m_Property.m_TransPetIndex = 0xff;
		ShapeshiftFightRes.m_Res = 0;
		g_StoreMessage(m_ClientIndex, &ShapeshiftFightRes, sizeof(SAShapeshiftFightMsg));
		return;
	}
	if (pMsg->m_PetIndex == m_Property.m_FollowPetIndex)
		m_Property.m_FollowPetIndex = 0xff;
	m_Property.m_TransPetIndex = pMsg->m_PetIndex;
	ShapeshiftFightRes.m_Res = 1;
	g_StoreMessage(m_ClientIndex, &ShapeshiftFightRes, sizeof(SAShapeshiftFightMsg));
}

void CPlayer::SendExtractPetRes(struct SAExtractPetMsg* pData)		//发送抽取到的宠物数据
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SAExtractPetMsg));
}

void CPlayer::SendClipComposePetRes(struct SAChipComposPetMsg* pData)	//发送碎片合成的宠物数据
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SAChipComposPetMsg));
}

void CPlayer::SendCardPetInfo(struct SAGetCardPetInf* pData)	//发送卡牌宠物列表信息
{
	int HeaderDataSize = sizeof(SAGetCardPetInf)-sizeof(void*);	//消息头数据大小
	int DataByteSize = sizeof(DWORD)* pData->m_PetCount;		//有效数据大小
	int TatolSize = HeaderDataSize + DataByteSize;
	BYTE *pBuf = new BYTE[TatolSize];
	memcpy(pBuf, pData, HeaderDataSize);
	if (pData->m_pPetID != NULL)	//
	{
		memcpy(pBuf + HeaderDataSize, pData->m_pPetID, DataByteSize);
	}
	g_StoreMessage(m_ClientIndex, pBuf, TatolSize);
	delete[] pBuf;
	pBuf = NULL;
}

BOOL CPlayer::UseItemGetPet(struct SPackageItem *pItem)		//使用道具获得宠物
{
	if (!pItem || 0 == pItem->wIndex || 0 == pItem->overlap)
		return FALSE;
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("UsePetItemGet"))[pItem->wIndex]();
	g_Script.CleanCondition();
}

void CPlayer::SendUseItemGetPet(struct SAUsePetItem *pData)	//发送使用宠物道具获得的宠物数据
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SAUsePetItem));
}

void CPlayer::OnHandleGetPetCardInfo(struct SQGetPetCardInf *pMsg)	//处理获取宠物卡牌信息
{
	g_Script.SetCondition(NULL, this, NULL, NULL);
	LuaFunctor(g_Script, FormatString("GetPetCardInfo"))();
	g_Script.CleanCondition();
}

void CPlayer::SendPetCardInfo(struct SAGetPetCardInf *pData)	//发送宠物卡牌信息
{
	int HeaderDataSize = sizeof(SAGetPetCardInf)-3 * sizeof(void*);	//消息头数据大小
	int DataByteSize1 = sizeof(BYTE)* pData->m_CardCount;		//有效数据大小
	int DataByteSize2 = sizeof(DWORD)* pData->m_CardCount;		//有效数据大小
	int DataByteSize3 = sizeof(DWORD)* pData->m_CardCount;		//有效数据大小
	int TatolSize = HeaderDataSize + DataByteSize1 + DataByteSize2 + DataByteSize3;
	BYTE *pBuf = new BYTE[TatolSize];
	memcpy(pBuf, pData, HeaderDataSize);
	if (pData->m_pCardCanExtractTimes != NULL)	//使用一次卡牌可以抽取的宠物的次数
	{
		memcpy(pBuf + HeaderDataSize, pData->m_pCardCanExtractTimes, DataByteSize1);
	}
	if (pData->m_pUsePrice != NULL)	//抽取价格
	{
		memcpy(pBuf + HeaderDataSize + DataByteSize1, pData->m_pUsePrice, DataByteSize2);
	}
	if (pData->m_pCardCountDown != NULL)	//卡牌刷新倒计时 到倒计时不为0时，表示不可以抽取操作
	{
		memcpy(pBuf + HeaderDataSize + DataByteSize1 + DataByteSize2, pData->m_pCardCountDown, DataByteSize3);
	}
	g_StoreMessage(m_ClientIndex, pBuf, TatolSize);
	delete[] pBuf;
	pBuf = NULL;
}


LPSNewPetData CPlayer::GetPetByID(DWORD PetID)	//获取宠物数据通过宠物ID
{
	for (BYTE i = 0; i < MAX_NEWPET_NUM; i++)
	{
		if (m_Property.m_NewPetData[i].m_PetID == PetID)
			return &m_Property.m_NewPetData[i];
	}
	return NULL;
}

LPSNewPetData CPlayer::GetPetByIndex(BYTE PetIndex)	//获取宠物数据通过宠物索引
{
	if (m_Property.m_NewPetData[PetIndex].m_PetID == 0)
		return NULL;
	return &m_Property.m_NewPetData[PetIndex];
}

BOOL CPlayer::AddPet(LPSNewPetData pPetData)	//添加宠物
{
	for (int i = 0; i < MAX_NEWPET_NUM; i++)
	{
		if (m_Property.m_NewPetData[i].m_PetID == 0)
		{
			memcpy(&m_Property.m_NewPetData[i], pPetData, sizeof(SNewPetData));
			m_Property.m_CurPetNum++;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CPlayer::DeletePet(BYTE PetIndex)		//删除指定索引的宠物
{
	if (PetIndex == -1)
		return FALSE;
	--m_Property.m_CurPetNum;
	//memcpy(&m_Property.m_NewPetData[PetIndex], &m_Property.m_NewPetData[m_Property.m_CurPetNum], sizeof(SNewPetData));
	memset(&m_Property.m_NewPetData[PetIndex], 0, sizeof(SNewPetData));
	return TRUE;
}

void CPlayer::SendPetMergerRes(struct SAMergerPetMsg* pData)	//发送合并后的宠物数据
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SAMergerPetMsg));
}

void CPlayer::SendPetBreachRes(struct SAPetBreachStarMsg* pData)	//发送宠物突破结果数据
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SAPetBreachStarMsg));
}

void CPlayer::SendPetRenameRes(struct SARenamePetMsg* pData)	//发送宠物重命名是否成功
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SARenamePetMsg));
}

void CPlayer::PetSkillUpdate(struct SQPetSkillUpgradeMsg* pMsg)		//宠物技能升级
{
	LPSNewPetData pPetData = GetPetByIndex(pMsg->m_PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(m_ClientIndex, "没有该宠物");
		return;
	}

	DWORD SkillID = pPetData->m_PetSkill[pMsg->m_PetSkillIndex].m_PetSkillID;

	const SSkillBaseData *pData = CSkillService::GetInstance().GetSkillBaseData(SkillID);

	if (!pData)
	{
		rfalse("技能配置表中找不到该ID！");
		return;
	}

	WORD pos = -1;
	for (int i = 0; i < 10; ++i)
	{
		if (pPetData->m_PetSkill[i].m_PetSkillID == SkillID)
		{
			pos = i;
			break;
		}
	}

	if (-1 == pos)
	{
		rfalse(1, 2, "技能ID不存在！");
		return;
	}

	// 得到当前技能等级
	WORD skillLevel = pPetData->m_PetSkill[pos].m_Level;

	if (MaxSkillLevel == skillLevel)
	{
		TalkToDnid(m_ClientIndex, "该技能已经满级,无法继续升级!");
		return;
	}

	// 交易中不能升级
	if (InExchange())
	{
		TalkToDnid(m_ClientIndex, "亲~交易中不能升级技能哦！");
		return;
	}

	// 普通攻击退出
	if (SPT_COMMAN == pData->m_SkillPropertyType)
		return;

	//玩家等级没有达到升级该技能的要求
	if (pPetData->m_PetLevel < pData->m_NeedPlayerLevel)
	{
		TalkToDnid(m_ClientIndex, "宠物等级不够，不能升级该技能！");
		return;
	}

	// 计算技能组升级消耗 
	g_Script.SetCondition(NULL, this, NULL);
	//这是之前调用的函数
	/*LuaFunctor(g_Script, FormatString("UpdateSkill%d", pData->m_SkillPropertyType))[skillLevel]();*/
	LuaFunctor(g_Script, "UpdateSkill")[skillLevel][pData->m_UpNeedSp][pData->m_UpNeedMoney]();
	g_Script.CleanCondition();

	if (m_updateSkill)
	{
		// 增加技能等级
		++pPetData->m_PetSkill[pos].m_Level;

		MY_ASSERT(pPetData->m_PetSkill[pos].m_Level <= MaxSkillLevel);

		SAPetSkillUpgradeMsg msg;
		msg.m_Result = 1;
		g_StoreMessage(m_ClientIndex, &msg, sizeof(msg));

		m_updateSkill = false;
	}
}

BOOL CPlayer::PlayerTransferPet(BYTE PetPos)	//玩家变身为宠物操作，保存玩家之前的属性并把宠物的的属性赋值给玩家，当宠物死亡后，变回玩家之前的状态
{
	//获取宠物数据
	SNewPetData &pPet = m_Property.m_NewPetData[PetPos];
	if (0 == pPet.m_PetID)
		return FALSE;

	if (pPet.m_CurPetDurable == 0)
	{
		TalkToDnid(m_ClientIndex, "宠物魂力为0，不能变身该宠物");
		return FALSE;
	}

	if (m_Property.m_Equip[EQUIP_P_PRECIOUS].details.wIndex == 0)
	{
		TalkToDnid(m_ClientIndex, "玩家没有武器2，不能变身该宠物");
		return FALSE;
	}
	DWORD PetIndex = _L_GetLuaValue("GetPetDataIndex", pPet.m_PetID, pPet.m_PetLevel);
	if (PetIndex == 0xffffffff)
		return FALSE;
	DWORD PetFightDataIndex = _L_GetLuaValue("GetPetBaseProperties", PetIndex, 4);
	if (PetFightDataIndex == 0xffffffff)
		PetFightDataIndex = 0;
	DWORD BaseGongJi = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 9);
	if (BaseGongJi == 0xffffffff)
		BaseGongJi = 0;
	DWORD BaseFangYu = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 10);
	if (BaseFangYu == 0xffffffff)
		BaseFangYu = 0;
	DWORD BaseBaoJi = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 11);
	if (BaseBaoJi == 0xffffffff)
		BaseBaoJi = 0;
	DWORD BaseHit = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 17);
	if (BaseHit == 0xffffffff)
		BaseHit = 0;
	DWORD BaseShanBi = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 18);
	if (BaseShanBi == 0xffffffff)
		BaseShanBi = 0;
	DWORD BaseMaxHp = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 5);
	if (BaseMaxHp == 0xffffffff)
		BaseMaxHp = 0;
	DWORD BaseMaxMp = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 6);
	if (BaseMaxMp == 0xffffffff)
		BaseMaxMp = 0;
	DWORD BaseUncrit = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 12);
	if (BaseUncrit == 0xffffffff)
		BaseUncrit = 0;
	DWORD BaseWreck = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 13);
	if (BaseWreck == 0xffffffff)
		BaseWreck = 0;
	DWORD BaseUnWreck = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 14);
	if (BaseUnWreck == 0xffffffff)
		BaseUnWreck = 0;
	DWORD BasePuncture = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 15);
	if (BasePuncture == 0xffffffff)
		BasePuncture = 0;
	DWORD BaseUnPuncture = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 16);
	if (BaseUnPuncture == 0xffffffff)
		BaseUnPuncture = 0;
	//保存宠物战斗属性
	m_PetFightPro.m_MaxHp = BaseMaxHp;				// 最大生命
	m_PetFightPro.m_CurHp = BaseMaxHp;				// 当前生命
	m_PetFightPro.m_GongJi = BaseGongJi;				// 攻击
	m_PetFightPro.m_FangYu = BaseFangYu;				// 防御
	m_PetFightPro.m_BaoJi = BaseBaoJi;				// 暴击
	m_PetFightPro.m_Hit = BaseHit;					//命中
	m_PetFightPro.m_ShanBi = BaseShanBi;				// 闪避
	m_PetFightPro.m_uncrit = BaseUncrit;		//抗暴
	m_PetFightPro.m_wreck = BaseWreck;	//破击
	m_PetFightPro.m_unwreck = BaseUnWreck;				//抗破
	m_PetFightPro.m_puncture = BasePuncture;			//穿刺
	m_PetFightPro.m_unpuncture = BaseUnPuncture;			//抗穿

	//保存战斗变身之前玩家的当前血量
	if (m_ParentRegion->DynamicCast(IID_DYNAMICREGION) != NULL)
	{
		m_TranPrePlayerHp = m_CurHp;
		m_TranPrePlayerMP = m_CurMp;
	}
	else
	{
		m_TranPrePlayerHp = m_MaxHp;
		m_TranPrePlayerMP = m_CurMp;
	}

	//设置战斗宠物的数据给玩家
	m_MaxHp = BaseMaxHp;				// 最大生命

	time_t CurTime = time(NULL);
	DWORD DieTranIntervalTime = _L_GetLuaValue("ReturnPlayerDieTranLimit");
	if (pPet.m_CurPetHp == 0 && CurTime - m_RecordPreDeadCTime >= DieTranIntervalTime && m_ParentRegion->DynamicCast(IID_DYNAMICREGION) != NULL)
	{
		pPet.m_CurPetHp = BaseMaxHp;
		pPet.m_CurPetMp = BaseMaxMp;
	}

	m_CurHp = pPet.m_CurPetHp;				// 当前生命
	m_MaxMp = BaseMaxMp;
	m_CurMp = pPet.m_CurPetMp;
	m_GongJi = BaseGongJi;				// 攻击
	m_FangYu = BaseFangYu;				// 防御
	m_BaoJi = BaseBaoJi;				// 暴击
	m_Hit = BaseHit;					//命中
	m_ShanBi = BaseShanBi;				// 闪避
	m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] = BaseUncrit;		//抗暴
	m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] = BaseWreck;	//破击
	m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] = BaseUnWreck;				//抗破
	m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] = BasePuncture;			//穿刺
	m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] = BaseUnPuncture;			//抗穿

	for (size_t i = 0; i < XA_MAX_EXP; i++)
		m_FightPropertyStatus[i] = true;


	for (int i = 0; i < PETMAXUSESKILLNUM; i++)
	{
		if (pPet.m_CurUseSkill[i] >= 10)
			continue;
		WORD SkillID = pPet.m_PetSkill[pPet.m_CurUseSkill[i]].m_PetSkillID;
		BYTE SkillLevel = pPet.m_PetSkill[pPet.m_CurUseSkill[i]].m_Level;
		ActiveTempSkill(SkillID, SkillLevel);
		m_Property.m_CurUsedSkill[i + 3] = SkillID;
	}
	//激活宠物的主动攻击
	ActiveTempSkill(pPet.m_PetSkill[0].m_PetSkillID, pPet.m_PetSkill[0].m_Level);

	//更新玩家当前可使用的技能
	SANotityUpdateSkillMsg SetData;
	memcpy(SetData.m_UpdateCurUsedSkill, m_Property.m_CurUsedSkill, 10 * sizeof(DWORD));
	g_StoreMessage(m_ClientIndex, &SetData, sizeof(SANotityUpdateSkillMsg));
}

BOOL CPlayer::TransferPlayer()		//变回玩家的普通状态，如果已经是普通形态就不用变身
{
	SNewPetData &pPet = m_Property.m_NewPetData[m_Property.m_TransPetIndex];
	if (0 == pPet.m_PetID)
		return FALSE;
	for (int i = 3; i < 6; i++)
	{
		if (m_Property.m_CurUsedSkill[i] == 0)
			continue;
		WORD SkillPos = -1;
		for (int j = 0; j < MAX_SKILLCOUNT; j++)
		{
			if (m_Property.m_CurUsedSkill[i] == m_Property.m_pSkills[j].wTypeID)
			{
				SkillPos = j;
				break;
			}
		}
		if (SkillPos == -1)
			continue;
		CoolingTempSkill(SkillPos);
		m_Property.m_CurUsedSkill[i] = 0;
	}
	//g_StoreMessage(m_ClientIndex, GetStateMsg(), sizeof(SASynPlayerMsg));

	//保存变身宠物的当前血量和内力
	pPet.m_CurPetHp = m_CurHp;
	pPet.m_CurPetMp = m_CurMp;

	//更新玩家当前可使用的技能
	SANotityUpdateSkillMsg SetData;
	memcpy(SetData.m_UpdateCurUsedSkill, m_Property.m_CurUsedSkill, 10 * sizeof(DWORD));
	g_StoreMessage(m_ClientIndex, &SetData, sizeof(SANotityUpdateSkillMsg));
	m_newAddproperty[SEquipDataEx::EEA_UNCRIT - SEquipDataEx::EEA_UNCRIT] = 0;		//抗暴
	m_newAddproperty[SEquipDataEx::EEA_WRECK - SEquipDataEx::EEA_UNCRIT] = 0;	//破击
	m_newAddproperty[SEquipDataEx::EEA_UNWRECK - SEquipDataEx::EEA_UNCRIT] = 0;				//抗破
	m_newAddproperty[SEquipDataEx::EEA_PUNCTURE - SEquipDataEx::EEA_UNCRIT] = 0;			//穿刺
	m_newAddproperty[SEquipDataEx::EEA_UNPUNCTURE - SEquipDataEx::EEA_UNCRIT] = 0;			//抗穿
	UpdateBaseProperties();
	UpdateAllProperties();


	//恢复战斗变身之前玩家的当前血量	
	time_t CurTime = time(NULL);
	DWORD DieTranIntervalTime = _L_GetLuaValue("ReturnPlayerDieTranLimit");
	if (m_TranPrePlayerHp == 0 && CurTime - m_RecordPreDeadCTime >= DieTranIntervalTime && m_ParentRegion->DynamicCast(IID_DYNAMICREGION) != NULL)
	{
		m_TranPrePlayerHp = m_MaxHp;
		m_TranPrePlayerMP = m_MaxMp;
	}
	m_CurHp = m_TranPrePlayerHp;
	m_CurMp = m_TranPrePlayerMP;

	m_FightPropertyStatus[XA_CUR_HP] = true;
	m_FightPropertyStatus[XA_CUR_MP] = true;
	return TRUE;
}


void CPlayer::OnHandleGetPetExpInf(struct SQPetExpInfMsg *pMsg)	//获取吞噬宠物可以获取的经验
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetPetExpInf"))
	{
		for (int i = 0; i < 5; i++)
			g_Script.PushParameter(pMsg->m_DestPetIndex[i]); 
		g_Script.PushParameter(pMsg->m_ExpRateItemID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnHandleBuyPet(struct SQGlodDirectBuyPetMsg *pMsg)	//金币直接购买宠物
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("DircetBuyPet"))
	{
		g_Script.PushParameter(pMsg->m_PetID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::SendBuyPet(struct SAGlodDirectBuyPetMsg *pData)	//发送金币购买获得的宠物数据
{
	g_StoreMessage(m_ClientIndex, pData, sizeof(SAGlodDirectBuyPetMsg));
}

//add by ly 2014/06/20 当宠物魂力为0时，在间隔时间达到后恢复满宠物的魂力值
void CPlayer::CheckResumePetHunli()
{
	INT64 CurTime = _time64(NULL);
	DWORD NeedTatalTime = _L_GetLuaValue("GetHunliResumeTime");
	if (NeedTatalTime == 0xffffffff)
		return;
	for (int i = 0; i < MAX_NEWPET_NUM; i++)
	{
		if (m_Property.m_NewPetData[i].m_PetID != 0 && m_Property.m_NewPetData[i].m_CurPetDurable == 0)
		{
			//判断是否恢复魂力
			if (m_Property.m_NewPetData[i].m_DurableResumeNeedTime <= CurTime - m_dwLoginTime)
			{
				DWORD PetIndex = _L_GetLuaValue("GetPetDataIndex", m_Property.m_NewPetData[i].m_PetID, m_Property.m_NewPetData[i].m_PetLevel);
				if (PetIndex == 0xffffffff)
					return;

				DWORD PetFightDataIndex = _L_GetLuaValue("GetPetBaseProperties", PetIndex, 4);
				if (PetFightDataIndex == 0xffffffff)
					PetFightDataIndex = 0;

				DWORD CurDurable = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 4);
				if (CurDurable == 0xffffffff)
					CurDurable = 0;

				SAPetSynDurableMsg SynPetDurable;
				SynPetDurable.m_PetIndex = i;
				SynPetDurable.m_CurDurable = CurDurable;
				g_StoreMessage(m_ClientIndex, &SynPetDurable, sizeof(SAPetSynDurableMsg));

				m_Property.m_NewPetData[i].m_CurPetDurable = CurDurable;
				m_Property.m_NewPetData[i].m_DurableResumeNeedTime = 0;
			}
		}
	}
}

void CPlayer::GetResumeDurableNeedGlod(struct SQResumeNdGoldMsg *pMsg)	//获取恢复宠物魂力需要的金币数
{
	LPSNewPetData pPetData = GetPetByIndex(pMsg->m_PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(m_ClientIndex, "没有该宠物");
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("ResumePetDurableNeedGold"))
	{
		g_Script.PushParameter(pPetData->m_PetLevel);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::ExcResumePetDurable(struct SQResumePetDurableMsg *pMsg)	//花费金币恢复宠物魂力
{
	LPSNewPetData pPetData = GetPetByIndex(pMsg->m_PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(m_ClientIndex, "没有该宠物");
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GoldResumePetDurable"))
	{
		g_Script.PushParameter(pMsg->m_PetIndex);
		g_Script.PushParameter(pPetData->m_PetID);
		g_Script.PushParameter(pPetData->m_PetLevel);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}


void CPlayer::SynPetDurable(BYTE PetIndex)		//同步宠物魂力
{
	LPSNewPetData pPetData = GetPetByIndex(PetIndex);
	if (pPetData == NULL)
	{
		TalkToDnid(m_ClientIndex, "没有该宠物");
		return ;
	}
	SAPetSynDurableMsg SynPetDurable;
	SynPetDurable.m_PetIndex = PetIndex;
	SynPetDurable.m_CurDurable = pPetData->m_CurPetDurable;
	g_StoreMessage(m_ClientIndex, &SynPetDurable, sizeof(SAPetSynDurableMsg));
}

void CPlayer::CheckResumeFullPetHpAndMp()	//核对回复满宠物血量和内力
{
	if (!m_ParentRegion)
	{
		return;
	}
	if (m_ParentRegion->DynamicCast(IID_DYNAMICREGION) == NULL)
	{
		for (int i = 0; i < MAX_NEWPET_NUM; i++)
		{
			LPSNewPetData pPet = GetPetByIndex(i);
			if (pPet == NULL)
				continue;
			DWORD PetIndex = _L_GetLuaValue("GetPetDataIndex", pPet->m_PetID, pPet->m_PetLevel);
			if (PetIndex == 0xffffffff)
				continue;
			DWORD PetFightDataIndex = _L_GetLuaValue("GetPetBaseProperties", PetIndex, 4);
			if (PetFightDataIndex == 0xffffffff)
				PetFightDataIndex = 0;
			DWORD BaseMaxHp = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 5);
			if (BaseMaxHp == 0xffffffff)
				BaseMaxHp = 0;
			DWORD BaseMaxMp = _L_GetLuaValue("GetPetFightBaseProperties", PetFightDataIndex, 6);
			if (BaseMaxMp == 0xffffffff)
				BaseMaxMp = 0;
			if (pPet->m_CurPetHp != BaseMaxHp)
				pPet->m_CurPetHp = BaseMaxHp;
			if (pPet->m_CurPetMp != BaseMaxMp)
				pPet->m_CurPetMp = BaseMaxMp;
		}
	}
}

void CPlayer::CheckResumeFullPlayerHpAndMp()//核对回复满玩家血量和内力
{
	if (!m_ParentRegion)
	{
		return;
	}

	if (m_ParentRegion->DynamicCast(IID_DYNAMICREGION) == NULL)
	{
		if (m_MaxHp != m_CurHp)
			RestoreFullHPDirectly();
		if (m_MaxMp != m_CurMp)
			RestoreFullMPDirectly();
	}
}

void CPlayer::OnHandlePetStudySkill(SQPetStudySkillMsg *pMsg)	//处理宠物学习技能操作
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("PetStudySkillOpt"))
	{
		g_Script.PushParameter(pMsg->m_PetIndex);
		g_Script.PushParameter(pMsg->m_PetSkillBookID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

BOOL CPlayer::PetActiveSkill(BYTE PetIndex, WORD SkillID)		//宠物激活技能
{
	LPSNewPetData pPetData = GetPetByIndex(PetIndex);
	if (pPetData == NULL)
		return FALSE;
	const SSkillBaseData *pSSkillBaseData = CSkillService::GetInstance().GetSkillBaseData(SkillID);
	if (pSSkillBaseData == NULL)
		return FALSE;
	for (int i = 0; i < PETSKILLMAXNUM; i++)
	{
		if (pPetData->m_PetSkill[i].m_PetSkillID == SkillID)
			return FALSE;
	}

	int Index = 0;
	for (; Index < PETSKILLMAXNUM; Index++)
	{
		if (pPetData->m_PetSkill[Index].m_PetSkillID == 0)
			break;
	}
	if (Index == PETSKILLMAXNUM)
		return FALSE;
	pPetData->m_PetSkill[Index].m_PetSkillID = SkillID;
	pPetData->m_PetSkill[Index].m_Level = 1;
	SASynPetSkillMsg SynSkillMsg;
	SynSkillMsg.m_PetIndex = PetIndex;
	SynSkillMsg.m_PetSkillIndex = Index;
	SynSkillMsg.m_PetSkillLevel = 1;
	SynSkillMsg.m_PetSkillID = SkillID;
	g_StoreMessage(m_ClientIndex, &SynSkillMsg, sizeof(SASynPetSkillMsg));
	return TRUE;
}

//add by ly 2014/7/22 排行榜
void CPlayer::OnHandleRandListMsg(struct SQPlayerRankList *pMsg)
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetRandData"))
	{
		g_Script.PushParameter(pMsg->m_Type);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::SendRandListData(struct SAPlayerRankList *pData)		//发送排行榜列表数据
{
	WORD DataLength = pData->m_TopNSize + pData->m_PlayerTopNSize;
	DWORD HeadDatasize = sizeof(SAPlayerRankList)-sizeof(void*);
	DWORD AllDataSize = sizeof(SAPlayerRankList)-sizeof(void*)+sizeof(PlayerRankList)*DataLength;
	BYTE *pBuf = new BYTE[AllDataSize];
	memcpy(pBuf, pData, HeadDatasize);
	memcpy(pBuf + HeadDatasize, pData->m_pPlayerRankList, AllDataSize - HeadDatasize);
	g_StoreMessage(m_ClientIndex, pBuf, AllDataSize);
	if (pBuf != NULL)
		delete[] pBuf;
}

void CPlayer::OnHandleGetRankAwardState(struct SQGetRankAwardState* pMsg)	//获取玩家排行榜领奖状态
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetRankAwardFlag"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnHandleRankAwardOpt(struct SQGetRankAward* pMsg)	//请求领取奖励
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetRankAwardOpt"))
	{
		g_Script.PushParameter(pMsg->m_Type);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

//add by ly 2014/7/24 活跃度
void CPlayer::OnGetActivenessInf(struct SQGetActivenessInfo *pMsg)	//获取玩家排行榜奖励领取状态
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetActivenessInfo"))
	{
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnGetActivenessAward(struct SQGetActivenessAward *pMsg)	//领取活跃度的对应奖励
{
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetActivenessAward"))
	{
		g_Script.PushParameter(pMsg->m_AwardLevel);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::SendActivenessInf(struct SAGetActivenessInfo *pData)	//发送玩家活跃度信息
{
	WORD HeadActivenessInfSize = sizeof(SAGetActivenessInfo)-sizeof(void *)* 2;
	WORD TaskInfSize = pData->m_ActivenessTaskNum * sizeof(ActivenessTaskInf);
	WORD AwardInfSize = pData->m_ActivenessAwardNum * sizeof(ActivenessAward);
	WORD TotalSize = HeadActivenessInfSize + TaskInfSize + AwardInfSize;
	BYTE *pBuf = new BYTE[TotalSize];
	memcpy(pBuf, pData, HeadActivenessInfSize);
	memcpy(pBuf + HeadActivenessInfSize, pData->m_pTaskInfo, TaskInfSize);
	memcpy(pBuf + HeadActivenessInfSize + TaskInfSize, pData->m_pAwardInfo, AwardInfSize);
	g_StoreMessage(m_ClientIndex, pBuf, TotalSize);
	if (pBuf != NULL)
		delete[] pBuf;	
}

/////////////////////////////////////////////////////////////////////////////////////////
///三国玩家属性
/////////////////////////////////////////////////////////////////////////////////////////

void CPlayer::DispatchSGPlayer(struct SSGPlayerMsg* pMsg)
{

	switch (pMsg->_protocol)
	{
	case SSGPlayerMsg::EPRO_SANGUO_ATTR:
		OnSetPlayerAttr ((SQSetAttrMsg*) pMsg );
		return;
	case SSGPlayerMsg::EPRO_SANGUO_STORAGE:
	{
		CSanguoPlayer::OnRecvStorageOperation(pMsg);
	}
		return;
	default:
		break;
	}
	CSanguoPlayer::OnRecvSanguoPlayerRequest(pMsg);

}

void CPlayer::OnSetPlayerAttr(struct SQSetAttrMsg* pMsg)
{
	//m_FixData. = pMsg->m_AttrValue;
	switch (pMsg->m_AttrEnum)
	{	
	case SSetAttrMsg::EPRO_ATTR_STORYPLAYED ://
		m_FixData.m_dwStoryPlayed = pMsg->m_AttrValue;
		break;
	case SSetAttrMsg::EPRO_ATTR_UPDATEPLAYERICON://更新玩家头像
	{
		m_FixData.m_PlayerIconID = pMsg->m_AttrValue;
		break;
	}
	/*case SSetAttrMsg:: ://
		m_FixData = pMsg->m_A.0ttrValue;
		break;*/
	default:
		break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
///仓库消息的分发，现在的仓库管理极为简单，暂时定义为一个成员数组，对齐制作更新操作
///这里m_StroageIndex的key为物品ID，value则是m_FixData.m_ItemList[]数组的下标，
/////////////////////////////////////////////////////////////////////////////////////////
void CPlayer::DsiapatchStorage(struct SSanguoItem * pMsg)
{
	SQStroageMsg* msg = (SQStroageMsg*)pMsg;
	std::map<DWORD, DWORD>::iterator storageIt;
	storageIt = m_StroageIndex.find(msg->m_ItmeInfo.m_dwItemID);
	if (storageIt == m_StroageIndex.end())//仓库没有该物品
	{
		int keyValue = msg->m_ItmeInfo.m_dwItemID;
		int newIndex;
		if (m_StroageIndex.empty()) //如果容器为空,则下标为0
			newIndex = 0;
		else
			newIndex = m_StroageIndex.size(); //新的索引

		
		m_StroageIndex[keyValue] = newIndex;
		m_FixData.m_ItemList[newIndex].m_dwItemID = msg->m_ItmeInfo.m_dwItemID;
		m_FixData.m_ItemList[newIndex].m_dwCount = msg->m_ItmeInfo.m_dwCount;
	}
	else //仓库存在该物品
	{
		int itemIndex = storageIt->second;
		if (m_FixData.m_ItemList[itemIndex].m_dwItemID != msg->m_ItmeInfo.m_dwItemID) //对应下标的物品ID如果和消息不匹配，则报错
		{
			rfalse("玩家%s更新物品ID%d不匹配",  m_FixData.m_Name,msg->m_ItmeInfo.m_dwItemID);
			return;
		}
		m_FixData.m_ItemList[itemIndex].m_dwCount = msg->m_ItmeInfo.m_dwCount;
	}
}

void CPlayer::OnTreasureMsg(struct SQTreasureMsg *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnTriggerTreasure"))
	{
		g_Script.PushParameter(pMsg->bTreasureType);
		g_Script.PushParameter(this->GetSID());
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnInitPrayerMsg(struct SQInitPrayerMsg *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnInitPrayer"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnStartPrayerMsg(struct SQPrayerMsg *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("PlayerGoldPrayer"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->bcount);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnShopRefresh(struct SQShopRefresh *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnGetShopData"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->brefreshtype);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnShopBusiness(struct SQShopBusiness *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnShopBuyItem"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->btype);
		g_Script.PushParameter(pMsg->bIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnRequestFactionScene(struct SQFactionSceneData *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnRequestTurnOnScene"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->btype);
		g_Script.PushParameter(pMsg->dsceneID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnRequestFactionInfo(struct SQFactionSceneMapData *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnRequestSceneInfo"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->dsceneID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnChallengeScene(struct SQChallengeSceneMsg *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnChallengeSceneMap"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->dsceneID);
		g_Script.PushParameter(pMsg->bscenemapIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnChallengeFinish(struct SQSceneFinishMsg *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnChallengeFinish"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->dHurtValue);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnRequestFactionSceneRank(struct SQSceneHurtRank *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("GetFactionHurtRank"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->dsceneID);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnInitFactionSalaryData(struct SQInitFactionSalary *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnInitFactionSalary"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnGetFactionSalary(struct SQGetFactionSalary *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnGetFactionSalary"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->bType);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnShowNotice(struct SQFactionNotice *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnShowFactionNotice"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnModifyNotice(struct SQFModifyNotice *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnModifyNotice"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->modifynotice);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnShowoperateLog(struct SQFactionOperateLog *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnShowFactionLog"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnSendFcEmailToAll(struct SQFcEmailToAll *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnSendToAllFMemberEmail"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->title);
		g_Script.PushParameter(pMsg->content);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnShowRequestEquipt(struct SQShowEquipt *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("ShowSceneEquipment"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnRequestEquipt(struct SQRequestEquipt *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("RequestSceneEquipment"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->blevle);
		g_Script.PushParameter(pMsg->bIndex);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnRequestStatus(struct SQRequestStatus *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("ShowRequestRank"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->bCanceled);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnCanceledQuest(struct SQCanceledQuest *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("PlayerCanceledQuest"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}

void CPlayer::OnGetGiftCode(struct SQGiftcodeActivity *pMsg)
{
	if (!pMsg)
	{
		return;
	}
	g_Script.SetPlayer(this);
	if (g_Script.PrepareFunction("OnActivityGiftCode"))
	{
		g_Script.PushParameter(this->GetSID());
		g_Script.PushParameter(pMsg->strgiftcode);
		g_Script.Execute();
	}
	g_Script.CleanPlayer();
}
