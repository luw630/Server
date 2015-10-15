#pragma  once

#include "networkmodule/playertypedef.h"
class CTiZhiService
{
private:
	CTiZhiService();

public:
	struct TiZhiData
	{
		DWORD dwMaxAttrs[SPlayerTiZhi::TZ_MAX];
		DWORD dwPrevTotalMaxAttrs[SPlayerTiZhi::TZ_MAX];
		DWORD dwIncrements[SPlayerTiZhi::TZ_MAX];
		DWORD dwUpgradeRate;
		DWORD dwGrowItem;
		DWORD dwUpgradeItem;
		byte  dwUpgradeItemNum;
		DWORD dwNeedPlayerLevel;
		DWORD dwSpConsume;
		DWORD dwBlessItemID;  //新添的,祝福道具
	};
	static CTiZhiService& GetInstance()
	{
		static CTiZhiService instance;
		return instance;
	}
	bool Init();
	const TiZhiData& getTiZhiData(int level);
private:
	TiZhiData m_data[SPlayerTiZhi::MaxLevel];
	//SPlayerTiZhi* m_data;
private:
	// 禁止拷贝构造和拷贝赋值
	CTiZhiService(CTiZhiService &);
	CTiZhiService& operator=(CTiZhiService &);
};