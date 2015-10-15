#pragma once

#include "Networkmodule\ItemTypedef.h"
#include <vector>
// �������������Ϣ
class SMonsterDropTable
{
public:
	SMonsterDropTable() { memset(this, 0, sizeof(SMonsterDropTable)); }

public:
	WORD	m_MonsterID;				// ����ID
	WORD	m_MoneyType;				// �����������
	DWORD	m_MoneyMount;				// ���������
	WORD	m_MoneyRate;				// ������Ҽ���
	DWORD	m_DropTable[10][4];			// ������Ʒ���飨���10����Ʒ��
};


// �������������Ϣ
class SMonsterDropSpecial
{
public:
	SMonsterDropSpecial() { memset(this, 0, sizeof(SMonsterDropSpecial)); }

public:
	DWORD				m_DropTime;					// ����ʱ����
	std::vector<WORD>	m_vGoodsID;					// ��ƷID
	std::map<WORD,WORD>	m_vGoods;					// ��ƷID����伸��
	std::vector<WORD>	m_vMonsterID;				// ����ID
	DWORD				g_DropCount;				// ȫ����������
	BYTE				m_DropType;					// �������ͱ���
};

///��ҵ���װ�����ʱ�
struct PlayerDropTable
{
	enum
	{
		STATE_DROPITEM_BODY,//��������װ��
		STATE_DROPITEM_BAG,//���䱳��װ��
		STATE_DROPITEM_MONEY,//�����Ǯ
	};
	WORD m_wDropItem_body;			//����װ���������
	WORD m_wDropItem_bag;			//����װ���������

};