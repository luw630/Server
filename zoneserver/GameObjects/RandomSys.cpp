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

// Ϊ�ɳ��������һ�����ʲ����������ض�װ���ȼ��µ��������������
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
//    // ���;ò���Ϊ0 װ��������ʧ
//    if ( equipData->wWearPrime != 0xffff && (equipment.attribute.curWear == 0 || equipment.attribute.maxWear == 0) )
//        return;
//
///*
//    // �����״����
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
//	// ָ����������װ��Ӽ���
//	levelCheck[equipData->nMainLevel][0] ++;
//	if ( levelCheck[equipData->nMainLevel][1] > equipment.level )
//		levelCheck[equipData->nMainLevel][1] = equipment.level;
//		
//    // ��ǿ���ȼ�ͳ�������
//    if ( equipData->nMainLevel >= 20 && equipData->nMainLevel <= 24 )
//    {
//        wuSuitCount++;
//        wuSuitLvSameCount[ equipment.level ]++;
//    }
//
//    // ����װ���Ƿ���ǰ׺���ԣ�ͳ��ǰ׺���ԼӲ�
//    //switch ( equipment.prefix >> 4 )
//    //{
//    //case 1: suiteData.wind     += ( equipment.prefix & 0xf ) * 10; break;
//    //case 2: suiteData.forest   += ( equipment.prefix & 0xf ) * 10; break;
//    //case 3: suiteData.fire     += ( equipment.prefix & 0xf ) * 10; break;
//    //case 4: suiteData.mountain += ( equipment.prefix & 0xf ) * 10; break;
//    //}
//
//    // �����������ӵ�����
//	DWORD rate = ( equipData->byType == ( EQUIPMENT_SEGMENT + 3 ) ) ? 3 : 4;
//
//    DWORD dwAEMrate = 100;
//    if ( equipData->wActiveLevelEM > 0 && equipData->wActiveLevelEM <= 300 && wPlayerLevel >= equipData->byLevelEM )
//        dwAEMrate = equipData->wActiveBase + wPlayerLevel - equipData->byLevelEM;
//    if ( dwAEMrate > 100 )
//        dwAEMrate = 100;
//
//	wDEFAD[0]    += ( AddValue( equipData->wDEFAD, equipment.level, rate ) * dwAEMrate / 100 );      // ������
//	wAGIAD[0]    += ( AddValue( equipData->wAGIAD, equipment.level, rate ) * dwAEMrate / 100 );      // ����
//	wPOWAD[0]    += ( AddValue( equipData->wPOWAD, equipment.level, rate ) * dwAEMrate / 100 );      // �ڹ�������
//	wDAMAD[0]    += ( AddValue( equipData->wDAMAD, equipment.level, rate ) * dwAEMrate / 100 );      // �⹦������
//	wRHPAD[0]    += ( AddValue( equipData->wRHPAD, equipment.level, rate ) * dwAEMrate / 100 );      // �������Զ��ָ��ٶ�
//	wRMPAD[0]    += ( AddValue( equipData->wRMPAD, equipment.level, rate ) * dwAEMrate / 100 );      // �������Զ��ָ��ٶ�
//	wRSPAD[0]    += ( AddValue( equipData->wRSPAD, equipment.level, rate ) * dwAEMrate / 100 );      // �������Զ��ָ��ٶ�
//	wAmuckAD[0]  += ( AddValue( equipData->wAmuckAD, equipment.level, rate ) * dwAEMrate / 100 );    // ��ɱ��
//	wLUAD[0]     += ( AddValue( equipData->wLUAD, equipment.level, rate ) * dwAEMrate / 100 );       // ������
//	wHPAD[0]     += ( AddValue( equipData->wHPAD, equipment.level, rate ) * dwAEMrate / 100 );       // �����������ֵ
//	wSPAD[0]     += ( AddValue( equipData->wSPAD, equipment.level, rate ) * dwAEMrate / 100 );       // �����������ֵ
//	wMPAD[0]     += ( AddValue( equipData->wMPAD, equipment.level, rate ) * dwAEMrate / 100 );       // �����������ֵ
//
//    // װ����ɫ�������� �Ѿ������ε��Ĺ��ܣ�����
//    // GetColorEffects( equipData, equipment.reserve & 0x3 );
//
//    // װ���ϵ���ʯ����
//    GetJewelEffects( equipment );
//
//    // װ���ϵ�������Ը���
//    GetRandomEffects( equipment );
// zeb 2009.11.19
}

// ����1.����׾��ж�10������װ���ġ�����Ч������С�ˣ��ŵ׹�Ч ���ȼ�����С�ȼ���10����װΪ׼ 
//       2.���������촩û��С�ˣ��ŵ׹�Ч
//       3.С�ˣ�14����  ��Ч��>7 14 +9 
//       4.�� �� �� ��ͨװ�����������Ӱ�죻 ����ף��Լ����Լ���
// ��ʾ�����������㱪 �� ������װ�ǲ���ͬ�ġ����Կ��Ժ��� 4 4(��ͬ��1-10�׼�) 3 3(��ͬ�������) ������װ�������ֲ��ϵ��߼����⡣
// ----------------------------
// ����˵����
// ����ֵ����ʾ��װ�����ͣ���ö��
// ���⹦�ܣ�ͳ����װ��һЩ����
// ----------------------------------
BOOL CEquipmentItemDataEx::SuiteApply()
{ 
    // �����
    bool suit_conmmonx8 = false;
    for ( int EquipMainLv = 20; EquipMainLv < 30; ++EquipMainLv )
    {
        if ( levelCheck[ EquipMainLv ][0] != 0xff && levelCheck[ EquipMainLv ][0] >= 7 ) 
        {
            suitCheck.type = EquipMainLv;                           // ���� ���ȼ�
            suitCheck.count = levelCheck[EquipMainLv][0] + 1;       // ��װ����
            suitCheck.level = levelCheck[EquipMainLv][1];           // ��С�ȼ�
            suitCheck.ucount = ( suitCheck.count == 14 ) ? 2 : 1;   // ����

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

    // ��ͨ levelCheck[ EquipMainLv ][0] ���ȡֵ13
    for ( int EquipMainLv = 1; EquipMainLv < 20; ++EquipMainLv )
    {             
        if ( levelCheck[ EquipMainLv ][0] != 0xff && ( levelCheck[ EquipMainLv ][0] >= 7 || (levelCheck[ EquipMainLv ][0] + wuSuitCount >= 7) ) )
        {
            suitCheck.type = EquipMainLv;   // �� 4 4 3 3��������װ�ֲ�Ҳû��ϵ
            suitCheck.count = 8;            // ��7��ͺ�
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

// ���װ����ɫ��Ч����ĿǰΪ3ɫ��
// void CEquipmentItemDataEx::GetColorEffects( const SItemData *itemData, int color )
// {
// zeb 2009.11.19
 //   // װ����ɫ�������� �Ѿ������ε��Ĺ��ܣ�����

 //   assert( color <= 3 && itemData != NULL );

 //   if ( color == 0 )
 //       return;

 //   //�Ƿ���ɫװ��,����ͳ������
 //   float rating[4] = { 0.f, 0.1f, 0.21f, 0.331f };
 //                                                                                   
 //   #define _COLOREQCOUNT( _cb ) (int)( _cb * rating[ color ] );
 //   #define DOUBLE_COLOREQCOUNT( _dst, _cb ) _dst[0] += _COLOREQCOUNT( _cb ); _dst[1] += _COLOREQCOUNT( _cb );

 //   // �����������ӵ�����
 //   DOUBLE_COLOREQCOUNT( wDEFAD,   itemData->wDEFAD     );    // ������
	//DOUBLE_COLOREQCOUNT( wAGIAD,   itemData->wAGIAD     );    // ����
	//DOUBLE_COLOREQCOUNT( wPOWAD,   itemData->wPOWAD     );    // �ڹ�������
	//DOUBLE_COLOREQCOUNT( wDAMAD,   itemData->wDAMAD     );    // �⹦������
	//DOUBLE_COLOREQCOUNT( wRHPAD,   itemData->wRHPAD     );    // �������Զ��ָ��ٶ�
	//DOUBLE_COLOREQCOUNT( wRMPAD,   itemData->wRMPAD     );    // �������Զ��ָ��ٶ�
	//DOUBLE_COLOREQCOUNT( wRSPAD,   itemData->wRSPAD     );    // �������Զ��ָ��ٶ�
	//DOUBLE_COLOREQCOUNT( wAmuckAD, itemData->wAmuckAD   );    // ��ɱ��
	//DOUBLE_COLOREQCOUNT( wLUAD,    itemData->wLUAD      );    // ������
 //   DOUBLE_COLOREQCOUNT( wHPAD,    itemData->wHPAD      );    // �����������ֵ
 //   DOUBLE_COLOREQCOUNT( wSPAD,    itemData->wSPAD      );    // �����������ֵ
 //   DOUBLE_COLOREQCOUNT( wMPAD,    itemData->wMPAD      );    // �����������ֵ

 //   #undef DOUBLE_COLOREQCOUNT
 //   #undef _COLOREQCOUNT
 // zeb 2009.11.19
/*}*/

// ���￼�ǵ������ֵӦ�ò�����ڹ���䶯���о�д��������
static int jeweDataList[ 12 ] = { 0, 1, 2, 3, 4, 5, 7, 9, 11, 14, 17, 25 };
static int jeweDataListYellow[ 12 ] = { 0, 1, 2, 3, 4, 5, 7, 11, 15, 20, 30, 50 }; // �����⴦��
static int jeweDataListBlue[ 12 ] = { 0, 1, 2, 3, 4, 5, 7, 11, 15, 19, 25, 40 };   // �����⴦��

void CEquipmentItemDataEx::GetJewelEffects( const SEquipment &equipment )
{
    for ( int i = 0; ( i < SEquipment::MAX_SLOTS ) && equipment.slots[i].isJewel(); i ++ )
	{
        LPWORD data = NULL;
        switch ( equipment.slots[i].type ) 
        {

            case 1: data = wDAMAD;    break; // ���� +�⹦
            case 2: data = wPOWAD;    break; // ���� +�ڹ�
            case 3: data = wDEFAD;    break; // ���� +����
            case 4: data = wAGIAD;    break; // ���� +����
            case 5: data = wLUAD;     break; // ���� +����
            case 6: data = wAmuckAD;  break; // ���� +ɱ��
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
    // �漴����
    int rank = equipment.level / 3;
    if ( rank > SEquipment::MAX_ACTIVATES ) 
        rank = SEquipment::MAX_ACTIVATES;
 
    // ǿ�����ӵȼ�ÿ3������һ������
    SEquipment::Attribute::Unit const * att = equipment.attribute.activates;
    for ( int i = 0; i < rank; i++ )
        AttAdd( &attribute, att[i].type - 1, att[i].value );

    // ��������
    att = &equipment.attribute.naturals;
    AttAdd( &attribute, att[0].type - 1, att[0].value );

    // ֻ�м������������Բ�����
    // if ( equipment.isDevelop == 0 ) ����߼����ܲ���Ҫ�ɣ�δ���������Ա�����Ϊ�գ�
    {
        att = equipment.attribute.qualitys;
        for ( int i = 0; i < SEquipment::MAX_QUALITYS ; i++ )
            AttAdd( &attribute, att[i].type - 1, att[i].value );
    }

    att = equipment.attribute.inherits;
    for ( int i = 0; i < SEquipment::MAX_INHERITS; i++ )
        AttAdd( &attribute, att[i].type - 1, att[i].value );

    // �书���ķ��Ӳ� Ϊר�õĹ̶��۷���
    {
        BYTE stype = equipment.attribute.unitSkill.skillType;
        BYTE svalue = equipment.attribute.unitSkill.skillValue;
        attribute.skillLevel = ( ( stype << 8 ) | svalue );

        BYTE ttype = equipment.attribute.unitSkill.teletgyType;
        BYTE tvalue = equipment.attribute.unitSkill.teletgyValue;
        attribute.telergyLevel = ( ( ttype << 8 ) | tvalue );
    }//*/
}
