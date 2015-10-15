#pragma once
#include "objectservice\include\objectmanager.h"
#include "ResourceObject.h"
class CStoreManager :
    public CObjectManager , CResourceObject
{
private:
    IObject *CreateObject(const __int32 IID);

public:
    IObject * GetResource(const char * szAccount ,const char * szPassword);
    bool SaveResource( IObject * SaveObject);

};
