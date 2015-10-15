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
	VarietyShop = 0,		///<�ӻ���/
	ArenaShop = 1,			///<�������̵�/
	ExpeditionShop = 2,		///<Զ���̵�/
	MiracleMerchant = 3,	///<��Ե����/
	GemMerchant = 4,		///<�䱦����/
	LegionShop = 5,			///<�����̵�/
	SoulExchange = 7,		///<����һ��̵�
	WarOfLeagueShop = 8,	///<��ս�̵�/
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
	/**@brief ��ȡ��Ʒ���ѻ������ͺͻ���
	@param [in] idnex ��Ʒ����
	@param [out] currency ������Ʒ���ĵĻ�������
	@param [out] cost ������Ʒ��������
	@return ���Ҳ�����Ʒ���ݻ������ƷID���Ҳ��������ļ�����false
	*/
	virtual bool GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const;
	/**@brief �̳��Ƿ񿪷�*/
	virtual bool IsOpened();
	/**@brief ��ȡ����;��*/
	virtual GoodsWay RechargeWay() const = 0;
	/**@brief VIP�ȼ��ı�ʱ�ĸ��²���*/
	virtual void VipLevelChange(int vipLevel);
	/**@brief ����ʱ�Ĳ���*/
	virtual void LoginProcess();
	/**@brief ��ȡ���������Ʒ����Ʒ���ñ�*/
	virtual void GetCommodityConfigs(vector<CommodityConfig*>& vecConfigs) = 0;
	/**@brief �Զ�ˢ��*/
	bool AutoRefreshCommoditys();
	const SSanguoCommodity* const GetAllCommodityData() const;
	const SSanguoCommodity* const GetCommodityData(UINT index) const;
	/**@brief �´�ˢ�µ�ʱ���*/
	DWORD GetNextRefreshTime() const;
	bool BuyCommodity(UINT index);
	/**@brief һ�����ֶ�ˢ�´���*/
	DWORD RequestRefreshedCount() const;
	bool AddRefreshCount();
	DWORD GetRefreshedCount() const;
	/**@brief �����ֶ�ˢ�´���*/
	void ResetRefreshedCount();
	/**@brief ��Ʒ�ɷ���
	@param [in] index ��Ʒ����
	@param [out] cost ��������
	@param [out] currrencyType ������Ʒ֧�������������
	*/
	bool CanBuyCommodity(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const;
	/**@brief �̵꿪�ŵȼ�*/
	DWORD OpenLevel() const;
	/**@brief ��ȡ�����ж��¼�*/
	std::function<void()>& GetOpenFunc() { return m_fOpenFunc; }
	/**@brief �����´�ˢ��ʣ�������*/
	DWORD GetNextRefreshRemainingSeconds();
	/**@brief ����ˢ����Ϣ
	@param autoRefresh [in] �Ƿ�Ϊ����ʱ����ˢ��(������Ϸ�����ж���Ҫˢ��)
	*/
	void SetRefreshInfo(bool autoRefresh = true);
	/**@brief ��ȡ��Ʒ����*/
	SSanguoCommodity* GetCommodityDatas();
protected:
	/**@brief �жϳ�ʼ��ʱ�Ƿ���Ҫˢ��*/
	bool JudgementRefresh(const STimeData& time);

	bool IsFristOpen() const;
protected:
	SSanguoMallData* m_pMallData;		///<��Ӧ���̵�����//
	vector<int32_t>* m_pVecRefreshClock;	///<ˢ���ӵ�//
	DWORD	m_dwCurClockIndex;		///<�ϴ�ˢ�µ��ӵ�����//
	DWORD	m_dwNextClockIndex;		///<�´�ˢ���ӵ�����//
	//DWORD	m_dwRefreshNeedSeconds;	///<�����´�ˢ�µĺ���ֵ//
	DWORD	m_dwLimitLevel;			///<�������Ƶȼ�
	std::function<void()> m_fOpenFunc;	///<�����ж��¼�
};

