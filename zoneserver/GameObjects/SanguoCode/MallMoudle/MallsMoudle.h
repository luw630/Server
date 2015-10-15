#pragma once
#include "Networkmodule\SanguoPlayerMsg.h"
#include"../PUB/Singleton.h"
#include "stdint.h"
#include <vector>
class CMallManager;
struct CGlobalConfig;
struct CommodityConfig;
enum MallType;

class CMallsMoudle : public CSingleton<CMallsMoudle>
{
public:
	CMallsMoudle();
	~CMallsMoudle();
	void DispatchMallMsg(CMallManager* pMallMgr, SMallMsgInfo* pMsg);
	/**@brief 商城自动刷新后将新的商品发给客户端*/
	void SendAutoRefreshMsgToClient(CMallManager* pMallMgr);
	/**@brief 随机商城商品*/
	bool RandomCommoditys(CMallManager* pMallMgr);
private:
	/**@brief 购买商品消息处理*/
	void _ProcessBuyCommodityMsg(CMallManager* pMallMgr, SBuyCommodityRequest* pMsg);
	/**@brief 手动刷新商品消息处理*/
	void _ProcessRefreshCommodityMsg(CMallManager* pMallMgr, SRefreshCommodityRequest* pMsg);
	/**@brief 对应商城刷新商品消耗*/
	int _GetRefreshCost(MallType type, DWORD dwRefreshedCount) const;
	/**@brief 对应商城是否能够刷新商品*/
	bool _CanRefresh(CMallManager* pMallMgr, DWORD refreshCost) const;
	/**@brief 随机商品ID 数量
	@note 在上一个方法的基础上考虑极端情况(抽取到可随机的列表时发现列表内的ID全是已随机到的物品ID 故重随机抽取剩下的列表)
	@param config [in] 商品配置
	@param dwID [out] 最终商品ID
	@param dwCount [out] 最终商品个数
	@param unusableItemIndex [in/out] 不可随机抽取的商品列表索引
	*/
	void _GetCommodityData(const CommodityConfig* config, DWORD& dwID, DWORD& dwCount, vector<int32_t>& unusableItemIndex);
	/**@brief 随机商品ID 数量
	@note 随机5个范围(1,物品列表权重)值,获取最大值索引则为最终物品ID来源列表
	@param config [in] 商品配置
	@param dwID [out] 最终物品ID
	@param dwCount [out] 最终物品个数
	*/
	void _GetCommodityData(const CommodityConfig* config, DWORD& dwID, DWORD& dwCount);
private:
	CGlobalConfig& m_GlobalConfig;
	vector<CommodityConfig*> m_vecTempCommodityConfig;	///<获取商品配置的临时容器//
	vector<int32_t> m_vecHasCommodity;	///<此次刷新操作已随机到的物品//
	vector<int32_t> m_vecUsableItem;	///<未免刷新重复商品 需从可刷新列表中去除已有物品 故用此存放可随机物品ID//
	vector<int32_t> m_vecUnusableItemsIndex;	///<存储可随机的列表中包含的全是已随到的物品的列表索引//
};

