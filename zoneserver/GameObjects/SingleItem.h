#pragma once
/********************************************************************
	created:	2012/02/13
	created:	13:2:2012   16:46
	filename: 	e:\GameServer\���������\GameObjects\SingleItem.h
	file path:	e:\GameServer\���������\GameObjects
	file base:	SingleItem
	file ext:	h
	author:		luwei
	
	purpose:	������Ʒ�ĵ��䣬���ڵ�����Ʒֱ�ӵ��䣬���ڵ������
*********************************************************************/
#include "BaseObject.h"
#include "networkmodule\itemtypedef.h"
#include <vector>
const __int32 IID_SINGLEITEM = 0x117d82c3; 

#define MakeLifeTime(m) ((m) * 60000)

#include <list>
#include <set>
using namespace std;

class CRegion;

class CSingleItem : public IObject
{
public:
	struct SParameter
    {
		SParameter()
		{
			xTile			= 0;
			yTile			= 0;
			ItemsNum		= 0;
			PackageModel	= IPM_ITEM;
			ProtectedGID	= 0;
			ProtectTeamID	= 0;
			dwLife			= 0xffffffff;
			ItemsID = 0 ;
			pItem = 0 ;
			MoneyType = 0;
		}

		WORD	xTile;			// ����X
		WORD	yTile;			// ����Y
		WORD	ItemsNum;	// ���߸���
		WORD	PackageModel;	// ����ģ��
 		WORD	MoneyType;		// ��������
// 		DWORD	Money;			// ��Ǯ
        DWORD	ProtectedGID;	// ��ұ���		
        DWORD	ProtectTeamID;	// ���鱣��
        DWORD	dwLife;			// ����ʱ�䣬�Ժ�����㣬��ʱ��õ�����ʧ��Ϊ0xffffffffʱ˵���õ�����Զ����
		SItemBase *pItem;	// ��������
		DWORD	ItemsID;	//����ID
    };

public:
	static BOOL GenerateNewGroundItem(CRegion *parentRegion, DWORD range, const CSingleItem::SParameter &args, SRawItemBuffer &item, const LogInfo &log);
	static void SaveAllLog();
public:
	int AddObject(LPIObject pChild);
	int DelObject(LPIObject pChild);
	void OnClose();

	void RemoveMe();
    struct SAAddGroundItemMsg *GetStateMsg();
	
	void AddChecker(CPlayer *pChecker);
	void RemoveChecker(CPlayer *pChecker);
	void UpdateItemPicked(WORD index);

	// ��������
	BOOL StartDispatch(CPlayer *pChecker);

	

protected:
	void OnRun(void);
	int OnCreate(_W64 long pParameter);

public:
    CSingleItem(void);
    ~CSingleItem(void);

public:
	WORD					m_xTile;						// ����X
	WORD					m_yTile;						// ����Y
	//WORD					m_ItemsInPack;					// �����е��߸���
	WORD					m_ItemsNum;					// ���߸���

	WORD					m_PackageModel;					// ������ģ�ͣ�������̴ľ�䣬�����䣬�ƽ��䣩
	//WORD					m_Remain;						// ������ʣ��ĵ�������������Ǯ��
    DWORD					m_dwGID;						// ������ұ���
    DWORD					m_dwTeamID;						// �������鱣��
	DWORD					m_dwSegTime;					// ���߲���ʱ��
	DWORD					m_dwLifeTime;					// ����ʱ�䣨Ϊ0xffffffffʱ������Զ���ڣ�
	DWORD					m_ItemsID;						//�����ڵ��߱��е�ID��
	//DWORD					m_Money;						// �����еĻ�����
	WORD					m_MoneyType;					// ����Ļ�������
	SRawItemBuffer			m_GroundItem;	// ��������
	BYTE					m_PickStatus;	// ����ʰȡ����
	BYTE					m_ExPosition;	// ԭʼ��λ��
	class CArea				*m_pParentArea;					// ��������
	
	std::set<CPlayer *>		m_Checker;						// ��ǰ�����в鿴��
	std::set<DWORD>			m_ChanceItems;					// ������ҡ�ŵ�����

	bool					m_IsDispatched;					// �Ƿ��ѷַ�

	int						m_IndexMask;	// һ����������

	std::vector<std::string> m_strItemlog;
};

inline void CSingleItem::AddChecker(CPlayer *pChecker)
{
	std::set<CPlayer *>::iterator iter = m_Checker.find(pChecker);
	if (iter == m_Checker.end()){
		m_Checker.insert(pChecker);
	}
}

inline void CSingleItem::RemoveChecker(CPlayer *pChecker)
{
	std::set<CPlayer *>::iterator iter = m_Checker.find(pChecker);
	if (iter != m_Checker.end()){
		m_Checker.erase(iter);
	}
	//m_Checker.erase(pChecker);
}
