// ----- ScriptManager.h -----
//
//   --  Author: RenYi
//   --  Date:   06/13/03
//   --  Desc:   �ű�������ͷ�ļ�
// --------------------------------------------------------------------
//   --  ���ｫ���й���������˵����нű�
//       ������ű������ݼӻ��Ĺ��ܺ�����ʵ��
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

	// ԭ��ͨ���޸Ľű�ϵͳ�����Ѿ�֧�ֿɿش�С�����ݱ������ָ�����ݶ��ǽű��������л���Ĵ洢�ռ�
	void *saveData;		// dbMgr[sid].data ���ڱ�������ݱ�  ��ʵ������ָ��������ݽṹ��Ԥ���õĻ���ռ�
	void *tempData;		// dbMgr[sid].temp ������ʱ�����ݱ�  �����ָ����������ʱ��������ģ�ֻ��Ϊ�˴������ݶ�����

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


	  BOOL TriggerEvent(const char* name, int nType);     // �����ű���ĳ���¼�
	  BOOL TriggerEvent(const stEvent *pTE);
	  int Restore( int flag = 1,const char* name = NULL );                       // �ָ���ʼ����״̬������ֵΪʧ�ܵĽű�����     

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
		  CFightObject	*m_OldpAttacker;//������
	  };

	  void PushFightObjectCondition( CScriptManager::TempFightObjectCondition &tmp );
	  void PopFightObjectCondition(CScriptManager::TempFightObjectCondition &tmp);
	  void PushCondition( CScriptManager::TempCondition &tmp );
	  void PopCondition( CScriptManager::TempCondition &tmp );
	  void GetCondition( CScriptManager::TempCondition &tmp );

	  int GetQuestSkill(const char* name, DWORD mAttackerGID, DWORD mDefenderGID, float mDefenderWorldPosX, float mDefenderWorldPosY, BYTE dwSkillIndex, struct SQuestSkill_C2S_MsgBody &sqQuestSkillMsg);
	  int GetDamageValue(const char* name, DWORD mAttackerGID, DWORD mDefenderGID, WORD dskillID, struct CFightObject::SDamage &sdamage);
	  int GetEquiptFightPower(const char* name, DWORD dEquiptUserGID, BYTE bType, BYTE bIndex, DWORD &dEquiptFightPower);//ȡ��װ��ս����
	  int GetWorldTalkLimit(CPlayer *pPlayer, const char* funcname,WORD &wresult);  //���纰�����ƣ������ƶ���lua���ж�
	  int GetMonsterVipExp(CPlayer *pPlayer, const char* funcname, DWORD dmonsterexp, DWORD &dmonsteraddexp);//��������VIP�ӳ�
	  int GetItemUseLimit(CPlayer *pPlayer, const char* funcname, DWORD ditemindex, WORD &buse);

	  int LoadAllFactioninfo(struct SaveFactionData_New *FactionData);//���ݿ��ж�ȡ�ľ���
public:
	// ���빦�ܺ���  Ҳ����Ҫ����LUA��ִ�еĺ���
	// --- ����ֵ������int  
	static int L_DoFile(lua_State *L);

	static int L_rfalse(lua_State *L);
	static int L_rint(lua_State *L);
	static int L_RFalse(lua_State *L);
	static int L_GetStartServerTime(lua_State *L);
	// �����3���߽ӿ�
	static int L_giveMoney(lua_State *L);
	static int L_giveGoods(lua_State *L);
	static int L_checkgoods(lua_State *L);
	static int L_getitemnum(lua_State *L);

	// ��չ��������
	static int L_ActivePackage(lua_State *L);	
	static int L_SendActivePackage(lua_State *L); // ������չ������
	static int L_GetActivePackageNum(lua_State *L); // ��ȡ,��ʼ�ı���������,�Ѿ�������չ��������,�������������Ŀ


	// �����3Buffϵͳ��	
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

	// �����3
	static int L_giveSp(lua_State *L);
	static int L_giveExp(lua_State *L);
	static int L_fpgiveExp(lua_State *L);//�������;���
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
	static int L_loaditem(lua_State *L);           // ��ȡ���׵���Ʒ
	static int L_addblood(lua_State *L); 
	static int L_FollowLoadItem(lua_State *L);
	static int L_LoadFollowMenuInfo(lua_State *L);
	static int L_ClearFollowShop(lua_State *L);
	static int L_setmessage(lua_State *L);			// ���Npc�� �������Ļ�Ӧ��Ϣ
	static int L_setmessageA(lua_State *L);			// ���ѡ�� �������Ļ�Ӧ��Ϣ
	static int L_setmessageB(lua_State *L);			// NPC�����Ʒ
	static int L_setmessageC(lua_State *L);			// ��ҳ�ʦ
	static int L_setmessageR(lua_State *L);			// ִ����Ϣ���ظ����
	static int L_settipmsg(lua_State *L);			// ϵͳ��ʾ
	static int L_playcg(lua_State *L);				// play cg

	static int L_addtask(lua_State *L);				// ������ǵ������б���
	static int L_buytask(lua_State *L);				// ���������
	static int L_checktask(lua_State *L);			// ����������
	static int L_GetNeedBlank(lua_State *L);		// ������Ҫ��X����ƷY�����ܷ�ɹ�

	static int L_checkmoney(lua_State *L);			// ����Ǯ�Ƿ��㹻,(money,del,playerid)
	static int L_deltask(lua_State *L);			    // ɾ������ɵ�����
	static int L_getplayerdata(lua_State *L);		// �õ���ҵ�����

	//add by ly 2014/4/9
	static int L_SetPlayerProperty(lua_State *L);	//��������������
	static int L_SetPlayerObject(lua_State *L);		//����ִ�нű���������Ҷ��󣨲���Ϊ�����ȫ��ΨһGID��Ϊ0ʱ��ʾ��ǰ��ң�

	static int L_setplayerdata(lua_State *L);		// �ı���ҵ�����
	static int L_setplayerproperty(lua_State *L);   // ������ҵ�����
	
	//add by ly 2014/4/4 
	static int L_GetPlayerSID(lua_State *L);	//ͨ��������ֻ�ȡ��ҵ�SID

	static int L_getplayerGID(lua_State *L);		// �õ���ҵ����� int GetPlayerGID(DWORD sid)
	static int L_getplayerSkillproperty(lua_State *L);   //�õ���ҵļ�������
	

	static int L_getattackername(lua_State *L);     // �õ������ߵ�����


	static int L_getmonsterdata(lua_State *L);     // �õ����������
	static int L_setmonsterdata(lua_State *L);
	static int L_subtasknum(lua_State *L);         // ���һ������һ����������������
	static int L_getnpcdata(lua_State *L);         // �õ�NPC������
	static int L_gettaskdata(lua_State *L);        // �õ������еı���
	static int L_getgoodsdata(lua_State *L);       // �õ����ߵ�����
	static int L_getforgedata(lua_State *L);       // �õ����Ƶ�����
	static int L_getmonsterdatanew(lua_State *L);     // �õ����������

	static int L_putplayerfromdynamic(lua_State *L);
	static int L_putplayerto(lua_State *L);  
	static int L_putplayerback(lua_State *L);
	static int L_reitem(lua_State *L);
	static int L_isfull(lua_State *L);
	static int L_isFullNum(lua_State *L);

	static int L_teachgest(lua_State *L);           // �����书 
	static int L_getgestlevel(lua_State *L);        // �õ��书�ȼ�
	static int L_setgestlevel(lua_State *L);        // �����书�ȼ�
	static int L_getcurtelergy(lua_State *L);       // �õ���ǰ�ķ�
	static int L_setcurtelergyval(lua_State *L);    // �ı䵱ǰ�ķ�ֵ 
	static int L_paytheplayer(lua_State *L);        // ����Ҷ���
	static int L_settaskstring(lua_State *L);       // �����������(������)
	static int L_checkmoreitem(lua_State *L);       // ����Ʒ���
	static int L_gettelergy(lua_State *L);     
	static int L_gettelergydata(lua_State *L);		// �õ��������ָ���ķ�������

	static int L_StartCollection(lua_State*L);		// ��ҿ�ʼ�ɼ�
	static int L_MakeNpcDisappear(lua_State *L);

	static int L_getitemname(lua_State *L);    
	static int L_gettaskitemname(lua_State *L);
	static int L_getMountFoodAddExp(lua_State *L);		// �õ����߸�������(Ѫ,��,��)
	static int L_lostitems(lua_State *L);   

	static int L_fillcopyitem(lua_State *L);
	static int L_getscores(lua_State *L);          // �õ��������а����� 
	static int L_setregionpktype(lua_State *L);   
	static int L_broadcast(lua_State *L);   

	static int L_setlistsize(lua_State *L);        // �������ɶ����б��С
	//test
	static int L_zero(lua_State *L); 
	static int L_getcurtime(lua_State *L);          // �õ���ǰ��ʱ��
	static int L_getcurtimeinmin(lua_State *L);
	static int L_getcurtimeA(lua_State *L);        

	static int L_compilefile(lua_State *L);         // ֻ���ļ�������
	static int L_talktoplayer(lua_State *L);        // ����ҵĶԻ�

	static int L_delitembypos(lua_State *L);        // ͨ������ɾ������
	static int L_GetKillInfo(lua_State *L);         // �õ�׷ɱ�����Ϣ
	static int L_teachteamgest(lua_State *L);       // ѧϰ���μ�
	//static int L_cure(lua_State *L);              // ����
	static int L_setplayerscriptstate(lua_State *L);    // ������ҽű�״ֵ̬
	static int L_getplayerscriptstate(lua_State *L);    // �õ���ҽű�״ֵ̬
	static int L_setplayer(lua_State *L);               // �ı䵱ǰ���
	static int L_setplayericon(lua_State *L);
	static int L_cleanplayer(lua_State *L); //�����ǰ���

	static int L_setmultipletime(lua_State *L);     // ���ö౶ʱ��
	static int L_setnmultiple(lua_State *L);        // ���ñ���
	static int L_getmultipletime(lua_State *L);     // �õ��౶
	static int L_getmulleavetime(lua_State *L);     // �õ��౶״̬ʣ��Сʱ��
	static int L_checkweekformul(lua_State *L);     // �౶����״̬
	static int L_setlonemanvalue(lua_State *L);     // ���ö�����״ֵ̬
	static int L_getmultipletimeinfo( lua_State *L );// ȡ�õ�ǰ�ܵ�˫��ʱ��ʱ��

	static int L_CreateRegion(lua_State *L);        // ��̬����һ������
	static int L_QueryEncouragement(lua_State *L);			// ��̬����һ������
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
	static int L_SendBoardCastMsg( lua_State *L );		//��㲥
	static int L_GetServerTime(lua_State *L);

	static int L_AddPlayerProperty(lua_State *L);//���ý�ɫ����:HP,MP,SP(����)
	static int L_AddMountsProperty(lua_State *L);//������������
	static int L_Dismount(lua_State *L);		//����

	// ������ؽӿ�
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
	static int L_CheckObject(lua_State *L); //�������Ƿ����

	// �ֿ���ؽӿ�
	static int L_OpenWareHouse(lua_State *L);

	// ս��ϵͳ��ؽӿ�
	static int L_DelFightBuffType(lua_State *L);
	static int L_AddFightBuff(lua_State *L);
	static int L_PlayerAddBuff(lua_State *L);
	static int L_AddBuff(lua_State *L);//�����һ�¸���BUFF,�����ѣ��޵е�
	static int L_FightPetAddBuff(lua_State *L);//���ͳ�ҩbuff
	static int GetWindMove(DWORD Type, DWORD DefencerID, WORD skillid, float& DestPosX, float& DestPosY);
	static int L_CheckWindMove(lua_State *L);
	static int L_StartWindMove(lua_State *L);
	static int L_SendAttackMsg(lua_State *L);
	static int L_MoveToPos(lua_State *L);//�ƶ������
	static int L_CheckFightState(lua_State *L);//��ȡ��ǰBUFF״̬
	static int L_print(lua_State *L);//��ӡ���

	static int L_Gettimelimit(lua_State *L);

	static int L_loaddropgoods(lua_State *L);
	static int L_loaddropgoodsnum(lua_State *L);
	static int L_loaddroptime(lua_State *L);
	static int L_loaddropmonsterid(lua_State *L);

	static int L_monsterdropitem(lua_State *L);///���������Ʒ
	static int L_setplayerpoint(lua_State *L);///����Ҽӵ�
	static int L_monstertalk(lua_State *L);///����˵��,����

	//���������������Լ������ӿ�
	static int L_getplayerteam(lua_State *L);
	static int L_getplayerteamname(lua_State *L);
	static int L_setplayerdatanew(lua_State *L);
	static int L_creategift(lua_State *L);	//����һ�����
	static int L_usegift(lua_State *L);	//ʹ��һ�����
	static int L_AddGiftJudgePackage(lua_State *L);		//���Ԫ���������ʱ���ж��Ƿ񱳰��㹻
	static int L_QuestUpdateBoss(lua_State *L);	//������¹�������
	static int L_setactivityNotice(lua_State *L);	//�����
	static int L_GetLiveMonster(lua_State *L);	//��ȡ�����й�������
	//������ҵĺ��Ѷ�
	static int L_setplayerDegree(lua_State *L);
	//����ѧϰ����
	static int L_fpactiveskill(lua_State* L);
	//����ϵͳ
	static int L_tizhiGrow(lua_State* L);		//��������
	static int L_tizhiUpgrade(lua_State* L);	//����ͻ��
	static int L_kylinArmActive(lua_State* L);  //��������۹���
	static int L_addYuanqi(lua_State* L);		//����Ԫ��

	// �ķ���������
	static int L_ConsumeTelergy(lua_State* L);
	static int L_ConsumeTelergyProperty(lua_State* L);

	// �����
	static int L_ProcessBlessInfo(lua_State* L);
	static int L_FillBlessData(lua_State* L);
	static int L_OnBlessResult(lua_State* L);
	static int L_OnPlayerBlessTime(lua_State* L);
	static int L_GetPlayerMoney(lua_State* L);
	static int L_ChangeMoney(lua_State* L);
	static int L_OnErrorStopBless(lua_State* L);
	static int L_IsHaveSpaceCell(lua_State* L);
	static int L_OnProcessNotice(lua_State* L);
	//��������������
	//static int L_StopTime(lua_State* L);
	//static int L_StartTime(lua_State* L);
	//static int L_CleanTime(lua_State* L);
	//static int L_IsStopTime(lua_State* L);
	static int L_UpdateCountDownGiftState(lua_State* L);
	static int L_UpdateOnlineGiftState(lua_State* L);
	static int L_ResetCountDownGiftState(lua_State* L);
	static int L_ResetOnlineGiftState(lua_State* L);
	static int L_ResetCountDownGiftTime(lua_State* L);
	static int L_GetCountDownGiftBeginTime(lua_State* L);//�õ�����ʱ��ʼʱ��
	static int L_GetCountDownGiftState(lua_State* L);//�õ�����ʱ��������׶�
	static int L_GetOnlineGiftState(lua_State* L);
	static int L_GetOnlineBeginTime(lua_State* L);
	static int L_SendBufferToClinet(lua_State* L);
	static int L_SendItemList(lua_State* L);
	static int L_GetNowTime(lua_State* L);
	static int L_SendGiftSucceed(lua_State* L);
	static int L_GetPlayerTime(lua_State* L); //��ȡ������ʱ�䣬0--�ϴε�¼ʱ�䣬1--�ϴ�����ʱ�䣬2--���ε�¼ʱ��
	static int L_SetPlayerVipLevel(lua_State* L); //�������VIP�ȼ�
	

	//��Ǯ�۳�
	static int L_CheckPlayerMoney(lua_State* L);

	//lua�����ݴ洢,���ڹ涨ÿ������Ϊһ��int�Ĵ�С
	static int L_SaveToPlayerData(lua_State* L);
	static int L_LoadFromPlayerData(lua_State* L);


	//lua�����ݴ洢table
	static int L_SavePlayerData(lua_State* L);
	static int L_LoadPlayerData(lua_State* L);

	//lua�����ݴ洢table
	static int L_SavePlayerTable(lua_State* L);
	static int L_LoadPlayerTable(lua_State* L);
	
	static int L_SendPlayerReward(lua_State* L);	//���ͽ���
	static int GetTableData(lua_State* L,std::vector<lite::Variant > &keyvariant,std::vector<lite::Variant> &vlvariant);//��ȡ�������
	static int MakeRewardData(int nReward,BYTE bState,BYTE blocation);
	

	static int findDataPos(const char* tablename ,bool bissave = true);//����һ����λ���ڴ洢����,���߲���һ���Ѿ����ڵ�����λ��
	static int getDataLenth();//ȡ�����������ܳ���
	static int UpdateLenth();//�������������ܳ���
	//AI�ӿ�
	static int L_PlayerClearBuff(lua_State* L);
	static int L_LoadPlayerModifValue(lua_State* L);
	static int L_UnLoadPlayerModifValue(lua_State* L);
	static int L_OnBackFightPet(lua_State* L);
	static int L_OnCallOutFightPet(lua_State* L);

	static int L_giveItemList(lua_State* L);
	

	//��������ͻ���¼
	static int L_AddPlayerRoseRecod(lua_State* L);
	// ���񵤵���
	static int L_HuiShenDanRecall(lua_State* L);
	// ȫװ�����Լӳ�
	static int L_ModifyPlayerAttributes(lua_State* L);
	// ���ֵ�ļ���ͷ���
	static int L_LuaRandRange(lua_State* L);

	static int L_SetCrilial(lua_State* L);

	static int L_LoadProtectInfo(lua_State * L);

	static int L_OnPartEnterNum(lua_State * L);//���븱���ɹ���ʱ���������

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

	//�����ӹؿ��½� ���͹ؿ���Ϣ
	static int L_SendSceneMsg(lua_State *L);
	//����ѡ��Ĺؿ���Ϣ
	static int L_SendSelectSceneMsg(lua_State *L);
	//���ͽ���ؿ�����Ϣ
	static int L_SendEnterSceneMsg(lua_State *L);
	static int L_CreateScene(lua_State* L);//��̬�����ؿ�

	static int L_SendRewardMsg(lua_State *L);//���͹ؿ�����
	static int L_SendWinRewardMsg(lua_State *L);//���ͳ��Ƶõ��Ľ�������
	static int L_SendDebrisinfo(lua_State *L);//������Ƭ�ռ���Ϣ
	static int L_SendDebrisAward(lua_State *L);//���ͷ��ͻ��½���
	static int L_SendSceneSDListMsg(lua_State *L);//���͹ؿ�ɨ�������б���Ϣ

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

	static int L_GetSceneCount(lua_State *L);//ȡ�ó����ؿ�����

	static int L_GetTableLenth(lua_State *L);	
	static int L_GetObjectType(lua_State *L);
	static int L_DestroyScene(lua_State *L);//���ٹؿ�
	static int L_Removeobj(lua_State *L);//���ٹؿ�ǰ�Ƴ����еĶ���

	static int L_GetBlankCell(lua_State *L);//��ȡ�ո�������

	static int L_SetQusetSkillMsg(lua_State *L);//����һ����������
	static int L_TimeGetTime(lua_State *L);
	static int L_ProcessQuestSkill(lua_State *L);//����ʹ�ü���
	static int L_GetSkillIDBySkillIndex(lua_State *L);//������������ҳ��ڡ��������ñ��еļ���ID
	static int L_SetCacheSkill(lua_State *L);//���û���ļ��ܵ�player
	


	static int L_GetPlayerInScene(lua_State *L);//�Ƿ�����ͨ����
	static int  L_SendPassFailMsg(lua_State *L);//����ͨ��ʧ����Ϣ
	static int L_PlayerRelive(lua_State *L); //��Ҹ���

	static int L_SendSDMsg(lua_State *L);//����ɨ����ֹ�����Ϣ
	static int L_SendSDReward(lua_State *L);//����ɨ��������ȡ�����Ϣ
	static int L_SendPlayerDeadAddMsg(lua_State *L);// �������ʱ�ĸ�����Ϣ�������͸��������������
	static int L_SendReliveResult(lua_State *L);//���͸�����
	static int L_SendSceneListMsg(lua_State *L);//���������½��б�
	static int L_SendSceneFinishMsg(lua_State *L);//���͹ؿ����
	static int L_GetSceneRegionID(lua_State *L);//���ڹؿ�GID

	static int L_GetObjectData(lua_State *L);//��ȡ��������
	static int L_GetObjectFightData(lua_State *L);//��ȡ����ս������
	static int L_SendObjectDamage(lua_State *L);//�����˺�
	static int L_SetObjectDamage(lua_State *L);//�����˺�����
	static int L_KillAllMonster(lua_State *L);//ɱ�����й���
	static int L_ChangeObjectData(lua_State *L);//�ı��������
	static int L_SetObjectData(lua_State *L);//���ö�������
	//װ�����
	static int L_GetEquipment(lua_State *L);//��ȡװ��
	static int L_SendIntensifyInfo(lua_State *L);//����װ��ǿ�������Ϣ
	static int L_SetEquipment(lua_State *L);//����װ��
	static int L_IsLockedEquipColumnCell(lua_State *L); // ���װ�����Ƿ�������		
	static int L_LockEquipColumnCell(lua_State *L);// �����������װ�����ĸ���
	static int L_SendIntensifyResult(lua_State *L);//����װ��ǿ�����
	static int L_SendQualityInfo(lua_State *L);//����װ������Ʒ�������Ϣ
	static int L_DeleteItem(lua_State *L);//ɾ������
	static int L_DelEquipment(lua_State *L);//ɾ��װ��
	static int L_AddItem(lua_State *L);//���ӵ�������
	static int L_RecvUseItem(lua_State *L);//ʹ��װ������
	static int L_SendQualityResult(lua_State *L);//װ�����׽��
	static int L_GetItemBaseData(lua_State *L);//��ȡ������ϸ��Ϣ
	static int L_SendCleanColdResult(lua_State *L);//�������ǿ����ȴʱ����
	static int L_SendRiseStarInfo(lua_State *L);//��������������Ϣ
	static int L_SendRiseStarResult(lua_State *L);//�������ǽ��
	static int L_SendEquiptKeYinInfo(lua_State *L);//���Ϳ�ӡ������Ϣ
	static int L_SendEquipJDingResult(lua_State *L);////װ���������
	static int L_SendEquipJDingInfo(lua_State *L);//����װ����������
	static int L_SendEquipJDingColor(lua_State *L);////���ͼ�������ɫ
	static int L_SendKeYinChangeData(lua_State *L);//���ͼ���ת������
	static int L_SendSuitcondition(lua_State *L);//������װ��Ϣ
	static int L_SendKeYinResult(lua_State *L);//���Ϳ�ӡ���

	//����IP
	static int L_GetPlayerIP(lua_State *L);//���ip
	  
	//���δ������
	static int L_GetIsNewPlayer(lua_State *L);//�Ƿ��������
	static int L_SetIsNewPlayer(lua_State *L);//���������

	static int L_SetTaskFinish(lua_State *L);//�������
	static int L_InitPlayerData(lua_State *L);//��ʼ�������������  
	static int L_ReloadPlayerData(lua_State *L);//�ض�
	static int L_KickPlayer(lua_State *L);//������
	//����
	static int L_GetTransformersLevel(lua_State *L);//��ȡ״̬����ȼ�
	static int L_SetTransformersLevel(lua_State *L);//����״̬����ȼ�,״̬
	static int L_OnSendTransformersInfoMsg(lua_State *L);
	static int L_SendTransformersSwitch(lua_State *L);
	static int L_SendMyState(lua_State *L);//ͬ�����״̬
	static int L_InitPlayerProperty(lua_State *L);//�ָ���ҳ�ʼ����
	static int L_GetCurrentSkillLevel(lua_State *L);//�õ����ܵȼ�
	static int L_SetCurrentSkillLevel(lua_State *L);//���ü��ܵȼ�
	static int L_GetSkillProficiency(lua_State *L);//�õ�����������
	static int L_AddSkillProficiency(lua_State *L);//���ü���������
	static int L_GetSkillLevel(lua_State *L);//�õ����ܵȼ�
	static int L_SetSkillLevel(lua_State *L);//���ü��ܵȼ�
	static int L_SendTransformersSkillInfo(lua_State *L);// ���ͱ�������Ϣ

	//ȫ����ȡ����
	static int L_LoadTable(lua_State *L);//��ȡȫ�ֱ�
	static int L_SaveTable(lua_State *L);//����ȫ�ֱ�

	//�������Ǽ��� add by ly 2014/3/18
	static int L_AddPlayerXinYangProperty(lua_State *L);	//������ҵ�����

	//��ҫ��� add by ly 2014/3/25
	static int L_InitNewPlayerGloryInfo(lua_State *L);		//��ʼ��������ҵ���ҫ��Ϣ
	static int L_SendClientGloryInfo(lua_State *L);			//��ͻ��˷�����ҫ��Ϣ
	static int L_QstGetGloryAward(lua_State *L);		//��������ҫ����
	static int L_QstHandleTitleMsg(lua_State *L);		//������ƺ���Ϣ
	static int L_SetKillAllMonster(lua_State *L);		//��������ڹؿ���ɱ�����й�����ڲ����ã�

	static int L_TaskKillMonster(lua_State *L);//����ɱ�ּ���
	static int L_SendBatchesMonster(lua_State *L);
	static int L_Syneffects(lua_State *L);//ͬ����Ч
	static int L_SetStopTracing(lua_State *L);//���ﷵ�س�����

	//��������Ϣ
	static int L_SendArenaInitMsg(lua_State *L);////�ظ��򿪾�����ѡ��Ľ���
	static int L_SendArenaRewardMsg(lua_State *L);////���;���������
	static int L_SendArenaQuestMsg(lua_State *L);////�ظ�����������


	static int L_CopyFromPlayer(lua_State *L);//������ҵ����ݵ�����
	static int L_CopyPlayerToTable(lua_State *L);//����������ݱ��浽����
	static int L_GetObjectReginType(lua_State *L);//���߶������ڳ�������
	static int L_GetSkillDamageRate(lua_State *L);//���ܵȼ���Ӧ�˺�����
	static int L_GetBuffData(lua_State *L);//��ȡBUff����
	

	//add by ly 2014/4/10  ������GM��ز���
	static int L_SendMessageToAllPlayer(lua_State *L);	//������Ϣ���������
	static int L_PutPlayerToDyncRegion(lua_State *L);	//������̬��ͼ���ҽ���ҷ���
	static int L_SetPlayerGMLevel(lua_State *L);	//�������GM�ȼ���ֻ�еȼ��ﵽ���п���Ȩ��
	static int L_BaseRelive(lua_State *L);	//ԭ�ظ���,�����L_PlayerReliveʵ���˷�װ
	static int L_CreateMonsterRegion(lua_State *L);		//��ͼˢ��mapid x y monsterid num
	static int L_SetMapCollision(lua_State *L);		//���õ�ͼ��ײ״̬���򿪻�رգ�
	static int L_GetOnlinePlayerNum(lua_State *L);	//��ȡ��ǰ����������
	static int L_SendSysCall(lua_State *L);		//����ϵͳ����
	static int L_SendGmExceResult(lua_State *L);	//����GM����ִ�н��


	//add by ly 2014/4/16  ���ز���
	static int L_ReturnDailyListInfo(lua_State *L);	//��ȡ����ճ���б���Ϣ����ÿ�춨ʱ���ù��ܣ�
	static int L_ResetDailyByTime(lua_State *L);	//��������Ѿ���ɵ��ճ���б���Ϣ����������Ϊ���ճ�����ʱ�䣻�Ѿ���ɵ��ճ����ݣ�����ʱ����������øñ�����
	static int L_DestroyDailyArea(lua_State *L);	//�����ճ������
	static int L_ResponseEntryDailyArea(lua_State *L);	//�����ճ�������Ӧ����,0��ʾ����ʧ�ܣ�1��ʾ����ɹ���4��ʾ�ɹ������
	static int L_SendDailyAwardToPlayer(lua_State *L);	//�����ճ���Ľ��������
	static int L_GetPlayerInDailyArea(lua_State *L);	//��ȡ����Ƿ����ճ��������
	static int L_SendNextBatchMonsterInf(lua_State *L);	//֪ͨ��ҵ�ǰ��������������һ��Ϊ�ڼ����ֺ���һ���ֻ��ж��ˢ��

	static int L_SendDailyOpenFlag(lua_State *L);	//����ʧ��԰���ű�־
	static int L_BroadCastDailyOpenFlag(lua_State *L);	//�㲥ʧ��԰���ű�־

	static int L_SendJuLongShanAward(lua_State *L);		//���;���ɽ̽�ս�����Ϣ

	static int L_SendTarotBaseData(lua_State *L);	//������������ƻ���������Ϣ
	static int L_SendAddTurnOnTarotData(lua_State *L);	//������ҷ�����������������Ϣ
	static int L_SendPlayerGetTarotAwardRes(lua_State *L);	//���������ȡ�����ƽ����Ĳ��������Ϣ
	static int L_SendResetTurnOnTarotData(lua_State *L);	//������������Ѿ�������������������Ϣ

	static int L_SendCurDayOnlineData(lua_State *L);	//������ҵ������߽�����Ļ���������Ϣ
	static int L_SendCurDayOnlineLong(lua_State *L);	//������ҵ������߽����������ʱ��
	static int L_SendCurDayOnlineGetAwardRes(lua_State *L);		//���������ȡ�������
	static int L_NotifyCurDayOnlineReset(lua_State *L);		//֪ͨ���ÿ������ʱ�������
	static int L_NotifyCurDayOnlineAddNewAward(lua_State *L);	//��Ҵﵽһ���µĿ�����ȡ�Ľ���

	static int L_GetPlayerAddUpLoginTime(lua_State *L);		//��ȡ����ۼƵ�½ʱ��
	static int L_SendPlayerAddUpLogintime(lua_State *L);	//��������ۼƵ�½ʱ��
	static int L_SendPlayerAddUpLoginInf(lua_State *L);		//��������ۼƵ�½������Ϣ
	static int L_NotifyAddUpLoginAddNewAward(lua_State *L);	//��Ҵﵽ���µĿ�����ȡ���ۼƵ�½����
	static int L_SendAddUpLoginGetAwardRes(lua_State *L);	//��������ۼƵ�½��ȡ�����Ľ��

	static int L_SendPlayerRFBInf(lua_State *L);	//������ҳ�ֵ���������Ϣ
	static int L_SendGetRFBAwardRes(lua_State *L);	//���������ȡ��ֵ�����������
	static int L_NotifyRFBCanGetNewAward(lua_State *L);	//֪ͨ��ң���ֵ�������µĽ���������ȡ

	static int L_SendPlayerLRInf(lua_State *L);		//������ҵȼ�����������Ϣ
	static int L_SendGetLRAwardRes(lua_State *L);	//���������ȡ�ȼ������������
	static int L_NotifyLRCanGetNewAward(lua_State *L);	//֪ͨ��ҿ��Ի�õȼ������µĽ���
	static int L_BroadCastLRResidueTimes(lua_State *L);	//�㲥�ȼ�����ʣ�����

	static int L_SendFWInf(lua_State *L);	//��������Ħ����������Ϣ
	static int L_SendPlayerFWInf(lua_State *L);	//�����������Ħ���ֵ���ʱ������Ϣ
	static int L_SendFWSelectGoodsInf(lua_State *L);	//����ѡ�е���Ʒ�����Ϣ
	static int L_SendFWGEtAwardRes(lua_State *L);	//����ѡ������Ʒ��ȡ�������
	static int L_BroadCastFWUptRecordInf(lua_State *L);	//�㲥����Ħ���ֵļ�¼��Ϣ�Ѿ����

	static int L_SynRechargeTatolGlod(lua_State *L);	//ͬ������ܵĳ�ֵ�������

	//add by ly 2014/4/28
	static int L_SendSignInAwardInfo(lua_State *L);	//����ÿ��ǩ��������Ϣ
	static int L_SendGetSignInAwardResult(lua_State *L);	//������ȡÿ��ǩ�������Ľ��

	//add by ly 2014/4/30
	static int L_GetPlayerSchool(lua_State *L);		//��ȡ��ҽ�ɫ����(3Ϊ��Ѫ��2Ϊ���ˣ�1Ϊ����)

	//add by ly 2014/5/8 �̳����
	static int L_SendShopListInfo(lua_State *L);		//�����̳���Ϣ
	static int L_SendShopGoodsListInfo(lua_State *L);		//�����̳�����Ʒ��Ϣ
	static int L_NotityGoodsUpdated(lua_State *L);		//֪ͨ��Ʒ���´���
	static int L_SendBuyGoodsRes(lua_State *L);		//���͹�����Ʒ�������
	static int L_GetGoodsIndexByTriIndex(lua_State *L);	//ͨ���̳����ͣ���Ʒ����1����Ʒ����2������Ʒ��Ψһ����
	static int L_GetTriIndexByGoodsIndex(lua_State *L);		//ͨ����ƷΨһ������ȡ�̳����ͣ���Ʒ����1����Ʒ����2
	static int L_SendShopCountDownRes(lua_State *L);	//�����̳�ˢ�µ���ʱʱ��
	static int L_SendGoodsInfo(lua_State *L);	//����һ����Ʒ��Ϣ

	//add by ly 2014/5/16 ��ҵ�һ��ע�����ã���������һ�����ܵ���ҵļ������������
	static int L_FirstRegCallSetPlayerSkill(lua_State *L);

	//add by ly 2014/5/16
	//VIP���
	static int L_SetCurVipFactor(lua_State *L);		//���õ�ǰVIP�ȼ���������
	static int L_SendVipStateInf(lua_State *L);		//�������VIP��״̬��Ϣ
	static int L_SendGiftInfo(lua_State *L);	//�������������Ϣ
	static int L_SendGetOrButGiftRes(lua_State *L);		//������ȡ���ȡVIP��������Ϣ
	static int L_AddSomeGoods(lua_State *L);	//���һϵ�е��ߵ�������

	//add by ly 2014/5/23 ���������Ϣ
	static int L_GetPlayerPetNum(lua_State *L);	//��ȡ����Ѿ�ӵ���˵ĳ�������
	static int L_AddNewPet(lua_State *L);	//��ӳ���
	static int L_SendCardPetInf(lua_State *L);	//���Ϳ��Ƴ����б���Ϣ
	static int L_SendPetCardInf(lua_State *L);	//���ͳ��￨����Ϣ
	static int L_ExceSwallowPetOpt(lua_State *L);	//���ɳ���
	static int L_PetBreachOpt(lua_State *L);	//����ͻ�ƣ��������ǣ�
	static int L_PetRenameOpt(lua_State *L);	//���ĳ�������
	static int L_SendPetMergerInf(lua_State *L);	//���ͳ������ɽ����Ϣ
	static int L_SendPetBreachInf(lua_State *L);	//���ͳ���ͻ����Ϣ
	static int L_SendPetRenameInf(lua_State *L);	//���ͳ�����������Ϣ
	static int L_GetPetIDAndStar(lua_State *L);		//��ȡ�����ID�ͳ�����Ǽ�ͨ����ҳ��������
	static int L_PetSkillCfg(lua_State *L);		//���ó��ﵱǰ����ʹ�õļ���
	static int L_GetPetOwer(lua_State *L);//��ȡ��������GID
	static int L_CalAndSendPetExp(lua_State *L);	//����������ؾ�����Ϣ�������͸����
	static int L_JudgePetDurableIsZero(lua_State *L);	//�жϳ�������Ƿ�Ϊ0
	static int L_SendResumeDurableNeedGold(lua_State *L);	//���ͻָ����������Ҫ�Ľ��
	static int L_SendGoldResumeDurableRes(lua_State *L);	//���ͽ�һָ���������Ľ��
	static int L_UptPetDurable(lua_State *L);	//���³������
	static int L_NotifyPetDurableUpt(lua_State *L);		//֪ͨ��������Ѿ�����
	static int L_PetStudySkillByBook(lua_State *L);		//����ͨ��������ѧϰ����
	static int L_ActivePetSkill(lua_State *L);		//������＼��

	//add by ly 2014/6/5 ��ȡս�������״̬����
	static int L_GetFightObjActionID(lua_State *L);

	//add by ly 2014/6/25 ֪ͨ�����Ϣ����lua��UTF_8����ת��ΪUnicode�룬�����͸���ң�
	static int L_NotifyMsgUtf8ToAnsi(lua_State *L);

	//add by ly 2014/6/27����
	static int L_GetFirstPayFlag(lua_State *L);		//��ȡ�״γ�ֵ״̬
	static int L_SetFirstPayFlag(lua_State *L);		//�����״γ�ֵ״̬
	static int L_GetEveryDayPayFlag(lua_State *L);	//��ȡÿ�ճ�ֵ��״̬
	static int L_SetEveryDayPayFlag(lua_State *L);	//����ÿ�ճ�ֵ��״̬
	static int L_SendEveryDayPayAwardState(lua_State *L);	//����ÿ�ճ�ֵ����״̬
	static int L_SendGetEDPAwardRes(lua_State *L);		//������ȡÿ�ճ�ֵ�������

	//add by ly 2014/7/22 ���а����
	static int L_SendRandListData(lua_State *L);	//�������а�����
	static int L_SendRandAwardFlag(lua_State *L);	//����������а��콱״̬
	static int L_SendGetRandAwardRes(lua_State *L);	//������ȡ���н����������

	//add by ly 2014/7/24 ��Ծ�����
	static int L_SendActivenessInfo(lua_State *L);	//���ͻ�Ծ����ϸ��Ϣ
	static int L_SendGetActivenessAwardRes(lua_State *L);	//������ȡ��Ծ�Ƚ����������
	//add by lpd 2015/5/15 ��ȡ����������ʱ��
	static int L_GetServerStartTime(lua_State *L); //��ȡ����ʱ��

	static int L_SendAddFriendToList(lua_State *L);//���Ӻ��ѵ��б�
	static int L_SendFriendFailMsg(lua_State * L);//���ͺ���ʧ����Ϣ
	static int L_GetPlayerSidByName(lua_State *L);
	static int L_SendFriendList(lua_State *L);
	static int L_GetAllPlayer(lua_State *L);
	static int L_GetAllPlayerSid(lua_State *L);

	static int L_SendTreasureResult(lua_State *L);//���;۱�����
	static int L_SavePlayerToDB(lua_State *L);//���ݱ���

	static int L_GetServerID(lua_State *L);//zoneserverID

	//--20150731 �������
	static int L_SendCreateFactionResult(lua_State *L); //���ʹ������Ž��
	static int L_SendFactioninfo(lua_State *L); //�������������Ϣ
	static int L_SendFactionList(lua_State *L); //���;����б�
	static int L_SendJoinResult(lua_State *L); //����������
	static int L_SendManagerFaction(lua_State *L); //���͹�����ŵ���Ϣ
	static int L_SDeleteFaction(lua_State *L); //ɾ��������Ϣ
	static int L_SendSynFaction(lua_State *L); //ͬ��������Ϣ
	static int L_SendFactionSceneList(lua_State *L); //���;��Ÿ����б���Ϣ
	static int L_SendSceneInfo(lua_State *L); //���;��Ÿ�����Ϣ
	static int L_SendChallengeSceneMsg(lua_State *L); //���;��Ÿ�����ս���
	static int L_SendMailToPlayer(lua_State *L); //�����ʼ�
	static int L_SendFactionSceneRank(lua_State *L); //���;��Ÿ����˺�����
	static int L_SendFactionSalaryData(lua_State *L); //����ٺ»��������
	static int L_SendGetSalaryResult(lua_State *L); //����ٺ»��ȡ���
	static int L_SaveAllFactionData(lua_State *L); //�������о�����Ϣ
	static int L_SendFactionNotice(lua_State *L);//���;��Ź���
	static int L_SendModifyNoticeResult(lua_State *L);//�����޸ľ��Ź�����
	static int L_SendFactionLog(lua_State *L);//���;�����־
	static int L_SendFcEmailResult(lua_State *L);//���;����ʼ����
	//- �������--


	static int L_SendInitPrayer(lua_State *L); //������ҵ�����ʼ������
	static int L_SendPrayerResult(lua_State *L); //������ҵ������
	static int L_SendShopItemList(lua_State *L); //��������̵�����б�
	static int L_SendShopBuyResult(lua_State *L); //��������̵깺��ṹ

	static int L_SendActivityCodeResult(lua_State *L); //������Ҷһ���������
	static int L_SendScriptData(lua_State *L); //���ͽű�����

	CMonster		*m_pMonster;
	CPlayer			*m_pPlayer;
	CNpc			*m_pNpc;
	CBuilding		*m_pBuilding;
	SPackageItem *m_pUseItem; //��ǰʹ�õ���Ʒ
	SRawItemBuffer *m_pItemBuffer;//��ǰ�������ӿռ�

	// ս�����
	CFightObject	*m_pAttacker;//������
	CFightObject	*m_pDefencer;//������

	// ��ҳ�˵�������NPC����
	static bool IfSendTheMultiPageMsg;
private:

	BYTE       m_CopyItem[8][8];                // ������ҵ���Ʒ��

	// �ű����ɶ�����б�
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
	//20150122 wk ���������ű�ģ��
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
	void SaveTaskEx( LPCSTR key ); // ��� key == NULL ���ʾ�������еĻ�������
	STaskExBuffer* LoadTaskEx( LPCSTR key );

public:
	static bool  AddMountsProperty(int mountIndex, int porperty, int v, int max=-1);

public:
	///////////////////////////////////////////////////////////////////////////////////////////////
	// �ճ�ʱ�������
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