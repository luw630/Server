#pragma once

#include "networkmodule\itemtypedef.h"

// ������µ�������Ա�
struct RandomAbilityTable
{
    enum RANDOM_TYPE { 
        TYPE_NULL = 0,
        DAMAGE = 1,				 // �⹥
        POWER,						// �ڹ�
        DEFENCE,						// ����
        AGILITY,							// ����
        LUCK,							// ����
        AMUCK,							// ɱ��
        INTELLECT,					 // ����
        HP_MAX,						// Ѫ������
        MP_MAX,						// ��������
        SP_MAX,						// ��������
        HP_REGENERATE,			// �����ָ�
        MP_REGENERATE,			// �����ָ�
        SP_REGENERATE,			// �����ָ�
        TYPE_MAX,
    };

    // ˵���ñ������Ƿ���Ч������ʹ�ÿ����ݲ������
    BOOL validate;

    // ��������luaʹ�ø�������ʾ��ת����ʹ�ó�1�ڵķ����������ʻ���
    // so... hitRating = double_rate * 100000000
    struct Unit {
        RANDOM_TYPE randomType;     // �ñ�Ŷ�Ӧ����������
        DWORD       hitRating;      // = double_rate * 100000000;

        union {
            struct SubUnit {
                DWORD   rating;         // = double_rate * 100000000;
                float   value;          // ��Ӧ�ĸ���ֵ(֮�����ø���������Ϊ�˷����Ժ������չ����%��ʾ����ֵ��)
            };

            SubUnit base_default;       // array[0] = default
            SubUnit array[8];           // 8���εı仯
        };

    } units[32];                        // units[0] = special

    static RandomAbilityTable RATable;
};

namespace RandomAbility
{
    // �������ݶ�Ӧ���趨��RANDOM_TYPE˳��
    const static LPCSTR RANDOM_TYPE_STRING[] = 
    {
        "",             // 0
        "�⹥",         // 1
        "�ڹ�",
        "����",
        "��",
        "����",
        "ɱ��",
        "����",
        "Ѫ������",
        "��������",
        "��������",
        "��Ѫ�ٶ�",
        "�����ٶ�",
        "�����ٶ�"
    };
}

// �ýṹ�������۽�ɫ��������װ������������
class CEquipmentItemData
{
public:
    CEquipmentItemData() { ZeroEquipExtraData(); }

    WORD GetEquipDAMData()  const  { return wDAMAD[0];     }
    WORD GetEquipPOWData()  const  { return wPOWAD[0];     }
    WORD GetEquipDEFData()  const  { return wDEFAD[0];     }
    WORD GetEquipAGIData()  const  { return wAGIAD[0];     }
    WORD GetEquipRHPData()  const  { return wRHPAD[0];     }
    WORD GetEquipRSPData()  const  { return wRSPAD[0];     }
    WORD GetEquipRMPData()  const  { return wRMPAD[0];     }
    WORD GetEquipAMUData()  const  { return wAmuckAD[0];   }
    WORD GetEquipLUData()   const  { return wLUAD[0];      }
    WORD GetEquipINData()   const  { return wIN[0];        }
    WORD GetEquipHPData()   const  { return wHPAD[0];      }
    WORD GetEquipSPData()   const  { return wSPAD[0];      }
    WORD GetEquipMPData()   const  { return wMPAD[0];      }

    void ZeroEquipExtraData(void) { ZeroMemory( this, sizeof( CEquipmentItemData ) ); } 
    //void operator += ( const SEquipment &equipment );
    //BOOL SuiteApply();                                  // Ӧ����װ�Ӳ�����

    //// ������Ч�����������Ч��ֻ����ʹ����װ���ϵĶ���
    //void GetJewelEffects( const SEquipment &equipment );

    //// ��������Ч�����������Ч��ֻ����ʹ����װ���ϵĶ���
    //void GetRandomEffects( const SEquipment &equipment );

    //// ���װ����ɫ��Ч����ĿǰΪ3ɫ��
    //void GetColorEffects( const SItemData *itemData, int color );

public:
    // �����3����Ԫ�ֱ��ʾ��������ֵ��װ��ԭ��������+���2� | ��Ƕ���������� | ���������������
    WORD wDAMAD[3];        // �ӹ�����
    WORD wPOWAD[3];        // ���ڹ�������
    WORD wDEFAD[3];        // �ӷ�����
    WORD wAGIAD[3];        // ��������
    WORD wAmuckAD[3];      // ��ɱ��
    WORD wLUAD[3];         // ������
    WORD wRHPAD[3];        // �������Զ��ָ��ٶ�
    WORD wRSPAD[3];        // �������Զ��ָ��ٶ�
    WORD wRMPAD[3];        // �������Զ��ָ��ٶ�
    WORD wHPAD[3];         // �����������ֵ
    WORD wSPAD[3];         // �����������ֵ
    WORD wMPAD[3];         // �����������ֵ
    WORD wIN[3];           // ����

public:
    struct {
        DWORD mainLevel;    // װ�����ȼ��������ж��Ƿ�Ϊ��װ��
        DWORD subLevel;     // װ���ӵȼ�������С�ȼ���ȷ���ӳ����ͣ�
        DWORD scale;        // �Ӳ����  �����ݵ�ǰ��װ����С�ȼ����
        DWORD type;         // �Ӳ�����  ��1:DAM 2:POW 3:DEF 4:AGI 5:AMUCK 6:LUCK��
        DWORD sameNumber;   // ��ͬ���ȼ����ߵ���������Ҫ��������Ϊһ��

        // ���ֻ�ɽ4���ԣ�
        int wind;
        int forest;
        int fire;
        int mountain;
    } suiteData;

// public:
//     SGemData attribute;
};

// ����ṹ���ڴ��һЩʵ�ַ��������ⱻʩ�ŵ�
class CEquipmentItemDataEx : public CEquipmentItemData
{
public:
    void operator += ( const SEquipment &equipment );
    BOOL SuiteApply();                                  // Ӧ����װ�Ӳ�����  ����ֵΪ��װ����

    // ������Ч�����������Ч��ֻ����ʹ����װ���ϵĶ���
    void GetJewelEffects( const SEquipment &equipment );

    // ��������Ч�����������Ч��ֻ����ʹ����װ���ϵĶ���
    void GetRandomEffects( const SEquipment &equipment );

    // ���װ����ɫ��Ч����ĿǰΪ3ɫ��
  /*  void GetColorEffects( const SItemData *itemData, int color );*/

    enum SUIT_TYPE{ 
        SUIT_NULL = 0,              // û����װ
        SUIT_COMMONX8,              // ��ͨ8��
        SUIT_COMMONX14,             // ��ͨ14��
        SUIT_COMMONX14MAX,          // ��ͨ14��+9

        SUIT_SENIOR_MERCY = 20,     // �� 14
        SUIT_SENIOR_REINS,          // ��
        SUIT_SENIOR_COMITY,         // ��
        SUIT_SENIOR_WISDOM,         // ��
        SUIT_SENIOR_INTEGRITY,      // ��

        SUIT_SENIOR_MERCYMAX = 30,  // �� 14 + 9   ��������չ
        SUIT_SENIOR_REINSMAX,       // ��
        SUIT_SENIOR_COMITYMAX,      // ��
        SUIT_SENIOR_WISDOMMAX,      // ��
        SUIT_SENIOR_INTEGRITYMAX,   // ��
        };

    // ��װ�������
public:
    struct SSuitCheck
    {
        BYTE count;	// ʵ�ʼ���
        BYTE type;	// ����
        BYTE ucount;	// 1 = ��ͨ8���� 2 = 14��
        BYTE level;	// ��С�ȼ�
    } suitCheck;

    //std::map< WORD, SSuitCheck > suitCheckMap;
    BYTE levelCheck[50][2];
    BYTE wuSuitLvSameCount[10];           // ���ȼ�ͳ��5���׵ļ��� 0-9
    WORD wuSuitCount;                     // ����׵ļ���

    WORD wPlayerLevel;                    // ��ҵȼ�

public:
    void ZeroEquipExtraData(void) { 
    	CEquipmentItemData::ZeroEquipExtraData(); 
	memset( &suitCheck, 0, sizeof( suitCheck ) );
	memset( levelCheck, -1, sizeof( levelCheck ) );
    memset( wuSuitLvSameCount, 0, sizeof(wuSuitLvSameCount) );
    wuSuitCount = 0;
    wPlayerLevel = 0;
    } 
};

