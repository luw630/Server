#pragma once
#include <unordered_map>
#include <list>
#include <functional>
#include "stdint.h"
class CBaseDataManager;
struct SSanguoItem;
enum SSanguoItemType;
struct ItemConfig;
using namespace std;

class CStorageManager
{
public:
	explicit CStorageManager(int playerSID);
	~CStorageManager();
	bool InitItemData(SSanguoItem* pData);
	//virtual bool ReleaseDataMgr();

	/**@brief 添加物品
	@param itemID [in] 物品ID
	@param num [in] 添加个数
	@return 成功返回true 反之false
	*/
	bool AddItem(const int32_t itemID, const int32_t num = 1);

	/**@brief 出售物品
	@param itemID [in] 物品ID
	@param num [in] 出售个数
	@return 成功返回true 反则false
	*/
	bool SellItem(const int32_t itemID, const int32_t num, BYTE& currencyType, DWORD& dwSellCount, DWORD& dwProperty);

	///@brief 分解一组指定的物品成魂点用
	///@param itemArrayNum 这一组物品的个数(NOTE：不是itemIDNumList数组的长度，是itemIDNumList数组的长度的一半)
	///@param itemIDList 这一组物品的ID和物品个数组成的数组
	///@param resolvedSoulPoints [out] 返回分解指定的物品得到的将魂数量
	bool GetResolvedPoints(int32_t itemArrayNum, const int32_t itemIDNumList[], OUT int& resolvedSoulPoints);

	/**@brief 获得经验药水能得到的经验值
	@param itemID [in] 物品ID
	@param num [out] 传进来看能否使用这么多个数，判断完后返回实际能使用的个数
	@param exp [out] 返回的经验值
	@return 成功返回true 反则false
	*/
	bool GetExpPotionAbility(const int32_t itemID, OUT int32_t& num, OUT int32_t& exp);

	/**@brief 穿戴装备
	@param ietmID [in] 物品ID
	@return 拥有装备则移除并返回true 反则发生false
	*/
	bool AttachEquipment(const int32_t equipID);

	/**@brief 检查扫荡券的个数是否足够
	@param itemID [in] 物品ID
	@param num [in] 检查的个数
	@return 没有该物品或物品数量不足返回false 否则返回true
	*/
	bool CheckSweepTicket(int32_t itemID, int32_t num = 1);

	/**@brief 使用物品
	@param itemID [in] 物品ID
	@param num [in] 使用个数
	@return 没有该物品或物品数量不足返回false 否则返回true
	*/
	bool TryToRemoveItem(const int32_t itemID, const int32_t num = 1);

	/**@brief 使用背包中的消耗品
	@param itemID [in] 物品ID
	@param num [in] 使用个数
	@return 没有该物品或物品数量不足返回false 否则返回true
	*/
	bool UseConsumable(int32_t itemID, int32_t num = 1);

	///@brief 检测某一个物品是否存在或者足够
	bool ItemExistOrEnough(int32_t itemID, int32_t num = 1);

	///@brief 获取指定物品在背包中的数量
	///@return 成功返回对应的个数，失败返回0
	int GetItemNum(int32_t itemID);
	///@brief 绑定添加物品事件
	void BindAddItemFunction(std::function<void(int)>& func);
	///移除添加物品事件
	void RemoveAddItemFunction() { m_funcAddItem = nullptr; }
public:
	static SSanguoItemType ParseItemType(const ItemConfig* config);
private:
	/**@brief 移除物品
	@param itemID [in] 物品ID
	@param num [in] 移除个数
	@return 返回真实删除的个数
	*/
	int32_t _RemoveItem(const int32_t itemID, const int32_t num = 1);
	void _Init();
private:
	int m_iCurPlayerSID; ///<当前玩家的ID
	SSanguoItem* m_pItemDatas;				//<物品数组指针//
	unordered_map<int32_t, int32_t> m_mapItemIndex;	//<保存拥有物品的信息 key值为物品ID value为物品数组索引//
	list<int32_t> m_listFreeIndex;			//<保存未被使用的物品数组索引,NOTE:将来如果有要求优先填充“背包”数组前面的空间，则改成set<int>//
	std::function<void(int)> m_funcAddItem;	//<添加物品绑定事件
};

