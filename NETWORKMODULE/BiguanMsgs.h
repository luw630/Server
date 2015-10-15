#pragma once

#include "NetModule.h"

#define ITEM_SHUERGUO_ID 8002022			//Êó¶ù¹ûµÀ¾ßID

DECLARE_MSG_MAP(SBiguanMsg, SMessage, SMessage::ERPO_BIGUAN_MESSAGE)
EPRO_BIGUAN_LINGQU,
END_MSG_MAP()

DECLARE_MSG(SBiguanLingquMsg,SBiguanMsg,SBiguanMsg::EPRO_BIGUAN_LINGQU)
struct SQBiguanLingquMsg : public SBiguanLingquMsg
{
	enum Type
	{
		BIGUAN_LINGQU_DOUBLE,
		BIGUAN_LINGQU_SINGLE,
		BIGUAN_LINGQU_GOUMAI
	};
	Type type;
};
struct SABiguanLingquMsg : public SBiguanLingquMsg
{
	enum
	{
		SUCCESS,
		FAILED,
		DOUBLE_LINGQU,
		SINGLE_LINGQU
	};
	BYTE result;
	BYTE type;
	DWORD sp;
	DWORD exp;
};