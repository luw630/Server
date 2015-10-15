// ----- CTollgateDrop.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ������Ĺؿ��ĵ������
// --------------------------------------------------------------------
//   --  ����ĳһ���ؿ��ĵ������
//---------------------------------------------------------------------   
#pragma once
#include <unordered_map>
#include <vector>

struct BattleDropConfig;
struct BossItemMissedCounter;

class CTollgateDrop
{
public:
	CTollgateDrop();
	~CTollgateDrop();

	void Init(const BattleDropConfig* dropConfig);
	///@brief ����������Ʒ
	///@param countData ��Ҵ��BOSS��Ʒ����Ĵ�����Ϣ
	///@param singleDrop �Ƿ�ÿ����Ʒֻ�����һ��
	///@param bMopUpOperation �Ƿ�Ϊɨ������
	void CalculateDropItem(BossItemMissedCounter* countData, bool singleDrop = false, bool bMopUpOperation = false);
	///@brief ��ʱר���ڵ�һ�ιؿ���������������Ӷ���
	void AddBossDropedItem(int itemID, int itemNum);
	///@brief ������һ�ε���Ʒ������Ϣ
	void Reset();
	///@brief ��ȡ��ͨ��Ʒ�����б�
	const unordered_map<int, int>& GetCommonDropItemList();
	///@brief ��ȡBOSS�������Ʒ�б�
	const unordered_map<int, int>& GetBossDropItemList();

protected:
	///@brief ������Ʒ������������
	///@param dropNum Ҫ���伸��
	///@param itemNum itemList��size
	///@param itemList �Ӹ����������һЩ��Ʒ��Ϊ������Ʒ
	///@param stepIndex ������õ��ļ�¼��һ�������������������Ʒ��λ�ã��������������λ��õ��µ��������Ʒ��λ�ò����س�����Ϊ���Ǿ����ܵı���singleDropΪfalse��ʱ��ͬһ����Ʒ�����ε����
	///@param dropedItemNum �Ѿ������˶��ٸ���Ʒ��������˺�ᱻ�޸ĵ���Ӧ�ĸ��������س���
	///@param singleDrop �Ƿ�����ͬһ����Ʒ����dropNumҪ��ĸ���
	void RandomItem(int dropNum, int itemNum, const vector<int>* itemList, OUT int& stepIndex, OUT int& dropedItemNum, bool singleDrop = false);

private:
	int m_iMinNumber;       ///��С������
	int m_iMaxNumber;       ///��������
	int m_iWhiteItemWeight;     ///��ɫ��Ʒ����Ȩ��
	int m_iGreenItemWeight;     ///��ɫ��Ʒ����Ȩ��
	int m_iBlueItemWeight;      ///��ɫ��Ʒ����Ȩ��
	int m_iPurpleItemWeight;    ///��ɫ��Ʒ����Ȩ��
	int m_iBossItem1;
	int m_iBossItemNumber1;
	int m_iBossItemDropPercent1;
	int m_iBossItem2;
	int m_iBossItemNumber2;
	int m_iBossItemDropPercent2;
	int m_iBossItem3;
	int m_iBossItemNumber3;
	int m_iBossItemDropPercent3;
	const vector<int>* m_whiteItemList;
	const vector<int>* m_greenItemList;
	const vector<int>* m_blueItemList;
	const vector<int>* m_purpleItemList;
	const vector<int>* m_propItem;		///<ɨ����������ƷID�б�
	const vector<int>* m_propItemNum;	///<ɨ����������Ʒ��Ӧ�ĸ���
	unordered_map<int, int> m_commonDropItemIdList; ///��ͨ���յ�����ƷID�б�,keyΪ��ƷID��valueΪ��Ʒ����
	unordered_map<int, int> m_bossDropItemIdList;   ///boss���յ�����ƷID�б�,keyΪ��ƷID��valueΪ��Ʒ����
};

