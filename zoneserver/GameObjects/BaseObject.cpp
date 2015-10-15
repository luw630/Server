#include "StdAfx.h"
#include "BaseObject.h"

CObjectManager::CObjectManager(void) {}

CObjectManager::~CObjectManager(void) {}

bool CObjectManager::Run()
{
	for ( std::map<unsigned __int32, LPIObject>::iterator it = objectMap.begin(); 
		it != objectMap.end(); )
	{
		if (it->second && it->second->isValid()) {
            it->second->OnRun();
            it++;
		} else 
            it = objectMap.erase( it );
	}

    return true;
}

bool CObjectManager::Initialize(long lMaxObject)
{
    return true;
}

bool CObjectManager::Destroy()
{
    Clear();
    return true;
}

LPIObject CObjectManager::LocateObject(const __int32 GID) 
{
    LPIObject ret;
    UGE::CObjectManager< unsigned int, LPIObject >::LocateObject( GID, ret );
    return ret;
}

extern DWORD g_GetGuid(__int32 IID);


LPIObject g_SearchInObjectList(check_list<LPIObject>& list, DWORD dwObjectID)
{
    check_list<LPIObject>::iterator iter;
    for (iter = list.begin(); iter != list.end(); ++iter) {
        if ((*iter)->GetGID() == dwObjectID)
            return *iter;
    }

    return LPIObject();
}

struct deleter { void operator()(IObject* p) { p->OnClose(); delete p; } };

LPIObject CObjectManager::GenerateObject(const __int32 IID, unsigned __int32 GID, LPARAM pParameter)
{
	// 创建对象，指定删除器
	LPIObject pObject(CreateObject(IID),  deleter());

    if (pObject != NULL)
    {
        // 初始化该对象的IID
        __int32 *pID = (__int32 *)&pObject->m_IID;
        *pID = IID;

        unsigned __int32 *pGID = (unsigned __int32 *)&pObject->m_GID;

        if (GID == 0) 
        {
			int i = 0;
            for (; i<0xffff; i++) 
            {
				GID = g_GetGuid(IID);
                if (objectMap.find(GID) == objectMap.end())
                    break;
            }

            if (i == 0xffff)
                return LPIObject();
        }

        *pGID = GID;
        pObject->self = pObject;

        if (0 == pObject->OnCreate(pParameter))
        {
            // OnCreate 失败， 返回一个无效的句柄
            return LPIObject();
        }

		if (!objectMap[GID])
			objectMap[GID] = pObject;
    }
	else
	{
		rfalse(4, 1, "LPIObject pObject(CreateObject(IID),  deleter());  fail   !!!!!!!!!!!");
	}
    return pObject;
}

__int32 CObjectManager::TraversalObjects(TRAVERSALFUNC TraversalFunc, LPARAM Param)
{
	for ( std::map<unsigned __int32, LPIObject>::iterator it = objectMap.begin(); 
		it != objectMap.end();
		it ++ )
	{
		IObject *pObject = it->second.get();
		if (pObject == NULL)
		{
			// m_RemoveList.push_back(it->first);
			continue;
		}

		if (IsBadReadPtr(pObject, 4))
		{
			// m_RemoveList.push_back(it->first);
			continue;
		}

		if (!pObject->isValid())
			continue; // m_RemoveList.push_back(it->first);
		else if (TraversalFunc(it->second, Param) == 0)
			break;
	}

	return (__int32)objectMap.size();
}

LPIObject CObjectManager::FindObjects( FINDFUNC FindFunc,LPARAM Param )
{
	LPIObject pobjdest;
	for ( std::map<unsigned __int32, LPIObject>::iterator it = objectMap.begin(); 
		it != objectMap.end();
		it ++ )
	{
		IObject *pObject = it->second.get();
		if (pObject == NULL)
		{
			// m_RemoveList.push_back(it->first);
			continue;
		}

		if (IsBadReadPtr(pObject, 4))
		{
			// m_RemoveList.push_back(it->first);
			continue;
		}

		if (!pObject->isValid())
			continue; // m_RemoveList.push_back(it->first);
		else
		{
			pobjdest = FindFunc(it->second,Param);
			if (pobjdest&&pobjdest->isValid())
			{
				return pobjdest;
			}
		}
			
	}

	return LPIObject();
}
