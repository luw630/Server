#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

class SFightPetBaseData;
class SFightPetRise;

struct foodmenu
{
	enum
	{
		FOOD_SOUP,		//汤
		FOOD_MEAT,		//荤菜
		FOOD_VEGETABLE,	//素菜
	};
	WORD foodid;
	char	 food_Name[20];
	WORD foodprice;
	BYTE		foodtype;
};

class CFightPetService
{
	typedef std::hash_map<WORD, SFightPetBaseData *> FightPetBaseData;
	typedef std::hash_map<BYTE, SFightPetRise *> FightPetRise;
#ifdef XYD_DC
	typedef std::hash_map<WORD, foodmenu *> Foodmenulist;
#endif
private:
	CFightPetService(){}
	bool LoadPetBaseData();
	bool LoadPetBaseRise();
	bool	Loadfoodmenulist();
public:
	static CFightPetService& GetInstance()
	{
		static CFightPetService instance;
		return instance;
	}

	bool ReLoad();
	bool Init();
	~CFightPetService() { CleanUp(); }

	const SFightPetBaseData *GetFightPetBaseData(WORD index) const;
	const SFightPetRise *GetFightPetRise(BYTE Rare) const;//获取成长数据
	const QWORD GetFightPetLevelExp(WORD Level,BYTE index=1)	const;//获取升级所需用经验
	bool SetFightPetBaseData(WORD index,BYTE Attri,WORD valuedata);
private:
	void CleanUp();

	// 禁止拷贝构造和拷贝赋值
	CFightPetService(CFightPetService &);
	CFightPetService& operator=(CFightPetService &);

private:
	FightPetBaseData m_baseData;			// 侠客的基本数据
	FightPetRise	 m_baseRiseData;			// 侠客的成长数据
	//Foodmenulist	m_foodmenulist;		
};

#pragma warning(pop)