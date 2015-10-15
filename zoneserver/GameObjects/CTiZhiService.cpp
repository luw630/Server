#include "Stdafx.h"
#include "CTiZhiService.h"


CTiZhiService::CTiZhiService()
{
	memset(&m_data,0,sizeof(m_data));
}

bool CTiZhiService::Init()
{
	ifstream cfg("Data\\体质数据表.txt");
	if (!cfg.is_open())
	{
		rfalse("无法打开体质数据表！");
		return false; 
	}
	char buffer[2048];
	cfg.getline(buffer,sizeof(buffer));	//跳过第一行
	int level = 0;
	while (!cfg.eof())
	{
		cfg.getline(buffer,sizeof(buffer));
		if (!buffer[0]) continue;
		std::strstream linebuf(buffer, (std::streamsize)strlen(buffer));
		linebuf>>level
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_HP]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_HP]
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_MP]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_MP]
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_TL]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_TL]
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_AT]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_AT]
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_DF]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_DF]
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_BJ]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_BJ]
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_SB]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_SB]
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_SH]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_SH]
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_MS]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_MS]
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_RS]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_RS]
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_AS]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_AS]
			   >>m_data[level].dwMaxAttrs[SPlayerTiZhi::TZ_MAX_QL]
			   >>m_data[level].dwIncrements[SPlayerTiZhi::TZ_MAX_QL]
			   >>m_data[level].dwUpgradeRate
			   >>m_data[level].dwGrowItem
			   >>m_data[level].dwUpgradeItem
			   >>m_data[level].dwUpgradeItemNum
			   >>m_data[level].dwNeedPlayerLevel
			   >>m_data[level].dwSpConsume
			   >>m_data[level].dwBlessItemID;


	   if (level > 1)
	   {
		   for (int i = 0;i < SPlayerTiZhi::TZ_MAX;i++)
		   {
			   m_data[level - 1].dwPrevTotalMaxAttrs[i] = m_data[level - 2].dwMaxAttrs[i] + m_data[level-2].dwPrevTotalMaxAttrs[i];
		   }	
	   }
	}
	return true;
}

const CTiZhiService::TiZhiData& CTiZhiService::getTiZhiData( int level )
{
	return m_data[level-1];
}
