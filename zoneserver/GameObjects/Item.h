#pragma once

#include "BaseObject.h"
#include "networkmodule\itemtypedef.h"
#include <vector>
const __int32 IID_ITEM = 0x1168eb14;

#define MakeLifeTime(m) ((m) * 60000)

#include <list>
#include <set>
using namespace std;

class CRegion;

class CItem : public IObject
{
public:
	struct SParameter
    {
		SParameter()
		{
			xTile			= 0;
			yTile			= 0;
			ItemsInPack		= 0;
			PackageModel	= IPM_ITEM;
			MoneyType		= 0;
			Money			= 0;
			ProtectedGID	= 0;
			ProtectTeamID	= 0;
			dwLife			= 0xffffffff;
			memset(pItem, 0, sizeof(pItem));
		}

		WORD	xTile;			// ����X
		WORD	yTile;			// ����Y
		WORD	ItemsInPack;	// �������߸���
		WORD	PackageModel;	// ����ģ��
		WORD	MoneyType;		// ��������
		DWORD	Money;			// ��Ǯ
        DWORD	ProtectedGID;	// ��ұ���		
        DWORD	ProtectTeamID;	// ���鱣��
        DWORD	dwLife;			// ����ʱ�䣬�Ժ�����㣬��ʱ��õ�����ʧ��Ϊ0xffffffffʱ˵���õ�����Զ����
		SItemBase *pItem[g_PackageMaxItem];	// ��������
    };

public:
	static BOOL GenerateNewGroundItem(CRegion *parentRegion, DWORD range, const CItem::SParameter &args, std::vector<SRawItemBuffer> &item, const LogInfo &log);
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
    CItem(void);
    ~CItem(void);

public:
	WORD					m_xTile;						// ����X
	WORD					m_yTile;						// ����Y
	WORD					m_ItemsInPack;					// �����е��߸���
	WORD					m_PackageModel;					// ������ģ�ͣ�������̴ľ�䣬�����䣬�ƽ��䣩
	WORD					m_Remain;						// ������ʣ��ĵ�������������Ǯ��
    DWORD					m_dwGID;						// ������ұ���
    DWORD					m_dwTeamID;						// �������鱣��
	DWORD					m_dwSegTime;					// ���߲���ʱ��
	DWORD					m_dwLifeTime;					// ����ʱ�䣨Ϊ0xffffffffʱ������Զ���ڣ�
	DWORD					m_Money;						// �����еĻ�����
	WORD					m_MoneyType;					// ����Ļ�������
	SRawItemBuffer			m_GroundItem[g_PackageMaxItem];	// ��������
	BYTE					m_PickStatus[g_PackageMaxItem];	// ����ʰȡ����
	BYTE					m_ExPosition[g_PackageMaxItem];	// ԭʼ��λ��
	class CArea				*m_pParentArea;					// ��������
	
	std::set<CPlayer *>		m_Checker;						// ��ǰ�����в鿴��
	std::set<DWORD>			m_ChanceItems;					// ������ҡ�ŵ�����

	bool					m_IsDispatched;					// �Ƿ��ѷַ�

	int						m_IndexMask[g_PackageMaxItem];	// һ����������

	std::vector<std::string> m_strItemlog;
};

inline void CItem::AddChecker(CPlayer *pChecker)
{
	m_Checker.insert(pChecker);
}

inline void CItem::RemoveChecker(CPlayer *pChecker)
{
	m_Checker.erase(pChecker);
}
