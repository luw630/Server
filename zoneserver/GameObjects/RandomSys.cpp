#include "StdAfx.h"
#include "randomsys.h"
#include "itemdata.h"
//#include "itemuser.h"

DWORD tickRandom()
{
    DWORD lowTicker;
    _asm rdtsc
    _asm mov lowTicker, eax

    DWORD result = ( ( lowTicker << 15 ) | ( rand() ) );

    srand( lowTicker );

    return result;
}

RandomAbilityTable RandomAbilityTable::RATable;

namespace RandomSys
{
    static struct RAII
    {
        RAII()
        {
            memset( &RandomAbilityTable::RATable, 0, sizeof( RandomAbilityTable::RATable ) );
        }

    } resource_acquisition_is_initialization;
}

// 为成长比率添加一个比率参数，用于特定装备等级下的特殊调整。。。
inline DWORD AddValue( DWORD k, DWORD level, DWORD rate = 4 )
{
    if ( k == 0 )
        return 0;

    DWORD pt1 = ( k * rate / 100 );
    if ( pt1 == 0 ) 
        return k + level;

    return k + ( k * rate * level / 100 );
}

void CEquipmentItemDataEx::operator += ( const SEquipment &equipment )
{
// zeb 2009.11.19
//    const SItemData *equipData = CItem::GetItemData( equipment.wIndex );
//    if ( ( equipment.wIndex == 0 ) || ( equipData == NULL ) )
//        return;
//
//    // 有耐久并且为0 装备作用消失
//    if ( equipData->wWearPrime != 0xffff && (equipment.attribute.curWear == 0 || equipment.attribute.maxWear == 0) )
//        return;
//
///*
//    // 检查套状个数
//    WORD index = equipData->nMainLevel + equipData->byGroupEM * 100;
//    std::map< WORD, SSuitCheck >::iterator iter = suitCheckMap.find( index );
//    if ( iter == suitCheckMap.end() )
//    {
//        SSuitCheck temp;
//        temp.suitCount = 1;
//        temp.subLevel = equipment.level;
//        suitCheckMap[ index ] = temp;
//    }
//    else
//    {
//        if ( iter->second.subLevel > equipment.level )
//            iter->second.subLevel = equipment.level;
//
//        iter->second.suitCount++;
//        if ( iter->second.suitCount >= 8 )
//        {
//            suiteData.mainLevel = iter->first;
//            suiteData.subLevel = iter->second.subLevel;
//            suiteData.sameNumber = iter->second.suitCount;
//        }
//    }
//*/
//
//	if ( equipData->nMainLevel >= 50 )
//		return;
//
//	// 指定阶数的套装添加件数
//	levelCheck[equipData->nMainLevel][0] ++;
//	if ( levelCheck[equipData->nMainLevel][1] > equipment.level )
//		levelCheck[equipData->nMainLevel][1] = equipment.level;
//		
//    // 按强化等级统计五大套
//    if ( equipData->nMainLevel >= 20 && equipData->nMainLevel <= 24 )
//    {
//        wuSuitCount++;
//        wuSuitLvSameCount[ equipment.level ]++;
//    }
//
//    // 看该装备是否有前缀属性，统计前缀属性加层
//    //switch ( equipment.prefix >> 4 )
//    //{
//    //case 1: suiteData.wind     += ( equipment.prefix & 0xf ) * 10; break;
//    //case 2: suiteData.forest   += ( equipment.prefix & 0xf ) * 10; break;
//    //case 3: suiteData.fire     += ( equipment.prefix & 0xf ) * 10; break;
//    //case 4: suiteData.mountain += ( equipment.prefix & 0xf ) * 10; break;
//    //}
//
//    // 基本道具增加的属性
//	DWORD rate = ( equipData->byType == ( EQUIPMENT_SEGMENT + 3 ) ) ? 3 : 4;
//
//    DWORD dwAEMrate = 100;
//    if ( equipData->wActiveLevelEM > 0 && equipData->wActiveLevelEM <= 300 && wPlayerLevel >= equipData->byLevelEM )
//        dwAEMrate = equipData->wActiveBase + wPlayerLevel - equipData->byLevelEM;
//    if ( dwAEMrate > 100 )
//        dwAEMrate = 100;
//
//	wDEFAD[0]    += ( AddValue( equipData->wDEFAD, equipment.level, rate ) * dwAEMrate / 100 );      // 防御力
//	wAGIAD[0]    += ( AddValue( equipData->wAGIAD, equipment.level, rate ) * dwAEMrate / 100 );      // 灵敏
//	wPOWAD[0]    += ( AddValue( equipData->wPOWAD, equipment.level, rate ) * dwAEMrate / 100 );      // 内功攻击力
//	wDAMAD[0]    += ( AddValue( equipData->wDAMAD, equipment.level, rate ) * dwAEMrate / 100 );      // 外功攻击力
//	wRHPAD[0]    += ( AddValue( equipData->wRHPAD, equipment.level, rate ) * dwAEMrate / 100 );      // 加生命自动恢复速度
//	wRMPAD[0]    += ( AddValue( equipData->wRMPAD, equipment.level, rate ) * dwAEMrate / 100 );      // 加内力自动恢复速度
//	wRSPAD[0]    += ( AddValue( equipData->wRSPAD, equipment.level, rate ) * dwAEMrate / 100 );      // 加体力自动恢复速度
//	wAmuckAD[0]  += ( AddValue( equipData->wAmuckAD, equipment.level, rate ) * dwAEMrate / 100 );    // 加杀气
//	wLUAD[0]     += ( AddValue( equipData->wLUAD, equipment.level, rate ) * dwAEMrate / 100 );       // 加气运
//	wHPAD[0]     += ( AddValue( equipData->wHPAD, equipment.level, rate ) * dwAEMrate / 100 );       // 增加生命最大值
//	wSPAD[0]     += ( AddValue( equipData->wSPAD, equipment.level, rate ) * dwAEMrate / 100 );       // 增加体力最大值
//	wMPAD[0]     += ( AddValue( equipData->wMPAD, equipment.level, rate ) * dwAEMrate / 100 );       // 增加内力最大值
//
//    // 装备颜色附加属性 已经被屏蔽掉的功能！！！
//    // GetColorEffects( equipData, equipment.reserve & 0x3 );
//
//    // 装备上的玉石附加
//    GetJewelEffects( equipment );
//
//    // 装备上的随机属性附加
//    GetRandomEffects( equipment );
// zeb 2009.11.19
}

// 规则：1.五大套具有对10阶以下装备的“听用效果”，小人，脚底光效 主等级和最小等级以10阶套装为准 
//       2.五大套自身混穿没有小人，脚底光效
//       3.小人：14件有  光效：>7 14 +9 
//       4.豹 虎 龙 普通装备：有五大套影响； 五大套：自己算自己的
// 提示：不可能满足豹 虎 龙的套装是不相同的。所以可以忽略 4 4(不同的1-10阶级) 3 3(不同的五大套) 这样的装备数量分布上的逻辑问题。
// ----------------------------
// 函数说明：
// 返回值：表示套装的类型，见枚举
// 额外功能：统计套装的一些性质
// ----------------------------------
BOOL CEquipmentItemDataEx::SuiteApply()
{ 
    // 五大套
    bool suit_conmmonx8 = false;
    for ( int EquipMainLv = 20; EquipMainLv < 30; ++EquipMainLv )
    {
        if ( levelCheck[ EquipMainLv ][0] != 0xff && levelCheck[ EquipMainLv ][0] >= 7 ) 
        {
            suitCheck.type = EquipMainLv;                           // 类型 主等级
            suitCheck.count = levelCheck[EquipMainLv][0] + 1;       // 套装数量
            suitCheck.level = levelCheck[EquipMainLv][1];           // 最小等级
            suitCheck.ucount = ( suitCheck.count == 14 ) ? 2 : 1;   // 类型

            suiteData.mainLevel = suitCheck.type;
            suiteData.subLevel = suitCheck.level;

            if ( suitCheck.count == 14 )
            {           
                if ( levelCheck[ EquipMainLv ][1] == 9 )
                    return EquipMainLv + 10;
                else
                    return EquipMainLv;
            }

            suit_conmmonx8 = true;
        }
    }

    // 普通 levelCheck[ EquipMainLv ][0] 最大取值13
    for ( int EquipMainLv = 1; EquipMainLv < 20; ++EquipMainLv )
    {             
        if ( levelCheck[ EquipMainLv ][0] != 0xff && ( levelCheck[ EquipMainLv ][0] >= 7 || (levelCheck[ EquipMainLv ][0] + wuSuitCount >= 7) ) )
        {
            suitCheck.type = EquipMainLv;   // 豹 4 4 3 3这样的套装分布也没关系
            suitCheck.count = 8;            // 比7大就好
            suitCheck.level = levelCheck[ EquipMainLv ][1];  
            suitCheck.ucount = 1;

            suiteData.mainLevel = suitCheck.type;
            suiteData.subLevel = suitCheck.level;

            if ( levelCheck[ EquipMainLv ][0] + 1 == 14 || levelCheck[ EquipMainLv ][0] + 1 + wuSuitCount == 14 )
            {              
                suitCheck.ucount = 2;

                if ( (levelCheck[ EquipMainLv ][0] + 1 == 14 && levelCheck[EquipMainLv][1] == 9)
                    || (levelCheck[ EquipMainLv ][0] + 1 + wuSuitCount == 14 && levelCheck[EquipMainLv][1] == 9 && wuSuitLvSameCount[9] == wuSuitCount) )
                {
                    suitCheck.count = 14;
                    return SUIT_TYPE::SUIT_COMMONX14MAX;
                }
                else
                    return SUIT_TYPE::SUIT_COMMONX14;
            }

            return SUIT_TYPE::SUIT_COMMONX8;
        }
    }

    if ( suit_conmmonx8 )
        return SUIT_TYPE::SUIT_COMMONX8;

    return SUIT_TYPE::SUIT_NULL;
}

// 算出装备颜色的效果，目前为3色阶
// void CEquipmentItemDataEx::GetColorEffects( const SItemData *itemData, int color )
// {
// zeb 2009.11.19
 //   // 装备颜色附加属性 已经被屏蔽掉的功能！！！

 //   assert( color <= 3 && itemData != NULL );

 //   if ( color == 0 )
 //       return;

 //   //是否有色装备,是则统计属性
 //   float rating[4] = { 0.f, 0.1f, 0.21f, 0.331f };
 //                                                                                   
 //   #define _COLOREQCOUNT( _cb ) (int)( _cb * rating[ color ] );
 //   #define DOUBLE_COLOREQCOUNT( _dst, _cb ) _dst[0] += _COLOREQCOUNT( _cb ); _dst[1] += _COLOREQCOUNT( _cb );

 //   // 基本道具增加的属性
 //   DOUBLE_COLOREQCOUNT( wDEFAD,   itemData->wDEFAD     );    // 防御力
	//DOUBLE_COLOREQCOUNT( wAGIAD,   itemData->wAGIAD     );    // 灵敏
	//DOUBLE_COLOREQCOUNT( wPOWAD,   itemData->wPOWAD     );    // 内功攻击力
	//DOUBLE_COLOREQCOUNT( wDAMAD,   itemData->wDAMAD     );    // 外功攻击力
	//DOUBLE_COLOREQCOUNT( wRHPAD,   itemData->wRHPAD     );    // 加生命自动恢复速度
	//DOUBLE_COLOREQCOUNT( wRMPAD,   itemData->wRMPAD     );    // 加内力自动恢复速度
	//DOUBLE_COLOREQCOUNT( wRSPAD,   itemData->wRSPAD     );    // 加体力自动恢复速度
	//DOUBLE_COLOREQCOUNT( wAmuckAD, itemData->wAmuckAD   );    // 加杀气
	//DOUBLE_COLOREQCOUNT( wLUAD,    itemData->wLUAD      );    // 加气运
 //   DOUBLE_COLOREQCOUNT( wHPAD,    itemData->wHPAD      );    // 增加生命最大值
 //   DOUBLE_COLOREQCOUNT( wSPAD,    itemData->wSPAD      );    // 增加体力最大值
 //   DOUBLE_COLOREQCOUNT( wMPAD,    itemData->wMPAD      );    // 增加内力最大值

 //   #undef DOUBLE_COLOREQCOUNT
 //   #undef _COLOREQCOUNT
 // zeb 2009.11.19
/*}*/

// 这里考虑到这个数值应该不会存在过多变动所有就写死在这里
static int jeweDataList[ 12 ] = { 0, 1, 2, 3, 4, 5, 7, 9, 11, 14, 17, 25 };
static int jeweDataListYellow[ 12 ] = { 0, 1, 2, 3, 4, 5, 7, 11, 15, 20, 30, 50 }; // 黄特殊处理
static int jeweDataListBlue[ 12 ] = { 0, 1, 2, 3, 4, 5, 7, 11, 15, 19, 25, 40 };   // 蓝特殊处理

void CEquipmentItemDataEx::GetJewelEffects( const SEquipment &equipment )
{
    for ( int i = 0; ( i < SEquipment::MAX_SLOTS ) && equipment.slots[i].isJewel(); i ++ )
	{
        LPWORD data = NULL;
        switch ( equipment.slots[i].type ) 
        {

            case 1: data = wDAMAD;    break; // 黄玉 +外功
            case 2: data = wPOWAD;    break; // 蓝玉 +内功
            case 3: data = wDEFAD;    break; // 绿玉 +防御
            case 4: data = wAGIAD;    break; // 白玉 +灵敏
            case 5: data = wLUAD;     break; // 红玉 +气运
            case 6: data = wAmuckAD;  break; // 黑玉 +杀气
            default :                 return;
        } 

        BYTE index = equipment.slots[i].value;
	    if ( index == 25 )
		    index = 11;
        if ( index > 11 )
		    continue;

        int addVal = 0;
        if ( equipment.slots[i].type == 1 )
            addVal = jeweDataListYellow[ index ];
        else if ( equipment.slots[i].type == 2 )
            addVal = jeweDataListBlue[ index ];
        else
            addVal = jeweDataList[ index ];

        data[1] += addVal;
        data[0] += addVal;
    }
}

// void AttAdd( SGemData *rs, DWORD idx, __int16 v )
// {
//     if ( idx > SGemData::GEMDATA_MAX )
//         return;
// 
//     ( ( __int16* )rs )[idx] += 
//         ( ( idx + 1 ) == SGemData::GEMDATA_ACTIONSPEED || ( idx + 1 ) == SGemData::GEMDATA_MOVESPEED ) 
//         ? ( v - 500 ) 
//         : v;
// }

void CEquipmentItemDataEx::GetRandomEffects( const SEquipment &equipment )
{
	/*
    // 随即属性
    int rank = equipment.level / 3;
    if ( rank > SEquipment::MAX_ACTIVATES ) 
        rank = SEquipment::MAX_ACTIVATES;
 
    // 强化附加等级每3条激活一个属性
    SEquipment::Attribute::Unit const * att = equipment.attribute.activates;
    for ( int i = 0; i < rank; i++ )
        AttAdd( &attribute, att[i].type - 1, att[i].value );

    // 天生附加
    att = &equipment.attribute.naturals;
    AttAdd( &attribute, att[0].type - 1, att[0].value );

    // 只有鉴定后这里属性才有用
    // if ( equipment.isDevelop == 0 ) 这个逻辑可能不需要吧，未鉴定的属性本来就为空！
    {
        att = equipment.attribute.qualitys;
        for ( int i = 0; i < SEquipment::MAX_QUALITYS ; i++ )
            AttAdd( &attribute, att[i].type - 1, att[i].value );
    }

    att = equipment.attribute.inherits;
    for ( int i = 0; i < SEquipment::MAX_INHERITS; i++ )
        AttAdd( &attribute, att[i].type - 1, att[i].value );

    // 武功和心法加层 为专用的固定槽放置
    {
        BYTE stype = equipment.attribute.unitSkill.skillType;
        BYTE svalue = equipment.attribute.unitSkill.skillValue;
        attribute.skillLevel = ( ( stype << 8 ) | svalue );

        BYTE ttype = equipment.attribute.unitSkill.teletgyType;
        BYTE tvalue = equipment.attribute.unitSkill.teletgyValue;
        attribute.telergyLevel = ( ( ttype << 8 ) | tvalue );
    }//*/
}
