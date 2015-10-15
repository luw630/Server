#pragma once


/////add by ly 2014/5/24   新宠物系统相关数据结构定义/////////
#define PETNAMEMAXLEN 32	//宠物名字最大长度
#define PETSKILLMAXNUM 20	//宠物可以拥有的技能最大数
#define PETMAXUSESKILLNUM 4  //宠物同时最多可以装备使用的技能

//宠物技能
typedef struct newpetskill
{
	DWORD m_PetSkillID;		//宠物技能ID
	BYTE m_Level;	//技能等级
	newpetskill()
	{
		m_PetSkillID = 0;
		m_Level = 0;
	}
}SNewPetSkill, *LPSNewPetSkill;

typedef struct newpetdata
{
	BYTE m_PetLevel;	//宠物等级
	BYTE m_CurUseSkill[PETMAXUSESKILLNUM];	//当前可以使用的技能
	char m_PetName[PETNAMEMAXLEN];		//宠物名字
	WORD m_PetStarAndMaxLevel;	//宠物星级和最大等级数；高字节为星级、低字节为最大等级数
	DWORD m_PetID;	//宠物ID
	DWORD m_CurPetExp;	//当前宠物经验
	DWORD m_CurPetHp;	//当前宠物血量
	DWORD m_CurPetMp;	//当前宠物精力
	DWORD m_CurPetDurable;	//当前宠物魂力
	DWORD m_DurableResumeNeedTime;	//魂力恢复还需要得时间
	SNewPetSkill m_PetSkill[PETSKILLMAXNUM];	//当前拥有的技能
	newpetdata()
	{
		m_PetLevel = 0;
		m_PetStarAndMaxLevel = 0;
		m_PetID = 0;
		m_CurPetExp = 0;
		m_CurPetHp = 0;
		m_CurPetMp = 0;
		m_CurPetDurable = 0;
		memset(m_CurUseSkill, 0xff, PETMAXUSESKILLNUM);
		memset(m_PetName, 0, PETNAMEMAXLEN);
		memset(m_PetSkill, 0, PETSKILLMAXNUM);
	}

}SNewPetData, *LPSNewPetData;