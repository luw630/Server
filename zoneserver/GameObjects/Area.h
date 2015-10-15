#pragma once

#include "BaseObject.h"

const __int32 IID_AREA = 0x110304ce; 

#define _AreaW	400
#define _AreaH		400

class CRegion;

// AddObject������ӽ�ɫ�ȴ���AI�Ķ���������Ҳ���Կ���һ�¶�̬�ı䳡��
class CArea : public IObject
{
public:
	int AddObject(LPIObject pChild);
	int DelObject(LPIObject pChild);
	void OnClose();

public:
	void SendMe(struct SMessage *pMsg, DWORD dwSize, DNID dnidExcept);	// ����Ϣ�㲥�������� pExceptClient����
	void SendAdj(struct SMessage *pMsg, DWORD dwSize, DNID dnidExcept);	// ����Ϣ�㲥������������������
	void SendAdjWithDirection(struct SMessage *pMsg, DWORD dwSize, DNID dnidExcept, CArea *lpPrevArea);

	void SendMyObjects(DNID dnidClient, DNID dnidExcept);				// ������㲥�������� pExceptClient����
	void SendAdjObjects(DNID dnidClient, DNID dnidExcept);				// ������㲥������������������
	void SendAdjObjectsWithDirection(DNID dnidClient, DNID dnidExcept, CArea *lpPrevArea);

	void SendMyState(DNID dnidClient, DNID dnidExcept);					// ��״̬�㲥�������� pExceptClient����
	void SendAdjState(DNID dnidClient, DNID dnidExcept);				// ��״̬�㲥������������������
private:	
	void SendDel(DNID dnidMe); // �������ɾ������	
	void SendDelObjectsWithDirection(DNID dnidMe, CArea *lpNewArea); // ����ҷ���  ɾ���뿪��ͬ����������ж��� ����Ϣ	
	void SendDelWithDirection(struct SMessage *pMsg, DWORD dwSize, DNID dnidMe, CArea *lpNewArea); // ���뿪��ͬ�������������� ����ɾ���������Ϣ
	
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
	WORD	m_X;							// �����������������еľ���λ��
	WORD	m_Y;
	CRegion *m_ParentRegion;
    

	check_list<LPIObject> m_PlayerList;		// ��ҵ��б�
	check_list<LPIObject> m_NpcList;		// NPC���б�
	check_list<LPIObject> m_MonsterList;	// ������б�
	check_list<LPIObject> m_ItemList;		// �������б�
	check_list<LPIObject> m_FightPetList;	// ���͵��б�
	check_list<LPIObject> m_BuildingList;	// ��������б�
	check_list<LPIObject> m_singleItemList;		// �������б�
	

	static check_list<LPIObject> *tempList;
	static check_list<LPIObject>::iterator tempIter;

public:
    typedef check_list<LPIObject>::iterator iterType;

    template<class _Fn1> 
	inline void for_each(check_list<LPIObject> &targetList, _Fn1 _Func)
	{
        if (tempList)
		{
			rfalse(1, 1, "������һ������ĵݹ�����߼���");
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
