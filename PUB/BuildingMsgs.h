#pragma once

#include "NetModule.h"

#include "buildingtypedef.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// �����������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SBuildingBaseMsg, SMessage, SMessage::EPRO_SPORT_MESSAGE + 1)
//{{AFX
EPRO_BUILDSTRUCTURE,            // ��ʼ���콨����
EPRO_SYN_BUILDING_INFO          // ͬ�����������Ϣ
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------

DECLARE_MSG( SBuildStructureMsg, SBuildingBaseMsg, SBuildingBaseMsg::EPRO_BUILDSTRUCTURE )
struct SQBuildStructureMsg : public SBuildStructureMsg
{
    WORD index, inverse, x, y;
};

DECLARE_MSG( SSynBuildingMsg, SBuildingBaseMsg, SBuildingBaseMsg::EPRO_SYN_BUILDING_INFO )
struct SASynBuildingMsg : public SSynBuildingMsg 
{
    DWORD gid;
    SBuildingSaveData data;
};
