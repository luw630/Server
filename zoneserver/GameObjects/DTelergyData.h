
#ifndef     _DTELERGYDATA_H_
#define     _DTELERGYDATA_H_

// 静态心法数据的最大值
const int MAX_TELETGY = 256;
 // 心法名字字数
const int LEN_TELERGYNAME = 16;          

#define MAX_INFO_LENTH		1024
enum E_TOTAL_TELERGY			// 心法数据读取统一接口辨识标记
{
    TELERGYR_NONE,
    TELERGYR_wDamCoef,           // 外攻击系数	
    TELERGYR_wPowCoef,           // 内攻击系数	

    TELERGYR_wDamAddCoef,        // 外功修炼	
    TELERGYR_wPowAddCoef,        // 内功修炼	
    TELERGYR_wDefAddCoef,        // 防御修炼	
    TELERGYR_wAgiAddCoef,        // 敏捷修炼	
    TELERGYR_wHPMaxAddCoef,      // 血量上限	
    TELERGYR_wMPMaxAddCoef,      // 内力上限	
    TELERGYR_wSPMaxAddCoef,      // 体力上限	
    TELERGYR_wKnockOdds,         // 击飞几率系数	
    TELERGYR_wKnockDefCoef,      // 抗击系数	
    TELERGYR_wOnVitalsTimeCoef,  // 点穴时间系数	
    TELERGYR_wOffVitalsTimeCoef, // 解穴时间系数	
    TELERGYR_wOnLmtAtkTimeCoef,  // 封招时间系数	
    TELERGYR_wOffLmtAtkTimeCoef, // 解招时间系数	
    TELERGYR_wOnDizzyTimeCoef,   // 眩晕时间系数	
    TELERGYR_wOffDizzyTimeCoef,  // 解晕时间系数	
    TELERGYR_wOnToxicTimeCoef,   // 中毒时间系数	
    TELERGYR_wOffToxicTimeCoef,  // 解毒时间系数	
    TELERGYR_wDamAddTimeCoef,    // 提攻时间系数	
    TELERGYR_wDefAddTimeCoef,    // 提防时间系数	
    TELERGYR_wAgiAddTimeCoef,    // 提敏时间系数	
    TELERGYR_wAmuAddTimeCoef,    // 提杀时间系数	

    TELERGYR_wDamDam,            // 手太阳伤害	
    TELERGYR_wPowDam,            // 手少阳伤害	
    TELERGYR_wDefDam,            // 足太阴伤害	
    TELERGYR_wAgiDam,            // 足少阴伤害	
    TELERGYR_wDamDef,            // 手太阳抵制	
    TELERGYR_wPowDef,            // 手少阳抵制	
    TELERGYR_wDefDef,            // 足太阴抵制	
    TELERGYR_wAgiDef,            // 足少阴抵制	
    TELERGYR_wDamMed,            // 手太阳疗伤	
    TELERGYR_wPowMed,            // 手少阳疗伤	
    TELERGYR_wDefMed,            // 足太阴疗伤	
    TELERGYR_wAgiMed,            // 足少阴疗伤	

    TELERGYR_wOffZeroSPTimeCoef, // 抵抗减体力降为0时间系数
    TELERGYR_wOffZeroMPTimeCoef, // 抵抗内力减少时间系数
    TELERGYR_wOffZeroHPTimeCoef, // 抵抗生命减少时间系数

    TELERGYR_MAX,
};


struct TelergyDataXiaYiShiJie
{
	/*
	侠义世界的心法数据在新侠的基础上有了改变，但是为了不影响以前的逻辑功能，目前安全的修改方法是
	在原来的心法数据结构的基础上添加新的数据结构---yuntao.liu
	*/
	enum TELE_PROP_DEF
	{
		addBlood = 0,					// 生命上限提升	
		addOutAttackBase,				// 外攻基础提升
		addOutAttackMastery,			// 外攻精通提升
		addInnerAttackBase,				// 内攻基础提升
		addInnerAttackMastery,			// 内攻精通提升
		addOutDefenseBase,				// 外防基础提升
		addOutDefenseMastery,			// 外防精通提升
		addInnerDefenseBase,			// 内防基础提升
		addInnerDefenseMastery,			// 内防精通提升
		addBodyWork,					// 身法提升
		addKillForce,					// 杀气提升
		addStaafType,					// 禅杖系
		addKungfuBaseForceByFistType,	// 拳系武攻基础力道的提升
		addSwordType,					// 剑系
		addPenType,						// 笔系
		addStabType,					// 刺系
		addQinType,						// 琴系
		addKnifeType,					// 刀系
		addHalberdType,					// 戟系
		addHookType,					// 钩系
		addRingType,					// 轮环系
		addHandTaiYang,					// 手太阳疗伤（名字跟新侠一样，但是意义不同了）
		addHandShaoYang,				// 手少阳疗伤
		addFootTaiYin,					// 足太阴疗伤
		addFootShaoYin,					// 足少阴疗伤
		addImmunityBuffFirst,			//免疫buff 1
		addImmunityBuffSecond,			//免疫buff 2

		MAX_TELERGY_XIA_YI_SHI_JIE
	};
	INT32 m_index;							// 编号
	char  m_name[ LEN_TELERGYNAME ];		// 名字
	int	  m_nSchool;						// 门派 

	INT32 m_data[ MAX_TELERGY_XIA_YI_SHI_JIE ];
	DWORD m_hardBase;						//心法难度基数
	DWORD m_hardCoefficient;				// 心法难度系数
	DWORD m_maxLevel;						// 心法层数上限 
	char  information[MAX_INFO_LENTH];		//技能说明
};

////////////////////////////////////////////////////////////////////// 心法相关数据
struct  TelergyData : TelergyDataXiaYiShiJie
{
	// Zeb 2009-11-19
    //WORD wIndex;						// 编号	
    //char szName[LEN_TELERGYNAME];       // 名称	
    //WORD wDamCoef;           			// 外攻击系数	
    //WORD wPowCoef;           			// 内攻击系数	
    //WORD wDamAddCoef;        			// 外功修炼	
    //WORD wPowAddCoef;        			// 内功修炼	
    //WORD wDefAddCoef;        			// 防御修炼	
    //WORD wAgiAddCoef;        			// 敏捷修炼	
    //WORD wHPMaxAddCoef;      			// 血量上限	
    //WORD wMPMaxAddCoef;      			// 内力上限	
    //WORD wSPMaxAddCoef;      			// 体力上限	
    //WORD wKnockOdds;         			// 击飞几率系数	
    //WORD wKnockDefCoef;      			// 抗击系数	
    //WORD wOnVitalsTimeCoef;  			// 点穴时间系数	
    //WORD wOffVitalsTimeCoef; 			// 解穴时间系数	

    //WORD wOnLmtAtkTimeCoef;  			// 封招时间系数	
    //WORD wOffLmtAtkTimeCoef; 			// 解招时间系数	

    //WORD wOnDizzyTimeCoef;   			// 眩晕时间系数	
    //WORD wOffDizzyTimeCoef;  			// 解晕时间系数	
    //WORD wOnToxicTimeCoef;   			// 中毒时间系数	
    //WORD wOffToxicTimeCoef;  			// 解毒时间系数	
    //WORD wDamAddTimeCoef;    			// 提攻时间系数	
    //WORD wDefAddTimeCoef;    			// 提防时间系数	
    //WORD wAgiAddTimeCoef;    			// 提敏时间系数	
    //WORD wAmuAddTimeCoef;    			// 提杀时间系数	
    //WORD wDamDam;            			// 手太阳伤害	
    //WORD wPowDam;            			// 手少阳伤害	
    //WORD wDefDam;            			// 足太阴伤害	
    //WORD wAgiDam;            			// 足少阴伤害	
    //WORD wDamDef;            			// 手太阳抵制	
    //WORD wPowDef;            			// 手少阳抵制	
    //WORD wDefDef;            			// 足太阴抵制	
    //WORD wAgiDef;            			// 足少阴抵制	
    //WORD wDamMed;            			// 手太阳疗伤	
    //WORD wPowMed;            			// 手少阳疗伤	
    //WORD wDefMed;            			// 足太阴疗伤	
    //WORD wAgiMed;            			// 足少阴疗伤
    //WORD wOffZeroSPTimeCoef; 			// 抵抗减体力降为0时间系数
    //WORD wOffZeroMPTimeCoef; 			// 抵抗内力减少时间系数
    //WORD wOffZeroHPTimeCoef; 			// 抵抗生命减少时间系数

    //WORD wDiffCoef;          			// 心法难度系数
    //WORD wMaxLevel;          			// 心法层数上限
    //int nMutexTelergy;      			// 互斥技能
	// Zeb 2009-11-19
};

class DTelergyData
{
    TelergyData m_Data[MAX_TELETGY];
    int m_iMaxTelergy;

public:
    DTelergyData(void);
    ~DTelergyData(void);

    BOOL LoadData(char* szPath);
    TelergyData *GetTelergy( WORD wIndex ); 
    int GetCount() {return m_iMaxTelergy;};

	static DWORD GetCurLevelMax(TelergyData* pTempData,int iLevel);	//得到当期心法等级所需要的最大熟练度
	static DWORD GetCurPractice(WORD wTime);
private:
	// 目前把侠义的心法数据表和老的心法数据在内存上是放在一起的，载入了老的心法数据后
	// 调用此函数载入新的心法数据表
	BOOL LoadDataXiaYiShiJie(char* szPath);
};

extern DTelergyData* g_pTelergyData;

#endif  //_DTELERGYDATA_H_