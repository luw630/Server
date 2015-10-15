#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

class CFriendService
{
	struct SFriendData
	{
		WORD		    nlevel;					//等级
		DWORD	    nrewarditemid;		//奖励物品ID			
	    WORD			nOnlineTime;		//在线时间间隔
		WORD			ntotalCount;			//累计次数
		DWORD		nrewardExp;			//在线好友奖励经验
	};
	typedef std::hash_map<WORD, SFriendData *> FriendBaseData;
private:
	CFriendService(){}
	bool LoadFriendBaseData();
public:
	static CFriendService& GetInstance()
	{
		static CFriendService instance;
		return instance;
	}

	bool ReLoad();
	bool Init();
	~CFriendService() { CleanUp(); }

 	const SFriendData *GetFriendBaseData(WORD levle) const; ///获取好友奖励数据
	const DWORD	GetFriendRewarditem(WORD levle) const;//获取等级奖励物品ID
	const DWORD	GetFriendRewardExp(WORD levle) const;//获取等级奖励经验
	const DWORD	GetFriendRewardTime(WORD levle) const;//获取等级奖励时间间隔
	const WORD		GetFriendRewardCount(WORD levle) const;//获取等级奖励时间间隔
// 	const SFightPetRise *GetFightPetRise(BYTE Rare) const;//获取成长数据
// 	const QWORD GetFightPetLevelExp(WORD Level,BYTE index=1)	const;//获取升级所需用经验
private:
	void CleanUp();

	// 禁止拷贝构造和拷贝赋值
	CFriendService(CFriendService &);
	CFriendService& operator=(CFriendService &);

private:
	FriendBaseData m_baseData;			// 好友奖励基本数据
};

#pragma warning(pop)