#pragma once

#include "Networkmodule\ItemTypedef.h"

enum FightPetAttri
{
	FPA_MAXHP,			// 生命上限
	FPA_MAXMP,			// 内力上限
	FPA_GONGJI,			// 攻击
	FPA_FANGYU,			// 防御
	FPA_BAOJI,			// 暴击
	FPA_SHANBI,			// 闪避

	FPA_MAX,
};

// 侠客类型
enum FightPetType
{
	FPT_WAIGONG,		// 外功型
	FPT_NEIGONG,		// 内功型
	FPT_WAIGONGFANG,	// 外功防御型
	FPT_NEIGONGFANG,	// 内功防御型
	FPT_ALL,			// 综合型
};

// 侠客的基本属性
class SFightPetBaseData
{
public:
	SFightPetBaseData() { memset(this, 0, sizeof(SFightPetBaseData)); };

public:
	char	m_Name[SFightPetBase::FP_NAME];
	WORD	m_ID;					// ID
	bool	m_CanChangeName;		// 能否改名
	BYTE	m_Type;					// 侠客门派
	WORD	m_Attri[FPA_MAX][3];	// 属性值
	BYTE		m_RareMin;					// 稀有度最小
	BYTE		m_RareMax;					// 稀有度最大
	DWORD	m_nexp;			// 升级经验
	WORD	m_nmodelID;			// 模型ID
	DWORD	m_niconID;			// 道具ICON
	WORD		m_nLimitLevel;		//最低携带等级
	BYTE			m_bSex;				//性别
};


// 侠客的成长属性取值
class SFightPetRise
{
public:
	SFightPetRise() { memset(this, 0, sizeof(SFightPetRise)); };

public:
	BYTE		m_nRare;// 稀有度
	WORD  m_nHpRiseMin;// 生命成长最小值
	WORD  m_nHpRiseMax;// 生命成长最大值
	WORD  m_nMpRiseMin;// 内力成长最小值
	WORD  m_nMpRiseMax;// 内力成长最大值
	WORD  m_nGongJiRiseMin;// 攻击成长最小值
	WORD  m_nGongJiRiseMax;// 攻击成长最大值
	WORD  m_nFangYuRiseMin;// 防御成长最小值
	WORD  m_nFangYuRiseMax;// 防御成长最大值

	WORD  m_nShanBiRiseMin;// 闪避成长最小值
	WORD  m_nShanBiRiseMax;// 闪避成长最大值
	WORD  m_nBaojiRiseMin;// 暴击成长最小值
	WORD  m_nBaojiRiseMax;// 暴击成长最大值
	WORD  m_nWuXinRiseMin;// 悟性成长最小值
	WORD  m_nWuXinRiseMax;// 悟性成长最大值

};

