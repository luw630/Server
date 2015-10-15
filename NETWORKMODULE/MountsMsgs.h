#pragma once

#include "NetModule.h"
#include "PlayerTypedef.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// 坐骑消息类
//=============================================================================================
DECLARE_MSG_MAP(SMountsMsg, SMessage, SMessage::ERPO_MOUNTS_MESSAGE)
//{{AFX
EPRO_ADD_MOUNTS,			// 添加坐骑
EPRO_EQUIP_MOUNTS,			// 装备坐骑
EPRO_UNEQUIP_MOUNTS,		// 取消装备坐骑
EPRO_UP_MOUNTS,				// 上马
EPRO_DOWN_MOUNTS,			// 下马
EPRO_FEED_MOUNTS,			// 喂食
EPRO_ADD_POINT,				// 加点
EPRO_DELETE_MOUNTS,			// 放生
EPRO_USE_SKILL,				// 使用技能
EPRO_SYNPLAYMOUNT_ACTION,	// 广播骑马状态
EPRO_ADD_PETS,				// 添加坐骑
EPRO_CALL_OUT_PET,			// 召唤宠物
EPRO_CALL_BACK_PET,			// 收回
EPRO_DELETE_PETS,			// 放生
EPRO_ADD_FIGHTPET,			// 添加侠客
EPRO_DEL_FIGHTPET,			// 放生侠客
EPRO_CALL_OUT_FIGHTPET,		// 侠客出战
EPRO_CALL_BACK_FIGHTPET,	// 侠客休息
EPRO_FIGHTPET_STATUSCHG,	// 侠客状态切换
EPRO_FIGHTPET_STARTATK,		// 侠客攻击
EPRO_FIGHTPET_MOVETOMSG,	// 侠客瞬移消息
EPRO_FIGHTPET_MOVEFINSHMSG,	// 侠客瞬移完毕消息
EPRO_FIGHTPET_DEAD,		// 侠客死亡（尸体）
EPRO_FIGHTPET_IMPEL,		// 激励侠客
EPRO_FIGHTPET_CHANGEATTTYPE,				//侠客切换攻击模式
//}}AFX
END_MSG_MAP()

// 侠客攻击
DECLARE_MSG(SFightPetStartAtk, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_STARTATK)
struct SAFightPetStartAtk : public SFightPetStartAtk
{
	DWORD dwFightPetID;
	DWORD dwTargetID;
};

// 侠客状态切换
DECLARE_MSG(SFightPetStatusChg, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_STATUSCHG)
struct SAFightPetStatusChg : public SFightPetStatusChg
{
	enum
	{
		SFPS_FOLLOW,	// 跟随
		SFPS_FIGHT,		// 战斗
	};

	BYTE  bCurStatus;
	DWORD dwPetGID;		// 侠客ID
};

// 侠客出战
DECLARE_MSG(SCallOutFightPet, SMountsMsg, SMountsMsg::EPRO_CALL_OUT_FIGHTPET)
struct SQCallOutFightPet : public SCallOutFightPet
{
	BYTE index;
	float x;	///侠客坐标
	float y;
};

struct SACallOutFightPet : public SCallOutFightPet
{
	BYTE  index;			// 索引
	DWORD fpGID;			// 侠客ID
	float x;	///侠客坐标
	float y;
};

// 宠物出战
DECLARE_MSG(SCallOutPet, SMountsMsg, SMountsMsg::EPRO_CALL_OUT_PET)
struct SQCallOutPet : public SCallOutPet
{
	BYTE index;
};

struct SACallOutPet : public SCallOutPet
{
	DWORD dwOwnerID;		// 宠物主人ID
	DWORD dwPetGlobalID;	// 宠物的唯一ID
	WORD  wPetID;			// 召唤出的宠物ID，不能为0
};

// 侠客休息
DECLARE_MSG(SCallBackFightPet, SMountsMsg, SMountsMsg::EPRO_CALL_BACK_FIGHTPET)
struct SQCallBackFightPet : public SCallBackFightPet
{
};

struct SACallBackFightPet : public SCallBackFightPet
{
	BYTE  index;			// 索引
	DWORD fpGID;			// 侠客ID
};

// 宠物休息
DECLARE_MSG(SCallBackPet, SMountsMsg, SMountsMsg::EPRO_CALL_BACK_PET)
struct SQCallBackPet : public SCallBackPet
{
};

struct SACallBackPet : public SCallBackPet
{
	DWORD dwOwnerID;		// 宠物主人ID
	DWORD dwPetGlobalID;	// 宠物的唯一ID
	WORD  wPetID;			// 召唤出的宠物ID，不能为0
};

// 添加坐骑
DECLARE_MSG(SAddMounts, SMountsMsg, SMountsMsg::EPRO_ADD_MOUNTS)
struct SAAddMounts : public SAddMounts 
{
	SPlayerMounts::Mounts mounts;
};

// 添加宠物
DECLARE_MSG(SAddPets, SMountsMsg, SMountsMsg::EPRO_ADD_PETS)
struct SAAddPets : public SAddPets
{
	SPlayerPets::Pets pet;
};

// 添加侠客
DECLARE_MSG(SAddFightPet, SMountsMsg, SMountsMsg::EPRO_ADD_FIGHTPET)
struct SAAddFightPet : public SAddFightPet
{
	SFightPetExt fightpet;
};

// 装备坐骑
DECLARE_MSG(SEquipMounts, SMountsMsg, SMountsMsg::EPRO_EQUIP_MOUNTS)
struct SQEquipMounts : public SEquipMounts 
{
	BYTE index;
};

struct SAEquipMounts : public SEquipMounts
{
	BYTE index;
};

// 取消装备坐骑
DECLARE_MSG(SUnEquipMounts, SMountsMsg, SMountsMsg::EPRO_UNEQUIP_MOUNTS)
struct SQUnEquipMounts : public SUnEquipMounts 
{
	BYTE index;
};

struct SAUnEquipMounts : public SUnEquipMounts 
{
	BYTE index;
};

// 上马
DECLARE_MSG(SUpMounts, SMountsMsg, SMountsMsg::EPRO_UP_MOUNTS)
struct SQUpMounts : public SUpMounts 
{
	BYTE index;
};

struct SAUpMount : public SUpMounts 
{
	BYTE index;
	BYTE result;
};

// 下马
DECLARE_MSG(SDownMounts, SMountsMsg, SMountsMsg::EPRO_DOWN_MOUNTS)
struct SQDownMounts : public SDownMounts 
{
};

struct SADownMounts : public SDownMounts 
{
	BYTE index;	
	BYTE result;
};

// 喂食
DECLARE_MSG(SFeedMounts, SMountsMsg, SMountsMsg::EPRO_FEED_MOUNTS)
struct SQFeedMounts : public SFeedMounts
{
	BYTE mountIndex;		// 骑乘索引
	WORD foodIndex;			// 食物索引
};

struct SAFeedMounts : public SFeedMounts
{
	BYTE  mountIndex;		// 骑乘索引
	BYTE  mountLevel;		// 骑乘等级
	DWORD curExp;			// 骑乘经验
	WORD  Attri[SPlayerMounts::SPM_MAX];	// 骑乘属性
};

// 加点
DECLARE_MSG(SMountAddPoint, SMountsMsg, SMountsMsg::EPRO_ADD_POINT)
struct SQMountAddPoint : public SMountAddPoint
{
	BYTE mountIndex;	// 马的索引
	WORD attri[5];		// 要加的属性点
};

struct SAMountAddPoint : public SMountAddPoint
{
	BYTE mountIndex;	// 马的索引
	WORD value[5];		// 当前的属性值
	WORD remainPoint;	// 剩余点数
};

// 删除宠物
DECLARE_MSG(SDeletePets, SMountsMsg, SMountsMsg::EPRO_DELETE_PETS)
struct SQDeletePets : public SDeletePets
{
	BYTE index;
};

struct SADeletePets : public SDeletePets
{
	BYTE index;
};

// 删除坐骑
DECLARE_MSG(SDeleteMounts, SMountsMsg, SMountsMsg::EPRO_DELETE_MOUNTS)
struct SQDeleteMounts : public SDeleteMounts
{
	BYTE index;
};

// 删除坐骑回馈
struct SADeleteMounts : public SDeleteMounts
{
	BYTE index;
};

// 删除侠客
DECLARE_MSG(SDeleteFightPet, SMountsMsg, SMountsMsg::EPRO_DEL_FIGHTPET)
struct SQDeleteFightPet : public SDeleteFightPet
{
	BYTE index;
};

// 删除侠客回馈
struct SADeleteFightPet : public SDeleteFightPet
{
	BYTE index;
};

// 使用坐骑技能
DECLARE_MSG(SUseMountSkill, SMountsMsg, SMountsMsg::EPRO_USE_SKILL)
struct SQUseMountSkill : public SUseMountSkill
{
	short int index;	// 坐骑索引
	short int skillType;// 技能类型(主动被动)
	short int skillIdx;	// 技能索引编号	
};

// 同步骑马状态
DECLARE_MSG(SMountAction, SMountsMsg, SMountsMsg::EPRO_SYNPLAYMOUNT_ACTION)
struct SAMountAction : public SMountAction
{
	enum
	{
		MA_READY,		// 准备动作
		MA_CANCEL_READY,// 取消准备
		MA_UP,			// 上马动作
		MA_DOWN,		// 下马动作
	};

	DWORD	dwGID;		// 玩家ID
	WORD	index;		// 马的ID
	BYTE	level;		// 马的等级
	BYTE	bAction;	// 动作类型 0 标示准备 1 标示马上动作
};

// 侠客瞬移消息
DECLARE_MSG(SFightPetMoveto, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_MOVETOMSG)
struct SQFightPetMoveto : public SFightPetMoveto
{
	DWORD	dwGID;		// 玩家ID
	//DWORD	fpGID;		// 侠客ID
	float     PosX;			//移动目标坐标
 	float     PosY;
	//float		direction;	// 方向
};

// 侠客瞬移完毕
DECLARE_MSG(SFightPetMoveFinsh, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_MOVEFINSHMSG)
struct SAFightPetMoveFinsh : public SFightPetMoveFinsh
{
	DWORD	dwGID;		// 玩家ID
	//DWORD	fpGID;		// 侠客ID
// 	float     PosX;			//移动目标坐标
// 	float     PosY;
	//float		direction;	// 方向
};

// 侠客死亡（尸体）
DECLARE_MSG(SFightPetDead, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_DEAD)
struct SQFightPetDead : public SFightPetDead
{
	//DWORD	dwGID;		// 玩家ID
		BYTE		m_index;		//侠客索引		
		float     PosX;			//坐标
	 	float     PosY;
};

//侠客激励
DECLARE_MSG(SFightPetImpel, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_IMPEL)
struct SQFightPetImpel : public SFightPetImpel
{
		BYTE		m_index;		//侠客索引		
		WORD	m_impelValue;	//使用的激励值.
};

struct SAFightPetImpel : public SFightPetImpel
{
	WORD	m_impelValue;	//返回剩余激励值
};


//侠客切换攻击模式
DECLARE_MSG(SfpchangeatttypeMsg, SMountsMsg, SMountsMsg::EPRO_FIGHTPET_CHANGEATTTYPE)
struct SQfpchangeatttypeMsg : public SfpchangeatttypeMsg
{
	BYTE		atttype;			//请求切换的模式
};

//侠客切换攻击模式
struct SAfpchangeatttypeMsg : public SfpchangeatttypeMsg
{
	BYTE		atttype;			//切换后的模式
};



