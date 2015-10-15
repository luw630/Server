
#ifndef     _VENAPOINTDATE_H_
#define     _VENAPOINTDATE_H_

////////////////////////////////////// 常量

const   int     LEN_VENAPOINTNAME   =   16;     // 经脉穴位
const   int     VENA_COUNT          =   9;      // 9条经脉
const   int     POINT_COUNT         =   32;     // 每条经脉上面的穴位数

typedef struct  _SVenapointData
{
	enum VENA_PROP_DEF
	{
		MAX_HP = 0,							// 生命上限 ->m_wMaxHPOnMad
		MAX_MP,								// 内力上限
		MAX_SP,								// 真气上限
		RE_HP,								// 生命恢复力（每次回复的量）
		RE_MP,								// 内力恢复力（每次回复的量）
		RE_SP,								// 真气恢复力（每次回复的量）
		OUT_ATTA_BASE,						// 外攻基础（单位1/1000）
		OUT_ATTA_MASTERY,					// 外攻精通
		INNER_ATTA_BASE,					// 内攻基础
		INNER_ATTA_MASTERY,					// 内攻精通
		OUT_DEF_BASE,						// 外防基础
		OUT_DEF_MASTERY,					// 外防精通
		INNER_DEF_BASE,						// 内防基础
		INNER_DEF_MASTERY,					// 内防精通
		AGILE,								// 身法
		AMUCK,								// 杀气 ->m_Property.m_byAmuck

		VENA_PROPERTY_MAX,
	};

	char		szName[LEN_VENAPOINTNAME];  // 名称
	WORD		wNum;                       // 道具编号
	WORD		wID;                        // 穴位编号
	int			data[VENA_PROPERTY_MAX];
	DWORD		dwUseTelergy;               // 需要消耗的心法数量
}SVenapointData,*LPSVenapointData;

typedef struct  _SVenapointDataEx : public _SVenapointData
{
    WORD    wAddIN;                         // 悟性
    WORD    wAddHitPro;                     // 命中加层万分比
    WORD    wAddDAMPro;                     // 外功加层万分比
    WORD    wAddPOWPro;                     // 内攻加层万分比
    WORD    wAddAGIPro;                     // 敏捷加层万分比
    WORD    wAddDodgePro;                   // 躲闪加层万分比
    WORD    wAddDefPro;                     // 防御加层万分比
    WORD    wAddSkillAdd[6];                // 技能加层[0：七杀诀 1：回生诀 2：战气诀 3：金钟罩 4：飞燕诀 5：天罡诀]
}SVenapointDataEx,*LPSVenapointDataEx;


////////////////////////////////////// 操作对象
class   DVenapointData
{
    BYTE            m_byVenapointCount[VENA_COUNT];					// 每条经脉的穴位数量
    SVenapointData  m_Data[VENA_COUNT][POINT_COUNT];				// 每条经脉里的每个穴位的数据，每条经脉的最后一个数据是
																	// 这条经脉的每个穴位都打通了后，附加的数据
    SVenapointData  m_TotalData;									// 所有经脉全部通了后的附加数据

    SVenapointDataEx  m_DataEx[VENA_COUNT][POINT_COUNT];
    BYTE            m_byVenapointCountEx[VENA_COUNT];
    SVenapointDataEx  m_TotalDataEx;


public:
    DVenapointData(void);
    ~DVenapointData(void);

    BOOL    LoadData( char* szPath  );
    LPSVenapointData    GetVenapoint( BYTE byVenaNum, BYTE byPointNum );
    BYTE    GetVenapointCount( BYTE byVenaNum );
	std::string GetAddDataName( int data);

    // 大周天
    BOOL LoadDataEx(char* szPath = "Data\\VenapointDataEx.def");
    LPSVenapointDataEx GetVenapointEx( BYTE byVenaNum, BYTE byPointNum );
    BYTE    GetVenapointCountEx( BYTE byVenaNum );
};

extern  DVenapointData  *g_pVenapointData;

#endif  //_VENAPOINTDATE_H_
