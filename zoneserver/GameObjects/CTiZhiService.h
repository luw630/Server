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
		DWORD dwBlessItemID;  //�����,ף������
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
	// ��ֹ��������Ϳ�����ֵ
	CTiZhiService(CTiZhiService &);
	CTiZhiService& operator=(CTiZhiService &);
};