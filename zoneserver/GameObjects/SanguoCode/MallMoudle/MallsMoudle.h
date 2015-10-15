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
	/**@brief �̳��Զ�ˢ�º��µ���Ʒ�����ͻ���*/
	void SendAutoRefreshMsgToClient(CMallManager* pMallMgr);
	/**@brief ����̳���Ʒ*/
	bool RandomCommoditys(CMallManager* pMallMgr);
private:
	/**@brief ������Ʒ��Ϣ����*/
	void _ProcessBuyCommodityMsg(CMallManager* pMallMgr, SBuyCommodityRequest* pMsg);
	/**@brief �ֶ�ˢ����Ʒ��Ϣ����*/
	void _ProcessRefreshCommodityMsg(CMallManager* pMallMgr, SRefreshCommodityRequest* pMsg);
	/**@brief ��Ӧ�̳�ˢ����Ʒ����*/
	int _GetRefreshCost(MallType type, DWORD dwRefreshedCount) const;
	/**@brief ��Ӧ�̳��Ƿ��ܹ�ˢ����Ʒ*/
	bool _CanRefresh(CMallManager* pMallMgr, DWORD refreshCost) const;
	/**@brief �����ƷID ����
	@note ����һ�������Ļ����Ͽ��Ǽ������(��ȡ����������б�ʱ�����б��ڵ�IDȫ�������������ƷID ���������ȡʣ�µ��б�)
	@param config [in] ��Ʒ����
	@param dwID [out] ������ƷID
	@param dwCount [out] ������Ʒ����
	@param unusableItemIndex [in/out] ���������ȡ����Ʒ�б�����
	*/
	void _GetCommodityData(const CommodityConfig* config, DWORD& dwID, DWORD& dwCount, vector<int32_t>& unusableItemIndex);
	/**@brief �����ƷID ����
	@note ���5����Χ(1,��Ʒ�б�Ȩ��)ֵ,��ȡ���ֵ������Ϊ������ƷID��Դ�б�
	@param config [in] ��Ʒ����
	@param dwID [out] ������ƷID
	@param dwCount [out] ������Ʒ����
	*/
	void _GetCommodityData(const CommodityConfig* config, DWORD& dwID, DWORD& dwCount);
private:
	CGlobalConfig& m_GlobalConfig;
	vector<CommodityConfig*> m_vecTempCommodityConfig;	///<��ȡ��Ʒ���õ���ʱ����//
	vector<int32_t> m_vecHasCommodity;	///<�˴�ˢ�²��������������Ʒ//
	vector<int32_t> m_vecUsableItem;	///<δ��ˢ���ظ���Ʒ ��ӿ�ˢ���б���ȥ��������Ʒ ���ô˴�ſ������ƷID//
	vector<int32_t> m_vecUnusableItemsIndex;	///<�洢��������б��а�����ȫ�����浽����Ʒ���б�����//
};

