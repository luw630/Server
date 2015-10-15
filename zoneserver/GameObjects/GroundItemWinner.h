#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>
#include <vector>

class CItem;
class CPlayer;
class CSingleItem;

#define CI_MAXPLAYER (5)

// ÿһ��ҡɫ����Ŀ
struct ChangeItem
{
	ChangeItem() { memset(this, 0, sizeof(*this)); }

	CItem	*m_pItem;					// ����
	CSingleItem	*m_pSingleItem;					// ����
	CPlayer	*m_Player[CI_MAXPLAYER];	// ��������
	WORD	m_MaxPoint[CI_MAXPLAYER];	// ������ҵĵ���
	BYTE	m_Index;					// �����е�����
	BYTE	m_InitPeople;				// ����μӵ�����
	DWORD	m_StartTime;				// ��ʼʱ��
};

// ����ҡ��ϵͳ
class CGroundItemWinner
{
private:
	CGroundItemWinner() {}

public:
	static CGroundItemWinner& GetInstance()
	{
		static CGroundItemWinner instance;
		return instance;
	}

	BOOL AddChanceItem(std::vector<CPlayer *> &playerList, CItem *pItem, BYTE index);		// ����һ��ҡ��
	BOOL AddChanceItem(std::vector<CPlayer *> &playerList, CSingleItem *pItem);		// ����һ��ҡ��

	BOOL AddDummyChance(std::vector<CPlayer *> &playerList, CItem *pItem, BYTE index);		// ����һ���Ϻ�
	BOOL AddDummyChance(std::vector<CPlayer *> &playerList, CSingleItem *pItem);		// ����һ���Ϻ�

	BOOL StartTakeChance(CPlayer *player, DWORD index, WORD playerIndex, bool IsGiveup);	// ��Ҳ���

	BOOL PlayerLogOut(CPlayer *player, DWORD index, WORD playerIndex);	// ���������
	BOOL GroundItemGone(CItem *pItem);									// ������ʧ��
	BOOL GroundItemGone(CSingleItem *pItem);									// ������ʧ��

	BOOL ChoiceTimeOut(DWORD index, ChangeItem *pChance);				// ҡ�ų�ʱ��
	BOOL FinishTakeChance(ChangeItem *pChance, DWORD index);			// ҡ�Ž���

	BOOL Run();

private:
	typedef std::hash_map<DWORD, ChangeItem *> ChanceItems;

	ChanceItems m_ChanceItem;

private:
	static DWORD counter;
};

#pragma warning(pop)