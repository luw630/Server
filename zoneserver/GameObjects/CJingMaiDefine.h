#pragma once
#include "pub\ConstValue.h"

// 经脉影响的属性
enum JingMaiProperty
{
	JMP_MHP,		// 生命上限
	JMP_MMP,		// 内力上限
	JMP_MTP,		// 体力上限
	JMP_GONGJI,		// 攻击
	JMP_FANGYU,		// 防御
	JMP_BAOJI,		// 暴击
	JMP_SHANBI,		// 闪避

	JMP_MAX,
};

class SJingMaiBaseData
{
public:
	SJingMaiBaseData() { memset(this, 0, sizeof(SJingMaiBaseData)); }

public:
	char	m_Name[CONST_USERNAME];
	WORD	m_MaiID;				// 脉线ID
	WORD	m_XueID;				// 穴位ID
	WORD	m_Factor[JMP_MAX];		// 附加属性
	DWORD	m_NeedItem;				// 所需道具
	WORD	m_Rate;					// 成功率
	DWORD	m_NeedSp;				// 所需真气
	DWORD   m_NeedFrontXueID;        //所需前置穴位
};

/////add by ly 2014/3/17	这里信仰系统和经脉系统为类似功能，几乎为仿照经脉系统功能来实现（该系统没有经脉系统这个功能）

enum XinYangProperty
{
	XYP_HP,		//生命
	XYP_JP,		//精力
	XYP_TP,		//体力
	XYP_HIT,	//命中
	XYP_GONGJI,	//攻击
	XYP_FANGYU,	//防御
	XYP_BAOJI,	//暴击
	XYP_POJI,	//破击
	XYP_CHUANCI,	//穿刺
	XYP_KANGBAO,	//抗暴
	XYP_KANGPO,	//抗破
	XYP_KANGCHUAN,	//抗穿
	XYP_SHANBI,	//闪避
	XYP_MAX,
};

class SXinYangBaseData
{
public:
	SXinYangBaseData() { memset(this, 0, sizeof(SXinYangBaseData)); }

public:
	char m_Name[128];
	BYTE m_ZhuShenID;		//主神ID
	BYTE m_XingxiuID;		//星宿ID, 主键
	WORD m_Factor[XYP_MAX];	//附加属性
	BYTE m_SuccessRate;		//升星成功率
	WORD m_ConsumeYinLiang;	//消耗银两
	WORD m_ConsumeJinBi;	//消耗金币
	DWORD m_ConsumeItem;	//消耗道具
	char m_XingXiuDescript[256];	//星宿描述
	BYTE m_PlayerVipLevel;		//玩家等级限制
	WORD m_CanHaveSkillID;	//主神对应升级技能的ID
	WORD m_ConsumeSp;	//消耗真气
};