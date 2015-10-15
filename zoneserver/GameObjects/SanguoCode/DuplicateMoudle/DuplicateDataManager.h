// ----- CDuplicateDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ������ĸ��������������Ĺؿ������ݹ���
// --------------------------------------------------------------------
//   --  ���ڽ��ؿ���ص������·ŵ�ÿ��tollgate����ģ��ȽϷ��ӣ�����ʱͳһ���ŵ�����������
//--------------------------------------------------------------------- 
#pragma once
#include <memory>
#include <list>
#include <functional>
#include <unordered_map>
#include "..\ExtendedDataManager.h"
using namespace std;
typedef function<void(int)> FunctionPtr;

enum InstanceType;
///@brief ��ʱ����ˢ�¸�������ս����
struct SFixData;
struct SSanguoTollgate;
struct SSanguoStoryEliteTollgate;
struct SSanguoStoryTollgate;
class TimerEvent_SG;

///@brief ������ͨ��������Ӣ�����еĹؿ������α����㷨
struct BossItemMissedCounter
{
public:
	BossItemMissedCounter()
	{
		m_iRef = 5;
		m_iBossItem1RefCount = 0;
		m_iBossItem2RefCount = 0;
		m_iBossItem3RefCount = 0;
	}

	bool AddItem1RefenceCount()
	{
		++m_iBossItem1RefCount;
		if (m_iBossItem1RefCount >= m_iRef)
		{
			m_iBossItem1RefCount = 0;
			return true;
		}
		else
			return false;
	}

	void ResetItem1Counter()
	{
		m_iBossItem1RefCount = 0;
	}

	bool AddItem2RefenceCount()
	{
		++m_iBossItem2RefCount;
		if (m_iBossItem2RefCount >= m_iRef)
		{
			m_iBossItem2RefCount = 0;
			return true;
		}
		else
			return false;
	}

	void ResetItem2Counter()
	{
		m_iBossItem2RefCount = 0;
	}

	bool AddItem3RefenceCount()
	{
		++m_iBossItem3RefCount;
		if (m_iBossItem3RefCount >= m_iRef)
		{
			m_iBossItem3RefCount = 0;
			return true;
		}
		else
			return false;
	}

	void ResetItem3Counter()
	{
		m_iBossItem3RefCount = 0;
	}

protected:
	int m_iRef; ///<�ж϶��ٴδ��BOSS�������Ʒ�󣬾ͱص���Ӧ��BOSS��Ʒ
private:
	int m_iBossItem1RefCount; ///<�ؿ������е�BOSS������Ʒ��α����㷨Ҫ�õ��ļ�����
	int m_iBossItem2RefCount; ///<�ؿ������е�BOSS������Ʒ��α����㷨Ҫ�õ��ļ�����
	int m_iBossItem3RefCount; ///<�ؿ������е�BOSS������Ʒ��α����㷨Ҫ�õ��ļ�����
};

///@brief �����ͨ���¸���BOSS�������Ʒ�ļ�����
struct NormalBossItemMissedCounter : public BossItemMissedCounter
{
public:
	NormalBossItemMissedCounter()
		:BossItemMissedCounter()
	{
		m_iRef = 5;
	}
};

///@brief �����Ӣ����BOSS�������Ʒ�ļ�����
struct EliteBossItemMissedCounter : public BossItemMissedCounter
{
public:
	EliteBossItemMissedCounter()
		:BossItemMissedCounter()
	{
		m_iRef = 3;
	}
};

class CDuplicateDataManager :
	public CExtendedDataManager
{
public:
	CDuplicateDataManager(CBaseDataManager& baseDataManager);
	virtual ~CDuplicateDataManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief ��ȡ��Ӧ��������ս���Ĵ���
	///@return �ɹ����ض�Ӧ��ֵ��ʧ����ʱ����0
	///@note ������ʱֻ�н���˵�������ؾ�Ѱ�������Ż��и�����ս���������ڣ���ͨ�������������¸���������Ӣ������û�и�����ս���������ƣ�ֻ�йؿ�����ս����������
	int GetDuplicateChallengedTimes(InstanceType type, int duplicateID);
	///@brief ��ȡ��Ӧ�ؿ�����ս��������ս������Ϣ
	///@param tollgateID Ҫ���ҵĹؿ���ID
	///@param starLevel[out] ��ȡ��ǰ�Ĺؿ���ս������û��ս���Ļ���Ϊ0��
	///@param challengeTime[out] ��ȡ��ǰ�ؿ�������ս�Ĵ���
	void GetTollgateData(InstanceType type, int tollgateID, OUT int& starLevel, OUT int& challengeTime);
	///@brief ��ȡ��ͨ�������������¸������ؿ�����ս��������ս�������Ƿ�Ϊ��һ����ս����Ϣ
	///@param tollgateID Ҫ���ҵĹ��¸����ؿ���ID
	///@param bFirstTime[out] ��ǰҪ���ҵĹؿ��Ƿ��ǵ�һ����ս�����ڵ�һ�ιؿ�������ж�
	///@param starLevel[out] ��ȡ��ǰ�Ĺؿ���ս������û��ս���Ļ���Ϊ0��
	///@param challengeTime[out] ��ȡ��ǰ�ؿ�������ս�Ĵ���
	void GetStroyTollgateData(int tollgateID, OUT bool& bFirstTime, OUT int& starLevel, OUT int& challengeTime);
	///@brief ��ȡ��ǰ��ս�佫�ĵľ���ֵ������
	int GetCurHeroExpIncreasement();
	///@brief ���¶�Ӧ�ؿ����͵���ս��������ս��������Ϣ
	///@param type Ҫ���µĹؿ������ͣ���ҪΪ��ͨ�������������¸������ؿ�����Ӣ�����ؿ�������˵�����ؿ����ؾ�Ѱ�������ؿ����ĸ��ؿ�
	///@param duplicateID ����ID
	///@param tollgateID Ҫ���µĹؿ���ID
	///@param starLevel �µ���ս�ķ���
	///@param challengeTime �µ���ս�Ĵ���
	void UpdateTollgateData(InstanceType type, int duplicateID, int tollgateID, int starLevel, int challengeTime);
	///@brief �����������ѡ���Ӣ������
	///@param heroNum ���ѡ�е�Ӣ�۵�����
	///@param heroArray ���ѡ�е�Ӣ�۵�ID����
	void UpdateSelectedHero(int heroNum, const int heroArray[]);
	///@brief �������µ���ͨ�������������¸������ؿ�ID
	void UpdateLatestNormalTollgateID(int tollgateID);
	///@brief �������µľ�Ӣ�����ؿ�ID
	void UpdateLatestEliteTollgateID(int tollgateID);
	///@brief ����ָ���ؿ�����ս������Ϣ,����¼�Ѿ����õĴ���
	bool ResetSpecifyTollgateChallegedTimes(InstanceType instanceType, int tollgateID);
	///@brief ��ȡָ���ؿ������Ѿ����ù��Ĵ���
	///@return �ɹ����ض�Ӧ��ֵ��ʧ�ܷ���-1
	int GetSpecifyTollgateResetedTimes(InstanceType instanceType, int tollgateID);
	///@brief ��ȡ���µ���ͨ�������������¸������ؿ�ID
	int GetLatestNormalTollgateID();
	///@brief ��ȡ���µľ�Ӣ�����ؿ�ID
	int GetLatestEliteTollgateID();
	///@brief ��ȡѡ�����ҵ��б�
	const list<int>& GetSelectedHeroList();
	///@brief ���õ�ǰ��ս�佫�ĵľ���ֵ������
	void SetHeroExpIncreasement(int value);
	///@brief �Ƿ����ø�����������ݵ��ж�
	///@param sendMsgFlag Ϊ0��������Ϣ�� ��0������Ϣ
	void RefreshData(int sendMsg = 0);
	///@brief ��ȡ��ҵĴ����ͨ�����ؿ�BOSS������Ʒ�Ĵ���
	NormalBossItemMissedCounter* GetMissedBossItemDataInStoryDup() { return &m_sBossItemMissedCount; }
	///@brief ��ȡ��ҵĴ����Ӣ�����ؿ�BOSS������Ʒ�Ĵ���
	EliteBossItemMissedCounter* GetMissedBossItemDataInEliteDup() { return &m_eBossItemMissedCount; }
	///@brief ��ȡ�����佫
	const list<int>& GetFightHeroes() const { return m_listSelectedHero; }
private:
	typedef unordered_map<int, int*>::iterator duplicateChallengedTimesItor;
	typedef unordered_map<int, SSanguoTollgate*>::iterator commonTollgateDataItor;
	typedef unordered_map<int, SSanguoStoryTollgate*>::iterator storyTollgateDataItor;
	typedef unordered_map<int, SSanguoStoryEliteTollgate*>::iterator storyEliteTollgateDataItor;

	int m_iLatestNormalTollgateID;	///<���µ���ͨ�������������¸������ؿ�ID
	int m_iLatestEliteTollgateID;	///<���µľ�Ӣ�����ؿ���ID
	int m_iCurHeroExpGain;			///<��ǰ�Ĳ�ս�佫���õľ���ֵ
	NormalBossItemMissedCounter m_sBossItemMissedCount;///<��¼����ͨ���¸������BOSS�������Ʒ�Ĵ���
	EliteBossItemMissedCounter m_eBossItemMissedCount;///<��¼�˾�Ӣ�������BOSS������Ʒ�Ĵ���
	list<int> m_listSelectedHero;///<���ѡ���Ӣ�۵�ID������
	SFixData* m_playerData;///<��ʱ�������Ϊ���ݳ�Ա
	unordered_map<int, int*> m_leagueOfLegendDuplicateChallengedTiems; ///< ����˵������ս������Ϣ
	unordered_map<int, int*> m_treasureHuntingDuplicateChallengedTimes; ///< �ؾ�Ѱ��������ս������Ϣ
	unordered_map<int, SSanguoStoryEliteTollgate*> m_eliteTollgateDatas; ///< ��Ӣ�ؿ�����
	unordered_map<int, SSanguoTollgate*> m_leagueOfLegendTollgateDatas; ///< ����˵�ؿ�����
	unordered_map<int, SSanguoTollgate*> m_treasureHuntingTollgateDatas; ///< �ؾ�Ѱ���ؿ�����
	unordered_map<int, SSanguoStoryTollgate*> m_normalTollgateDatas; ///<��ͨ�ؿ�����
};