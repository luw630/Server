#pragma once

//荣耀增加数据
enum GloryAddType
{
	GLORY_AYINLIANG,		// 奖励银两
	GLORY_AJINBI,		// 奖励金币
	GLORY_AGLORYDOT,		// 奖励荣耀点

	GLORY_AMAX,
};

//荣耀获得相关数据条件
enum GloryConditionData
{
	GLORY_CLEVEL,	//玩家等级
	GLORY_CCONTINUELOGIN,	//连续登陆
	GLORY_CONLINE,		//在线时间
	GLORY_CCLEARANCETIMES,		//通过次数
	GLORY_CKILLMONSTERNUM,		//杀怪数
	GLORY_CYINLIANGNUM,		//银两数
	GLORY_CYUANBAONUM,		//元宝数
	GLORY_CEQUITNUM,		//装备数
	GLORY_CEQUITINTENSIFYLEVEL,		//装备强化等级
	GLORY_CEQUITGRADELEVEL,		//装备升阶等级
	GLORY_CEQUITSRARLEVEL,	//装备升星等级
	GLORY_CEQUITKEYINGLEVEL,	//刻印等级
	GLORY_CGONGJI,		//攻击力
	GLORY_CBIANSHENGNUM,		//可变身数
	GLORY_CBELIEFSKILLNUM,	//拥有的信仰技能数
	GLORY_CPACKAGENUM,		//背包数
	GLORY_CFINISHGLORYNUM,	//完成荣耀数


	GLORY_CMAX,
};

//获得条件索引
enum GloryConditionID
{
	GLORY_CONLEVELID,	//玩家达到x等级
	GLORY_CONONLINEID,	//累积在线时间
	GLORY_CONEQUITINTENSIFYLEVELID,	//拥有x件装备强化到x级
	GLORY_CONEQUITGRADELEVELID,	//拥有x件装备升阶到x级
	GLORY_CONEQUITSRARLEVELID,	//拥有x件装备升星到x级
	GLORY_CONEQUITKEYINGLEVELID,	//拥有x件附加x个刻印的装备
	GLORY_CONYINLIANGANDYUANBAOID,	//拥有游戏币x，拥有元宝x
	GLORY_CONGONGJIID,	//战斗力达到x点
	GLORY_CONBIANSHENGNUMID,	//拥有x个变身形态
	GLORY_CONBELIEFSKILLNUMID,	//获得信仰技能x个
	GLORY_CONPACKAGENUMID,	//激活x个包裹
	GLORY_CONFINISHGLORYNUMID,	//完成x个荣耀

};


//荣耀基本数据
class SGloryBaseData
{
public:
	SGloryBaseData() { memset(this, 0, sizeof(SGloryBaseData)); }

public:
	WORD m_GloryID;		//荣耀ID
	char m_GloryName[128];	//荣耀名
	BYTE m_GloryType;	//荣耀类型
	DWORD m_GloryIcon;		//荣耀图标
	BYTE m_GloryConditionIndex;	//激活条件索引
	char m_ConditionDetail[256];		//条件说明
	DWORD m_AwardFactor[GloryAddType::GLORY_AMAX];	//奖励数据
	DWORD m_GetConditionFactor[GloryConditionData::GLORY_CMAX];	//获取荣耀条件
};

//称号增加数据说明
enum TitleAddType
{
	TITLE_AHP,		//增加血量
	TITLE_AGONGJI,		//增加攻击
	TITLE_AFANGYU,		//增加防御

	TITLE_AMAX,
};

//称号消耗类型
enum TitleConsumeType
{
	TITLE_CGLORYDOT,		//消耗荣耀点
	TITLE_CGLORYITEMNUM,	//消耗荣耀道具数

	TITLE_CMAX
};

//称号基本数据
class STitleBaseData
{
public:
	STitleBaseData() { memset(this, 0, sizeof(STitleBaseData)); }

public:
	
	WORD m_TitleID;		//称号ID
	char m_TitleName[128];	//称号名
	DWORD m_ConsumeItemID;	//消耗道具的ID
	DWORD m_TitleIcon;		//称号图标
	DWORD m_AddFactor[TitleAddType::TITLE_AMAX];	//增加数据
	DWORD m_ConsumeFactor[TitleConsumeType::TITLE_CMAX];	//消耗数据
	char m_TitleDescript[256];		//称号描述
};
