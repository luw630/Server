#include "stdafx.h"
#include "MallManager.h"
#include "../../../../NETWORKMODULE/SanguoPlayer.h"
#include "Random.h"
#include "../Common/ConfigManager.h"
#include "../Common/PubTool.h"
#include "../TimerEvent_SG/TimerEvent_SG.h"
#include "../BaseDataManager.h"
#include "MallsMoudle.h"
#include "ScriptManager.h"
#include "extraScriptFunctions/lite_lualibrary.hpp"

CMallManager::CMallManager(CBaseDataManager& baseDataMgr)
	: CExtendedDataManager(baseDataMgr),
	m_pMallData(nullptr),
	m_dwNextClockIndex(0), 
	m_dwCurClockIndex(0),
	//m_dwRefreshNeedSeconds(0),
	m_dwLimitLevel(0),
	m_fOpenFunc(nullptr),
	m_pVecRefreshClock(nullptr)
{

}


CMallManager::~CMallManager()
{
	m_pMallData = nullptr;
}

const SSanguoCommodity* const CMallManager::GetAllCommodityData() const
{
	return m_pMallData->m_arrCommodity;
}

const SSanguoCommodity* const CMallManager::GetCommodityData(UINT index) const
{
	if (index >= MALL_COMMODITY_NUM)
		return nullptr;
	return &m_pMallData->m_arrCommodity[index];
}

bool CMallManager::BuyCommodity(UINT index)
{
	if (index >= MALL_COMMODITY_NUM)
		return false;
	if (m_pMallData->m_arrCommodity[index].m_bSoldOut == TRUE)
		return false;
	m_pMallData->m_arrCommodity[index].m_bSoldOut = TRUE;
	return true;
}

DWORD CMallManager::GetNextRefreshTime() const
{
	return m_pMallData->m_dwNextRefreshTime;
}

DWORD CMallManager::RequestRefreshedCount() const
{
	return m_pMallData->m_dwRefreshedCount;
}

bool CMallManager::JudgementRefresh(const STimeData& time)
{
	if (IsFristOpen())
		return true;

	if (nullptr == m_pVecRefreshClock)
		return false;

	tm tsLogin, tsLastRefresh;
	__time64_t tLogin = *((__time64_t*)time.m_LoginTime);
	_localtime64_s(&tsLogin, &tLogin);
	__time64_t tLastRefresh = *((__time64_t*)m_pMallData->m_LastRefreshTime);
	_localtime64_s(&tsLastRefresh, &tLastRefresh);

	DWORD lastRefreshClock = tsLastRefresh.tm_hour;
	DWORD curClock = tsLogin.tm_hour;
	DWORD dwLastClockIndex = 0;
	int maxIndex = m_pVecRefreshClock->size() - 1;
	//��ȡ�ϴ�ˢ���ӵ������//
	for (int i = 0; i <= maxIndex; ++i)
	{
		if ((*m_pVecRefreshClock)[i] == lastRefreshClock)
		{
			dwLastClockIndex = i;
			break;
		}
	}

	bool bTomorrowRefresh = false;
	//��ǰʱ��(hour)���ڵ������ˢ��ʱ��,��Ϊ����ˢ��//
	if (curClock >= (*m_pVecRefreshClock)[maxIndex])
	{
		m_dwNextClockIndex = 0;
		m_pMallData->m_dwNextRefreshTime = (*m_pVecRefreshClock)[0] + 24;	///<����24��ʾ������ˢ��//
		bTomorrowRefresh = true;
	}
	else
	{
		//�ж��´�ˢ�µ�ʱ���//
		for (int i = 0; i <= maxIndex; ++i)
		{
			if ((*m_pVecRefreshClock)[i] > curClock)
			{
				m_dwNextClockIndex = i;
				m_pMallData->m_dwNextRefreshTime = (*m_pVecRefreshClock)[i];
				break;
			}
		}
	}

	int dayInterval = Days_Distance(tLogin, tLastRefresh);
	//��ǰ�����ʱ�����ϴ�ˢ��ʱ�����һ�� ���ʾ��ǰ��Ҫˢ��(�̳�ÿ������ˢ��һ��)����ˢ�µĴ�����0
	if (dayInterval >= 1)
	{
		m_pMallData->m_dwRefreshedCount = 0;
		return true;
	}

	/// �ж��Ƿ�����ˢ�´���
	__time64_t tLogout = *((__time64_t*)time.m_LogoutTime);
	__time64_t nextRefreshTime = tLogout;
	__time64_t now = _time64(nullptr);
	tm curTime;
	_localtime64_s(&curTime, &now);
	curTime.tm_hour = 5;
	curTime.tm_min = 0;
	curTime.tm_sec = 0;
	nextRefreshTime = _mktime64(&curTime);

	if (RefreshJudgement::JudgeCrossed(tLogout, nextRefreshTime))
	{
		m_pMallData->m_dwRefreshedCount = 0;
	}

	//���ȵ��������ʱ�����ж� ����0��ʾ����ʱ������ٸ�����//
	int result = CompareTimeOfYear(tsLogin, tsLastRefresh);
	///<�����ϵ��ϴ�ˢ������
	m_dwCurClockIndex = m_dwNextClockIndex == 0 ? maxIndex : m_dwNextClockIndex - 1;
	if (result > 0)
	{
		//ÿ��ˢ��ʱ������1��//
		if (maxIndex > 1)
		{
			//�ϴ�ˢ�µ�ʱ��С������ˢ��ʱ��,����Ҫˢ��//
			if (dwLastClockIndex < maxIndex)
				return true;
			//��ǰʱ���������ˢ��ʱ��,����Ҫˢ��//
			if (curClock >= (*m_pVecRefreshClock)[0])
				return true;
		}
		else //ÿ��ֻ��Ҫˢ��һ��//
		{
			//�ϴ�ˢ�µ�ʱ��С��ˢ�µ�ʱ����ߵ�ǰʱ�����ˢ�µ�ʱ��,����Ҫˢ��//
			if (lastRefreshClock < (*m_pVecRefreshClock)[0] || curClock >= (*m_pVecRefreshClock)[0])
				return true;
		}
	}
	else if (result == 0) //�ϴ�ˢ��ʱ���뵱ǰ��¼ʱ����ͬһ��//
	{
		//ÿ��ˢ��ʱ������1��//
		if (maxIndex > 1)
		{
			//�ϴ���Ϸˢ��ʱ���С�������ϵ��ϴ�ˢ��ʱ�����ˢ��//
			if (dwLastClockIndex < m_dwCurClockIndex)
				return true;
		}
		else
		{
			//��ǰʱ�䳬�������ˢ��ʱ���,����Ҫˢ��//
			if (lastRefreshClock < (*m_pVecRefreshClock)[0] && curClock >= (*m_pVecRefreshClock)[0])
				return true;
		}
	}

	return false;
}

bool CMallManager::IsFristOpen() const
{
	if (m_pMallData == nullptr)
	{
		rfalse("������Ʒ����δ��ʼ��!");
		return false;
	}
	return m_pMallData->m_arrCommodity[0].m_dwID == 0;
}

void CMallManager::SetRefreshInfo(bool autoRefresh /* = true */)
{
	if (nullptr == m_pVecRefreshClock)
		return;

	__time64_t now;
	_time64(&now);
	tm tmNow;
	_localtime64_s(&tmNow, &now);
	DWORD nextClock = 0;
	int maxIndex = m_pVecRefreshClock->size() - 1;
	///<ÿ��ֻˢ��һ��
	if (maxIndex == 0)
	{
		m_dwCurClockIndex = 0;
		m_dwNextClockIndex = 0;
		///<������Զ�ˢ��
		if (autoRefresh)
		{
			///<�����´�ˢ��Ϊһ��(86400s)
			///<�´�ˢ��ʱ��+24,�����ǵڶ��������ӵ�ˢ��
			//m_dwRefreshNeedSeconds = 86400;
			m_pMallData->m_dwRefreshRemainingSeconds = 86400;
			m_pMallData->m_dwNextRefreshTime = (*m_pVecRefreshClock)[m_dwNextClockIndex] + 24;
			ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(tmNow, (*m_pVecRefreshClock)[0]), m_pMallData->m_LastRefreshTime);
		}
		else
		{
			nextClock = (*m_pVecRefreshClock)[0];
			///<�����ǰʱ������´�ˢ��ʱ��,����Ҫ�ڶ������ʱ����Զ�ˢ��
			if (tmNow.tm_hour >= nextClock)
			{
				///<�´�ˢ���������� = ��ǰʱ�����0������� + 0�����ڶ���ˢ���ӵ������
				//m_dwRefreshNeedSeconds = (23 - tmNow.tm_hour) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec) + nextClock * 3600;
				m_pMallData->m_dwRefreshRemainingSeconds = (23 - tmNow.tm_hour) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec) + nextClock * 3600;
				m_pMallData->m_dwNextRefreshTime = (*m_pVecRefreshClock)[m_dwNextClockIndex] + 24;
				ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(tmNow, (*m_pVecRefreshClock)[0]), m_pMallData->m_LastRefreshTime);
			}
			else ///<��ǰʱ��С���´�ˢ��ʱ��,���ڵ�����ˢ��
			{
				///<�´�ˢ���������� = ��ǰʱ�����0�������
				//m_dwRefreshNeedSeconds = (nextClock - tmNow.tm_hour - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwRefreshRemainingSeconds = (nextClock - tmNow.tm_hour - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwNextRefreshTime = (*m_pVecRefreshClock)[m_dwNextClockIndex];
				ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(GobackPreDay(tmNow), (*m_pVecRefreshClock)[0]), m_pMallData->m_LastRefreshTime);
			}
		}
	}
	else ///<ÿ�첻֪ˢ��һ��
	{
		///<������Զ�ˢ��
		if (autoRefresh)
		{
			///<��һ��ˢ���ӵ������
			m_dwNextClockIndex = m_dwCurClockIndex == maxIndex ? 0 : m_dwCurClockIndex + 1;
			///<��һ��ˢ�µ��ӵ�
			nextClock = (*m_pVecRefreshClock)[m_dwNextClockIndex];
			///<��ǰ��ˢ�¹����ӵ�(��һ��ˢ���ӵ�)
			DWORD curClock = (*m_pVecRefreshClock)[m_dwCurClockIndex];
			ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(tmNow, curClock), m_pMallData->m_LastRefreshTime);
			///<����´�ˢ������Ϊ0,���ʾ��Ҫ���ڶ���ˢ��
			if (m_dwNextClockIndex == 0)
			{
				//m_dwRefreshNeedSeconds = (23 - curClock) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec) + nextClock * 3600;
				m_pMallData->m_dwRefreshRemainingSeconds = (23 - curClock) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec) + nextClock * 3600;
				m_pMallData->m_dwNextRefreshTime = nextClock + 24;
			}
			else
			{
				//m_dwRefreshNeedSeconds = (nextClock - curClock - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwRefreshRemainingSeconds = (nextClock - curClock - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwNextRefreshTime = nextClock;
			}
		}
		else ///<���Զ�ˢ��(��¼��Ϸʱ���ж���Ҫ��Ҫˢ��)
		{
			///<��ǰʱ��������ˢ��ʱ��,��ڶ���ˢ��
			if (tmNow.tm_hour >= (*m_pVecRefreshClock)[maxIndex])
			{
				m_dwCurClockIndex = maxIndex;
				m_dwNextClockIndex = 0;
				nextClock = (*m_pVecRefreshClock)[0];
				//m_dwRefreshNeedSeconds = (23 - tmNow.tm_hour) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec) + nextClock * 3600;
				m_pMallData->m_dwRefreshRemainingSeconds = (23 - tmNow.tm_hour) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec) + nextClock * 3600;
				m_pMallData->m_dwNextRefreshTime = nextClock + 24;
				ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(tmNow, (*m_pVecRefreshClock)[m_dwCurClockIndex]), m_pMallData->m_LastRefreshTime);
			}
			else if (tmNow.tm_hour < (*m_pVecRefreshClock)[0]) ///<��ǰʱ��С����Сˢ��ʱ��
			{
				m_dwCurClockIndex = maxIndex;
				m_dwNextClockIndex = 0;
				nextClock = (*m_pVecRefreshClock)[0];
				//m_dwRefreshNeedSeconds = (nextClock - tmNow.tm_hour - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwRefreshRemainingSeconds = (nextClock - tmNow.tm_hour - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwNextRefreshTime = nextClock;
				ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(GobackPreDay(tmNow), (*m_pVecRefreshClock)[m_dwCurClockIndex]), m_pMallData->m_LastRefreshTime);
			}
			else ///<��ǰʱ��������ˢ��ʱ��������
			{
				///<����ˢ��ʱ���б�,��һ�����ڵ�ǰʱ��������´�ˢ��ʱ��
				for (int i = 1; i <= maxIndex; ++i)
				{
					if ((*m_pVecRefreshClock)[i] > tmNow.tm_hour)
					{
						m_dwCurClockIndex = i - 1;
						m_dwNextClockIndex = i;
						break;
					}
				}
				nextClock = (*m_pVecRefreshClock)[m_dwNextClockIndex];
				//m_dwRefreshNeedSeconds = (nextClock - tmNow.tm_hour - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwRefreshRemainingSeconds = (nextClock - tmNow.tm_hour - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwNextRefreshTime = nextClock;
				ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(tmNow, (*m_pVecRefreshClock)[m_dwCurClockIndex]), m_pMallData->m_LastRefreshTime);
			}
		}
	}
}

bool CMallManager::AutoRefreshCommoditys()
{
	if (nullptr == m_pVecRefreshClock)
		return false;

	if (!CMallsMoudle::getSingleton()->RandomCommoditys(this))
		return false;
	m_dwCurClockIndex++;
	m_dwCurClockIndex = m_dwCurClockIndex >= (*m_pVecRefreshClock).size() ? 0 : m_dwCurClockIndex;
	SetRefreshInfo(true);
	return true;
}

bool CMallManager::AddRefreshCount()
{
	m_pMallData->m_dwRefreshedCount++;
	return true;
}

DWORD CMallManager::GetRefreshedCount() const
{
	return m_pMallData->m_dwRefreshedCount;
}

void CMallManager::ResetRefreshedCount()
{
	m_pMallData->m_dwRefreshedCount = 0;
}

bool CMallManager::ReleaseDataMgr()
{
	m_pMallData = nullptr;
	return true;
}

bool CMallManager::CanBuyCommodity(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const
{
	auto pCommodityData = GetCommodityData(index);
	/// �����ȡ������Ʒ��Ϣ��ȡ��Ʒ������ �򲻿ɹ���
	if (nullptr == pCommodityData || pCommodityData->m_bSoldOut)
		return false;

	/// ��ȡ��Ʒ�������ͼ���������
	if (!GetCommodityRechargeTypeAddCost(index, currencyType, cost))
		return false;

	/// ����Ƿ����㹻�Ļ��ҿɹ�����Ʒ
	switch (currencyType)
	{
	case SSanguoCurrencyType::Currency_Money:
		return m_pBaseDataMgr.GetMoney() >= cost;
	case SSanguoCurrencyType::Currency_Diamond:
		return m_pBaseDataMgr.GetDiamond() >= cost;
	case SSanguoCurrencyType::Currency_Honor:
		return m_pBaseDataMgr.GetHonor() >= cost;
	case SSanguoCurrencyType::Currency_Exploit:
		return m_pBaseDataMgr.GetExploit() >= cost;
	case SSanguoCurrencyType::Currency_Token:
		{
			lite::lua_variant ret;
			BOOL result = FALSE;
			if (g_Script.PrepareFunction("GetPlayerFactionMoney"))
			{
				g_Script.PushParameter(m_pBaseDataMgr.GetSID());
				result = g_Script.Execute(&ret);
			}
			else
			{
				return false;
			}

			if (!result || ret.dataType == LUA_TNIL)
			{
				rfalse("GetPlayerFactionMoney Failed");
				return false;
			}

			int token = 0;
			try
			{
				token = (int)(ret);
			}
			catch (lite::Xcpt &e)
			{
				rfalse(2, 1, e.GetErrInfo());
				return false;
			}
			return token >= cost;
		}
		break;
	case SSanguoCurrencyType::Currency_SoulPoints:
		return m_pBaseDataMgr.GetGoods(GoodsType::soulPoints) >= cost;
	case SSanguoCurrencyType::Currency_Prestige:
		return m_pBaseDataMgr.GetPrestige() >= cost;
	default:
		break;
	}

	return false;
}

DWORD CMallManager::OpenLevel() const
{
	return m_dwLimitLevel;
}

bool CMallManager::GetCommodityRechargeTypeAddCost(UINT index, OUT SSanguoCurrencyType& currencyType, OUT DWORD& cost) const
{
	auto pCommodityData = GetCommodityData(index);
	if (nullptr == pCommodityData)
		return false;

	auto itemConfig = CConfigManager::getSingleton()->GetItemConfig(pCommodityData->m_dwID);
	if (nullptr != itemConfig)
	{
		/// ��ɫΪ4 ��������װ,����Ϊ3����˵���Ϊ������Ƭ ��ɫ����1����˽����һ�ǽ���
		if (itemConfig->Quality == 4 || (itemConfig->Type == 3 && itemConfig->Quality > 1))
		{
			currencyType = SSanguoCurrencyType::Currency_Diamond;
			cost = pCommodityData->m_dwCount * itemConfig->BuyDiamondPrice;
			return true;
		}
		
		currencyType = SSanguoCurrencyType::Currency_Money;
		cost = pCommodityData->m_dwCount * itemConfig->BuyGoldPrice;
		return true;
	}

	auto equipConfig = CConfigManager::getSingleton()->GetEquipment(pCommodityData->m_dwID);
	if (nullptr != equipConfig)
	{
		/// Ʒ�ʴ���2 �����װ��Ϊ ��װ����װ
		if (equipConfig->Quality > 2)
		{
			currencyType = SSanguoCurrencyType::Currency_Diamond;
			cost = pCommodityData->m_dwCount * equipConfig->buyDiamondPrice;
			return true;
		}

		currencyType = SSanguoCurrencyType::Currency_Money;
		cost = pCommodityData->m_dwCount * equipConfig->buyGoldPrice;
		return true;
	}

	return false;
}

bool CMallManager::IsOpened()
{
	return m_dwLimitLevel <= m_pBaseDataMgr.GetPlayerLevel();
}

DWORD CMallManager::GetNextRefreshRemainingSeconds()
{
	return m_pMallData->m_dwRefreshRemainingSeconds;
}

void CMallManager::VipLevelChange(int vipLevel)
{

}

void CMallManager::LoginProcess()
{

}

SSanguoCommodity* CMallManager::GetCommodityDatas()
{
	return m_pMallData->m_arrCommodity;
}
