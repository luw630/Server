//-----------ChaseThiefTollgate.h-------------
//	----Author : LiuWeiWei----
//  ----Date : 04/10/2015----
//	----Desc : ׷�������ؿ�, ����Ҫ��¼��ս����,Ĭ�ϼ���
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
	/// @brief ��ʼ��ս�ؿ�,�������
	void Action();
	/// @brief �ؿ�����
	void EarningClear(CBaseDataManager* pBaseDataManager);
	///@brief ��ȡ��ǰ�ؿ��������ͨ��Ʒ��Ϣ
	const unordered_map<int, int>* GetCommonDropOutItemList();
	///@brief ��ȡ��ǰ�ؿ�Boss�������Ʒ��Ϣ
	const unordered_map<int, int>* GetBossDropItemList();
	///@brief ���Ƶȼ�
	int AstrictLevel() const;
	int TollgateID() { return m_iTollgateID; };
private:
	///@brief �����ս�ɹ�����ջ����Ʒ,��֧�ּӸ���������Item
	void AddEarningItem(int itemID, int itemNum);

	int m_iTollgateID;
	int m_iAstrictLevel;
	int m_iTeamExp;     //ս�Ӿ���//
	int m_iGeneralsExp; //�佫����//
	//int m_iGloryValue; //����ֵ//
	//int m_iSalaryValue; //ٺ»ֵ//
	int m_iMoney;
	unordered_map<int, int> m_listItem;///�ջ����Ʒ�б�keyֵΪitemID��valueֵΪitemID��Ӧ�ĸ���
	unique_ptr<CTollgateDrop> m_pTollgateDrop;
};

