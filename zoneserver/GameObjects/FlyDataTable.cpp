#include "stdafx.h"
#include "FlyDataTable.h"
#include "NetworkModule/PlayerTypedef.h"

CFlyDataTable::TFLyMap	CFlyDataTable::m_mFlyData;

bool CFlyDataTable::Load( const char* filename )
{
// 	dwt::ifstream stream(filename);
// 	if (!stream.is_open())
// 		return false;
// 
// 	char lineBuffer[1024];
// 
// 	// ������ͷ
// 	stream.getline(lineBuffer, sizeof(lineBuffer));
// 
// 	// ���Ṧ����
// 	while (!stream.eof())
// 	{
// 		stream.getline( lineBuffer, sizeof( lineBuffer ) );
// 		if ( *lineBuffer == 0 || *( LPWORD )lineBuffer == '//' || *( LPWORD )lineBuffer == '--' || stream.eof() )
// 			continue;
// 
// 		std::strstream lineStream( lineBuffer, ( int )strlen( lineBuffer ), ios_base::in );
// 		
// 		SFlyData data;
// 		int nId;
// 
// 		lineStream	>> nId >> data.name  >> data.type >>data.school
// 					>> data.data[ MaxLevel] 
// 					>> data.data[DiffcultBase] >> data.data[DiffcultFactor]
// 					>> data.data[RemainTimeBase] >> data.data[RemainTimeFactor]
// 					>> data.data[JumpTileBase] 
// 					>> data.data[UpSpeedBase] >> data.data[UpSpeedFactor]
// 					>> data.data[AGBase] >> data.data[AGFactor]
// 					>> data.data[ReduceCDBase] >> data.data[ReduceCDFactor]
// 					>> data.data[ReduceDelayBase] >> data.data[ReduceDelayFactor]
// 					>> data.data[AbsoluteParryBase] >> data.data[AbsoluteParryFactor]
// 					>> data.data[ConsumHPBase] >> data.data[ConsumHPFactor]
// 					>> data.data[ConsumMPBase] >> data.data[ConsumMPFactor]
// 					>> data.data[ConsumSPBase] >> data.data[ConsumSPFactor]
// 					>> data.data[ColdTime] >> data.information;
// 
// 		--data.type;		//���ô�1��ʼ,�����0��ʼ
// 		m_mFlyData[nId-1] = data;
// 
// 		if (data.type < 0 || data.type >= EFST_MAX)
// 		{
// 			rfalse(2, 1, "�Ṧ���ݱ����ô���[%s] Type����", data.name);
// 			return false;
// 		}
// 
// 	}
// 
// 	stream.close();

	return true;
}

void CFlyDataTable::Destroy()
{
	m_mFlyData.clear();
}

const CFlyDataTable::SFlyData* CFlyDataTable::GetData( int nId )
{
	TFLyMap::const_iterator citr = m_mFlyData.find( nId );
	return citr==m_mFlyData.end() ? 0 : &(citr->second) ;
}