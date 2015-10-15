#pragma once

// 骑乘品质
enum EMountQuality
{
	EMQ_YIBAN = 1,		// 一般
	EMQ_YOULIANG,		// 优良
	EMQ_SHANGCHENG,		// 上乘
	EMQ_XIYOU,			// 稀有
	EMQ_SHISHI,			// 史诗
	EMQ_JUESHI,			// 绝世
};

// 骑乘影响属性
enum EMountAttribute
{
	EMA_MHP,			// 生命上限
	EMA_MMP,			// 内力上限
	EMA_MTP,			// 体力上限
	EMA_GONGJI,			// 攻击
	EMA_FANGYU,			// 防御
	EMA_BAOJI,			// 暴击
	EMA_SHANBI,			// 闪避

	EMA_MAX,			
};

const int MAX_MOUNT_LEVEL = 9;		// 骑乘最大等级

// 骑乘
class SMountBaseData
{
public:
	SMountBaseData()
	{
		memset(this, 0, sizeof(SMountBaseData));
	}

public:
	char	m_Name[13];				// 名称
	WORD	m_ID;					// ID
	BYTE	m_Level;				// 等级
	BYTE	m_Quality;				// 品级
	BYTE	m_RideNum;				// 可骑乘人数
	BYTE	m_CanUpdate;			// 是否可升级
	DWORD	m_NeedExp;				// 升级所需经验
	WORD	m_SpeedUp;				// 加速度系数
	WORD	m_Attri[EMA_MAX][2];	// 属性加成
	WORD	m_ModelID;				// 模型ID
};