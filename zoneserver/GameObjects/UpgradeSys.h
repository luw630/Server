#pragma once

#include "FightObject.h"

const int PMAXLEVEL = 70;
// 每一个等级所要改变的各项属性
typedef	struct	_SLevelUnit
{
	DWORD	dwMaxHP;					// 本级该有的生命最大值
	WORD	wMaxMP;					// 本级该有的内力最大值
	WORD	wMaxSP;					// 本级该有的体力最大值
	WORD	wAmuck;				    // 本级累计杀气
	WORD	wPoint;					// 本级累计分配点数
    UINT64	dwExp;					// 本级所需经验值
}SLevelUnit,*LPSLevelUnit;

// 每一个门派的基本属性以及每一个等级所要改变的各项属性
typedef	struct	_SLevelData
{
	// 基本属性
    WORD	wBaseEN;				// 活力 （体质）
    WORD	wBaseST;				// 体魄 （强壮）
    WORD	wBaseIN;				// 悟性 （智慧）
    WORD	wBaseAG;				// 身法 （灵敏）
    WORD	wBaseLU;				// 气运 （运气）
    // 能力属性
	WORD	wBaseDAM;				// 武功攻击力初始值：4位数内，和门派，装备，基础能力有关
	WORD	wBasePOW;				// 内功攻击力初始值：4位数内
	WORD	wBaseDEF;				// 防御力初始值：4位数内
	WORD	wBaseAGI;				// 灵敏度初始值：4位数内
    // 基本属性变化量
    WORD	wHPVal;					// 生命的增加量
    WORD	wMPVal;					// 内力的增加量
    WORD	wSPVal;					// 体力的增加量
    // 攻击段位修正系数
    WORD    wWound[3];              // 上中下三段被击修正系数
    
    SLevelUnit	Data[PMAXLEVEL];			// 每一级的具体数据
}SLevelData,*LPSLevelData;

class   DUpgradeSys
{
    SLevelData  SchoolData[5];
public: 
    DUpgradeSys(void);
    ~DUpgradeSys(void);

    BOOL    LoadLevelData(void);
    SLevelData *GetLevelData(BYTE bySchool);

    //void    SendAddPlayerExp( IObject *pObj, WORD wExpVal );
};

extern  DUpgradeSys     *g_pUpGradeData;
