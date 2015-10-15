#ifndef CEQUIPSTRENGTHEN_H
#define CEQUIPSTRENGTHEN_H

#include "..\Networkmodule\ItemTypedef.h"

// 提供对玩家装备强化的操作
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
	// 禁止拷贝构造和拷贝赋值
	CEquipStrengthenServer(CEquipStrengthenServer &);
	CEquipStrengthenServer& operator=(CEquipStrengthenServer &);

public:
	// ===================装备强化===================ahj
	// 精炼
	// 处理拖入装备
	static bool checkCanDragStrengthenEquip(BYTE &MsgFlag, BYTE byRefineType, const struct SEquipment *pEquip, const struct SItemBaseData *srcData); // 检测是否可拖装备
	static BOOL handleElemDragStrengthenEquip(CPlayer *pPlayer, const struct SEquipment* pEquip, BYTE byRefineType); // 处理拖装备
	static BOOL handleElemEquipDecomposition(CPlayer *pPlayer, const struct SEquipment *pEquip, BYTE byRefineType, WORD wEquipPos); // 处理装备分解	
	static BOOL handleElemEquipSmelting(CPlayer *pPlayer, const struct SEquipment *pEquip, BYTE byRefineType, WORD wEquipPos); // 处理装备熔炼	
	static BOOL insertMetrialItem2NeedMetrial(struct SANeedMETRIAL *msg, struct MetrialItem *newMetrial, BOOL bMerge = TRUE);// Func:插入材料到材料需求结构中
	static BOOL handleElemNeedMETRIALInfo(CPlayer *pPlayer, const struct SEquipment *pEquip, BYTE byRefineType, struct SANeedMETRIAL &msg); // 处理材料需求
	static BOOL handleElemEquipGrade(CPlayer *pPlayer, struct SEquipment *pEquip, BYTE byRefineType, byte byStrengthenExternChoose); // 处理装备升阶	
	static BOOL handleElemRefineStar(CPlayer *pPlayer, struct SEquipment *pEquip, BYTE byRefineType, byte byStrengthenExternChoose); // 处理装备强化单元
	static BOOL handleElemEquipSpiritAttachBody(CPlayer *pPlayer, struct SEquipment *const pEquip, BYTE byRefineType, WORD wEquipPos, WORD wScrollPos); // 处理装备熔炼

	static BOOL RecvDragStrengthenEquip(CPlayer *pPlayer, struct SQAddEquip* pMsg);					// 拖入装备
	static BOOL RecvDragStrengthenMetrial(CPlayer *pPlayer, struct SQAddMetrial *pMsg);				// 拖入材料

	static BOOL RecvBeginRefineGrade(CPlayer *pPlayer, struct SQBeginRefineGrade *pMsg);				// 升阶
	static BOOL RecvBeginRefineStar(CPlayer *pPlayer, struct SQBeginRefineStar *pMsg);				// 升星
	static BOOL RecvBeginReMoveStar(CPlayer *pPlayer, struct SQBeginReMoveStar *pMsg);				// 洗星	
	static BOOL RecvBeginUpgradeQuality(CPlayer *pPlayer, struct SQStartQuality *pMsg);				// 升品质
	static BOOL RecvBeginUpgradeLevel(CPlayer *pPlayer, struct SQBeginEquipLevelUpMsg *pMsg);		// 装备升级

	static BOOL RecvBeginDecomposition(CPlayer *pPlayer, struct SQBeginIdentifyMsg* pMsg);			// 装备分解
	static BOOL RecvBeginShengJie(CPlayer *pPlayer, struct SQBeginIdentifyMsg* pMsg);				// 装备升阶（提升附加属性的上限）
	static BOOL RecvBeginJingLian(CPlayer *pPlayer, struct SQBeginIdentifyMsg *pMsg);				// 装备精炼（提升装备的附加属性值）			
	static BOOL RecvBeginPolished(CPlayer *pPlayer, struct SQBeginIdentifyMsg *pMsg);				// 装备洗练（生成附加属性条目，随机没有锁定的附加属性值）	

	static BOOL IsFullEquipJingLian(const struct SEquipment *equip);// [检测装备精炼是否满了]
	static BYTE	GetEquipStarNum( const struct SEquipment *equip); // [获取装备的星数] 

	static BOOL RecvIdentifyExchange(CPlayer *pPlayer, struct SQIdentifyExchangeMsg *pMsg);// [2012-8-24 10-11 gw: +收到洗练后的交换消息]

	static BOOL RecvEndRefine(CPlayer *pPlayer, struct SQEndRefineMsg *pMsg);						// 结束强化
};

#endif // CPLAYERSERVICE_H