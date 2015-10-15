// ----------------------------------
// ------- author : LiuWeiWei -------
// ------- desc : 英雄缘分查询器 -----
// ------- date : 06/27/2015 --------
// ----------------------------------
#pragma once
#include "../PUB/Singleton.h"
#include "stdint.h"
#include <unordered_map>
#include <vector>
using namespace std;
class CHeroDataManager;
struct FateConfig;

class CHeroFateInterrogator : public CSingleton<CHeroFateInterrogator>
{
public:
	CHeroFateInterrogator();
	~CHeroFateInterrogator();
	/**@brief 获取武将相关缘分武将列表
	@param [in] heroID 待查找武将ID
	*/
	const vector<int32_t>* GetRelatedHeroes(int32_t heroID);
	/**@brief 获取武将已激活的缘分配置
	@param [in] heroID 待查找的武将ID
	@param [in] pHeroDataManager 武将数据管理类
	@param [out] vecFateConfigs 将已激活的缘分配置指针压入到此列表
	*/
	void GetActivatedFateConfigs(IN int32_t heroID, IN const CHeroDataManager* pHeroDataMgr, OUT vector<FateConfig*>& vecFateConfigs);
private:
	unordered_map<int32_t, vector<FateConfig>>& m_FateConfigMap;	///< 保存ID未Key值的英雄的所有缘分配置
	unordered_map<int32_t, vector<int32_t>> m_RelatedHeroMap;	///< 保存与ID为key值的英雄有缘分羁绊的武将列表
};

