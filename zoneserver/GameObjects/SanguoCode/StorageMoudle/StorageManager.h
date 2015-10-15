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

	/**@brief �����Ʒ
	@param itemID [in] ��ƷID
	@param num [in] ��Ӹ���
	@return �ɹ�����true ��֮false
	*/
	bool AddItem(const int32_t itemID, const int32_t num = 1);

	/**@brief ������Ʒ
	@param itemID [in] ��ƷID
	@param num [in] ���۸���
	@return �ɹ�����true ����false
	*/
	bool SellItem(const int32_t itemID, const int32_t num, BYTE& currencyType, DWORD& dwSellCount, DWORD& dwProperty);

	///@brief �ֽ�һ��ָ������Ʒ�ɻ����
	///@param itemArrayNum ��һ����Ʒ�ĸ���(NOTE������itemIDNumList����ĳ��ȣ���itemIDNumList����ĳ��ȵ�һ��)
	///@param itemIDList ��һ����Ʒ��ID����Ʒ������ɵ�����
	///@param resolvedSoulPoints [out] ���طֽ�ָ������Ʒ�õ��Ľ�������
	bool GetResolvedPoints(int32_t itemArrayNum, const int32_t itemIDNumList[], OUT int& resolvedSoulPoints);

	/**@brief ��þ���ҩˮ�ܵõ��ľ���ֵ
	@param itemID [in] ��ƷID
	@param num [out] ���������ܷ�ʹ����ô��������ж���󷵻�ʵ����ʹ�õĸ���
	@param exp [out] ���صľ���ֵ
	@return �ɹ�����true ����false
	*/
	bool GetExpPotionAbility(const int32_t itemID, OUT int32_t& num, OUT int32_t& exp);

	/**@brief ����װ��
	@param ietmID [in] ��ƷID
	@return ӵ��װ�����Ƴ�������true ������false
	*/
	bool AttachEquipment(const int32_t equipID);

	/**@brief ���ɨ��ȯ�ĸ����Ƿ��㹻
	@param itemID [in] ��ƷID
	@param num [in] ���ĸ���
	@return û�и���Ʒ����Ʒ�������㷵��false ���򷵻�true
	*/
	bool CheckSweepTicket(int32_t itemID, int32_t num = 1);

	/**@brief ʹ����Ʒ
	@param itemID [in] ��ƷID
	@param num [in] ʹ�ø���
	@return û�и���Ʒ����Ʒ�������㷵��false ���򷵻�true
	*/
	bool TryToRemoveItem(const int32_t itemID, const int32_t num = 1);

	/**@brief ʹ�ñ����е�����Ʒ
	@param itemID [in] ��ƷID
	@param num [in] ʹ�ø���
	@return û�и���Ʒ����Ʒ�������㷵��false ���򷵻�true
	*/
	bool UseConsumable(int32_t itemID, int32_t num = 1);

	///@brief ���ĳһ����Ʒ�Ƿ���ڻ����㹻
	bool ItemExistOrEnough(int32_t itemID, int32_t num = 1);

	///@brief ��ȡָ����Ʒ�ڱ����е�����
	///@return �ɹ����ض�Ӧ�ĸ�����ʧ�ܷ���0
	int GetItemNum(int32_t itemID);
	///@brief �������Ʒ�¼�
	void BindAddItemFunction(std::function<void(int)>& func);
	///�Ƴ������Ʒ�¼�
	void RemoveAddItemFunction() { m_funcAddItem = nullptr; }
public:
	static SSanguoItemType ParseItemType(const ItemConfig* config);
private:
	/**@brief �Ƴ���Ʒ
	@param itemID [in] ��ƷID
	@param num [in] �Ƴ�����
	@return ������ʵɾ���ĸ���
	*/
	int32_t _RemoveItem(const int32_t itemID, const int32_t num = 1);
	void _Init();
private:
	int m_iCurPlayerSID; ///<��ǰ��ҵ�ID
	SSanguoItem* m_pItemDatas;				//<��Ʒ����ָ��//
	unordered_map<int32_t, int32_t> m_mapItemIndex;	//<����ӵ����Ʒ����Ϣ keyֵΪ��ƷID valueΪ��Ʒ��������//
	list<int32_t> m_listFreeIndex;			//<����δ��ʹ�õ���Ʒ��������,NOTE:���������Ҫ��������䡰����������ǰ��Ŀռ䣬��ĳ�set<int>//
	std::function<void(int)> m_funcAddItem;	//<�����Ʒ���¼�
};

