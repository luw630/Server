#pragma once
#include "networkmodule/PlayerTypedef.h"

#ifndef     _DFiGHTDATA_H_
#define     _DFiGHTDATA_H_

#define     FILENAME_SKILL  "Data\\SkillData.def"     // 外功数据文件
#define     FILENAME_BASEF  "Data\\BaseFight.def"     // 基本攻击文件
#define     FILENAME_SITEM  "Data\\DropItems.def"     // 特殊掉落物品数据文件
#define     FILENAME_DRITEM "Data\\RandDropItems.def" // 特殊掉落物品数据文件
#define		FILENAME_ATTACKMATIX  "Data\\AttackMatrix.def"	 //	基本攻击文件

const   int     COEF_ATTACK     = 1;        // 攻击系数修正

const   int     MAX_ATTACKAREASIZE  = 9;    // 范围攻击的矩阵大小
const   int     MAX_ACTURLEVEL  = 20;       // 最大实际加成等级数量

const   int     LEN_NAMECHAR    = 16;       // 基本名称字串长度


//对应每个属性提供5%的加成
//[如果改变客户断也要相应改动]
enum E_TEAMSKILLTYPE
{
    ESKT_WIND = 55,      // 风：其疾如风 5％ 灵敏加成
    ESKT_WOOD,           // 林：其徐如林 5％ 基本内功攻击力加成
    ESKT_FIRE,           // 火：侵掠如火 5％ 基本外功攻击力加成
    ESKT_HILL,           // 山：不动如山 5％ 基本防御力加成
      
    ESKT_CLOUD,          // 流云阵  同时防，敏提升  采用原公式 学会后替换 风队形技
    ESKT_TAIYI,          // 太乙阵  同时内，外提升  采用原公式 学会后替换 林队形技           
    ESKT_HEYI,           // 鹤翼阵  移动速度提升，闪避率提升  每组一个人移动速度提升1%，闪避率提升1% 学会后替换 火队形技
    ESKT_FENGSHI,        // 锋矢阵  攻击速度提升，命中率提升  每组一个人攻击速度提升1%，命中率提升1% 学会后替换 山队形技

    ESKT_MAX
};

//extern int g_FightBaseCount[EFT_MAX];

// 受伤种类
enum E_WOUND
{
    EW_WOUND,
    EW_WOUND2,
    EW_WOUND3,
    EW_NKOCKOUT,
    EW_DIZZY,
    EW_VITALS,
    EW_TOXIC,
    EW_DEFENCE,

    EW_MAX
};

// 基本受伤数据
struct SWoundData
{
    BYTE    byWoundFrame;       // 受伤速度
};

//////////////////////////////////////////////////////////////////////// 基本武功
struct SBaseFightType
{
    short    wAttackTime;        // 攻击速度（整个攻击的总帧数）
    short    byNextTime;         // 下次攻击时间间隔
    short    wHitTime;           // 命中速度（在第几帧命中）
    short    byDist;             // 攻击距离
};

//////////////////////////////////////////////////////////////////////// 武功招式
struct SSkillData
{
    short   wIndex;             // 编号	
    char    szName[LEN_NAMECHAR];         // 名称                       
    short   wPicId;             // 对应动画                             
    short   wIsAtack;           // 是否辅助攻击                      
    short   wWoundNum;          // 攻击段位                          
    short   wAttackType;        // 攻击方式                       
    short   wDistance;          // 攻击距离                             
    short   wTargetArea;        // 攻击范围                             
    short   wAttackTime;        // 攻击速度（时间）                     
    short   wHitTime;           // 命中时间                             
    short   wAttackCoef;        // 攻击系数                             
    short   wHitOdds;           // 命中率                               
    short   wHarmHP;            // 生命伤害                             
    short   wHarmMP;            // 内力伤害                             
    short   wHarmSP;            // 体力伤害                             
    short   wKnockoutOdds;      // 击飞几率                             
    short   wKnockoutDist;      // 击飞距离                             
    short   wVitalsOdds;        // 点穴几率                             
    short   wVitalsTime;        // 点穴时间                             
    short   wDizzyOdds;         // 晕眩几率                       
    short   wDizzyTime;         // 晕眩时间    
    short   wDropWeaponOdda;    // 打落武器几率p         
    short   wDropEquipmentOdds; // 打落防具几率         
    short   wLimitAttackOdds;   // 封招几率                             
    short   wLimitAttackTime;   // 封招时间                             
    short   wAddHPRate;         // 恢复生命比率p                        
    short   wAddSPRate;         // 恢复体力比率p                        
    short   wAddDamTime;        // 提升外功攻击力时间                   
    short   wAddDamRate;        // 提升外功攻击力比率p                  
    short   wAddPowTime;        // 提升内功攻击力时间                   
    short   wAddPowRate;        // 提升内功攻击力比率p                  
    short   wAddDefTime;        // 提升防御力时间                       
    short   wAddDefRate;        // 提升防御力比率p                      
    short   wAddAgiTime;        // 提升灵敏度时间                       
    short   wAddAgiRate;        // 提升灵敏度比率p                      
    short   wRedAgiTime;        // 降低灵敏度时间                       
    short   wRedAgiRate;        // 降低灵敏度比率                       
    short   wToxicOdds;         // 中毒几率                             
    short   wToxicTime;         // 中毒总时间                           
    short   wToxicRedHPRate;    // 中毒损生命比率                       
    short   wAddAmuckTime;      // 提升杀气时间                         
    short   wAddAmuckVal;       // 提升杀气数量p                        
    short   wDoubleMoneyOdds;   // 双倍钱物几率m    
    short   wAddWeaponTime;     // 提升武器性能时间                     
    short   wAddWeaponRate;     // 提升武器性能比率p                    
    short   wAddEquipTime;      // 提升防具性能时间                     
    short   wAddEquipRate;      // 提升防具性能比率p                    
    short   wDemToHPRate;       // 吸收伤害转生命比率                   
    short   wDemToSPRate;       // 吸收伤害转体力比率                   
    short   wUseupSP;           // 消耗体力   
    short   wUseupMP;           // 消耗内力   
    short   wUseupHP;           // 消耗生命
    WORD    wAtkHardCoef;       // 武功难度系数                         
    WORD    wAttackRvs;         // 武功攻击修正                         
    short   wDemMPToHPRate;     // 吸收内力转生命比率    
    short   wZeroSPOdds;        // 直接将体力变成零的几率
    short   wAddHPMaxTime;      // 提升HP时间                   
    short   wAddHPMaxRate;      // 提升HP比率
};    

////////////////////////////////////////////////////////////////////// 攻击相关数据
class DFightData
{
public:
    SBaseFightType  m_BaseFightList[6];                    // 5个门派的基本武功 + 怪物的
    SSkillData      m_Data[MAX_SKILLCOUNT]; // 武功列表

    SWoundData      m_WoundData[EW_MAX];

    // 范围攻击的伤害比率矩阵
    int             m_AttackArea[MAX_ATTACKAREASIZE*8][MAX_ATTACKAREASIZE];

    // 掉落物品的两个数据
    int             *m_iSpecialItem;
    int             m_iDropItemCount;

    struct  SDropItemBaseRand
    {
        WORD    wMax;
        WORD    wMin;
        WORD    wReduceVal;
        WORD    wItemCount;
        WORD    *pItemList;
        SDropItemBaseRand(void)
        {
            pItemList = NULL;
        }
    };

public:
    SDropItemBaseRand   m_DropItemBaseRand;

public:
    DFightData(void);
    ~DFightData(void);
    
    BOOL    LoadFightDate(void);                        // 载入武功数据

    bool    LoadDropItemData(void);
    void    ReleaseDropItemData(void);
    bool    LoadRandDropItemData(void);
    void    ReleaseRandDropItemData(void);

    SSkillData  *GetSkill( int iSkillID );              // 获取武功数据，Level=0不会武功
    int         *GetAreaMatrix( int iSkillID, BYTE byDir );
    int         GetAttackPosCoef( int iSkillID, BYTE byDir, int x, int y );

    WORD    GetRandDropItem(void);
    SBaseFightType  *GetBaseFightOnSchool( BYTE school );

    WORD    GetDropItemByRand( WORD wBaseRand );
};

extern  DFightData  *g_pFightData;

////////////////////////////////////////////////////////////////////// 攻击范围数据结构
//typedef struct  _SFightArea
//{
//    BOOL    bAxisSymm;          // 是否轴对称
//    BYTE    bySymmRadius;       // 对称半径，用于从中心产生偏移量遍历攻击范围
//}SFightArea,*LPSFightArea;

/*
编号	
名称	
对应动画	
攻击或辅助	
攻击段位	
攻击方式	
攻击距离	
攻击范围	
攻击时间	
命中时间	
攻击系数	
命中率	
生命伤害	
内力伤害	
体力伤害	
击飞几率	
击飞距离	
点穴几率	
点穴时间	
眩晕几率	
眩晕时间	
打落武器几率	
打落防具几率	
封招几率	
封招时间	
恢复生命比例	
恢复体力比例	
提升外功攻击力时间	
提升外功攻击力比例	
提升内功攻击力时间	
提升内功攻击力比例	
提升防御力时间	
提升防御力比例	
提升灵敏度时间	
提升灵敏度比例	
降低灵敏度时间	
降低灵敏度比例	
中毒几率	
中毒总时间	
中毒损生命比例	
提升杀气时间	
提升杀气数量	
双倍钱物几率	
提升武器性能时间	
提升武器性能比例	
提升防具性能时间	
提升防具性能比例	
吸收伤害转生命比例	
吸收伤害转体力比例	
消耗体力	
消耗内力	
消耗生命	
武功难度系数	
武功攻击修正	
吸收内力转生命比率	
直接将体力变成零的几率
*/

#endif  //_DFiGHTDATA_H_