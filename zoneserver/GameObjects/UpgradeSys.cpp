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

// ������������
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
//             return  rfalse( "�޷�����������[%s]�б��ļ���", SZ_SCHOOLFILE[i] );
//         }
// 
//         stream >> SchoolData[i].wBaseEN     // ����
//             >> SchoolData[i].wBaseIN        // ����
//             >> SchoolData[i].wBaseST        // ����
//             >> SchoolData[i].wBaseAG        // ��
//             >> SchoolData[i].wBaseLU;       // ����
// 
//         stream >> SchoolData[i].wBaseDAM    // �书������
//             >> SchoolData[i].wBasePOW       // �ڹ�������
//             >> SchoolData[i].wBaseDEF       // ������
//             >> SchoolData[i].wBaseAGI;      // ������
// 
//         stream >> SchoolData[i].wHPVal      // ������������
//             >> SchoolData[i].wMPVal         // ������������
//             >> SchoolData[i].wSPVal;        // ������������
// 
//         stream >> SchoolData[i].wWound[0]  // �϶α�������ϵ��
//             >> SchoolData[i].wWound[1]     // �жα�������ϵ��
//             >> SchoolData[i].wWound[2];    // �¶α�������ϵ��
// 
//         j = 0;
//         while( !stream.eof() )
//         {
//             stream >> iTemp
//                 >> iTemp
//                 >> SchoolData[i].Data[j].dwMaxHP    // �������е�����
//                 >> iTemp
//                 >> SchoolData[i].Data[j].wMaxMP    // �������е�����
//                 >> iTemp
//                 >> SchoolData[i].Data[j].wMaxSP    // �������е�����
//                 >> iTemp
//                 >> SchoolData[i].Data[j].wAmuck   // �������е�ɱ��
//                 >> iTemp
//                 >> SchoolData[i].Data[j].wPoint    // �������еĻ񽱵�����
//                 >> SchoolData[i].Data[j].dwExp;    // ���辭��
//             j++;
// 
//             // if( j>=255 )    break;
// 			if( j >= MAXPLAYERLEVEL-1 ) break;
// 
//             iTemp = iTemp;
//         }
// 
//         // ��300������������Ϊһ����ֵ, ֻ����10�����Ե���!
//         memset( &SchoolData[i].Data[MAXPLAYERLEVEL], 0, sizeof( SchoolData[i].Data[MAXPLAYERLEVEL] ) );
//         SchoolData[i].Data[MAXPLAYERLEVEL].wPoint = SchoolData[i].Data[MAXPLAYERLEVEL-1].wPoint + 10;
// 
//         stream.close();
//     }

    return  TRUE;
}

// ȡ����������
SLevelData *DUpgradeSys::GetLevelData(BYTE bySchool)
{
//     if( bySchool<SD_MAX )   return  &SchoolData[bySchool];
//     else                    return  NULL;
	return 0;
}
