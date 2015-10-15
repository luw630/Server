//-----------ChaseThiefTollgate.h-------------
//	----Author : LiuWeiWei----
//  ----Date : 04/10/2015----
//	----Desc : 追击盗贼关卡, 不需要记录挑战数据,默认激活
//--------------------------------------------

#pragma once
#include <unordered_map>
#include <memory>
struct BattleLevelConfig;
class CBaseDataManager;
class CTollgateDrop;
enum CheckResult;

class CChaseThiefTollgate
{
public:
	CChaseThiefTollgate();
	~CChaseThiefTollgate();
	void Init(const BattleLevelConfig* config);
	/// @brief 开始挑战关卡,计算掉落
	void Action();
	/// @brief 关卡结算
	void EarningClear(CBaseDataManager* pBaseDataManager);
	///@brief 获取当前关卡掉落的普通物品信息
	const unordered_map<int, int>* GetCommonDropOutItemList();
	///@brief 获取当前关卡Boss掉落的物品信息
	const unordered_map<int, int>* GetBossDropItemList();
	///@brief 限制等级
	int AstrictLevel() const;
	int TollgateID() { return m_iTollgateID; };
private:
	///@brief 添加挑战成功后会收获的物品,不支持加负数个数的Item
	void AddEarningItem(int itemID, int itemNum);

	int m_iTollgateID;
	int m_iAstrictLevel;
	int m_iTeamExp;     //战队经验//
	int m_iGeneralsExp; //武将经验//
	//int m_iGloryValue; //荣誉值//
	//int m_iSalaryValue; //俸禄值//
	int m_iMoney;
	unordered_map<int, int> m_listItem;///收获的物品列表，key值为itemID，value值为itemID对应的个数
	unique_ptr<CTollgateDrop> m_pTollgateDrop;
};

