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
	//获取上次刷新钟点的索引//
	for (int i = 0; i <= maxIndex; ++i)
	{
		if ((*m_pVecRefreshClock)[i] == lastRefreshClock)
		{
			dwLastClockIndex = i;
			break;
		}
	}

	bool bTomorrowRefresh = false;
	//当前时间(hour)大于等于最大刷新时间,即为明日刷新//
	if (curClock >= (*m_pVecRefreshClock)[maxIndex])
	{
		m_dwNextClockIndex = 0;
		m_pMallData->m_dwNextRefreshTime = (*m_pVecRefreshClock)[0] + 24;	///<加上24表示到明日刷新//
		bTomorrowRefresh = true;
	}
	else
	{
		//判断下次刷新的时间点//
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
	//当前登入的时间与上次刷新时间大于一天 则表示当前需要刷新(商城每日至少刷新一次)购买刷新的次数归0
	if (dayInterval >= 1)
	{
		m_pMallData->m_dwRefreshedCount = 0;
		return true;
	}

	/// 判断是否重置刷新次数
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

	//精度到天的两个时间点的判断 大于0表示两个时间点至少隔天了//
	int result = CompareTimeOfYear(tsLogin, tsLastRefresh);
	///<理论上的上次刷新索引
	m_dwCurClockIndex = m_dwNextClockIndex == 0 ? maxIndex : m_dwNextClockIndex - 1;
	if (result > 0)
	{
		//每日刷新时间点大于1个//
		if (maxIndex > 1)
		{
			//上次刷新的时间小于最晚刷新时间,则需要刷新//
			if (dwLastClockIndex < maxIndex)
				return true;
			//当前时间大于最早刷新时间,则需要刷新//
			if (curClock >= (*m_pVecRefreshClock)[0])
				return true;
		}
		else //每日只需要刷新一次//
		{
			//上次刷新的时间小于刷新点时间或者当前时间大于刷新点时间,则需要刷新//
			if (lastRefreshClock < (*m_pVecRefreshClock)[0] || curClock >= (*m_pVecRefreshClock)[0])
				return true;
		}
	}
	else if (result == 0) //上次刷新时间与当前登录时间在同一天//
	{
		//每日刷新时间点大于1个//
		if (maxIndex > 1)
		{
			//上次游戏刷新时间点小于理论上的上次刷新时间点则刷新//
			if (dwLastClockIndex < m_dwCurClockIndex)
				return true;
		}
		else
		{
			//当前时间超过或等于刷新时间点,则需要刷新//
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
		rfalse("三国商品数据未初始化!");
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
	///<每天只刷新一次
	if (maxIndex == 0)
	{
		m_dwCurClockIndex = 0;
		m_dwNextClockIndex = 0;
		///<如果是自动刷新
		if (autoRefresh)
		{
			///<距离下次刷新为一天(86400s)
			///<下次刷新时间+24,代表是第二天的这个钟点刷新
			//m_dwRefreshNeedSeconds = 86400;
			m_pMallData->m_dwRefreshRemainingSeconds = 86400;
			m_pMallData->m_dwNextRefreshTime = (*m_pVecRefreshClock)[m_dwNextClockIndex] + 24;
			ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(tmNow, (*m_pVecRefreshClock)[0]), m_pMallData->m_LastRefreshTime);
		}
		else
		{
			nextClock = (*m_pVecRefreshClock)[0];
			///<如果当前时间大于下次刷新时间,则需要第二天这个时间点自动刷新
			if (tmNow.tm_hour >= nextClock)
			{
				///<下次刷新所需秒数 = 当前时间距离0点的秒数 + 0点距离第二天刷新钟点的秒数
				//m_dwRefreshNeedSeconds = (23 - tmNow.tm_hour) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec) + nextClock * 3600;
				m_pMallData->m_dwRefreshRemainingSeconds = (23 - tmNow.tm_hour) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec) + nextClock * 3600;
				m_pMallData->m_dwNextRefreshTime = (*m_pVecRefreshClock)[m_dwNextClockIndex] + 24;
				ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(tmNow, (*m_pVecRefreshClock)[0]), m_pMallData->m_LastRefreshTime);
			}
			else ///<当前时间小于下次刷新时间,则在当日内刷新
			{
				///<下次刷新所需秒数 = 当前时间距离0点的秒数
				//m_dwRefreshNeedSeconds = (nextClock - tmNow.tm_hour - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwRefreshRemainingSeconds = (nextClock - tmNow.tm_hour - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwNextRefreshTime = (*m_pVecRefreshClock)[m_dwNextClockIndex];
				ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(GobackPreDay(tmNow), (*m_pVecRefreshClock)[0]), m_pMallData->m_LastRefreshTime);
			}
		}
	}
	else ///<每天不知刷新一次
	{
		///<如果是自动刷新
		if (autoRefresh)
		{
			///<下一次刷新钟点的索引
			m_dwNextClockIndex = m_dwCurClockIndex == maxIndex ? 0 : m_dwCurClockIndex + 1;
			///<下一次刷新的钟点
			nextClock = (*m_pVecRefreshClock)[m_dwNextClockIndex];
			///<当前已刷新过的钟点(上一次刷新钟点)
			DWORD curClock = (*m_pVecRefreshClock)[m_dwCurClockIndex];
			ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(tmNow, curClock), m_pMallData->m_LastRefreshTime);
			///<如果下次刷新索引为0,则表示需要到第二天刷新
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
		else ///<非自动刷新(登录游戏时会判断需要需要刷新)
		{
			///<当前时间大于最大刷新时间,则第二日刷新
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
			else if (tmNow.tm_hour < (*m_pVecRefreshClock)[0]) ///<当前时间小于最小刷新时间
			{
				m_dwCurClockIndex = maxIndex;
				m_dwNextClockIndex = 0;
				nextClock = (*m_pVecRefreshClock)[0];
				//m_dwRefreshNeedSeconds = (nextClock - tmNow.tm_hour - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwRefreshRemainingSeconds = (nextClock - tmNow.tm_hour - 1) * 3600 + (59 - tmNow.tm_min) * 60 + (60 - tmNow.tm_sec);
				m_pMallData->m_dwNextRefreshTime = nextClock;
				ConverTool::ConvertInt64ToBytes(RoundTimeOfHour(GobackPreDay(tmNow), (*m_pVecRefreshClock)[m_dwCurClockIndex]), m_pMallData->m_LastRefreshTime);
			}
			else ///<当前时间再所有刷新时间区间内
			{
				///<遍历刷新时间列表,第一个大于当前时间的则是下次刷新时间
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
	/// 如果获取不到商品信息获取商品已售罄 则不可购买
	if (nullptr == pCommodityData || pCommodityData->m_bSoldOut)
		return false;

	/// 获取商品货币类型及购买消耗
	if (!GetCommodityRechargeTypeAddCost(index, currencyType, cost))
		return false;

	/// 检查是否有足够的货币可购买商品
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
		/// 颜色为4 代表是紫装,类型为3代表此道具为将魂碎片 颜色大于1代表此将魂非一星将魂
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
		/// 品质大于2 代表此装备为 蓝装或紫装
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
