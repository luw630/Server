#pragma once
#include "objectservice\include\objectmanager.h"

class CZoneManager :
	public CObjectManager
{
public:
	IObject *GenerateObject(__int32 IID);
};
