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
		FOOD_SOUP,		//��
		FOOD_MEAT,		//���
		FOOD_VEGETABLE,	//�ز�
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
	const SFightPetRise *GetFightPetRise(BYTE Rare) const;//��ȡ�ɳ�����
	const QWORD GetFightPetLevelExp(WORD Level,BYTE index=1)	const;//��ȡ���������þ���
	bool SetFightPetBaseData(WORD index,BYTE Attri,WORD valuedata);
private:
	void CleanUp();

	// ��ֹ��������Ϳ�����ֵ
	CFightPetService(CFightPetService &);
	CFightPetService& operator=(CFightPetService &);

private:
	FightPetBaseData m_baseData;			// ���͵Ļ�������
	FightPetRise	 m_baseRiseData;			// ���͵ĳɳ�����
	//Foodmenulist	m_foodmenulist;		
};

#pragma warning(pop)