#pragma once

#include "networkmodule\itemtypedef.h"

// 定义的新的随机属性表
struct RandomAbilityTable
{
    enum RANDOM_TYPE { 
        TYPE_NULL = 0,
        DAMAGE = 1,				 // 外攻
        POWER,						// 内功
        DEFENCE,						// 防御
        AGILITY,							// 敏捷
        LUCK,							// 幸运
        AMUCK,							// 杀气
        INTELLECT,					 // 悟性
        HP_MAX,						// 血量上限
        MP_MAX,						// 内力上限
        SP_MAX,						// 内力上限
        HP_REGENERATE,			// 生命恢复
        MP_REGENERATE,			// 内力恢复
        SP_REGENERATE,			// 内力恢复
        TYPE_MAX,
    };

    // 说明该表数据是否有效，避免使用空数据参与计算
    BOOL validate;

    // 命中率在lua使用浮点数表示，转换后，使用乘1亿的方法来做概率基数
    // so... hitRating = double_rate * 100000000
    struct Unit {
        RANDOM_TYPE randomType;     // 该编号对应的属性类型
        DWORD       hitRating;      // = double_rate * 100000000;

        union {
            struct SubUnit {
                DWORD   rating;         // = double_rate * 100000000;
                float   value;          // 对应的附加值(之所以用浮点数，是为了方便以后进行扩展，如%表示的数值！)
            };

            SubUnit base_default;       // array[0] = default
            SubUnit array[8];           // 8个段的变化
        };

    } units[32];                        // units[0] = special

    static RandomAbilityTable RATable;
};

namespace RandomAbility
{
    // 该项数据对应于设定的RANDOM_TYPE顺序
    const static LPCSTR RANDOM_TYPE_STRING[] = 
    {
        "",             // 0
        "外攻",         // 1
        "内功",
        "防御",
        "身法",
        "气运",
        "杀气",
        "悟性",
        "血量上限",
        "内力上限",
        "体力上限",
        "回血速度",
        "回内速度",
        "回体速度"
    };
}

// 该结构用来积累角色身上所有装备的增加属性
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
    //BOOL SuiteApply();                                  // 应用套装加层数据

    //// 算出玉的效果，现在玉的效果只能在使用能装备上的东西
    //void GetJewelEffects( const SEquipment &equipment );

    //// 算出随机的效果，现在玉的效果只能在使用能装备上的东西
    //void GetRandomEffects( const SEquipment &equipment );

    //// 算出装备颜色的效果，目前为3色阶
    //void GetColorEffects( const SItemData *itemData, int color );

public:
    // 数组的3个单元分别表示：总属性值（装备原配置属性+后边2项） | 镶嵌物增加属性 | 随机能力附加属性
    WORD wDAMAD[3];        // 加攻击力
    WORD wPOWAD[3];        // 加内功攻击力
    WORD wDEFAD[3];        // 加防御力
    WORD wAGIAD[3];        // 加灵敏度
    WORD wAmuckAD[3];      // 加杀气
    WORD wLUAD[3];         // 加气运
    WORD wRHPAD[3];        // 加生命自动恢复速度
    WORD wRSPAD[3];        // 加体力自动恢复速度
    WORD wRMPAD[3];        // 加内力自动恢复速度
    WORD wHPAD[3];         // 增加生命最大值
    WORD wSPAD[3];         // 增加体力最大值
    WORD wMPAD[3];         // 增加内力最大值
    WORD wIN[3];           // 悟性

public:
    struct {
        DWORD mainLevel;    // 装备主等级（用于判断是否为套装）
        DWORD subLevel;     // 装备子等级（用最小等级来确定加成类型）
        DWORD scale;        // 加层比例  （根据当前的装备最小等级查表）
        DWORD type;         // 加层类型  （1:DAM 2:POW 3:DEF 4:AGI 5:AMUCK 6:LUCK）
        DWORD sameNumber;   // 相同主等级道具的数量，需要５个才能为一套

        // 风林火山4属性！
        int wind;
        int forest;
        int fire;
        int mountain;
    } suiteData;

// public:
//     SGemData attribute;
};

// 这个结构用于存放一些实现方法，避免被施放掉
class CEquipmentItemDataEx : public CEquipmentItemData
{
public:
    void operator += ( const SEquipment &equipment );
    BOOL SuiteApply();                                  // 应用套装加层数据  返回值为套装类型

    // 算出玉的效果，现在玉的效果只能在使用能装备上的东西
    void GetJewelEffects( const SEquipment &equipment );

    // 算出随机的效果，现在玉的效果只能在使用能装备上的东西
    void GetRandomEffects( const SEquipment &equipment );

    // 算出装备颜色的效果，目前为3色阶
  /*  void GetColorEffects( const SItemData *itemData, int color );*/

    enum SUIT_TYPE{ 
        SUIT_NULL = 0,              // 没有套装
        SUIT_COMMONX8,              // 普通8件
        SUIT_COMMONX14,             // 普通14件
        SUIT_COMMONX14MAX,          // 普通14件+9

        SUIT_SENIOR_MERCY = 20,     // 仁 14
        SUIT_SENIOR_REINS,          // 义
        SUIT_SENIOR_COMITY,         // 礼
        SUIT_SENIOR_WISDOM,         // 智
        SUIT_SENIOR_INTEGRITY,      // 信

        SUIT_SENIOR_MERCYMAX = 30,  // 仁 14 + 9   用于以扩展
        SUIT_SENIOR_REINSMAX,       // 义
        SUIT_SENIOR_COMITYMAX,      // 礼
        SUIT_SENIOR_WISDOMMAX,      // 智
        SUIT_SENIOR_INTEGRITYMAX,   // 信
        };

    // 套装数量检测
public:
    struct SSuitCheck
    {
        BYTE count;	// 实际件数
        BYTE type;	// 类型
        BYTE ucount;	// 1 = 普通8件， 2 = 14件
        BYTE level;	// 最小等级
    } suitCheck;

    //std::map< WORD, SSuitCheck > suitCheckMap;
    BYTE levelCheck[50][2];
    BYTE wuSuitLvSameCount[10];           // 按等级统计5大套的件数 0-9
    WORD wuSuitCount;                     // 五大套的件数

    WORD wPlayerLevel;                    // 玩家等级

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

