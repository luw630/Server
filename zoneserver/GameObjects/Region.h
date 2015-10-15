#pragma once

//#define USEMAPCOLLISION 

#include "BaseObject.h"
#include "networkmodule/BuildingTypedef.h"
#include "networkmodule/UpgradeMsgs.h"
#include "player.h"
#include "trigger.h"
#include "MapServer.h"

const __int32 IID_REGION = 0x11030636; 

enum
{
	LimitItem = 0,		
	UnDealItem,		
	LimitSkill,		
	TrigerBuffMonster,	
	TrigerBuffPlayer,	
	Limit_max,
};

#define MY_MAP_VERSION	4

#define GetCurArea(pos, len) ((pos)/(len)) // (((pos+1)/len) + (((pos+1)%len)?1:0) - 1)

// 本次方案中的场景结构说明
// Region是场景的主体，以一张地图为单位，使用.map文件
// 在Region中存在一系列的Area
// Area以矩阵的方式排列，作为一个个的小场景，以解决大场景造成的负担
// AddObject用来添加角色等存在AI的东西，不过也可以考虑一下动态改变场景
class CMonster;
class CNpc;
class CBuilding;

class CRegion : public IObject, public CObjectManager
{
public:
	typedef SAUpdateNpcListMsg::NpcInfo NpcList;

private:
	IObject *CreateObject(const __int32 IID);

public:
    IObject *VDC(const __int32 IID) { return (IID == IID_REGION) ? this : IObject::VDC(IID); }

public:
	CRegion() : m_MutateMonsterNum(0) {}

	virtual int AddObject(LPIObject pChild);
	virtual int DelObject(LPIObject pChild);
	void OnClose();

protected:
	void OnRun(void);
	int OnCreate(_W64 long pParameter); // 这里传入的只是一个ID号，根据具体的ID载入相应的场景

private:
	int LoadTileMark(WORD ID);			// 装载地图阻挡信息
	int LoadNpc(LPCSTR filename);		// 装载场景NPC
	int LoadMonster(LPCSTR filename);	// 装载场景怪物
	int LoadTrigger(LPCSTR filename);	// 装载场景陷阱触发器
	int LoadTimer(LPCSTR filename);		// 装载场景时间触发器
    int LoadMonsterBR(LPCSTR filename); // 装载场景怪物批量刷新配置
	///新地图碰撞
	 int LoadRegincollision(WORD ID); // 装载地图阻挡碰撞
	 
public:
	LPIObject &GetArea(WORD x, WORD y);
    const class CTrigger *GetTrapTrigger(WORD index);
	BOOL Move2Area(LPIObject pObj, WORD x, WORD y, bool bUseTile = false);	// 将一个对象转移到目标区块
	void GetAreaNpcList(std::vector<CRegion::NpcList>& npcList);
	bool IsCanMove(float x,float y);
	bool		m_bUseCollision;
	void SetUseMapCollision(bool use){m_bUseCollision=use;}
public:
	LPIObject SearchNpcByScriptID(DWORD dwID);
	LPIObject SearchObjectListByRegion(DWORD dwGID);
	LPIObject SearchObjectListInArea(DWORD dwGID, WORD x, WORD y);
	LPIObject SearchObjectListInAreas(DWORD dwGID, WORD xCenter, WORD yCenter);

    LPIObject SearchObjectByPos(WORD xCenter, WORD yCenter);

	// 这个函数是以下函数的扩展版，表现的具体方式是该对象朝着场景上一个具体位置进行攻击，所以这一个接口会是场景的成员
	// 这种情况可以说是单体攻击的真实表现，但是由于网络延迟，这个最好不对应单体攻击，而是作为范围攻击的一个必要接口
	// pFStyle是攻击的方式，SFightStyle这个结构包含了所有可能的攻击方式，以及该攻击方式所附带的攻击参数
	// ptPosition是攻击的具体场景坐标，返回值只是一个成功与否的标志，没有实际的意义，用于以后扩展
	DWORD AttackPositon(struct SFightStyle *pFStyle, POINT ptPosition);

    int Broadcast(struct SMessage *pMsg, WORD wSize, DNID dnidExcept);

	LPIObject GetObjectInRegin(DWORD dwGID);///在地图中查找玩家

public:
    DWORD GetTileMark(WORD CurX, WORD CurY);
    void MarkTileStop(WORD CurX, WORD CurY, DWORD &PrevMark, DWORD GID);
    bool isTileWalkAble(WORD CurX, WORD CurY);
    bool isTileStopAble(WORD CurX, WORD CurY, DWORD GID);

private:
	LPIObject	*AreaTable;		// 总的区块列表，呈矩阵排列

public:
	WORD		m_nAreaW;		// 以Area为单位的场景宽高
	WORD		m_nAreaH;				

public:
	WORD		m_wRegionID;		// 本场景编号
	WORD		m_wMapID;			// 本场景地图编号

	WORD		m_RegionW;			// 以Tile为单位的场景宽高
	WORD		m_RegionH;			
	DWORD		*m_pdwTiles;		// 整个场景的Tile属性数据
	DWORD		*m_pdwTileMarks;	// 整个场景的Tile标记属性数据
#ifdef USEMAPCOLLISION
	MapServer   *m_pmapsever;		//场景碰撞数据
#endif
public:
	// 这里还可以存放一些
	// 玩家列表等，需要使用场景来管理的数据
	check_list<LPIObject>		m_PlayerList;		// 容纳对象的列表
    std::map<QWORD, LPIObject>	m_BuildingMap;		// 容纳建筑物的列表，如果没有他，建筑物的保存就麻烦了
    int							prevBuildingNumber;	// 上次保存时，有效的建筑物数量

	DWORD	m_MapProperty;			// 场景属性
	WORD	m_wReLiveRegionID;		// 重生点场景编号
	POINT	m_ptReLivePoint;		// 重生点座标
	WORD	m_BgMusic;				// 播放WAV编号
	int		m_bLoopPlay;			// 播放是否循环
	DWORD	m_Weather;				// 天气列表
	DWORD   m_dwNeedLevel;			// 等级限制
	WORD	m_dwPKAvailable;		// PK限制
    DWORD   m_dwPKValueLimit;		// PK值限制
    DWORD   m_dwSpecialAccess;		// 特殊技能限制
	WORD	m_wSkillAccess;			// 技能限制
	BOOL	m_bAllowedMount;		// 是否允许坐骑
	BYTE		m_nSafeZoneInfo;			//PK区域1为安全区域，0为可以PK区域

    BOOL    m_IsHangup;				// 挂机限制
    BOOL    m_IsChildProtect;		// 新手保护
    BOOL    m_IsDamageVenation;		// 死亡经脉受伤
    BOOL    m_IsBlock;				// 是否存在阻挡

    BOOL    m_IsBatchrRefresh;		// 批量刷怪
    float   m_wRegionMultExp;		// 场景多倍

	WORD	m_MutateMonsterNum;		// 色怪数

	// 侠义世界副本附加特殊场景限制相关
	std::vector<WORD>	m_vLimitItemID;			// 被限制使用的ID列表
	std::vector<WORD>	m_vUnDealItemID;		// 被限制交易以及丢弃的物品列表
	std::vector<WORD>	m_vLimitSkillID;		// 被限制使用的技能ID列表
	std::vector<BYTE>	m_vTrigerBuffMonster;	// 怪物触发的BUFF列表
	std::vector<BYTE>	m_vTrigerBuffPlayer;	// 玩家触发的BUFF列表
	WORD				m_wLimitMaxHP;			// 限制生命上限，0为不限制
	WORD				m_wLimitMaxMP;			// 限制魔法上限，0为不限制
	WORD				m_wLimitMaxSP;			// 限制体力上限，0为不限制
	BOOL				m_bIsLimitChangeEquip;	// 是否限制更换装备
	BOOL				m_bIsLimitAllItem;		// 是否限制所有的Item
	WORD				m_wPracticeSkillType;	// 该地图可以修炼的武功类型	

    // 场景技能效果
    struct SRegionSkillEffect
    {
        enum
        {
            SkillEffectMAX = 16,
        };

        WORD id;    // 武功ID
        WORD odds;  // 千分比
    };
    SRegionSkillEffect skillEffect[SRegionSkillEffect::SkillEffectMAX];

    // [普通PK] [场景等级PK值加权] [不能PK] [自由PK] [PK经脉不受伤，不增加PK值，不掉装备]
	enum PKA_TYPE { PAT_REGIONLEVEL, PAT_NORMAL, PAT_NULL, PAT_PKFREE, PAT_PKCHAN }; 

	enum ZONE_INFO
	{
		PK_SAFE,		///安全区域
		PK_FREE,		///可以PK
		PK_NOTPKVALUE, //竞技模式
	};

	// 场景奖惩关系等
	// 关系： { 没有， 不关系PK值， 不关系侠义值, 包含2、3项 }
	enum PKX_TYPE { PXT_NULL, PXT_NOTPKVALUE, PXT_NOTXVALUE, PXT_BOTHPKX };
	DWORD	m_dwPKX;			

    // 陷阱触发器列表
    BYTE					m_byMaxTrap;
    class CTrigger			*m_pTrap;
    std::map<int, CTrigger> trapMap;
    std::set<DWORD>			m_LimitItemList;	// 场景被限制使用的物品列表
    DWORD					m_dwAIRefCount;

private:
    std::vector<BYTE>		m_TrapList;			// 动态生成陷阱相关数据列表
    WORD					m_ExperMul;			// 经验倍数

public:
    //class AstarPathFind *m_pAstar;
    
    WORD        GetExperMul(void) const { return m_ExperMul; }
    void        SetExperMul(WORD wNum) { m_ExperMul = wNum; }
    // 陷阱触发器 相关操作
    BYTE        GetMaxTrap() const { return m_byMaxTrap; }
    class       CTrigger *GetTrapPtr() { return m_pTrap; }
    void        DestroyTrap();  
	bool		SetTrap( WORD byOrder, WORD x, WORD y, CTrigger *pTrap );
    class       CTrigger *MakeNewTrap(BYTE byMax);

    struct DummyFunctor { void operator () ( ... ) {} };
	
	// 生成对象
	CFightPet			*CreateFightPet(PVOID pParameter);
	CMonster			*CreateMonster(PVOID pParameter, DWORD controlId = 0);
	virtual CNpc        *CreateNpc(PVOID pParameter, DWORD controlId = 0);
    virtual CBuilding   *CreateBuilding(PVOID pParameter, CPlayer *pPlayer, BOOL isGenerate = FALSE, DWORD controlId = 0); 

    // 删除对象
	int					DelFightPet(DWORD dwGID);
    virtual int			DelMonster(DWORD dwGID);
    virtual int         DelNpc(DWORD dwGID);
    virtual int         DelBuilding(DWORD dwGID); 

	//add by yuntao.liu
	check_list<LPIObject>* GetMonsterListByAreaIndex(INT32 x,INT32 y);
	check_list<LPIObject>* GetPlayerListByAreaIndex(INT32 x,INT32 y);

	//add by ab_yue
	void SetParamToList(WORD type, char* str);		//根据类型将读到的数据串格式化到对应链表里
	void ClearVectorByType(WORD type);
	void PushItemInVevtor(WORD type,const char* str);
	bool CanUseItem(WORD id);
	bool CanDealItem(WORD id);
	bool CanUseSkill(WORD id);
	//end

private:
    std::map< DWORD, DWORD > controlIdMap;

public:
    DWORD       CheckObjectByControlId( DWORD controlId );
    LPIObject   GetObjectByControlId( DWORD controlId );
    BOOL        RemoveObjectByControlId( DWORD controlId );
    BOOL        RemoveBuilding( QWORD uniqueId );
	BOOL		RemoveBuilding( CBuilding *pBuilding, BOOL isDfRemove = TRUE );
    BOOL        SaveBuildings();
    BOOL        LoadBuildings( struct SABuildingGetMsg *pMsg );
	int			GetMonsterCount(bool CountDead = false);
	int			GetLiveMonster(WORD monsterID);//怪物ID
	int			GetNpcCount();
	int			GetPlayerCount(int state);
	void			KillMonster(CPlayer *player);

    DWORD       m_dwTickTime;

    void        SetRegionBRType( DWORD dwBRTeamNumber, WORD wRefreshStyle );
};

