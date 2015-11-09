#pragma once

#include "BaseObject.h"

const __int32 IID_AREA = 0x110304ce; 

#define _AreaW	400
#define _AreaH		400

class CRegion;

// AddObject用来添加角色等存在AI的东西，不过也可以考虑一下动态改变场景
class CArea : public IObject
{
public:
	int AddObject(LPIObject pChild);
	int DelObject(LPIObject pChild);
	void OnClose();

public:
	void SendMe(struct SMessage *pMsg, DWORD dwSize, DNID dnidExcept);	// 将消息广播到本区块 pExceptClient除外
	void SendAdj(struct SMessage *pMsg, DWORD dwSize, DNID dnidExcept);	// 将消息广播到本区块与相邻区块
	void SendAdjWithDirection(struct SMessage *pMsg, DWORD dwSize, DNID dnidExcept, CArea *lpPrevArea);

	void SendMyObjects(DNID dnidClient, DNID dnidExcept);				// 将对象广播到本区块 pExceptClient除外
	void SendAdjObjects(DNID dnidClient, DNID dnidExcept);				// 将对象广播到本区块与相邻区块
	void SendAdjObjectsWithDirection(DNID dnidClient, DNID dnidExcept, CArea *lpPrevArea);

	void SendMyState(DNID dnidClient, DNID dnidExcept);					// 将状态广播到本区块 pExceptClient除外
	void SendAdjState(DNID dnidClient, DNID dnidExcept);				// 将状态广播到本区块与相邻区块
private:	
	void SendDel(DNID dnidMe); // 本区块的删除处理	
	void SendDelObjectsWithDirection(DNID dnidMe, CArea *lpNewArea); // 向玩家发送  删除离开的同步区域的所有对象 的消息	
	void SendDelWithDirection(struct SMessage *pMsg, DWORD dwSize, DNID dnidMe, CArea *lpNewArea); // 向离开的同步区域的所有玩家 发送删除对象的消息
	
protected:
	void OnRun(void);
	int OnCreate(_W64 long pParameter);

public:
	struct SInitParam
	{
		WORD	x;
		WORD	y;
		CRegion *pParentRegion;
	};

public:
	WORD	m_X;							// 本区块在整个场景中的矩阵位置
	WORD	m_Y;
	CRegion *m_ParentRegion;
    

	check_list<LPIObject> m_PlayerList;		// 玩家的列表
	check_list<LPIObject> m_NpcList;		// NPC的列表
	check_list<LPIObject> m_MonsterList;	// 怪物的列表
	check_list<LPIObject> m_ItemList;		// 包裹的列表
	check_list<LPIObject> m_FightPetList;	// 侠客的列表
	check_list<LPIObject> m_BuildingList;	// 建筑物的列表
	check_list<LPIObject> m_singleItemList;		// 包裹的列表
	

	static check_list<LPIObject> *tempList;
	static check_list<LPIObject>::iterator tempIter;

public:
    typedef check_list<LPIObject>::iterator iterType;

    template<class _Fn1> 
	inline void for_each(check_list<LPIObject> &targetList, _Fn1 _Func)
	{
        if (tempList)
		{
			rfalse(1, 1, "发现了一个错误的递归叠代逻辑！");
            return;
        }

        iterType _First = targetList.begin();
        iterType _Last  = targetList.end();
        tempList = &targetList;
	    
		for ( ; _First != _Last; )
        {
            tempIter = _First;
		    _Func(*_First);
            if (tempIter != _First)
                _First = tempIter;
            else
                ++ _First;
        }

        tempList = 0;
	}

    template <typename _Callback>
    void for_each(DWORD flags, _Callback &cb)
    {
        if (flags & 1) for_each(m_PlayerList,  cb);
        if (flags & 2) for_each(m_MonsterList, cb);
        if (flags & 4) for_each(m_ItemList,    cb);
        if (flags & 8) for_each(m_NpcList,     cb);
    }

    template <typename _Callback>
    void adjoin_for_each(DWORD flags, _Callback &cb)
    {
        for_each(flags, cb);

        if (m_ParentRegion)
        {
            extern POINT vdir[8];
            for (int i=0; i<8; i++)
            {
                CArea *area = (CArea* )m_ParentRegion->GetArea((WORD)(m_X + vdir[i].x ), (WORD)(m_Y + vdir[i].y))->DynamicCast(IID_AREA);
				if (area) 
					area->for_each(flags, cb);
            }
        }
    }
};
