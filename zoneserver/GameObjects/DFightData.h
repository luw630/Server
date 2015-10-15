#pragma once
#include "networkmodule/PlayerTypedef.h"

#ifndef     _DFiGHTDATA_H_
#define     _DFiGHTDATA_H_

#define     FILENAME_SKILL  "Data\\SkillData.def"     // �⹦�����ļ�
#define     FILENAME_BASEF  "Data\\BaseFight.def"     // ���������ļ�
#define     FILENAME_SITEM  "Data\\DropItems.def"     // ���������Ʒ�����ļ�
#define     FILENAME_DRITEM "Data\\RandDropItems.def" // ���������Ʒ�����ļ�
#define		FILENAME_ATTACKMATIX  "Data\\AttackMatrix.def"	 //	���������ļ�

const   int     COEF_ATTACK     = 1;        // ����ϵ������

const   int     MAX_ATTACKAREASIZE  = 9;    // ��Χ�����ľ����С
const   int     MAX_ACTURLEVEL  = 20;       // ���ʵ�ʼӳɵȼ�����

const   int     LEN_NAMECHAR    = 16;       // ���������ִ�����


//��Ӧÿ�������ṩ5%�ļӳ�
//[����ı�ͻ���ҲҪ��Ӧ�Ķ�]
enum E_TEAMSKILLTYPE
{
    ESKT_WIND = 55,      // �磺�伲��� 5�� �����ӳ�
    ESKT_WOOD,           // �֣��������� 5�� �����ڹ��������ӳ�
    ESKT_FIRE,           // ��������� 5�� �����⹦�������ӳ�
    ESKT_HILL,           // ɽ��������ɽ 5�� �����������ӳ�
      
    ESKT_CLOUD,          // ������  ͬʱ����������  ����ԭ��ʽ ѧ����滻 ����μ�
    ESKT_TAIYI,          // ̫����  ͬʱ�ڣ�������  ����ԭ��ʽ ѧ����滻 �ֶ��μ�           
    ESKT_HEYI,           // ������  �ƶ��ٶ�����������������  ÿ��һ�����ƶ��ٶ�����1%������������1% ѧ����滻 ����μ�
    ESKT_FENGSHI,        // ��ʸ��  �����ٶ�����������������  ÿ��һ���˹����ٶ�����1%������������1% ѧ����滻 ɽ���μ�

    ESKT_MAX
};

//extern int g_FightBaseCount[EFT_MAX];

// ��������
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

// ������������
struct SWoundData
{
    BYTE    byWoundFrame;       // �����ٶ�
};

//////////////////////////////////////////////////////////////////////// �����书
struct SBaseFightType
{
    short    wAttackTime;        // �����ٶȣ�������������֡����
    short    byNextTime;         // �´ι���ʱ����
    short    wHitTime;           // �����ٶȣ��ڵڼ�֡���У�
    short    byDist;             // ��������
};

//////////////////////////////////////////////////////////////////////// �书��ʽ
struct SSkillData
{
    short   wIndex;             // ���	
    char    szName[LEN_NAMECHAR];         // ����                       
    short   wPicId;             // ��Ӧ����                             
    short   wIsAtack;           // �Ƿ�������                      
    short   wWoundNum;          // ������λ                          
    short   wAttackType;        // ������ʽ                       
    short   wDistance;          // ��������                             
    short   wTargetArea;        // ������Χ                             
    short   wAttackTime;        // �����ٶȣ�ʱ�䣩                     
    short   wHitTime;           // ����ʱ��                             
    short   wAttackCoef;        // ����ϵ��                             
    short   wHitOdds;           // ������                               
    short   wHarmHP;            // �����˺�                             
    short   wHarmMP;            // �����˺�                             
    short   wHarmSP;            // �����˺�                             
    short   wKnockoutOdds;      // ���ɼ���                             
    short   wKnockoutDist;      // ���ɾ���                             
    short   wVitalsOdds;        // ��Ѩ����                             
    short   wVitalsTime;        // ��Ѩʱ��                             
    short   wDizzyOdds;         // ��ѣ����                       
    short   wDizzyTime;         // ��ѣʱ��    
    short   wDropWeaponOdda;    // ������������p         
    short   wDropEquipmentOdds; // ������߼���         
    short   wLimitAttackOdds;   // ���м���                             
    short   wLimitAttackTime;   // ����ʱ��                             
    short   wAddHPRate;         // �ָ���������p                        
    short   wAddSPRate;         // �ָ���������p                        
    short   wAddDamTime;        // �����⹦������ʱ��                   
    short   wAddDamRate;        // �����⹦����������p                  
    short   wAddPowTime;        // �����ڹ�������ʱ��                   
    short   wAddPowRate;        // �����ڹ�����������p                  
    short   wAddDefTime;        // ����������ʱ��                       
    short   wAddDefRate;        // ��������������p                      
    short   wAddAgiTime;        // ����������ʱ��                       
    short   wAddAgiRate;        // ���������ȱ���p                      
    short   wRedAgiTime;        // ����������ʱ��                       
    short   wRedAgiRate;        // ���������ȱ���                       
    short   wToxicOdds;         // �ж�����                             
    short   wToxicTime;         // �ж���ʱ��                           
    short   wToxicRedHPRate;    // �ж�����������                       
    short   wAddAmuckTime;      // ����ɱ��ʱ��                         
    short   wAddAmuckVal;       // ����ɱ������p                        
    short   wDoubleMoneyOdds;   // ˫��Ǯ�Ｘ��m    
    short   wAddWeaponTime;     // ������������ʱ��                     
    short   wAddWeaponRate;     // �����������ܱ���p                    
    short   wAddEquipTime;      // ������������ʱ��                     
    short   wAddEquipRate;      // �����������ܱ���p                    
    short   wDemToHPRate;       // �����˺�ת��������                   
    short   wDemToSPRate;       // �����˺�ת��������                   
    short   wUseupSP;           // ��������   
    short   wUseupMP;           // ��������   
    short   wUseupHP;           // ��������
    WORD    wAtkHardCoef;       // �书�Ѷ�ϵ��                         
    WORD    wAttackRvs;         // �书��������                         
    short   wDemMPToHPRate;     // ��������ת��������    
    short   wZeroSPOdds;        // ֱ�ӽ����������ļ���
    short   wAddHPMaxTime;      // ����HPʱ��                   
    short   wAddHPMaxRate;      // ����HP����
};    

////////////////////////////////////////////////////////////////////// �����������
class DFightData
{
public:
    SBaseFightType  m_BaseFightList[6];                    // 5�����ɵĻ����书 + �����
    SSkillData      m_Data[MAX_SKILLCOUNT]; // �书�б�

    SWoundData      m_WoundData[EW_MAX];

    // ��Χ�������˺����ʾ���
    int             m_AttackArea[MAX_ATTACKAREASIZE*8][MAX_ATTACKAREASIZE];

    // ������Ʒ����������
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
    
    BOOL    LoadFightDate(void);                        // �����书����

    bool    LoadDropItemData(void);
    void    ReleaseDropItemData(void);
    bool    LoadRandDropItemData(void);
    void    ReleaseRandDropItemData(void);

    SSkillData  *GetSkill( int iSkillID );              // ��ȡ�书���ݣ�Level=0�����书
    int         *GetAreaMatrix( int iSkillID, BYTE byDir );
    int         GetAttackPosCoef( int iSkillID, BYTE byDir, int x, int y );

    WORD    GetRandDropItem(void);
    SBaseFightType  *GetBaseFightOnSchool( BYTE school );

    WORD    GetDropItemByRand( WORD wBaseRand );
};

extern  DFightData  *g_pFightData;

////////////////////////////////////////////////////////////////////// ������Χ���ݽṹ
//typedef struct  _SFightArea
//{
//    BOOL    bAxisSymm;          // �Ƿ���Գ�
//    BYTE    bySymmRadius;       // �Գư뾶�����ڴ����Ĳ���ƫ��������������Χ
//}SFightArea,*LPSFightArea;

/*
���	
����	
��Ӧ����	
��������	
������λ	
������ʽ	
��������	
������Χ	
����ʱ��	
����ʱ��	
����ϵ��	
������	
�����˺�	
�����˺�	
�����˺�	
���ɼ���	
���ɾ���	
��Ѩ����	
��Ѩʱ��	
ѣ�μ���	
ѣ��ʱ��	
������������	
������߼���	
���м���	
����ʱ��	
�ָ���������	
�ָ���������	
�����⹦������ʱ��	
�����⹦����������	
�����ڹ�������ʱ��	
�����ڹ�����������	
����������ʱ��	
��������������	
����������ʱ��	
���������ȱ���	
����������ʱ��	
���������ȱ���	
�ж�����	
�ж���ʱ��	
�ж�����������	
����ɱ��ʱ��	
����ɱ������	
˫��Ǯ�Ｘ��	
������������ʱ��	
�����������ܱ���	
������������ʱ��	
�����������ܱ���	
�����˺�ת��������	
�����˺�ת��������	
��������	
��������	
��������	
�书�Ѷ�ϵ��	
�书��������	
��������ת��������	
ֱ�ӽ����������ļ���
*/

#endif  //_DFiGHTDATA_H_