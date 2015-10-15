#pragma once

#include "NetModule.h"
#include "../pub/ConstValue.h"
#include "PlayerTypedef.h"

#define MAX_ITEM_DATA		256
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 变身相关消息类
//=============================================================================================
DECLARE_MSG_MAP(STransformersMsgs, SMessage, SMessage::EPRO_TRANSFORMERS_MESSAGE)
EPRO_TRANSFORMERS_INIT,				//变身激活，升级
EPRO_TRANSFORMERS_INFO,				//变身信息
EPRO_TRANSFORMERS_SKILLINFO,				//变身技能相关信息
EPRO_TRANSFORMERS_SWITCH,				//变身切换
EPRO_TRANSFORMERS_SKILL,				//变身技能
EPRO_TRANSFORMERS_UNDOSHAPE,	//取消玩家的变身形态
END_MSG_MAP()

// 请求变身激活，升级
DECLARE_MSG(STransformersInit, STransformersMsgs, STransformersMsgs::EPRO_TRANSFORMERS_INIT)
struct SQTransformersInit : public STransformersInit
{
	BYTE		bTransformerIndex;			//请求变身索引
};
struct SATransformersInit : public STransformersInit
{
	BYTE		bresult;			//变身结果
};

// 请求变身消耗材料信息
DECLARE_MSG(STransformersInfo, STransformersMsgs, STransformersMsgs::EPRO_TRANSFORMERS_INFO)
struct SQTransformersInfo : public STransformersInfo
{
	BYTE		bTransformerIndex;			//请求变身索引
};
struct SATransformersInfo : public STransformersInfo
{
	BYTE   dmaterialnum;//材料数量
	DWORD dmoney;//需要金钱
	DWORD dmaterialIndex;//材料需求
	DWORD dcurrentHP;//当前等级HP
	DWORD dnextHP;//变身升级以后的HP

};


// 请求变身
DECLARE_MSG(STransformersSwitch, STransformersMsgs, STransformersMsgs::EPRO_TRANSFORMERS_SWITCH)
struct SQTransformersSwitch : public STransformersSwitch
{
	BYTE		bTransformerIndex;			//请求变身索引
};
struct SATransformersSwitch : public STransformersSwitch
{
	BYTE bresult;
};

//变身技能相关信息
DECLARE_MSG(STransformersSkillInfo, STransformersMsgs, STransformersMsgs::EPRO_TRANSFORMERS_SKILLINFO)
struct SQTransformersSkillInfo : public STransformersSkillInfo
{
	BYTE		bTransformerIndex;			//请求变身索引
};
struct SATransformersSkillInfo : public STransformersSkillInfo
{
	WORD	skillCurrentdamage;//造成的伤害
	WORD	skillLevelUpdamage;//升级后造成的伤害
	WORD CurrentTriggerate;//当前触发几率 10000
	WORD LevelUpTriggerate;//升级后触发几率 10000
	WORD wSkillIndex; //变身附加技能index
	DWORD currentSkillExp;//变身技能当前熟练度
	DWORD levelUpSkillExp;//变身技能升级需要熟练度
	DWORD silveritem; //使用技能消耗普通道具
	DWORD goditem; //使用技能消耗金币道具
	BYTE  bCurrentLevel;//当前技能等级
	
};

//取消玩家的变身形态
DECLARE_MSG(SUnTransformers, STransformersMsgs, STransformersMsgs::EPRO_TRANSFORMERS_UNDOSHAPE)
struct SAUnTransformers : public SUnTransformers
{
};
