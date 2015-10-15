#pragma once
#include "stdint.h"
#include "../ExtendedDataManager.h"
#include <vector>
#include <functional>

using namespace std;
class CBaseDataManager;
struct SSanguoCommodity;
struct SSanguoMallData;
struct CommodityConfig;
enum GoodsWay;
enum SSanguoCurrencyType;

enum MallType
{
	VarietyShop = 0,		///<杂货店/
	ArenaShop = 1,			///<竞技场商店/
	ExpeditionShop = 2,		///<远征商店/
	MiracleMerchant = 3,	///<奇缘商人/
	GemMerchant = 4,		///<珍宝商人/
	LegionShop = 5,			///<军团商店/
	SoulExchange = 7,		///<将魂兑换商店
	WarOfLeagueShop = 8,	///<国战商店/
};

struct STimeData;
class CMallManager : public CExtendedDataManager
{
public:
	CMallManager(CBaseDataManager& baseDataMgr);
	virtual ~CMallManager();
	virtual MallType GetType() const = 0;
	virtual bool InitDataMgr(void * pData) = 0;
	virtual bool ReleaseDataMgr();
	/**@brief 获取商品花费货币类型和花费
	@param [in] idnex 商品索引
	@param [out] currency 返回商品消耗的货币类型
	@param [out] cost 购买商品所需消耗
	@return 查找不到商品数据或根据商品ID查找不到配置文件返回false
	*/
	virtual bool GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const;
	/**@brief 商城是否开放*/
	virtual bool IsOpened();
	/**@brief 获取消费途径*/
	virtual GoodsWay RechargeWay() const = 0;
	/**@brief VIP等级改变时的更新操作*/
	virtual void VipLevelChange(int vipLevel);
	/**@brief 上线时的操作*/
	virtual void LoginProcess();
	/**@brief 获取用于随机物品的商品配置表*/
	virtual void GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs) = 0;
	/**@brief 自动刷新*/
	bool AutoRefreshCommoditys();
	const SSanguoCommodity* const GetAllCommodityData() const;
	const SSanguoCommodity* const GetCommodityData(UINT index) const;
	/**@brief 下次刷新的时间点*/
	DWORD GetNextRefreshTime() const;
	bool BuyCommodity(UINT index);
	/**@brief 一日内手动刷新次数*/
	DWORD RequestRefreshedCount() const;
	bool AddRefreshCount();
	DWORD GetRefreshedCount() const;
	/**@brief 重置手动刷新次数*/
	void ResetRefreshedCount();
	/**@brief 物品可否购买
	@param [in] index 商品索引
	@param [out] cost 购买消耗
	@param [out] currrencyType 购买商品支付所需货币类型
	*/
	bool CanBuyCommodity(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const;
	/**@brief 商店开放等级*/
	DWORD OpenLevel() const;
	/**@brief 获取激活判定事件*/
	std::function<void()>& GetOpenFunc() { return m_fOpenFunc; }
	/**@brief 距离下次刷新剩余的秒数*/
	DWORD GetNextRefreshRemainingSeconds();
	/**@brief 设置刷新信息
	@param autoRefresh [in] 是否为到达时间点的刷新(或者游戏上线判断需要刷新)
	*/
	void SetRefreshInfo(bool autoRefresh = true);
	/**@brief 获取商品数据*/
	SSanguoCommodity* GetCommodityDatas();
protected:
	/**@brief 判断初始化时是否需要刷新*/
	bool JudgementRefresh(const STimeData& time);

	bool IsFristOpen() const;
protected:
	SSanguoMallData* m_pMallData;		///<对应的商店数据//
	vector<int32_t>* m_pVecRefreshClock;	///<刷新钟点//
	DWORD	m_dwCurClockIndex;		///<上次刷新的钟点索引//
	DWORD	m_dwNextClockIndex;		///<下次刷新钟点索引//
	//DWORD	m_dwRefreshNeedSeconds;	///<距离下次刷新的毫秒值//
	DWORD	m_dwLimitLevel;			///<开放限制等级
	std::function<void()> m_fOpenFunc;	///<开启判断事件
};

