#include "stdafx.h"
#include "ProtectedDataTable.h"

CProtectedDataTable::TPDMap CProtectedDataTable::m_mProtectedData;

bool CProtectedDataTable::Load( const char* filename )
{
	dwt::ifstream stream(filename);
	if (!stream.is_open())
		return false;

	char lineBuffer[1024];

	// 跳过表头
	stream.getline(lineBuffer, sizeof(lineBuffer));

	// 读护体数据
	while (!stream.eof())
	{
		stream.getline( lineBuffer, sizeof( lineBuffer ) );
		if ( *lineBuffer == 0 || *( LPWORD )lineBuffer == '//' || *( LPWORD )lineBuffer == '--' || stream.eof() )
			continue;

		std::strstream lineStream( lineBuffer, ( int )strlen( lineBuffer ), ios_base::in );

		ProtectedData data;
		int nId;

		lineStream  >> nId >> data.name  >> data.type >> data.school
					>> data.data[ MaxLevel] 
					>> data.data[DiffcultBase] >> data.data[DiffcultFactor]
					>> data.data[RemainTimeBase] >> data.data[RemainTimeFactor]
					>> data.data[OutDefenceBase] >> data.data[OutDefenceFactor]
					>> data.data[InnerDefenceBase] >> data.data[InnerDefenceFactor]
					>> data.data[HPLimitedBase] >> data.data[HPLimitedFactor]
					>> data.data[ReduceHurtRateBase] >> data.data[ReduceHurtRateFactor]
					>> data.data[ReduceAbsoluteHurtBase] >> data.data[ReduceAbsoluteHurtFactor]
					>> data.data[CountAttackRateBase] >> data.data[CountAttackRateFactor]
					>> data.data[ConsumHPBase] >> data.data[ConsumHPFactor]
					>> data.data[ConsumMPBase] >> data.data[ConsumMPFactor]
					>> data.data[ConsumSPBase] >> data.data[ConsumSPFactor]
					>> data.data[ColdTime] >> data.information;

		--data.type;
		m_mProtectedData[nId-1] = data;
	}

	stream.close();

	return true;
}

void CProtectedDataTable::Destroy()
{
	m_mProtectedData.clear();
}

const CProtectedDataTable::ProtectedData* CProtectedDataTable::GetData( int id )
{
	TPDMap::const_iterator citr = m_mProtectedData.find( id );
	return citr==m_mProtectedData.end() ? 0 : &(citr->second);
}