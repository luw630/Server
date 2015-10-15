// ----- ScriptManager.h -----
//
//   --  Author: RenYi
//   --  Date:   06/13/03
//   --  Desc:   脚本管理类头文件
// --------------------------------------------------------------------
//   --  这里将集中管理服务器端的所有脚本
//       程序与脚本的数据加换的功能函数的实现
//---------------------------------------------------------------------    
#pragma once

#include <time.h>
#include <cassert>
#include <xydclient\extraframework\dgamescript.h>
#include <liteserializer/variant.h>
#include "Networkmodule\ItemTypedef.h"
#include "FightObject.h"
//#include <pub/waitqueue.h>

//#pragma comment (lib, "DGameScript.lib")
#pragma comment (lib, "lualib.lib")


struct stEvent {

	enum {
		TE_MONSTER_DEAD = 1,
		TE_MONSTER_REFRESH,
		TE_MONSTER_AI,
		TE_NPC_TALK,
	}TE_Type;
}; 

struct stLuaData
{
	char	cstrDataKey[30];	//table--name
	WORD	wDataLen;	//table--len
	stLuaData::stLuaData():wDataLen(0){memset(cstrDataKey,0,30);}
};

enum ObjectType {  

	OT_MONSTER = 1, 
	OT_PLAYER, 
	OT_NPC 
};

struct STaskExBuffer
{
	STaskExBuffer() : saveData(0), tempData(0), saveSize(0), tempSize(0), flags(0) {}
	~STaskExBuffer() 
	{ 
		if (saveSize && saveData && ((flags & SIZE1_FIXED) == 0)) 
			delete saveData; 
		if (tempSize && tempData) 
			delete tempData; 
	}

	// 原理，通过修改脚本系统，现已经支持可控大小的数据表，这里的指针数据都是脚本数据序列化后的存储空间
	void *saveData;		// dbMgr[sid].data 用于保存的数据表！  这实际上是指向玩家数据结构中预备好的缓存空间
	void *tempData;		// dbMgr[sid].temp 用于临时性数据表！  这里的指针数据是临时分配出来的，只是为了传递数据而存在

	int saveSize;
	int tempSize;

	enum { SIZE1_FIXED = 1, SIZE2_FIXED = 2, ERROR_IN_SERIAL = 16 };
	DWORD flags;
};

///////////////////////////////////////////////////////////////////////////////////
class CScriptManager : public CGameScript
{
protected:  
	BOOL  AddGoods(void);

public:
	CScriptManager() : 
	  m_pMonster( NULL ),
		  m_pPlayer( NULL ),
		  m_pNpc( NULL ),
		  m_dwLoopTime(0), 
		  m_pAttacker(NULL),
		  m_pDefencer(NULL),
		  prepared_paramnum( -1 ), 
		  load_errs( 0 ) 
	  {
		  _time64(&m_EveryDayManagerTime);
		  m_MonsterList.resize(65536);
		  m_NpcList.resize(65536);
	  };

	  void SetScriptPath(const char *rootpath);          
	  BOOL RegisterFunc(void);
	  void LuaRegisterFunc();
	  void SetCondition(class CMonster *pMonster, class CPlayer *pPlayer, class CNpc *pNpc, class CBuilding *pBuilding = NULL );
	  void CleanCondition(void); 

	  void SetFightCondition(class CFightObject *pAttacker);
	  void CleanFightCondition();

	  void SetFightDefencer(class CFightObject *pDefencer);
	  void CleanFightDefencer();

	  void SetInUseItemPackage(SPackageItem *pitem);
	  void CleanItemPackage();

	  void SetRawItemBuffer(SRawItemBuffer *ItemBuffer);
	  void CleanRawItemBuffer();

	  void SetPlayer(class CPlayer *pPlayer);
	  void CleanPlayer();

	  void SetMonster(class CMonster *pmonster);
	  void CleanMonster();


	  BOOL TriggerEvent(const char* name, int nType);     // 触发脚本的某个事件
	  BOOL TriggerEvent(const stEvent *pTE);
	  int Restore( int flag = 1,const char* name = NULL );                       // 恢复初始化的状态，返回值为失败的脚本数量     

	  BOOL DoFunc(const char *filename, DWORD wScriptID, DWORD dwChoseID = 0);    
	  //BOOL LostItems(ObjectType Who, WORD wItemID, WORD wLife, WORD wRange, DWORD dwMoney);

	  DWORD  GetMonsterID(WORD wIndex) const  { return ( wIndex < m_MonsterList.size() ) ? m_MonsterList[wIndex] : 0; } 
	  DWORD  GetNpcID(WORD wIndex) const  { return ( wIndex < m_NpcList.size() ) ? m_NpcList[wIndex] : 0; } 
	  void SetMonsterID(WORD wMonsterID, DWORD dwGID) { if ( wMonsterID < m_MonsterList.size() ) m_MonsterList[wMonsterID] = dwGID; }
	  void SetNpcID(WORD wNpcID, DWORD dwGID) { if ( wNpcID < m_NpcList.size() ) m_NpcList[wNpcID] = dwGID; }
	  void SetMListSize(int nSize) { m_MonsterList.resize(nSize); }
	  void SetNListSize(int nSize) { m_NpcList.resize(nSize); }

	  void SetLoop(void);

	  struct TempCondition {
		  CMonster   *m_pOldMonster;
		  CPlayer    *m_pOldPlayer;
		  CNpc       *m_pOldNpc;
		  CBuilding  *m_pOldBuilding;
	  };
	  struct TempFightObjectCondition
	  {
		  CFightObject	*m_OldpAttacker;//攻击者
	  };

	  void PushFightObjectCondition( CScriptManager::TempFightObjectCondition &tmp );
	  void PopFightObjectCondition(CScriptManager::TempFightObjectCondition &tmp);
	  void PushCondition( CScriptManager::TempCondition &tmp );
	  void PopCondition( CScriptManager::TempCondition &tmp );
	  void GetCondition( CScriptManager::TempCondition &tmp );

	  int GetQuestSkill(const char* name, DWORD mAttackerGID, DWORD mDefenderGID, float mDefenderWorldPosX, float mDefenderWorldPosY, BYTE dwSkillIndex, struct SQuestSkill_C2S_MsgBody &sqQuestSkillMsg);
	  int GetDamageValue(const char* name, DWORD mAttackerGID, DWORD mDefenderGID, WORD dskillID, struct CFightObject::SDamage &sdamage);
	  int GetEquiptFightPower(const char* name, DWORD dEquiptUserGID, BYTE bType, BYTE bIndex, DWORD &dEquiptFightPower);//取得装备战斗力
	  int GetWorldTalkLimit(CPlayer *pPlayer, const char* funcname,WORD &wresult);  //世界喊话限制，现在移动到lua中判断
	  int GetMonsterVipExp(CPlayer *pPlayer, const char* funcname, DWORD dmonsterexp, DWORD &dmonsteraddexp);//怪物死亡VIP加成
	  int GetItemUseLimit(CPlayer *pPlayer, const char* funcname, DWORD ditemindex, WORD &buse);

	  int LoadAllFactioninfo(struct SaveFactionData_New *FactionData);//数据库中读取的军团
public:
	// 加入功能函数  也就是要放入LUA中执行的函数
	// --- 返回值必须是int  
	static int L_DoFile(lua_State *L);

	static int L_rfalse(lua_State *L);
	static int L_rint(lua_State *L);
	static int L_RFalse(lua_State *L);
	static int L_GetStartServerTime(lua_State *L);
	// 侠义道3道具接口
	static int L_giveMoney(lua_State *L);
	static int L_giveGoods(lua_State *L);
	static int L_checkgoods(lua_State *L);
	static int L_getitemnum(lua_State *L);

	// 扩展背包激活
	static int L_ActivePackage(lua_State *L);	
	static int L_SendActivePackage(lua_State *L); // 发送扩展处理结果
	static int L_GetActivePackageNum(lua_State *L); // 获取,初始的背包格子数,已经激活扩展背包数量,背包最大上限数目


	// 侠义道3Buff系统用	
	static int L_activetelegry(lua_State* L);
	static int L_activeskill(lua_State* L);
	static int L_activeTempskill(lua_State* L);
	static int L_CoolingTempSkill(lua_State* L);
	static int L_SendTemplateSkill(lua_State* L);
	static int L_GetEventID(lua_State* L);
	static int L_activate(lua_State *L);
	static int L_deactivate(lua_State *L);
	static int L_getAttribute(lua_State *L);
	static int L_setAttribute(lua_State *L);

	// 侠义道3
	static int L_giveSp(lua_State *L);
	static int L_giveExp(lua_State *L);
	static int L_fpgiveExp(lua_State *L);//增加侠客经验
	static int L_giveMount(lua_State *L);
	static int L_givePet(lua_State *L);
	static int L_giveFightPet(lua_State *L);

	static int L_ClearPlayerCSItem(lua_State *L);
	static int L_requarefactionname(lua_State *L);
	static int L_createfaction(lua_State *L);
	static int L_leavefaction(lua_State *L);

	//static int L_createmonster(lua_State *L);      
	static int L_createnpc(lua_State *L);      
	static int L_delobject(lua_State *L);

	static int L_removecurrobj(lua_State *L);

	static int L_monsterai(lua_State *L);
	static int L_setnpcgoodscount(lua_State *L);
	static int L_loaditem(lua_State *L);           // 读取交易的物品
	static int L_addblood(lua_State *L); 
	static int L_FollowLoadItem(lua_State *L);
	static int L_LoadFollowMenuInfo(lua_State *L);
	static int L_ClearFollowShop(lua_State *L);
	static int L_setmessage(lua_State *L);			// 点击Npc后 服务器的回应消息
	static int L_setmessageA(lua_State *L);			// 点击选项 服务器的回应消息
	static int L_setmessageB(lua_State *L);			// NPC填充物品
	static int L_setmessageC(lua_State *L);			// 玩家出师
	static int L_setmessageR(lua_State *L);			// 执行消息返回给玩家
	static int L_settipmsg(lua_State *L);			// 系统提示
	static int L_playcg(lua_State *L);				// play cg

	static int L_addtask(lua_State *L);				// 将任务记到任务列表中
	static int L_buytask(lua_State *L);				// 购买任务号
	static int L_checktask(lua_State *L);			// 检查任务情况
	static int L_GetNeedBlank(lua_State *L);		// 检查如果要放X个物品Y个，能否成功

	static int L_checkmoney(lua_State *L);			// 检查金钱是否足够,(money,del,playerid)
	static int L_deltask(lua_State *L);			    // 删除已完成的任务
	static int L_getplayerdata(lua_State *L);		// 得到玩家的数据

	//add by ly 2014/4/9
	static int L_SetPlayerProperty(lua_State *L);	//设置玩家相关数据
	static int L_SetPlayerObject(lua_State *L);		//设置执行脚本函数的玩家对象（参数为：玩家全局唯一GID，为0时表示当前玩家）

	static int L_setplayerdata(lua_State *L);		// 改变玩家的属性
	static int L_setplayerproperty(lua_State *L);   // 设置玩家的属性
	
	//add by ly 2014/4/4 
	static int L_GetPlayerSID(lua_State *L);	//通过玩家名字获取玩家的SID

	static int L_getplayerGID(lua_State *L);		// 得到玩家的数据 int GetPlayerGID(DWORD sid)
	static int L_getplayerSkillproperty(lua_State *L);   //得到玩家的技能属性
	

	static int L_getattackername(lua_State *L);     // 得到攻击者的姓名


	static int L_getmonsterdata(lua_State *L);     // 得到怪物的数据
	static int L_setmonsterdata(lua_State *L);
	static int L_subtasknum(lua_State *L);         // 完成一个减少一个，到零任务就完成
	static int L_getnpcdata(lua_State *L);         // 得到NPC的数据
	static int L_gettaskdata(lua_State *L);        // 得到任务中的变量
	static int L_getgoodsdata(lua_State *L);       // 得到道具的数据
	static int L_getforgedata(lua_State *L);       // 得到炼制的数据
	static int L_getmonsterdatanew(lua_State *L);     // 得到怪物的数据

	static int L_putplayerfromdynamic(lua_State *L);
	static int L_putplayerto(lua_State *L);  
	static int L_putplayerback(lua_State *L);
	static int L_reitem(lua_State *L);
	static int L_isfull(lua_State *L);
	static int L_isFullNum(lua_State *L);

	static int L_teachgest(lua_State *L);           // 传授武功 
	static int L_getgestlevel(lua_State *L);        // 得到武功等级
	static int L_setgestlevel(lua_State *L);        // 设置武功等级
	static int L_getcurtelergy(lua_State *L);       // 得到当前心法
	static int L_setcurtelergyval(lua_State *L);    // 改变当前心法值 
	static int L_paytheplayer(lua_State *L);        // 给玩家东西
	static int L_settaskstring(lua_State *L);       // 设置任务怪物(现在是)
	static int L_checkmoreitem(lua_State *L);       // 多物品检查
	static int L_gettelergy(lua_State *L);     
	static int L_gettelergydata(lua_State *L);		// 得到玩家身上指定心法的数据

	static int L_StartCollection(lua_State*L);		// 玩家开始采集
	static int L_MakeNpcDisappear(lua_State *L);

	static int L_getitemname(lua_State *L);    
	static int L_gettaskitemname(lua_State *L);
	static int L_getMountFoodAddExp(lua_State *L);		// 得到道具附加属性(血,法,体)
	static int L_lostitems(lua_State *L);   

	static int L_fillcopyitem(lua_State *L);
	static int L_getscores(lua_State *L);          // 得到各个排行榜的情况 
	static int L_setregionpktype(lua_State *L);   
	static int L_broadcast(lua_State *L);   

	static int L_setlistsize(lua_State *L);        // 设置生成对象列表大小
	//test
	static int L_zero(lua_State *L); 
	static int L_getcurtime(lua_State *L);          // 得到当前的时间
	static int L_getcurtimeinmin(lua_State *L);
	static int L_getcurtimeA(lua_State *L);        

	static int L_compilefile(lua_State *L);         // 只对文件做编译
	static int L_talktoplayer(lua_State *L);        // 对玩家的对话

	static int L_delitembypos(lua_State *L);        // 通过坐标删除道具
	static int L_GetKillInfo(lua_State *L);         // 得到追杀令的信息
	static int L_teachteamgest(lua_State *L);       // 学习队形技
	//static int L_cure(lua_State *L);              // 治疗
	static int L_setplayerscriptstate(lua_State *L);    // 设置玩家脚本状态值
	static int L_getplayerscriptstate(lua_State *L);    // 得到玩家脚本状态值
	static int L_setplayer(lua_State *L);               // 改变当前玩家
	static int L_setplayericon(lua_State *L);
	static int L_cleanplayer(lua_State *L); //清除当前玩家

	static int L_setmultipletime(lua_State *L);     // 设置多倍时间
	static int L_setnmultiple(lua_State *L);        // 设置倍数
	static int L_getmultipletime(lua_State *L);     // 得到多倍
	static int L_getmulleavetime(lua_State *L);     // 得到多倍状态剩余小时数
	static int L_checkweekformul(lua_State *L);     // 多倍的周状态
	static int L_setlonemanvalue(lua_State *L);     // 设置独行侠状态值
	static int L_getmultipletimeinfo( lua_State *L );// 取得当前周的双倍时间时间

	static int L_CreateRegion(lua_State *L);        // 动态创建一个场景
	static int L_QueryEncouragement(lua_State *L);			// 动态创建一个场景
	static int L_GetTeamInfo( lua_State *L );
	static int L_DeleteTeamMember( lua_State* L );

	static int L_ViewFactionMsg( lua_State *L );
	static int L_GetMemberInfo( lua_State *L );
	static int L_GetJoinTime( lua_State *L );
	static int L_GetFactionRes( lua_State *L );
	static int L_OperFacRes( lua_State *L );
	static int L_SetPlayerAmassCent( lua_State *L );
	static int L_AddPlayerAmassCent( lua_State *L );
	static int L_GetMarrowInfo( lua_State *L );
	static int L_GetRegionInfo( lua_State *L );
	static int L_SendBoardCastMsg( lua_State *L );		//活动广播
	static int L_GetServerTime(lua_State *L);

	static int L_AddPlayerProperty(lua_State *L);//设置角色属性:HP,MP,SP(增量)
	static int L_AddMountsProperty(lua_State *L);//增加坐骑属性
	static int L_Dismount(lua_State *L);		//下马

	// 副本相关接口
	static int L_LockPlayer(lua_State* L);
	static int L_UnLockPlayer(lua_State* L);
	static int L_NotifyCopySceneTaskReady(lua_State* L);
	static int L_GetDynamicScenePlayerNumber(lua_State* L);
	static int L_DestroyDynamicScene(lua_State* L);
	static int L_BackFromDynamicScene(lua_State* L);
	static int L_SetPlyaerCopySceneState( lua_State* L);
	static int L_GetCurCopyScenesGID(lua_State *L);
	static int L_CheckAddGoods( lua_State* L);

	static int L_MonsterAddHpEvent( lua_State* L);
	static int L_MonsterAddBuffEvent( lua_State* L);
	static int L_CheckObject(lua_State *L); //检测对象是否存在

	// 仓库相关接口
	static int L_OpenWareHouse(lua_State *L);

	// 战斗系统相关接口
	static int L_DelFightBuffType(lua_State *L);
	static int L_AddFightBuff(lua_State *L);
	static int L_PlayerAddBuff(lua_State *L);
	static int L_AddBuff(lua_State *L);//仅添加一下辅助BUFF,如清醒，无敌等
	static int L_FightPetAddBuff(lua_State *L);//侠客吃药buff
	static int GetWindMove(DWORD Type, DWORD DefencerID, WORD skillid, float& DestPosX, float& DestPosY);
	static int L_CheckWindMove(lua_State *L);
	static int L_StartWindMove(lua_State *L);
	static int L_SendAttackMsg(lua_State *L);
	static int L_MoveToPos(lua_State *L);//移动坐标点
	static int L_CheckFightState(lua_State *L);//获取当前BUFF状态
	static int L_print(lua_State *L);//打印输出

	static int L_Gettimelimit(lua_State *L);

	static int L_loaddropgoods(lua_State *L);
	static int L_loaddropgoodsnum(lua_State *L);
	static int L_loaddroptime(lua_State *L);
	static int L_loaddropmonsterid(lua_State *L);

	static int L_monsterdropitem(lua_State *L);///怪物掉落物品
	static int L_setplayerpoint(lua_State *L);///给玩家加点
	static int L_monstertalk(lua_State *L);///怪物说话,附近

	//侠义道三队伍相关以及新增接口
	static int L_getplayerteam(lua_State *L);
	static int L_getplayerteamname(lua_State *L);
	static int L_setplayerdatanew(lua_State *L);
	static int L_creategift(lua_State *L);	//创建一个礼包
	static int L_usegift(lua_State *L);	//使用一个礼包
	static int L_AddGiftJudgePackage(lua_State *L);		//添加元宝礼包道具时，判断是否背包足够
	static int L_QuestUpdateBoss(lua_State *L);	//请求更新怪物数据
	static int L_setactivityNotice(lua_State *L);	//活动内容
	static int L_GetLiveMonster(lua_State *L);	//获取场景中怪物数量
	//设置玩家的好友度
	static int L_setplayerDegree(lua_State *L);
	//侠客学习技能
	static int L_fpactiveskill(lua_State* L);
	//体质系统
	static int L_tizhiGrow(lua_State* L);		//体质增长
	static int L_tizhiUpgrade(lua_State* L);	//体质突破
	static int L_kylinArmActive(lua_State* L);  //激活麒麟臂功能
	static int L_addYuanqi(lua_State* L);		//增加元气

	// 心法升级消耗
	static int L_ConsumeTelergy(lua_State* L);
	static int L_ConsumeTelergyProperty(lua_State* L);

	// 祈福相关
	static int L_ProcessBlessInfo(lua_State* L);
	static int L_FillBlessData(lua_State* L);
	static int L_OnBlessResult(lua_State* L);
	static int L_OnPlayerBlessTime(lua_State* L);
	static int L_GetPlayerMoney(lua_State* L);
	static int L_ChangeMoney(lua_State* L);
	static int L_OnErrorStopBless(lua_State* L);
	static int L_IsHaveSpaceCell(lua_State* L);
	static int L_OnProcessNotice(lua_State* L);
	//在线新手礼包相关
	//static int L_StopTime(lua_State* L);
	//static int L_StartTime(lua_State* L);
	//static int L_CleanTime(lua_State* L);
	//static int L_IsStopTime(lua_State* L);
	static int L_UpdateCountDownGiftState(lua_State* L);
	static int L_UpdateOnlineGiftState(lua_State* L);
	static int L_ResetCountDownGiftState(lua_State* L);
	static int L_ResetOnlineGiftState(lua_State* L);
	static int L_ResetCountDownGiftTime(lua_State* L);
	static int L_GetCountDownGiftBeginTime(lua_State* L);//得到倒计时开始时间
	static int L_GetCountDownGiftState(lua_State* L);//得到倒计时奖励给与阶段
	static int L_GetOnlineGiftState(lua_State* L);
	static int L_GetOnlineBeginTime(lua_State* L);
	static int L_SendBufferToClinet(lua_State* L);
	static int L_SendItemList(lua_State* L);
	static int L_GetNowTime(lua_State* L);
	static int L_SendGiftSucceed(lua_State* L);
	static int L_GetPlayerTime(lua_State* L); //获取玩家相关时间，0--上次登录时间，1--上次离线时间，2--本次登录时间
	static int L_SetPlayerVipLevel(lua_State* L); //设置玩家VIP等级
	

	//金钱扣除
	static int L_CheckPlayerMoney(lua_State* L);

	//lua中数据存储,现在规定每个数据为一个int的大小
	static int L_SaveToPlayerData(lua_State* L);
	static int L_LoadFromPlayerData(lua_State* L);


	//lua中数据存储table
	static int L_SavePlayerData(lua_State* L);
	static int L_LoadPlayerData(lua_State* L);

	//lua中数据存储table
	static int L_SavePlayerTable(lua_State* L);
	static int L_LoadPlayerTable(lua_State* L);
	
	static int L_SendPlayerReward(lua_State* L);	//发送奖励
	static int GetTableData(lua_State* L,std::vector<lite::Variant > &keyvariant,std::vector<lite::Variant> &vlvariant);//获取表格数据
	static int MakeRewardData(int nReward,BYTE bState,BYTE blocation);
	

	static int findDataPos(const char* tablename ,bool bissave = true);//查找一个空位用于存储数据,或者查找一个已经存在的数据位置
	static int getDataLenth();//取出现有数据总长度
	static int UpdateLenth();//更新现有数据总长度
	//AI接口
	static int L_PlayerClearBuff(lua_State* L);
	static int L_LoadPlayerModifValue(lua_State* L);
	static int L_UnLoadPlayerModifValue(lua_State* L);
	static int L_OnBackFightPet(lua_State* L);
	static int L_OnCallOutFightPet(lua_State* L);

	static int L_giveItemList(lua_State* L);
	

	//测试添加送花记录
	static int L_AddPlayerRoseRecod(lua_State* L);
	// 回神丹调用
	static int L_HuiShenDanRecall(lua_State* L);
	// 全装备属性加成
	static int L_ModifyPlayerAttributes(lua_State* L);
	// 随机值的计算和返回
	static int L_LuaRandRange(lua_State* L);

	static int L_SetCrilial(lua_State* L);

	static int L_LoadProtectInfo(lua_State * L);

	static int L_OnPartEnterNum(lua_State * L);//进入副本成功的时候更新数据

	static int L_OnTeamPartEnterNum(lua_State * L);

	static int L_IsPersonalEnterPart(lua_State * L);

	static int L_IsTeamEnterPart(lua_State * L);

	static int L_IsNonTeamPartEnter(lua_State * L);

	static int L_SetGmakeParm(lua_State * L);
	static int L_GetGmakeParm(lua_State * L);

	static int L_SetJingMaiShareExpTimes(lua_State * L);
	static int L_MystringFormat(lua_State * L);
	/*static int L_MystringFormat(lua_State * L);*/
	static int L_logtable(lua_State * L);

	//新增加关卡章节 发送关卡信息
	static int L_SendSceneMsg(lua_State *L);
	//发送选择的关卡信息
	static int L_SendSelectSceneMsg(lua_State *L);
	//发送进入关卡的信息
	static int L_SendEnterSceneMsg(lua_State *L);
	static int L_CreateScene(lua_State* L);//动态创建关卡

	static int L_SendRewardMsg(lua_State *L);//发送关卡奖励
	static int L_SendWinRewardMsg(lua_State *L);//发送抽牌得到的奖励道具
	static int L_SendDebrisinfo(lua_State *L);//发送碎片收集信息
	static int L_SendDebrisAward(lua_State *L);//发送发送徽章奖励
	static int L_SendSceneSDListMsg(lua_State *L);//发送关卡扫荡奖励列表消息

	//static int GetTableNumber(lua_State *L, int nIndex, const char* key, BYTE  bType,void *Buffer);

	static int GetTableNumber(lua_State *L,int nIndex,const char* key,int &nvalue);
	static int GetTableNumber(lua_State *L,int nIndex,const char* key,float &nvalue);
	static int GetTableNumber(lua_State *L,int nIndex,const char* key,DWORD &nvalue);
	static int GetTableNumber(lua_State *L, int nIndex, const char* key, WORD &nvalue);
	static int GetTableNumber(lua_State *L, int nIndex, const char* key, BYTE &nvalue);
	static int GetTableNumber(lua_State *L, int nIndex, const char* key, QWORD &nvalue);
	static int GetTableNumber(lua_State *L, int nIndex, const char* key, double &nvalue);

	static int GetTableStringValue(lua_State *L, int nIndex, const char* key, std::string &str);
	static int GetTableStringValue(lua_State *L, int nIndex, const char* key, char strvalue[CONST_USERNAME]);
	static int GetTableArrayString(lua_State *L, int nIndex, int narrayIndex, char strvalue[CONST_USERNAME]);

	static int SetTableNumber(lua_State *L,const char *key,int nvalue);
	static int SetTableArrayNumber(lua_State *L,int nIndex,int nvalue);
	static int SetTableStringValue(lua_State *L, const char *key, char strvalue[CONST_USERNAME]);

	static int GetTableArrayNumber(lua_State *L,int nIndex,int narrayIndex,int &nvalue);
	static int GetTableArrayNumber(lua_State *L, int nIndex, int narrayIndex, BYTE &nvalue);
	static int GetTableArrayNumber(lua_State *L, int nIndex, int narrayIndex, float &nvalue);
	static int GetTableArrayNumber(lua_State *L, int nIndex, int narrayIndex, DWORD &nvalue);
	static int GetTableArrayNumber(lua_State *L, int nIndex, int narrayIndex, WORD &nvalue);



	static int L_GetDynamicRegionID(lua_State *L);

	static int L_GetSceneCount(lua_State *L);//取得场景关卡数量

	static int L_GetTableLenth(lua_State *L);	
	static int L_GetObjectType(lua_State *L);
	static int L_DestroyScene(lua_State *L);//销毁关卡
	static int L_Removeobj(lua_State *L);//销毁关卡前移除所有的对象

	static int L_GetBlankCell(lua_State *L);//获取空格子数量

	static int L_SetQusetSkillMsg(lua_State *L);//设置一个连续技能
	static int L_TimeGetTime(lua_State *L);
	static int L_ProcessQuestSkill(lua_State *L);//请求使用技能
	static int L_GetSkillIDBySkillIndex(lua_State *L);//这里根据索引找出在“技能配置表”中的技能ID
	static int L_SetCacheSkill(lua_State *L);//设置缓存的技能到player
	


	static int L_GetPlayerInScene(lua_State *L);//是否正在通关中
	static int  L_SendPassFailMsg(lua_State *L);//发送通关失败消息
	static int L_PlayerRelive(lua_State *L); //玩家复活

	static int L_SendSDMsg(lua_State *L);//发送扫荡终止相关消息
	static int L_SendSDReward(lua_State *L);//发送扫荡奖励领取相关消息
	static int L_SendPlayerDeadAddMsg(lua_State *L);// 玩家死亡时的附近消息，仅发送给死亡的玩家自身
	static int L_SendReliveResult(lua_State *L);//发送复活结果
	static int L_SendSceneListMsg(lua_State *L);//发送所需章节列表
	static int L_SendSceneFinishMsg(lua_State *L);//发送关卡完成
	static int L_GetSceneRegionID(lua_State *L);//所在关卡GID

	static int L_GetObjectData(lua_State *L);//获取对象属性
	static int L_GetObjectFightData(lua_State *L);//获取对象战斗属性
	static int L_SendObjectDamage(lua_State *L);//发送伤害
	static int L_SetObjectDamage(lua_State *L);//设置伤害数据
	static int L_KillAllMonster(lua_State *L);//杀死所有怪物
	static int L_ChangeObjectData(lua_State *L);//改变对象属性
	static int L_SetObjectData(lua_State *L);//设置对象属性
	//装备相关
	static int L_GetEquipment(lua_State *L);//获取装备
	static int L_SendIntensifyInfo(lua_State *L);//发送装备强化相关信息
	static int L_SetEquipment(lua_State *L);//设置装备
	static int L_IsLockedEquipColumnCell(lua_State *L); // 检测装备栏是否被锁定了		
	static int L_LockEquipColumnCell(lua_State *L);// 锁定或解锁定装备栏的格子
	static int L_SendIntensifyResult(lua_State *L);//发送装备强化结果
	static int L_SendQualityInfo(lua_State *L);//发送装备提升品质相关信息
	static int L_DeleteItem(lua_State *L);//删除道具
	static int L_DelEquipment(lua_State *L);//删除装备
	static int L_AddItem(lua_State *L);//增加单个道具
	static int L_RecvUseItem(lua_State *L);//使用装备道具
	static int L_SendQualityResult(lua_State *L);//装备升阶结果
	static int L_GetItemBaseData(lua_State *L);//获取道具详细信息
	static int L_SendCleanColdResult(lua_State *L);//发送清除强化冷却时间结果
	static int L_SendRiseStarInfo(lua_State *L);//发送升星描述信息
	static int L_SendRiseStarResult(lua_State *L);//发送升星结果
	static int L_SendEquiptKeYinInfo(lua_State *L);//发送刻印描述信息
	static int L_SendEquipJDingResult(lua_State *L);////装备鉴定结果
	static int L_SendEquipJDingInfo(lua_State *L);//发送装备鉴定数据
	static int L_SendEquipJDingColor(lua_State *L);////发送鉴定条颜色
	static int L_SendKeYinChangeData(lua_State *L);//发送鉴定转换数据
	static int L_SendSuitcondition(lua_State *L);//发送套装信息
	static int L_SendKeYinResult(lua_State *L);//发送刻印结果

	//网络IP
	static int L_GetPlayerIP(lua_State *L);//玩家ip
	  
	//初次创建玩家
	static int L_GetIsNewPlayer(lua_State *L);//是否是新玩家
	static int L_SetIsNewPlayer(lua_State *L);//设置新玩家

	static int L_SetTaskFinish(lua_State *L);//任务完成
	static int L_InitPlayerData(lua_State *L);//初始化玩家升级数据  
	static int L_ReloadPlayerData(lua_State *L);//重读
	static int L_KickPlayer(lua_State *L);//踢下线
	//变身
	static int L_GetTransformersLevel(lua_State *L);//获取状态变身等级
	static int L_SetTransformersLevel(lua_State *L);//设置状态变身等级,状态
	static int L_OnSendTransformersInfoMsg(lua_State *L);
	static int L_SendTransformersSwitch(lua_State *L);
	static int L_SendMyState(lua_State *L);//同步玩家状态
	static int L_InitPlayerProperty(lua_State *L);//恢复玩家初始属性
	static int L_GetCurrentSkillLevel(lua_State *L);//得到技能等级
	static int L_SetCurrentSkillLevel(lua_State *L);//设置技能等级
	static int L_GetSkillProficiency(lua_State *L);//得到技能熟练度
	static int L_AddSkillProficiency(lua_State *L);//设置技能熟练度
	static int L_GetSkillLevel(lua_State *L);//得到技能等级
	static int L_SetSkillLevel(lua_State *L);//设置技能等级
	static int L_SendTransformersSkillInfo(lua_State *L);// 发送变身技能信息

	//全部读取储存
	static int L_LoadTable(lua_State *L);//读取全局表
	static int L_SaveTable(lua_State *L);//保存全局表

	//信仰升星计算 add by ly 2014/3/18
	static int L_AddPlayerXinYangProperty(lua_State *L);	//增加玩家的属性

	//荣耀相关 add by ly 2014/3/25
	static int L_InitNewPlayerGloryInfo(lua_State *L);		//初始化新手玩家的荣耀信息
	static int L_SendClientGloryInfo(lua_State *L);			//向客户端返回荣耀信息
	static int L_QstGetGloryAward(lua_State *L);		//请求获得荣耀奖励
	static int L_QstHandleTitleMsg(lua_State *L);		//请求处理称号消息
	static int L_SetKillAllMonster(lua_State *L);		//设置玩家在关卡中杀死所有怪物（用于测试用）

	static int L_TaskKillMonster(lua_State *L);//任务杀怪计数
	static int L_SendBatchesMonster(lua_State *L);
	static int L_Syneffects(lua_State *L);//同步特效
	static int L_SetStopTracing(lua_State *L);//怪物返回出生地

	//竞技场消息
	static int L_SendArenaInitMsg(lua_State *L);////回复打开竞技场选择的界面
	static int L_SendArenaRewardMsg(lua_State *L);////发送竞技场奖励
	static int L_SendArenaQuestMsg(lua_State *L);////回复竞技场奖励


	static int L_CopyFromPlayer(lua_State *L);//复制玩家的数据到怪物
	static int L_CopyPlayerToTable(lua_State *L);//复制玩家数据保存到表中
	static int L_GetObjectReginType(lua_State *L);//或者对象所在场景类型
	static int L_GetSkillDamageRate(lua_State *L);//技能等级对应伤害比例
	static int L_GetBuffData(lua_State *L);//获取BUff数据
	

	//add by ly 2014/4/10  服务器GM相关操作
	static int L_SendMessageToAllPlayer(lua_State *L);	//发送消息给所有玩家
	static int L_PutPlayerToDyncRegion(lua_State *L);	//创建动态地图并且将玩家放入
	static int L_SetPlayerGMLevel(lua_State *L);	//设置玩家GM等级，只有等级达到才有控制权限
	static int L_BaseRelive(lua_State *L);	//原地复活,里面对L_PlayerRelive实现了封装
	static int L_CreateMonsterRegion(lua_State *L);		//地图刷怪mapid x y monsterid num
	static int L_SetMapCollision(lua_State *L);		//设置地图碰撞状态（打开或关闭）
	static int L_GetOnlinePlayerNum(lua_State *L);	//获取当前服务器人数
	static int L_SendSysCall(lua_State *L);		//发送系统公告
	static int L_SendGmExceResult(lua_State *L);	//发送GM命令执行结果


	//add by ly 2014/4/16  活动相关操作
	static int L_ReturnDailyListInfo(lua_State *L);	//获取玩家日常活动列表信息（有每天定时重置功能）
	static int L_ResetDailyByTime(lua_State *L);	//玩家重置已经完成的日常活动列表信息（参数依次为：日常重置时间；已经完成的日常数据（重置时间满足后重置该表）；）
	static int L_DestroyDailyArea(lua_State *L);	//销毁日常活动场景
	static int L_ResponseEntryDailyArea(lua_State *L);	//进入日常活动后的响应操作,0表示进入失败，1表示进入成功，4表示成功活动结束
	static int L_SendDailyAwardToPlayer(lua_State *L);	//发送日常活动的奖励给玩家
	static int L_GetPlayerInDailyArea(lua_State *L);	//获取玩家是否在日常活动场景中
	static int L_SendNextBatchMonsterInf(lua_State *L);	//通知玩家当前波怪物死亡，下一波为第几波怪和下一波怪还有多久刷新

	static int L_SendDailyOpenFlag(lua_State *L);	//发送失乐园开放标志
	static int L_BroadCastDailyOpenFlag(lua_State *L);	//广播失乐园开放标志

	static int L_SendJuLongShanAward(lua_State *L);		//发送巨龙山探险奖励信息

	static int L_SendTarotBaseData(lua_State *L);	//发送玩家塔罗牌基本数据信息
	static int L_SendAddTurnOnTarotData(lua_State *L);	//发送玩家翻开的塔罗牌数据信息
	static int L_SendPlayerGetTarotAwardRes(lua_State *L);	//发送玩家领取塔罗牌奖励的操作结果消息
	static int L_SendResetTurnOnTarotData(lua_State *L);	//发送重置玩家已经翻开的塔罗牌数据信息

	static int L_SendCurDayOnlineData(lua_State *L);	//发送玩家当天在线奖励活动的基本数据信息
	static int L_SendCurDayOnlineLong(lua_State *L);	//发送玩家当天在线奖励活动的在线时长
	static int L_SendCurDayOnlineGetAwardRes(lua_State *L);		//发送玩家领取奖励结果
	static int L_NotifyCurDayOnlineReset(lua_State *L);		//通知玩家每日在线时长活动重置
	static int L_NotifyCurDayOnlineAddNewAward(lua_State *L);	//玩家达到一个新的可以领取的奖励

	static int L_GetPlayerAddUpLoginTime(lua_State *L);		//获取玩家累计登陆时间
	static int L_SendPlayerAddUpLogintime(lua_State *L);	//发送玩家累计登陆时间
	static int L_SendPlayerAddUpLoginInf(lua_State *L);		//发送玩家累计登陆基本信息
	static int L_NotifyAddUpLoginAddNewAward(lua_State *L);	//玩家达到了新的可以领取的累计登陆奖励
	static int L_SendAddUpLoginGetAwardRes(lua_State *L);	//发送玩家累计登陆领取奖励的结果

	static int L_SendPlayerRFBInf(lua_State *L);	//发送玩家充值回馈金币信息
	static int L_SendGetRFBAwardRes(lua_State *L);	//发送玩家领取充值回馈奖励结果
	static int L_NotifyRFBCanGetNewAward(lua_State *L);	//通知玩家，充值回馈有新的奖励可以领取

	static int L_SendPlayerLRInf(lua_State *L);		//发送玩家等级竞赛数据信息
	static int L_SendGetLRAwardRes(lua_State *L);	//发送玩家领取等级竞赛奖励结果
	static int L_NotifyLRCanGetNewAward(lua_State *L);	//通知玩家可以获得等级竞赛新的奖励
	static int L_BroadCastLRResidueTimes(lua_State *L);	//广播等级竞赛剩余次数

	static int L_SendFWInf(lua_State *L);	//发送幸运摩天轮数据信息
	static int L_SendPlayerFWInf(lua_State *L);	//发送玩家幸运摩天轮倒计时数据信息
	static int L_SendFWSelectGoodsInf(lua_State *L);	//发送选中的物品序号信息
	static int L_SendFWGEtAwardRes(lua_State *L);	//发送选中中物品领取操作结果
	static int L_BroadCastFWUptRecordInf(lua_State *L);	//广播幸运摩天轮的记录信息已经变更

	static int L_SynRechargeTatolGlod(lua_State *L);	//同步玩家总的充值金币数量

	//add by ly 2014/4/28
	static int L_SendSignInAwardInfo(lua_State *L);	//发送每日签到奖励信息
	static int L_SendGetSignInAwardResult(lua_State *L);	//发送领取每日签到奖励的结果

	//add by ly 2014/4/30
	static int L_GetPlayerSchool(lua_State *L);		//获取玩家角色类型(3为吸血鬼；2为狼人；1为人类)

	//add by ly 2014/5/8 商城相关
	static int L_SendShopListInfo(lua_State *L);		//发送商城信息
	static int L_SendShopGoodsListInfo(lua_State *L);		//发送商城中商品信息
	static int L_NotityGoodsUpdated(lua_State *L);		//通知商品更新处理
	static int L_SendBuyGoodsRes(lua_State *L);		//发送购买商品操作结果
	static int L_GetGoodsIndexByTriIndex(lua_State *L);	//通过商城类型，商品索引1和商品索引2计算商品的唯一索引
	static int L_GetTriIndexByGoodsIndex(lua_State *L);		//通过商品唯一索引获取商城类型，商品索引1和商品索引2
	static int L_SendShopCountDownRes(lua_State *L);	//发送商城刷新倒计时时间
	static int L_SendGoodsInfo(lua_State *L);	//发送一条商品信息

	//add by ly 2014/5/16 玩家第一次注册后调用；用于配置一个技能到玩家的技能配置面板中
	static int L_FirstRegCallSetPlayerSkill(lua_State *L);

	//add by ly 2014/5/16
	//VIP相关
	static int L_SetCurVipFactor(lua_State *L);		//设置当前VIP等级附加属性
	static int L_SendVipStateInf(lua_State *L);		//发送玩家VIP的状态信息
	static int L_SendGiftInfo(lua_State *L);	//发送礼包数据信息
	static int L_SendGetOrButGiftRes(lua_State *L);		//发送领取或获取VIP礼包结果信息
	static int L_AddSomeGoods(lua_State *L);	//添加一系列道具到背包中

	//add by ly 2014/5/23 宠物相关消息
	static int L_GetPlayerPetNum(lua_State *L);	//获取玩家已经拥有了的宠物数量
	static int L_AddNewPet(lua_State *L);	//添加宠物
	static int L_SendCardPetInf(lua_State *L);	//发送卡牌宠物列表信息
	static int L_SendPetCardInf(lua_State *L);	//发送宠物卡牌信息
	static int L_ExceSwallowPetOpt(lua_State *L);	//吞噬宠物
	static int L_PetBreachOpt(lua_State *L);	//宠物突破（宠物升星）
	static int L_PetRenameOpt(lua_State *L);	//更改宠物名字
	static int L_SendPetMergerInf(lua_State *L);	//发送宠物吞噬结果消息
	static int L_SendPetBreachInf(lua_State *L);	//发送宠物突破消息
	static int L_SendPetRenameInf(lua_State *L);	//发送宠物重命名消息
	static int L_GetPetIDAndStar(lua_State *L);		//获取宠物的ID和宠物的星级通过玩家宠物的索引
	static int L_PetSkillCfg(lua_State *L);		//设置宠物当前可以使用的技能
	static int L_GetPetOwer(lua_State *L);//获取侠客主人GID
	static int L_CalAndSendPetExp(lua_State *L);	//计算宠物的相关经验信息，并发送给玩家
	static int L_JudgePetDurableIsZero(lua_State *L);	//判断宠物魂力是否为0
	static int L_SendResumeDurableNeedGold(lua_State *L);	//发送恢复宠物魂力需要的金币
	static int L_SendGoldResumeDurableRes(lua_State *L);	//发送金币恢复宠物魂力的结果
	static int L_UptPetDurable(lua_State *L);	//更新宠物魂力
	static int L_NotifyPetDurableUpt(lua_State *L);		//通知宠物魂力已经更新
	static int L_PetStudySkillByBook(lua_State *L);		//宠物通过技能书学习技能
	static int L_ActivePetSkill(lua_State *L);		//激活宠物技能

	//add by ly 2014/6/5 获取战斗对象的状态动作
	static int L_GetFightObjActionID(lua_State *L);

	//add by ly 2014/6/25 通知玩家消息（从lua的UTF_8编码转换为Unicode码，并发送给玩家）
	static int L_NotifyMsgUtf8ToAnsi(lua_State *L);

	//add by ly 2014/6/27活动相关
	static int L_GetFirstPayFlag(lua_State *L);		//获取首次充值状态
	static int L_SetFirstPayFlag(lua_State *L);		//设置首次充值状态
	static int L_GetEveryDayPayFlag(lua_State *L);	//获取每日充值的状态
	static int L_SetEveryDayPayFlag(lua_State *L);	//设置每日充值的状态
	static int L_SendEveryDayPayAwardState(lua_State *L);	//发送每日充值奖励状态
	static int L_SendGetEDPAwardRes(lua_State *L);		//发送领取每日充值奖励结果

	//add by ly 2014/7/22 排行榜相关
	static int L_SendRandListData(lua_State *L);	//发送排行榜数据
	static int L_SendRandAwardFlag(lua_State *L);	//发送玩家排行榜领奖状态
	static int L_SendGetRandAwardRes(lua_State *L);	//发送领取排行奖励操作结果

	//add by ly 2014/7/24 活跃度相关
	static int L_SendActivenessInfo(lua_State *L);	//发送活跃度详细信息
	static int L_SendGetActivenessAwardRes(lua_State *L);	//发送领取活跃度奖励操作结果
	//add by lpd 2015/5/15 获取服务器启动时间
	static int L_GetServerStartTime(lua_State *L); //获取开服时间

	static int L_SendAddFriendToList(lua_State *L);//增加好友到列表
	static int L_SendFriendFailMsg(lua_State * L);//发送好友失败信息
	static int L_GetPlayerSidByName(lua_State *L);
	static int L_SendFriendList(lua_State *L);
	static int L_GetAllPlayer(lua_State *L);
	static int L_GetAllPlayerSid(lua_State *L);

	static int L_SendTreasureResult(lua_State *L);//发送聚宝盆结果
	static int L_SavePlayerToDB(lua_State *L);//备份保存

	static int L_GetServerID(lua_State *L);//zoneserverID

	//--20150731 军团相关
	static int L_SendCreateFactionResult(lua_State *L); //发送创建军团结果
	static int L_SendFactioninfo(lua_State *L); //发送自身军团信息
	static int L_SendFactionList(lua_State *L); //发送军团列表
	static int L_SendJoinResult(lua_State *L); //发送申请结果
	static int L_SendManagerFaction(lua_State *L); //发送管理军团的信息
	static int L_SDeleteFaction(lua_State *L); //删除军团信息
	static int L_SendSynFaction(lua_State *L); //同步军团信息
	static int L_SendFactionSceneList(lua_State *L); //发送军团副本列表信息
	static int L_SendSceneInfo(lua_State *L); //发送军团副本信息
	static int L_SendChallengeSceneMsg(lua_State *L); //发送军团副本挑战结果
	static int L_SendMailToPlayer(lua_State *L); //发送邮件
	static int L_SendFactionSceneRank(lua_State *L); //发送军团副本伤害排行
	static int L_SendFactionSalaryData(lua_State *L); //发送俸禄配置数据
	static int L_SendGetSalaryResult(lua_State *L); //发送俸禄领取结果
	static int L_SaveAllFactionData(lua_State *L); //保存所有军团信息
	static int L_SendFactionNotice(lua_State *L);//发送军团公告
	static int L_SendModifyNoticeResult(lua_State *L);//发送修改军团公告结果
	static int L_SendFactionLog(lua_State *L);//发送军团日志
	static int L_SendFcEmailResult(lua_State *L);//发送军团邮件结果
	//- 军团相关--


	static int L_SendInitPrayer(lua_State *L); //发送玩家的祈福初始化数据
	static int L_SendPrayerResult(lua_State *L); //发送玩家的祈福结果
	static int L_SendShopItemList(lua_State *L); //发送玩家商店道具列表
	static int L_SendShopBuyResult(lua_State *L); //发送玩家商店购买结构

	static int L_SendActivityCodeResult(lua_State *L); //发送玩家兑换激活码结果
	static int L_SendScriptData(lua_State *L); //发送脚本数据

	CMonster		*m_pMonster;
	CPlayer			*m_pPlayer;
	CNpc			*m_pNpc;
	CBuilding		*m_pBuilding;
	SPackageItem *m_pUseItem; //当前使用的物品
	SRawItemBuffer *m_pItemBuffer;//当前背包格子空间

	// 战斗相关
	CFightObject	*m_pAttacker;//攻击者
	CFightObject	*m_pDefencer;//被攻击

	// 分页菜单，控制NPC买卖
	static bool IfSendTheMultiPageMsg;
private:

	BYTE       m_CopyItem[8][8];                // 镜像玩家的物品栏

	// 脚本生成对象的列表
	std::vector<DWORD>  m_MonsterList;
	std::vector<DWORD>  m_NpcList;    

	friend class CSMonsterFunc;
	friend class CSNPCFunc;
	friend class CSPlayerFunc;
	friend class CSOtherFunc;
	friend class LuaFunctor;
	friend int NotifyMaster(CPlayer *);

	DWORD m_dwLoopTime;

	friend struct ExtraLuaFunctions;
	//20150122 wk 三国独立脚本模块
	friend class CSSanGuoFunc;
public:
	BOOL PrepareFunction( LPCSTR funcname );
	void PrepareParameters( lite::Serialreader &slr );
	void PushParameter( lite::Variant const &lvt );
	void PushVariantArray( lite::Serialreader &slr );
	void PushVariantTable( lite::Serialreader &slr, int tableType );
	void PushDWORDArray( std::list< DWORD > &lst );

	BOOL Execute( lite::Variant *result = NULL );
	void PrepareBreak();

private:
	int prepared_paramnum;

public:
	int load_errs;

public:
	std::map< std::string, STaskExBuffer > taskExMgr;
	void SaveTaskEx( LPCSTR key ); // 如果 key == NULL 则表示保存所有的缓存数据
	STaskExBuffer* LoadTaskEx( LPCSTR key );

public:
	static bool  AddMountsProperty(int mountIndex, int porperty, int v, int max=-1);

public:
	///////////////////////////////////////////////////////////////////////////////////////////////
	// 日常时间管理器
	INT64		m_EveryDayManagerTime;

	int OnEveryDayManagerRun();
	static int L_ReadEveryDayManagerTime(lua_State *L);
	static int L_WriteEveryDayManagerTime(lua_State *L);

	///////////////////////////////////////////////////////////////////////////////////////////////
};

class LuaFunctor
{
public:
	LuaFunctor( CScriptManager &script, LPCSTR funcName );
	LuaFunctor& operator [] ( lite::Variant const &lvt );
	BOOL operator () ( lite::Variant *result = NULL );

private:
	BOOL            initFailre;
	lua_State       *callStack;
	const LPCSTR    funcName;
	const int       stackTop;
};

extern CScriptManager g_Script; 

class CScriptState
{
public:
	CScriptState(CMonster* pMonster, CPlayer* pPlayer, CNpc* pNpc, CBuilding* pBuilding)
	{
		m_pMonster = g_Script.m_pMonster;
		m_pPlayer = g_Script.m_pPlayer;
		m_pNpc = g_Script.m_pNpc;
		m_pBuilding = g_Script.m_pBuilding;
		g_Script.SetCondition(pMonster, pPlayer, pNpc, pBuilding);
	}

	~CScriptState()
	{
		g_Script.SetCondition(m_pMonster, m_pPlayer, m_pNpc, m_pBuilding);
	}

private:
	CMonster* m_pMonster;
	CPlayer* m_pPlayer;
	CNpc* m_pNpc;
	CBuilding* m_pBuilding;
};