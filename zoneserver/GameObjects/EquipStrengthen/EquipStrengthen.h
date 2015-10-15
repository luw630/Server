#ifndef CEQUIPSTRENGTHEN_H
#define CEQUIPSTRENGTHEN_H

#include "..\Networkmodule\ItemTypedef.h"

// �ṩ�����װ��ǿ���Ĳ���
class CEquipStrengthenServer
{
	friend class CItemUser;
private:
	CEquipStrengthenServer();

public:
	static CEquipStrengthenServer& GetInstance()
	{
		static CEquipStrengthenServer instance;
		return instance;
	}	
	~CEquipStrengthenServer();
private:
	// ��ֹ��������Ϳ�����ֵ
	CEquipStrengthenServer(CEquipStrengthenServer &);
	CEquipStrengthenServer& operator=(CEquipStrengthenServer &);

public:
	// ===================װ��ǿ��===================ahj
	// ����
	// ��������װ��
	static bool checkCanDragStrengthenEquip(BYTE &MsgFlag, BYTE byRefineType, const struct SEquipment *pEquip, const struct SItemBaseData *srcData); // ����Ƿ����װ��
	static BOOL handleElemDragStrengthenEquip(CPlayer *pPlayer, const struct SEquipment* pEquip, BYTE byRefineType); // ������װ��
	static BOOL handleElemEquipDecomposition(CPlayer *pPlayer, const struct SEquipment *pEquip, BYTE byRefineType, WORD wEquipPos); // ����װ���ֽ�	
	static BOOL handleElemEquipSmelting(CPlayer *pPlayer, const struct SEquipment *pEquip, BYTE byRefineType, WORD wEquipPos); // ����װ������	
	static BOOL insertMetrialItem2NeedMetrial(struct SANeedMETRIAL *msg, struct MetrialItem *newMetrial, BOOL bMerge = TRUE);// Func:������ϵ���������ṹ��
	static BOOL handleElemNeedMETRIALInfo(CPlayer *pPlayer, const struct SEquipment *pEquip, BYTE byRefineType, struct SANeedMETRIAL &msg); // �����������
	static BOOL handleElemEquipGrade(CPlayer *pPlayer, struct SEquipment *pEquip, BYTE byRefineType, byte byStrengthenExternChoose); // ����װ������	
	static BOOL handleElemRefineStar(CPlayer *pPlayer, struct SEquipment *pEquip, BYTE byRefineType, byte byStrengthenExternChoose); // ����װ��ǿ����Ԫ
	static BOOL handleElemEquipSpiritAttachBody(CPlayer *pPlayer, struct SEquipment *const pEquip, BYTE byRefineType, WORD wEquipPos, WORD wScrollPos); // ����װ������

	static BOOL RecvDragStrengthenEquip(CPlayer *pPlayer, struct SQAddEquip* pMsg);					// ����װ��
	static BOOL RecvDragStrengthenMetrial(CPlayer *pPlayer, struct SQAddMetrial *pMsg);				// �������

	static BOOL RecvBeginRefineGrade(CPlayer *pPlayer, struct SQBeginRefineGrade *pMsg);				// ����
	static BOOL RecvBeginRefineStar(CPlayer *pPlayer, struct SQBeginRefineStar *pMsg);				// ����
	static BOOL RecvBeginReMoveStar(CPlayer *pPlayer, struct SQBeginReMoveStar *pMsg);				// ϴ��	
	static BOOL RecvBeginUpgradeQuality(CPlayer *pPlayer, struct SQStartQuality *pMsg);				// ��Ʒ��
	static BOOL RecvBeginUpgradeLevel(CPlayer *pPlayer, struct SQBeginEquipLevelUpMsg *pMsg);		// װ������

	static BOOL RecvBeginDecomposition(CPlayer *pPlayer, struct SQBeginIdentifyMsg* pMsg);			// װ���ֽ�
	static BOOL RecvBeginShengJie(CPlayer *pPlayer, struct SQBeginIdentifyMsg* pMsg);				// װ�����ף������������Ե����ޣ�
	static BOOL RecvBeginJingLian(CPlayer *pPlayer, struct SQBeginIdentifyMsg *pMsg);				// װ������������װ���ĸ�������ֵ��			
	static BOOL RecvBeginPolished(CPlayer *pPlayer, struct SQBeginIdentifyMsg *pMsg);				// װ��ϴ�������ɸ���������Ŀ�����û�������ĸ�������ֵ��	

	static BOOL IsFullEquipJingLian(const struct SEquipment *equip);// [���װ�������Ƿ�����]
	static BYTE	GetEquipStarNum( const struct SEquipment *equip); // [��ȡװ��������] 

	static BOOL RecvIdentifyExchange(CPlayer *pPlayer, struct SQIdentifyExchangeMsg *pMsg);// [2012-8-24 10-11 gw: +�յ�ϴ����Ľ�����Ϣ]

	static BOOL RecvEndRefine(CPlayer *pPlayer, struct SQEndRefineMsg *pMsg);						// ����ǿ��
};

#endif // CPLAYERSERVICE_H