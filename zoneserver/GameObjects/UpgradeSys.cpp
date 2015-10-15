#include "StdAfx.h"
#include "UpgradeSys.h"
#include "networkmodule/itemtypedef.h"

#include "sportsarea.h"
#include "player.h"
#include "ScriptManager.h"
#include "Networkmodule\ChatMsgs.h"
#include <time.h>


DUpgradeSys *g_pUpGradeData = NULL;

// static const char SZ_SCHOOLFILE[SD_MAX][32] = 
// {
//     "Data\\ShaolinData.txt",
//     "Data\\WuDangData.txt",
//     "Data\\EMeiData.txt",
//     "Data\\TangMenData.txt",
//     "Data\\MoJiaoData.txt",
// };

DUpgradeSys::DUpgradeSys(void)
{
    g_pUpGradeData = this;
}

DUpgradeSys::~DUpgradeSys(void)
{
}

// 载入门派数据
BOOL DUpgradeSys::LoadLevelData(void)
{
//     int     iTemp,i,j;
//     dwt::ifstream stream;
// 
//     for( i=0; i<SD_MAX; i++ )
//     {
//         stream.open( SZ_SCHOOLFILE[i] );
//         if( !stream.is_open() )
//         {
//             return  rfalse( "无法打开门派数据[%s]列表文件！", SZ_SCHOOLFILE[i] );
//         }
// 
//         stream >> SchoolData[i].wBaseEN     // 活力
//             >> SchoolData[i].wBaseIN        // 体魄
//             >> SchoolData[i].wBaseST        // 悟性
//             >> SchoolData[i].wBaseAG        // 身法
//             >> SchoolData[i].wBaseLU;       // 气运
// 
//         stream >> SchoolData[i].wBaseDAM    // 武功攻击力
//             >> SchoolData[i].wBasePOW       // 内功攻击力
//             >> SchoolData[i].wBaseDEF       // 防御力
//             >> SchoolData[i].wBaseAGI;      // 灵敏度
// 
//         stream >> SchoolData[i].wHPVal      // 生命的增加率
//             >> SchoolData[i].wMPVal         // 内力的增加率
//             >> SchoolData[i].wSPVal;        // 体力的增加率
// 
//         stream >> SchoolData[i].wWound[0]  // 上段被击修正系数
//             >> SchoolData[i].wWound[1]     // 中段被击修正系数
//             >> SchoolData[i].wWound[2];    // 下段被击修正系数
// 
//         j = 0;
//         while( !stream.eof() )
//         {
//             stream >> iTemp
//                 >> iTemp
//                 >> SchoolData[i].Data[j].dwMaxHP    // 本级该有的生命
//                 >> iTemp
//                 >> SchoolData[i].Data[j].wMaxMP    // 本级该有的内力
//                 >> iTemp
//                 >> SchoolData[i].Data[j].wMaxSP    // 本级该有的体力
//                 >> iTemp
//                 >> SchoolData[i].Data[j].wAmuck   // 本级该有的杀气
//                 >> iTemp
//                 >> SchoolData[i].Data[j].wPoint    // 本级该有的获奖点数量
//                 >> SchoolData[i].Data[j].dwExp;    // 所需经验
//             j++;
// 
//             // if( j>=255 )    break;
// 			if( j >= MAXPLAYERLEVEL-1 ) break;
// 
//             iTemp = iTemp;
//         }
// 
//         // 将300级的数据设置为一个定值, 只奖励10点属性点数!
//         memset( &SchoolData[i].Data[MAXPLAYERLEVEL], 0, sizeof( SchoolData[i].Data[MAXPLAYERLEVEL] ) );
//         SchoolData[i].Data[MAXPLAYERLEVEL].wPoint = SchoolData[i].Data[MAXPLAYERLEVEL-1].wPoint + 10;
// 
//         stream.close();
//     }

    return  TRUE;
}

// 取得门派数据
SLevelData *DUpgradeSys::GetLevelData(BYTE bySchool)
{
//     if( bySchool<SD_MAX )   return  &SchoolData[bySchool];
//     else                    return  NULL;
	return 0;
}
