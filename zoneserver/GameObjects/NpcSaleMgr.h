#pragma once

// NPC销售管理系统，对现有的NPC销售做了优化（空间，时间），将货物装载及更新移到服务器启动的时候
// 避免在运行期的大量运算。并且和NPC系统隔离开来，以便于以后的扩展。

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>
struct NpcGoods
{
	static const WORD ItemCount = 64;

	DWORD	m_UpdateTime;			// 用于以后优化
	DWORD	m_Goods[ItemCount*2];	// 物品栏，最多支持64种
	WORD	m_GoodCount;			// 货物类别总数

	NpcGoods() { memset(this, 0, sizeof(*this)); }
};

class CNpcSaleService
{
	typedef std::hash_map<WORD, NpcGoods*> NpcGoodInfo;

private:
	CNpcSaleService(){}

public:
	static CNpcSaleService& GetInstance()
	{
		static CNpcSaleService instance;
		return instance;
	}

	void ReSetInfo();
	void LoadItemInfo(WORD npcIndexID, DWORD itemID, WORD itemCount);
	NpcGoods* GetSaleInfo(WORD npcIndexID);

	~CNpcSaleService();

private:
	CNpcSaleService(CNpcSaleService &);
	CNpcSaleService& operator=(CNpcSaleService);

private:
	NpcGoodInfo	m_NpcGoodInfo;			// 道具属性
};
struct FollowShopStruct
{
	DWORD m_Itemlist[120];
	WORD  m_Num;
	FollowShopStruct():m_Num(0)
	{
		memset(&m_Itemlist[0],0,sizeof(DWORD) * 120);
		
	}
};
class CFollowShopService
{
	typedef std::map<long,FollowShopStruct> FollowInfoMap;
private:
	CFollowShopService()
	{
		if (m_FollowShopInfo.size() != 0)
		{
			m_FollowShopInfo.clear();
		}
		memset(&m_name[0][0], 0 ,sizeof(m_name));
		m_nameNum = 0;
	}
public:
	static CFollowShopService& GetInstance()
	{
		static CFollowShopService instance;
		return instance;
	}
	char  m_name[8][10];
	WORD  m_nameNum;
	FollowInfoMap m_FollowShopInfo;
	void ClearFollowShop();
	void LoadItemInfo(WORD Index,DWORD ItemID);
	void LoadFollowMenuInfo(WORD Index,std::string Menuename);
	void PrintfInfo();
	bool GetFollowShopList(WORD Index,DWORD *itemList,WORD & num,char m_name[8][10],WORD & nameNum);
	bool IsExitFollowItemID(WORD Index,DWORD ItemID);
	FollowShopStruct * FindFollowStruct(long Index);
};
struct ProtectInfoStruct
{
	DWORD ItemID;
	BYTE MoneyType;
	long Money;
};
class CProtectInfo
{
typedef std::map<long,ProtectInfoStruct> ProtectInfoMap;
public:
	static CProtectInfo& GetInstance()
	{
		static CProtectInfo instance;
		return instance;
	}
	ProtectInfoMap m_ProtectInfoMap;
	void LoadProtectInfo(WORD Index,DWORD ItemID,BYTE MoneyType,long Money);
	ProtectInfoStruct * FindProtectInfoStruct(long Index); 
};
#pragma warning(pop)