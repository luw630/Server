#include "StdAfx.h"
#include "activeobject.h"
#include "player.h"
#include "region.h"
#include "area.h"
#include "pathfind\pathServer.h"
#include "NetWorkModule\ScriptMsgs.h"
#include "NetWorkModule\MoveMsgs.h"
#include "NetWorkModule\UpgradeMsgs.h"
#include "NETWORKMODULE\RegionMsgs.h"
#include "Pet.h"
#include "math.h"
#include "Random.h"
#include <d3dx9math.h>
#include "CSkillService.h"


extern void  MonsterTalkInfo(CMonster *pmonster,LPCSTR info);
#define N_JUMP_ACTION_FRAME 36

#define GETDIR(_dir, _x1, _y1, _x2, _y2)  if (_x2 > _x1) if (_y2 > _y1) _dir = 3; else if (_y2 < _y1) _dir = 1; else _dir = 2; \
    else if (_x2 < _x1) if (_y2 > _y1) _dir = 5; else if (_y2 < _y1) _dir = 7; else _dir = 6; \
    else if (_y2 > _y1) _dir = 4; else if (_y2 < _y1) _dir = 0; else return 0;

POINT vdir[8] =
{ 
    {0, -1},
    {1, -1},
    {1, 0},
    {1, 1},
    {0, 1},
    {-1, 1},
    {-1, 0},
    {-1, -1},
};

#define USEDIR(_dir, _x, _y)  if (dir < 8) _x += (WORD)vdir[_dir].x, _y += (WORD)vdir[_dir].y;

// ����[ԭ�㡢У����]��ȡ���·������ У����Χ = 1
int GetPtLength(WORD sx, WORD sy, WORD dx, WORD dy)
{
	int w = abs(((short)dx) - ((short)sx));
	int h = abs(((short)dy) - ((short)sy));

    return max(w,h);
}

int GetDir(WORD sx, WORD sy, WORD dx, WORD dy)
{
	int w = ((short)dx) - ((short)sx);
	int h = ((short)dy) - ((short)sy);

    if (w == 0)
    {
        if (h > 0) return 4;
        if (h < 0) return 0;
    }

    if (h == 0)
    {
        if (w > 0) return 2;
        if (w < 0) return 6;
    }

    if (w > 0)
    {
        if (h > 0) return 3;
        if (h < 0) return 1;
    }

    if (w < 0)
    {
        if (h > 0) return 5;
        if (h < 0) return 7;
    }

    return 4;
}

static BYTE sHexDirMap[0x88] = {0};

BYTE InitHexDirMap()
{
    sHexDirMap[0x01] = 0;
    sHexDirMap[0x02] = 1;
    sHexDirMap[0x04] = 2;
    sHexDirMap[0x08] = 3;
    sHexDirMap[0x10] = 4;
    sHexDirMap[0x20] = 5;
    sHexDirMap[0x40] = 6;
    sHexDirMap[0x80] = 7;

    return 0xff;
}

// �����ȡһ��δ��ȡ���ķ���pos��һ��ͳ��ֵ
int RandDir(BYTE &pos)
{
    if (sHexDirMap[0] != 0xff)
        sHexDirMap[0] = InitHexDirMap();

    // ���������ȡһ������
    BYTE rPos = 1 << (rand()%8);

    // ��������򱻻�ȡ������˳�����²���
    if (pos & rPos)
    {
        for (int i=0; i<7; i++)
        {
            if (rPos == 0)
                rPos = 1;           // ����Ǵ�����λ������
            else
                rPos = rPos << 1;   // ���û���������λ

            if (!(pos & rPos)) 
                goto _ret;
        }
    }
    else
    {
        //1 2 4 8 16 32 64 128
        //0 1 2 3 4  5  6  7
_ret:
        pos |= rPos;
        return sHexDirMap[rPos];
    }

    pos = 0xff;
    return -1;
}

//////////////////////////////////////////////////////////////////////////
CActiveObject::CActiveObject(void)
{
	// �ö������Ϊ���ʱ�䣬һ��������ѭ��FrameΪ��λ
	m_fActionFrame = 0.0;

	// ��ʼ����ʱ�����õ�ǰ״̬ΪStand״̬
	SetCurActionID(EA_STAND);
	m_byActionBak		= 0xff;

	m_ParentRegion		= NULL;
	m_ParentArea		= NULL;
	m_byJumpMaxTile		= 0;
    m_wEndX				= 0;
    m_wEndY				= 0;
    m_PrevMark			= 0;
	m_byDir				= 0;
	m_bySendOver		= 0;
    m_byCurStep			= 0;
	m_byCurWayIndex		= MAX_TRACK_LENGTH+1;

	memset(mMoveNode.m_WayTrack, 0, MAX_TRACK_LENGTH+1);
	memset(mMoveNode.mNodeData, 0 ,sizeof(mMoveNode.mNodeData));

    m_fAttFrameDec		= 1.0f;
    m_fActFrameDec		= 1.0f;
    m_fWalkSpeedCoef	= 1.0f;
	mCurDesNode			= 0;
	mMoveSpeed			= SRC_MOVE_SPEED;

	// XYD3
	m_OriSpeed = PLAYER_BASIC_SPEED;
	m_Speed = m_OriSpeed/1000.0f;
	m_IsMove2Attack = 0;
	m_curTileX = m_curTileY = 0;
 	m_curX = m_curY = m_curZ = m_Direction = 0.0;
 	m_MoveStartTime = m_NodeCount = m_CurNode = m_Elapsed = m_isRunEnd = 0;
 	memset(m_NodeData, 0, sizeof(m_NodeData));

	memset(m_bPlayerAction, 1, sizeof(m_bPlayerAction));
	m_Elapsed = 200;
}

CActiveObject::~CActiveObject(void) {}

void CActiveObject::SetBackupActionID(INT32 newBackupActionID)
{
	// ����AI�����״̬��ʵʱ�л��ģ�����к󱸵Ķ�����
	// �ᵼ�ºܶ�ط����ֳ�ͻ������ֱ��ȥ�����Ժ�����
	// ���ԡ�״̬��ʵʱ�л��ġ�Ϊ������
	// ԭʼ����μ�SVN
}

void CActiveObject::BackupActionID2CurActionID()
{
	// ԭ��μ�SetBackupActionID�����еĽ���
}

void CActiveObject::OnChangeState(EActionState newActionID) { return; }

void CActiveObject::SetCurActionID(EActionState newActionID)
{
	if (newActionID == m_CurAction)
		return;

	OnChangeState(newActionID);

	m_CurAction = newActionID;
}

void CActiveObject::OnRun(void)
{
	if (!m_ParentRegion || !m_ParentArea)
		return;

//	BackupActionID2CurActionID();
	DoCurAction();

	return;
}

BOOL CActiveObject::EndPrevAction()
{
	switch (GetCurActionID())
	{
	case EA_STAND:
	case EA_RUN:
		break;

    default:
        return FALSE;
	}
    return TRUE;
}

//----------------------------------------------------------------------------------------------------------	
BOOL CActiveObject::SetCurAction()
{
    switch (GetCurActionID())
	{
	case EA_STAND:
    case EA_JUMP:
	case EA_COLLECT:
		break;

	case EA_RUN:
		break;

    default:
        return FALSE;
	}
    return TRUE;
}

BOOL CActiveObject::UpdateWalking()
{
	if (!m_ParentRegion)
		return FALSE;

	if (m_NodeCount == 0 || (m_CurNode == m_NodeCount))
	{
		return FALSE;
	}

	CMonster *pMonster = (CMonster *)DynamicCast(IID_MONSTER);
	if (pMonster)
	{
		//rfalse(2, 1, "MonsterID = %d Pos = %f,%f",pMonster->GetGID(),pMonster->m_curX,pMonster->m_curY);
		// �Ƿ�ֹͣ׷��
		if (SQSynPathMsg::attack == pMonster->m_IsMove2Attack && pMonster->StopTracing())
			return FALSE;

		if (SQSynPathMsg::attack == pMonster->m_IsMove2Attack || pMonster->m_BackProtection)
			pMonster->IntoFightState(0);
	}

// 	if (CMonster *pmonster = (CMonster*)DynamicCast(IID_MONSTER))
// 	{
// 		rfalse(2, 1, "MonsterPosX = %4.2f,MonsterPosY = %4.2f", m_curX, m_curY);
// 	}
// 	else
// 	{
// 		rfalse(2, 1, "PlayerPosX = %4.2f,PlayerPosY = %4.2f", m_curX, m_curY);
// 	}


	WORD curAreaX = GetCurArea(m_curTileX, _AreaW);
	WORD curAreaY = GetCurArea(m_curTileY, _AreaH);

	DWORD curTime = timeGetTime();
	DWORD temp = (curTime-m_MoveStartTime);
	m_MoveStartTime = curTime;
	m_Elapsed = temp;

	float passBy = (float)temp;
	

	DWORD curNode = m_CurNode;

	D3DXVECTOR2 curPos(m_curX, m_curY);
	D3DXVECTOR2 desPos(m_NodeData[curNode].m_X, m_NodeData[curNode].m_Y);
	D3DXVECTOR2 desVector = desPos - curPos;

	D3DXVec2Normalize(&desVector, &desVector);

	// ���㷽��
	m_Direction = atan2(desVector.y, desVector.x);

	// �������һ֡���������˶��پ���
	float len = passBy * m_Speed;

	D3DXVECTOR2 speed  = desVector * len;
	D3DXVECTOR2 newPos = curPos + speed;
	newPos.x = m_Speed* cos(m_Direction) *temp+ curPos.x;
	newPos.y = m_Speed* sin(m_Direction) *temp+ curPos.y;

	float newLen = D3DXVec2Length(&(newPos - curPos));
	float desLen = D3DXVec2Length(&(desPos - curPos));

	float tempx=0,tempy=0;
	bool bIsdesLen=false; ///�Ƿ��߹�����
	// �߹��ˣ���ô������ľ��룬�ۼƵ���һ֡�ƶ�
	if (newLen >= desLen)
	{
		//������ǹ���
		//if(!pMonster)
		{
			m_curX = desPos.x;
			m_curY = desPos.y;

			float offset = newLen - desLen;
			DWORD overTime = DWORD(offset/m_Speed);
			//m_Elapsed += overTime;

			
			m_CurNode++;	
			if (m_CurNode == m_NodeCount)
			{
				/*if (m_NodeCount >= 2)
				{
					m_curX = m_NodeData[m_NodeCount - 1].m_X;
					m_curY = m_NodeData[m_NodeCount - 1].m_Y;
				}*/
				// ��һ֡���ƶ����
				SetCurActionID(EA_STAND);
				SetBackupActionID(EA_STAND);


				m_NodeCount = m_CurNode = 0;
				m_isRunEnd = true;

// 				if (CFightPet *pet = (CFightPet*)DynamicCast(IID_FIGHT_PET))
// 				{
// 					rfalse(2, 1, "CFightPetEndMove %4.2f,%4.2f ", m_curX, m_curY);
// 				}
// 				if (CMonster *pmonster = (CMonster*)DynamicCast(IID_MONSTER))
// 				{
// 					rfalse(2, 1, "MonsterEndMove %4.2f,%4.2f ", m_curX, m_curY);
// 				}
// 				else
// 				{
// 					rfalse(2, 1, "PlayerEndMove %4.2f,%4.2f ", m_curX, m_curY);
// 				}

			}
		}
// 		else
// 		{
// // 				tempx=desPos.x;
// // 				tempy=desPos.y;
// // 				bIsdesLen = true;
// 		}
		
	}
	else
	{
		m_curX = newPos.x;
		m_curY = newPos.y;
	}

// 	if (pMonster)
// 	{
// 		if (pMonster->IsCanMovePostion(tempx,tempy)||SQSynPathMsg::attack != pMonster->m_IsMove2Attack)
// 		{
// 				m_curX = tempx;
// 			 	m_curY = tempy;
// 				if (bIsdesLen)
// 				{
// 					float offset = newLen - desLen;
// 					DWORD overTime = DWORD(offset/m_Speed);
// 					m_Elapsed += overTime;
// 
// 					m_CurNode++;	
// 					if (m_CurNode == m_NodeCount)
// 					{
// 						// ��һ֡���ƶ����
// 						SetCurActionID(EA_STAND);
// 						SetBackupActionID(EA_STAND);
// 
// 						m_NodeCount = m_CurNode = 0;
// 						m_isRunEnd = true;
// 					}
// 				}
// 		}
// 
// 	}

	if (m_curX < 0.0f || m_curY < 0.0f)
	{
		rfalse(2, 1, "PosError m_curX = %f , m_curY = %f ",m_curX,m_curY);
		return FALSE;
	}

	m_curTileX	= (DWORD)(m_curX) >> TILE_BITW; 
	m_curTileY	= (DWORD)(m_curY) >> TILE_BITH;

	WORD newAreaX = GetCurArea(m_curTileX, _AreaW);
	WORD newAreaY = GetCurArea(m_curTileY, _AreaH);

	if ((curAreaX != newAreaX || curAreaY != newAreaY))
	{
		if (0 == m_ParentRegion->Move2Area(self.lock(), newAreaX, newAreaY))
			rfalse( "Move2Area Failed" );
	}

	if (m_isRunEnd)
	{
		m_isRunEnd = false;
		OnRunEnd();
	}

	return TRUE;
}

bool CActiveObject::UpdateCollect()
{
	return true;
}

BOOL CActiveObject::DoCurAction()
{
	switch (GetCurActionID())
	{
	case EA_STAND:
		break;
        
	case EA_ZAZEN:
		break;

	case EA_RUN:
		UpdateWalking();
		break;

	case EA_JUMP:
        if (m_ParentRegion)
        {
// 			float checker = ( ( GetCurActionID() != EA_ATTACK ) ? (float)m_fActFrameDec : (float)m_fAttFrameDec ) * 4;
// 
// 			if (m_fActionFrame <= checker ) 
// 			{
// 				WORD ax2 = GetCurArea( m_wCurX, _AreaW );
// 				WORD ay2 = GetCurArea( m_wCurY, _AreaH );
// 		        WORD ax1 = GetCurArea( m_wEndX, _AreaW );
// 		        WORD ay1 = GetCurArea( m_wEndY, _AreaH );
// 
// 				if ( ( ax1 != ax2 ) || ( ay1 != ay2 ) )
//                     if ( m_ParentRegion->Move2Area( self.lock(), ax2, ay2 ) == 0 )
// 						rfalse( 2, 1, "Move2Area fail" );
// 
// 				m_wCurX = m_wEndX;
// 				m_wCurY = m_wEndY;
// 
// 				mCurShowX = float(m_wCurX << TILE_BITW);
// 				mCurShowY = float(m_wCurY << TILE_BITH);
// 
// 				SetCurActionID(EA_STAND);
// 				m_dwJumpSegment = timeGetTime();
// 			}
        }
        break;

	case EA_COLLECT:
		UpdateCollect();
		break;

    default:
        return FALSE;
	}
    return TRUE;
}

void CActiveObject::InitPlayerMove(struct SQSynPathMsg *pMsg)
{
	if (!m_bPlayerAction[CST_CANMOVE])
		return;

	//rfalse(2, 1, "��ǰ�ƶ�����Ϊ0%f, %f,��ǰ�ƶ�����Ϊ1%f, %f\r\n", pMsg->m_Path.m_path[0].m_X, pMsg->m_Path.m_path[0].m_Y,pMsg->m_Path.m_path[1].m_X, pMsg->m_Path.m_path[1].m_Y);

	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
		pPlayer->CancelCollect();	// ȡ���ɼ�
		//�ܶ��в�����������
		////////////////////////////////////
		// [2012-7-21 12-56 gw: -���������Ҳ��Ҫ���]
		//-if(pPlayer->GetReadyEvent()&&!pPlayer->GetReadyEvent()->IsEvent("UpMountEvent")){
		//-		pPlayer->BreakOperation();	// �жϲ���
		//if(pPlayer->GetReadyEvent()){
		//			pPlayer->BreakOperation();	// �жϲ���
		//}
		pPlayer->m_BatchReadyEvent.BreakOperationReadyEvent(Event::EEventInterrupt_Move);
	}

	if (EA_SKILL_ATTACK == GetCurActionID())
	{
		if (pPlayer)
		{
//			if (SQSynPathMsg::attack != pMsg->m_move2What)
//				pPlayer->m_EnemyInEye = 0;
 
	 		if (SASkillCancel::SSC_SING == pPlayer->m_AtkContext.m_Phase)
 			{
 				SASkillCancel msg;
 				msg.dwGlobal	= GetGID();
 				msg.bPhase		= pPlayer->m_AtkContext.m_Phase;
 				msg.bSkillIndex	= pPlayer->m_AtkContext.dwSkillIndex;
 				g_StoreMessage(pPlayer->m_ClientIndex, &msg, sizeof(msg));
				pPlayer->m_AtkContext.m_TargetList.clear();
				pPlayer->m_AtkContext.m_bSkillAtkOver = true;
			}
			m_curX = pMsg->m_Path.m_path[1].m_X;
			m_curY = pMsg->m_Path.m_path[1].m_Y;
			m_curTileX = (DWORD)(m_curX) >> TILE_BITW;
			m_curTileY = (DWORD)(m_curY) >> TILE_BITH;
			Stand(EA_SKILL_ATTACK);
			return;//���ڹ����׶Σ�Ӧ���޷��ƶ�,ֱ��ͬ���ͻ�������
		}
		
		if (CFightPet *pFightPet = (CFightPet *)DynamicCast(IID_FIGHT_PET))
		{
			pFightPet->m_TargetCache.first  = 
			pFightPet->m_TargetCache.second = 
			pFightPet->m_CurEnemy.first     = 
			pFightPet->m_CurEnemy.second    = 0; 
		}
	}

// 	// ������ǰ�Ĺ���״̬
// 	CFightObject *pFightObj = (CFightObject *)DynamicCast(IID_FIGHT_PET);
// 	if (pFightObj && EA_SKILL_ATTACK == GetCurActionID())
// 	{

// 	}

	// �����ǰ���ƶ�״̬��˵���ƶ���ת���ˣ���ôͬ��һ��λ��
	//if (EA_RUN == GetCurActionID())
	{
		m_curX = pMsg->m_Path.m_path[0].m_X;
		m_curY = pMsg->m_Path.m_path[0].m_Y;
	}

	if (EA_ZAZEN==GetCurActionID())
	{
		SetZazen(false);
	}

	m_Elapsed = 100;

	// �����ƶ�·��
	if (SetWay(pMsg) == false)
	{
		// ���ʧ�ܵĻ���ͣ����
		Stand();
	}
}

void CActiveObject::AddUpSpeedProficiency( int nMoveTiles )
{
// 	CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
// 	if ( nMoveTiles>0 && pPlayer && pPlayer->m_pFlyBuffs[EFST_UP_SPEED] && pPlayer->m_pFlyBuffs[EFST_UP_SPEED]->IsActive() ) {
// 		rfalse( 2, 1, "Up Spead Add Proficiency : %d \n", nMoveTiles);	
// 		pPlayer->m_pFlyBuffs[EFST_UP_SPEED]->AddProficiency( nMoveTiles );
// 	}
};

bool CActiveObject::Stand()
{
	SASynPosMsg	msg;
	msg.m_GID		= GetGID();
	msg.m_Action	= EA_STAND;
	msg.m_X			= m_curX;
	msg.m_Y			= m_curY;
	msg.m_Z			= m_curZ;

	// ���켣ת������Ч������������
	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea)
		pArea->SendAdj(&msg, sizeof(SASynPosMsg), -1);

	m_NodeCount = m_CurNode = 0;
	memset(&m_NodeData, 0, sizeof(m_NodeData));

	SetCurActionID(EA_STAND);
	SetBackupActionID(EA_STAND);

	return TRUE;
}

bool CActiveObject::Stand(EActionState eaction)
{
	SASynPosMsg	msg;
	msg.m_GID = GetGID();
	msg.m_Action = eaction;
	msg.m_X = m_curX;
	msg.m_Y = m_curY;
	msg.m_Z = m_curZ;

	// ���켣ת������Ч������������
	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea)
		pArea->SendAdj(&msg, sizeof(SASynPosMsg), -1);

	m_NodeCount = m_CurNode = 0;
	memset(&m_NodeData, 0, sizeof(m_NodeData));

	SetCurActionID(eaction);
	SetBackupActionID(eaction);
	return TRUE;
}

bool CActiveObject::SetZazen(bool bZazen)
{
    if (bZazen)						// ����
    {
		SASynPosMsg msg;
		msg.m_GID		= GetGID();
		msg.m_Action	= EA_ZAZEN;
		msg.m_X			= m_curX;
		msg.m_Y			= m_curY;
		msg.m_Z			= m_curZ;
		msg.m_Dir		= m_Direction;

	    // ���켣ת������Ч������������
	    CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	    if (pArea)
		    pArea->SendAdj(&msg, sizeof(SASynPosMsg), -1);

		SetCurActionID(EA_ZAZEN);
		SetBackupActionID(EA_ZAZEN);

		

	    return true;
    }
    else						// վ��
    {
        return  Stand();
    }
}

void CActiveObject::MakeRadomPath(DWORD moveArea, float srcX, float srcY, SQSynPathMsg &msg)
{
	// �����������Ҫ����·����
	int WayPoints = CRandom::RandRange(1,1);

	if (moveArea == 0)
		moveArea = 1;
	
	int iXOffset	= 0;
	int iYOffset	= 0;
	float endX		= 0.0f;
	float endY		= 0.0f;

	msg.m_GID				 = GetGID();
	msg.m_move2What			 = SQSynPathMsg::normal;
	msg.m_Path.m_wMapID		 = m_ParentRegion->m_wMapID;
	msg.m_Path.m_wayPoints	 = WayPoints+1;
	// [2012-5-24 12-25 gw: -��������Ѳ�߸���EA_WALK]msg.m_Path.m_moveType	 = EA_RUN;
	// [2012-5-24 12-43 gw: -]msg.m_Path.m_moveType = EA_WALK; // [2012-5-24 12-26 gw: +����walk]
	msg.m_Path.m_moveType	 = EA_RUN;

	msg.m_Path.m_path[0].m_X = m_curX;
	msg.m_Path.m_path[0].m_Y = m_curY;

	for(int i=0; i<WayPoints; i++)
	{
		iXOffset = moveArea - (CRandom::SimpleRandom() + GetGID())%(moveArea*2+1);
		iYOffset = moveArea - (CRandom::SimpleRandom() + GetGID())%(moveArea*2+1);

		// ·�����Ƿ���������ʱû���ж�
		
		endX = srcX + iXOffset;
		endY = srcY + iYOffset;
#ifdef USEMAPCOLLISION
		CMonster *pmonster=(CMonster*)DynamicCast(IID_MONSTER);
		if (pmonster)
		{
			if(pmonster->m_ParentRegion)
			{
// 				if (!pmonster->m_ParentRegion->IsCanMove(endX,endY))
// 				{
// 					return;
// 				}
			}
		}
#endif
		msg.m_Path.m_path[i+1].m_X = endX;
		msg.m_Path.m_path[i+1].m_Y = endY;
	}

	//rfalse(2, 1, "MakeRadomPath %f,%f  -> %f,%f", m_curX, m_curY, msg.m_Path.m_path[WayPoints ].m_X, msg.m_Path.m_path[WayPoints].m_Y);
	return;
}

void CActiveObject::MakeRadomPath(WORD wMoveArea, const WORD wSrcX, const WORD wSrcY, int &iXEnd, int &iYEnd)
{
    if (wMoveArea > 4)
        wMoveArea = 4;

    if (wMoveArea == 0)
        wMoveArea = 1;

    int  iXOffset;
    int  iYOffset;

    iXOffset = (rand()+ GetGID()) % wMoveArea*2;
    iXOffset -= wMoveArea;
    if (iXOffset == 0)
    {
        // try do rand again?
        iXOffset = (rand()%2)?1:-1;
    }

    iYOffset = (rand()+GetGID()) % wMoveArea*2;
    iYOffset -= wMoveArea;
    if (iYOffset == 0)
    {
        // try do rand again?
        iYOffset = (rand()%2)?1:-1;
    }

    iXEnd = wSrcX + iXOffset;
    iYEnd = wSrcY + iYOffset;
}

bool CActiveObject::SetWay(struct SQSynPathMsg *pMsg)
{
	// ���·��
	// ��ʱֻ�����ӿ�
	
	// �洢·��
	StorePath(pMsg);

	// ���켣ת������Ч������������
	SASynPathMsg msg;
	msg.m_GID = pMsg->m_GID;
	memcpy(&msg.m_Path, &pMsg->m_Path, sizeof(pMsg->m_Path) - (MAX_TRACK_LENGTH-pMsg->m_Path.m_wayPoints)*sizeof(SWayPoint));

// 	WORD num = msg.m_Path.m_wayPoints - 1;
//  	if (CMonster *pmonster = (CMonster*)DynamicCast(IID_MONSTER))
//  	{
//  		rfalse(2, 1, "MonsterSendWay %4.2f,%4.2f ---%4.2f,%4.2f ", m_curX, m_curY, msg.m_Path.m_path[num].m_X, msg.m_Path.m_path[num].m_Y);
//  	}
// 	else
// 	{
// 		rfalse(2, 1, "PlayerSendWay %4.2f,%4.2f ---%4.2f,%4.2f ", m_curX, m_curY, msg.m_Path.m_path[num].m_X, msg.m_Path.m_path[num].m_Y);
// 	}

	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea)
		pArea->SendAdj(&msg, sizeof(SASynPathMsg) - (MAX_TRACK_LENGTH-pMsg->m_Path.m_wayPoints)*sizeof(SWayPoint), -1);

	return TRUE;
}

//ƽ��������㹫ʽ
#define DISTANCE_SQUARE(x1,y1,x2, y2) ( ((x1)-(x2))*((x1)-(x2)) + ((y1)-(y2))*((y1)-(y2)) )
#define DISTANCE(x1,y1, x2, y2) (sqrt( (double)DISTANCE_SQUARE(x1, y1, x2, y2)) )

bool CActiveObject::SetJump(SQASynWayJumpMsg *pMsg)
{
//     if (m_ParentRegion == NULL)
//         return false;
// 
//     // ...(�ݲ�����������״̬���ͻ�����������ս���С��ƶ���)
//     //if( 0 )     return  true;
// 
// 	if (GetGID() != pMsg->dwGlobalID)
// 		return false;
//     
//     // �ж�������յ�
// 	WORD x, y;
//     int iDx,iDy;
// 	GetCurPos(x, y);
//     iDx = ((short)x) - ((short)pMsg->wCurX);
//     iDy = ((short)y) - ((short)pMsg->wCurY);
// 
// 	if ((abs(iDx) > MAX_MOVEOFFSET) || (abs(iDy) > MAX_MOVEOFFSET))
//     {
//         // ���˲�ͬ���������ϵĴ���...
//         Stand();
//         return  false;
//     }
// 
// 
// 	//commont by chenj
//  //   iDx = ((short)pMsg->NextPos.x) - ((short)pMsg->wCurX);
//  //   iDy = ((short)pMsg->NextPos.y) - ((short)pMsg->wCurY);
// 	//if ((abs(iDx) > MAX_JUMPOFFSETX) || (abs(iDy) > MAX_JUMPOFFSETX))
//  //   {
//  //       // ���������Ļ�ľ��� ������
//  //       return  true;
//  //   }
// 	
// 	// add by chenj
// 	// ÿ����Ҷ����Լ����Ṧ ��Զ��Ծ����
// 
// 	//���Ŀ���λ��ǰ���±ߵĻ� ��0������,ͬʱ�������Ҳ����������Ӧ
// 	if( pMsg->wCurX ==pMsg->NextPos.x && pMsg->wCurY == pMsg->NextPos.y )
// 		return true;
// 
// 	int jumpDistanceRequest = (int)DISTANCE(pMsg->wCurX, pMsg->wCurY, pMsg->NextPos.x, pMsg->NextPos.y);
// 	if ( jumpDistanceRequest > m_byJumpMaxTile )
// 		return false;
// 
//     // �ж���Ծ���м�·��
//     if( !CheckJump( pMsg->wCurX, pMsg->wCurY, pMsg->NextPos.x, pMsg->NextPos.y, TRUE ) )
//         return  false;
// 
// 	// �������ң�������������...
// 	if( GetIID()==IID_PLAYER )
// 	{
// 		CPlayer *pPlayer;
// 		pPlayer = (CPlayer *)DynamicCast( IID_PLAYER );
// 		if( pPlayer )
// 		{
// 			//if( pPlayer->m_wSP<10 )
// 			//    pPlayer->m_wSP = 0;
// 			//else
// 			//    pPlayer->m_wSP -= 10;
// 			// ���͡�����
// 
// 			//����������BuFF
// 			if( !pPlayer->StartJumpBuff() )
// 				return false;
// 		}
// 	}
// 
//     // ����״̬
// 	SetBackupActionID(EA_JUMP);    
// 
//     // ����Ϣ������Ч������������
// 	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
// 	if (pArea)
// 	{
//         CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER);
//         if( pPlayer )
//             pArea->SendAdj( pMsg, sizeof(SQASynWayJumpMsg), -1 );//pPlayer->m_ClientIndex //// 2005-07-13 ����书-��Ծ-�����еĲ�ͬ������
//         //!4
//         iMsgCount++;
// 	}
// 
//     m_wEndX = pMsg->NextPos.x;
//     m_wEndY = pMsg->NextPos.y;
// 
//     // ���ȱ���յ㲻��վ��
//     if (m_ParentRegion != NULL)
//         m_ParentRegion->MarkTileStop(pMsg->NextPos.x, pMsg->NextPos.y, m_PrevMark, GetGID());

    return  true;
}

void CActiveObject::FillSynPos(struct SPath *pSSP)
{
	if (!pSSP)
		return;

	pSSP->m_wMapID = m_ParentRegion->m_wMapID;
	pSSP->m_moveType = GetCurActionID();
	memset(pSSP->m_path, 0, MAX_TRACK_LENGTH*sizeof(SWayPoint));

	pSSP->m_path[0].m_X = m_curX;
	pSSP->m_path[0].m_Y = m_curY;

	CPlayer *pPlayer = (CPlayer*)DynamicCast(IID_PLAYER);
	if (pPlayer)
	{
	}

	if (EA_RUN == GetCurActionID())
	{
		int points = m_NodeCount - m_CurNode;
		pSSP->m_wayPoints = points+1;

		DWORD j = 1;
		for (DWORD i=m_CurNode; i<m_NodeCount; i++)
		{
			pSSP->m_path[j].m_X = m_NodeData[i].m_X;
			pSSP->m_path[j].m_Y = m_NodeData[i].m_Y;

			j++;
		}
	}
	else
	{
		// ֻ��һ��·���㣬�����0ƫ�ƣ���ʾ��ǰ������
		pSSP->m_wayPoints = 1;			
	}
}

void CActiveObject::GetCurPos(WORD &x, WORD &y)
{
	x = m_curTileX;
	y = m_curTileY;
}

void CActiveObject::GetOffPos(WORD &x, WORD &y, char alen)
{
// 	x = m_wCurX;
// 	y = m_wCurY;
// 
// 	if (m_byCurWayIndex < MAX_TRACK_LENGTH)
// 	{
// 		int count = m_byCurWayIndex;
//         if (alen == -1)
//         {
// 		    while (count < MAX_TRACK_LENGTH)
// 		    {
// 			    char len = mMoveNode.m_WayTrack[count].len;
// 			    char dir = mMoveNode.m_WayTrack[count].dir;
// 			    if (len == 0) break;
// 			    if ((dir < 4) && (dir > 0)) x+=len; else if (dir > 4) x-=len;
// 			    if ((dir < 2) || (dir > 6)) y-=len; else if ((dir > 2) && (dir < 6)) y+=len;
// 
// 			    count++;
// 		    }
//         }
//         else
//         {
// 		    while (count < MAX_TRACK_LENGTH)
// 		    {
// 			    char len = mMoveNode.m_WayTrack[count].len;
// 			    char dir = mMoveNode.m_WayTrack[count].dir;
// 			    if (len == 0) break;
// 
//                 do
//                 {
//                     if ((dir < 4) && (dir > 0)) x++; else if (dir > 4) x--;
// 			        if ((dir < 2) || (dir > 6)) y--; else if ((dir > 2) && (dir < 6)) y++;
// 
//                     len--; 
//                     alen--; 
// 
//                     if (alen <= 0) return; 
// 
//                 } while (len>0);
// 
// 			    count++;
// 		    }
//         }
// 	}
}

BOOL CActiveObject::CheckPath(SQSynWayTrackMsg *pMsg)
{
// 	if (m_ParentRegion == NULL) 
// 		return false;
// 
// 	LPDWORD pdwTiles	= m_ParentRegion->m_pdwTiles;
// 	WORD	rw			= m_ParentRegion->m_RegionW;
// 	WORD	rh			= m_ParentRegion->m_RegionH;
// 
// 	if (pdwTiles == 0)
// 		return false;
// 
// 	WORD x = pMsg->ssw.wSegX;
// 	WORD y = pMsg->ssw.wSegY;
// 	int count = 0;
// 
// 	SWayTrack Track[MAX_TRACK_LENGTH];
// 	memcpy (Track, pMsg->ssw.Track, MAX_TRACK_LENGTH);
// 
// 	// �����������ڲ���������ס������
//     bool isSeg = false;
// 
// 	do
// 	{
// 		// �ƶ�Խ��
// 		if ((x >= rw) || (y >= rh)) 
// 			return false;	
// 
// 		// Tile��������
// 		if ((pdwTiles[x + y*rw]&1) == 0) 
//         {
//             if (isSeg) 
// 				return false;
//         }
//         else 
// 			isSeg = true;
// 
//         // �����߲�����
//         LPDWORD pdwTilesMarks = m_ParentRegion->m_pdwTileMarks;
//         if ( pdwTilesMarks == 0 )
//             return false;
// 
//         // ��������߲�������ô�ʹ����赲
//         if ( m_ParentRegion->m_IsBlock )
//         {
//             if ( x != pMsg->ssw.wSegX || y != pMsg->ssw.wSegY )
//             {
//                 DWORD index = x + y*rw;
//                 if ( !(pdwTiles[index] & TILETYPE_JUMP) )
//                 {
//                     if ( pdwTilesMarks[index] != 0 && pdwTilesMarks[index] != GetGID() )
//                         return false;
//                 }
//             }
//         }
// 
// 		if (Track[count].len)
// 		{
// 			if ((Track[count].dir < 4) && (Track[count].dir > 0)) 
// 				x++; 
// 			else if (Track[count].dir > 4) 
// 				x--;
// 
// 			if ((Track[count].dir < 2) || (Track[count].dir > 6)) 
// 				y--; 
// 			else if ((Track[count].dir > 2) && (Track[count].dir < 6)) 
// 				y++;
// 
// 			Track[count].len--;
// 
// 			if (Track[count].len == 0)
// 				count++;
// 		}
// 		else
// 			break;
// 	}
// 	while (count < MAX_TRACK_LENGTH);
// 
// 	// ������һ��Tile������
//     if (!m_ParentRegion->isTileStopAble(x, y, GetGID()))
//     {
//         if (count == 0) return false;
//         pMsg->ssw.Track[count-1].len--;
//     }

	return true;
}

// �ж���Ծ·����ͬʱ����·����
bool CActiveObject::CheckJump(int x,int y,int x1,int y1,BOOL bCheckEndPos)// bCheckEndPosʵ���������ڼ�鹥����͸
{
//     const   int TILE_SIZE_X = 64;
//     const   int TILE_SIZE_Y = 32;
//     const   int TILE_BIT_X  = 6;
//     const   int TILE_BIT_Y  = 5;
//     int iDx,iDy,iLen,iJumpStep;
// 
//     // ��ȡ�ر�����
// 	if (m_ParentRegion == NULL) return false;
// 	LPDWORD pdwTiles = m_ParentRegion->m_pdwTiles;
// 	WORD	rw = m_ParentRegion->m_RegionW;
// 	WORD	rh = m_ParentRegion->m_RegionH;
// 	if (pdwTiles == 0) return false;
// 
//     // �ж��յ��Ƿ�ɴ�
//     if ( x1 >= rw || y1 >= rh )
//         return false;
// 
//     if ( !(pdwTiles[y1*rw + x1] & TILETYPE_MOVE) )
//         return false;
// 
//     // �ж��յ��Ƿ��ܹ�վ��
//     if( bCheckEndPos )
//     {
//         if (!m_ParentRegion->isTileStopAble(x1, y1, GetGID()) || !m_ParentRegion->isTileWalkAble(x1, y1))
//         {
//             return false;
//         }
//     }
// 
//     // ����·��
//     iDx = x1-x;
//     iDy = y1-y;
//     // ��Ծ��̾��� 1��
//     if( bCheckEndPos )
//     {
//         if( 1>=(iDx>0?iDx:-iDx) && 1>=(iDy>0?iDy:-iDy) )
//         {
//             return  false;
//         }
//     }
// 
//     // ����·������
//     iLen = (int)sqrt( (double)((x1-x)*(x1-x)+(y1-y)*(y1-y)) );///////////////////////////////////////
//     iJumpStep = (int)((((iLen-2)/3) + 2)<<1) * 3  ;    // �� 2-10 �� 2.5-4.5 MY_JUMP_FRAMESKIP
// 
//     iLen = (iDx>0?iDx:-iDx) + (iDy>0?iDy:-iDy);     // �ֳɼ���
//     if( iLen!=0 )   
//     {
//         //fStepX = iDx*TILE_SIZE_X/(float)iLen;
//         //fStepY = iDy*TILE_SIZE_Y/(float)iLen;
// 
//         //int xSeg = ( x << TILE_BIT_X ) + ( TILE_SIZE_X >> 1 ) + ( iDx > 0 ? 1 : -1 );
//         //int ySeg = ( y << TILE_BIT_Y ) + ( TILE_SIZE_Y >> 1 ) + ( iDy > 0 ? 1 : -1 );
// 
//         //for( i=0; i<=iLen; i++ )                    // �����˵�
//         //{
//         //    iCurPosX = (int)(xSeg + fStepX*i);
//         //    iCurPosY = (int)(ySeg + fStepY*i);
//         //    iCurPosX = iCurPosX>>TILE_BIT_X;
//         //    iCurPosY = iCurPosY>>TILE_BIT_Y;
//         //    
//         //    if ( iCurPosX >= rw || iCurPosY >= rh ) 
//         //        return false;  // ����
// 
//         //    if ( bCheckEndPos && !(pdwTiles[iCurPosY*rw+iCurPosX]&TILETYPE_JUMP) )  
//         //        return false;
// 
//         //    if ( !bCheckEndPos && !(pdwTiles[iCurPosY*rw+iCurPosX]&TILETYPE_MOVE) )  
//         //        return false;
// 
//         //    if (i >= 32)
//         //        return false;
// 
//         //    m_JumpTrack[i].x = iCurPosX;
//         //    m_JumpTrack[i].y = iCurPosY;
//         //}
// 
//         if (bCheckEndPos)
//         {
//             //m_byJumpStepCount = iLen;
//             //m_byJumpCurStep = 0;
//             //m_byJumpFrame = (BYTE)(iJumpStep*0.5/iLen); // 0.7=��ͻ���ͬ��������ֵ
//  //           m_fActionFrame = N_JUMP_ACTION_FRAME; //m_byJumpFrame;
//             m_dwJumpSegment = timeGetTime();
// 
//             // ת��Ϊ���룬�������ݵ���������ֵ
//             //m_fJumpPerStep = iJumpStep * 0.5f / iLen * ( 50 / (float)m_fActFrameDec );
//         }
//    }
//    else if ( bCheckEndPos )
//        return false;

   return  true;
}

BOOL CActiveObject::CheckDist(CActiveObject *pDest, BYTE byDist, BOOL bCheckWay,int x,int y)
{
//     if( x==-1 || y==-1 )
//     {
//         x = m_wCurX;
//         y = m_wCurY;
//     }
//     if( abs(pDest->m_wCurX-x)<=byDist && abs(pDest->m_wCurY-y)<=byDist )
//     {
//         if( !bCheckWay ) 
//             return TRUE;
// 
//         // ������Ծ���Ͳ��ɹ�����͸
//         if( !CheckJump( x, y, pDest->m_wCurX, pDest->m_wCurY, FALSE ) )
//         {
//             SendErrorMsg( SABackMsg::B_FARFIGHT );
//             return  FALSE;
//         }
// 
//         return  TRUE;
//     }
//     else    
//         return  FALSE;

	return FALSE;
}

BOOL CActiveObject::StorePath(struct SQSynPathMsg *pMsg)
{
	if (!pMsg)
	{
		rfalse(4, 1, "ActiveObject.cpp - StorePath() - !pMsg");
		return FALSE;
	}

	if (pMsg->m_Path.m_wayPoints < 2)
		return FALSE;

	// ����·��
	memcpy(&m_NodeData, pMsg->m_Path.m_path, pMsg->m_Path.m_wayPoints*sizeof(SWayPoint));
// 	if (CMonster *pmonster = (CMonster*)DynamicCast(IID_MONSTER))
// 	{
// 		if (m_NodeData[0].m_X == m_NodeData[0].m_Y == 0.0f)
// 		{
// 			for (size_t i = 0; i < pMsg->m_Path.m_wayPoints; i++)
// 			{
// 				rfalse(2, 1, "Index = %d,X = %4.2f,Y = %4.2f",i,m_NodeData[i].m_X,m_NodeData[i].m_Y);
// 			}
// 			
// 		}
// 	}

	m_IsMove2Attack = pMsg->m_move2What;
	m_NodeCount		= pMsg->m_Path.m_wayPoints;
	m_CurNode		= 1;
	m_isRunEnd		= false;
	//m_MoveStartTime = timeGetTime();

	// ����һ֡�Ϳ�ʼ�ƶ�����/�ߣ�
	SetCurActionID(EA_RUN);
	SetBackupActionID(EA_RUN);

// 	CMonster *pmonster = (CMonster*)DynamicCast(IID_MONSTER);
// 	if (pmonster)
// 	{
// 		pmonster->m_nMoveStartTime = GetTickCount64();
// 		rfalse("MonsterMoveStart GID = %d,MoveTime = %d,MoveNode = %d,Lenth = % f", pmonster->GetGID(), (m_NodeCount * 200), m_NodeCount, lenth);
// 	}

	return TRUE;
}

BOOL CActiveObject::StorePathXYSJ(struct SQSynWayTrackMsg* pMsg)
{
// 	if (pMsg->ssw.Track[0].len == 0) 
// 	{
// 		return FALSE;
// 	}
// 
// 	memcpy(mMoveNode.m_WayTrack, pMsg->ssw.Track, MAX_TRACK_LENGTH);
// 
// 	SetBackupActionID(pMsg->ssw.byAction);
// 	SetCurActionID(( EActionState )pMsg->ssw.byAction);
// 
// 	m_fMovePerStep = (((GetCurActionID() == EA_WALK ) ? MY_WALK_FRAMESKIP : MY_RUN_FRAMESKIP)) * (50 / (float)m_fActFrameDec * 0.95f); 
// 
// 	m_dwMoveSegment = timeGetTime();
// 
// 	if (GetCurActionID() == EA_WALK || GetCurActionID() == EA_RUN) 
// 	{
// 		m_dwMoveSegment -= (DWORD)(max(210,m_fMovePerStep) / 2);
// 	}
// 
// 	m_byCurWayIndex = 0;
// 	m_byCurStep = 0;

	return 1;
}

#if 0	
//----------------------------------------------------------------------------------------------------------	
void CActiveObject::CalcDirByMoveNode()
{
	if (m_byCurWayIndex > MAX_TRACK_LENGTH)
	{
		return;
	}

	mMoveNode.mDir[ 0 ].x = 0.0;
	mMoveNode.mDir[ 0 ].y = 0.0;

	D3DXVECTOR2 last(mCurShowX,mCurShowY);

	for(int i = 0;i < MAX_TRACK_LENGTH;i++)
	{
		if(mMoveNode.mNodeData[ i ].x == -1 || mMoveNode.mNodeData[ i ].y == -1)
		{
			break;
		}

		D3DXVECTOR2 current(mMoveNode.mNodeData[ i ].x << TILE_BITW,mMoveNode.mNodeData[ i ].y << TILE_BITH);

		D3DXVECTOR2 dir = current - last;
		D3DXVec2Normalize(&dir,&dir);

		last = current;
		mMoveNode.mDir[ i ].x = dir.x;
		mMoveNode.mDir[ i ].y = dir.y;
	}
}
#endif

//----------------------------------------------------------------------------------------------------------	
void CActiveObject::ConverWayTrack2WayNode(SSynWay* pSynWay,SWayNode* pNodeTemp)
{
// 	INT32 iDx = 0;
// 	INT32 iDy = 0;
// 
// 
// 	pNodeTemp[ 0 ].x = pSynWay->wSegX;
// 	pNodeTemp[ 0 ].y = pSynWay->wSegY;
// 
// 	INT32 stratNodeIndex = 1;
// 
// 	for(int i = 0;i < MAX_TRACK_LENGTH;i++)
// 	{
// 		// ���˹켣�յ�
// 		if(pSynWay->Track[i].len == 0)                                  
// 		{
// 			pNodeTemp[stratNodeIndex].x = -1;
// 			pNodeTemp[stratNodeIndex].y = -1;
// 			mNodeCount = stratNodeIndex;
// 			mCurDesNode = 0;
// 			break;
// 		}
// 
// 		g_Direction2XY( pSynWay->Track[i].dir, iDx, iDy );
// 
// 		// �����һ��Ŀ���
// 		pNodeTemp[ stratNodeIndex ].x = pNodeTemp[stratNodeIndex - 1].x + iDx*pSynWay->Track[ i ].len;
// 		pNodeTemp[ stratNodeIndex ].y = pNodeTemp[stratNodeIndex - 1].y + iDy*pSynWay->Track[ i ].len;
// 		stratNodeIndex ++;
// 	}
// 
// 	/*
// 	if (DynamicCast(IID_PLAYER))
// 	{
// 		OutputDebugStr("******ת��Ϊ������******\n");
// 		
// 		int i=0;
// 		char temp[128];
// 
// 		while (pNodeTemp[i].x != -1 && pNodeTemp[i].y != -1)
// 		{
// 			sprintf(temp, "�켣��%d: [X:%d, Y:%d]\n", i+1, pNodeTemp[i].x, pNodeTemp[i].y);
// 			OutputDebugStr(temp);
// 			++i;
// 		}
// 	} //*/
// 
// 	// ���������ӳ���200���룬��ô�����ﲹ����
// 	mElapsed += 200;
// 	UpdateWalkByXYSJ();
}

int CActiveObject::FindPath(ASHORT xend, ASHORT yend, SQSynWayTrackMsg &WayMsg)
{
//*
//     WayMsg.dwGlobalID = 0;
//     ZeroMemory(&WayMsg.ssw, sizeof(WayMsg.ssw));
// 
//     if (this == NULL) 
//         return 0;
// 
//     WayMsg.dwGlobalID = GetGID();
// 
//     if (m_ParentRegion == NULL) 
//         return 0;
// 
//     if (m_wCurX >= m_ParentRegion->m_RegionW) 
//         return m_wCurX = 0;
// 
//     if (m_wCurY >= m_ParentRegion->m_RegionH) 
//         return m_wCurY = 0;
// 
//     if (xend >= m_ParentRegion->m_RegionW) 
//         return 0;
// 
//     if (yend >= m_ParentRegion->m_RegionH) 
//         return 0;
// 
//     if (!m_ParentRegion->isTileWalkAble(m_wCurX, m_wCurY))
//         return 0;
// 
//     BYTE dir = 4;
//     BYTE except_dir = 8;
// 
//     GETDIR(dir, m_wCurX, m_wCurY, xend, yend);
// 
//     WORD x, y;
// 
//     x = m_wCurX, y = m_wCurY;
// 
//     USEDIR(dir, x, y);
// 
//     if (!m_ParentRegion->isTileWalkAble(x, y))
//     {
//         int accs = (rand() < (RAND_MAX >> 1)) ? -1 : 1;
//         for (int i=0; i<8; i++, dir += accs)
//         {
//             x = m_wCurX, y = m_wCurY;
// 
//             dir = dir % 8;
// 
//             USEDIR(dir, x, y);
// 
//             if (m_ParentRegion->isTileWalkAble(x, y))
//                 goto _set_msg;
//         }
// 
//         return WayMsg.ssw.Track[0].len;
//     }
// 
// _set_msg:
// 
//     WayMsg.ssw.byAction = EA_WALK;
//     WayMsg.ssw.wSegX = m_wCurX;
//     WayMsg.ssw.wSegY = m_wCurY;
//     WayMsg.ssw.Track[0].len = 1;
//     WayMsg.ssw.Track[0].dir = dir;
// 
//     if ((xend != x) || (yend != y))
//     {
//         WayMsg.ssw.Track[0].len ++;
//     }
// 
//     return WayMsg.ssw.Track[0].len;

	return 0;
}

bool CActiveObject::SetMovePath(SQSynPathMsg *pWayMsg)
{
	if (!pWayMsg)
	{
		rfalse(4, 1, "ActiveObject.cpp - SetMovePath() - !pWayMsg");
		return FALSE;
	}

	if (pWayMsg->m_Path.m_wayPoints>=MAX_TRACK_LENGTH)//·�������
	{
		return false;
	}


	if (SetWay(pWayMsg))
		return true;
	else
		return false;
}

void CActiveObject::ClearMarkStop()
{
    if (m_ParentRegion != NULL)
        m_ParentRegion->MarkTileStop(0, 0, m_PrevMark, GetGID());
}

void CActiveObject::GetMarkOffPos(WORD &x, WORD &y)
{
    x = HIWORD(m_PrevMark);
    y = LOWORD(m_PrevMark);
}

void CActiveObject::SendErrorMsg( BYTE byMsgId )
{
    SABackMsg msg;
    msg.byType = byMsgId;
    SendMsg( &msg, sizeof(SABackMsg) );
}

void CActiveObject::SendMsg(LPVOID msg, size_t size)
{
	if (CPlayer *pPlayer = (CPlayer *)DynamicCast(IID_PLAYER))
        g_StoreMessage(pPlayer->m_ClientIndex, msg, (DWORD)size);
}

void CActiveObject::SendActSpeed(void)
{
    SAActSpeedChangeMsg     Msg;
    Msg.dwGlobalID = GetGID();
    Msg.fActSpeedDec = m_fActFrameDec;

    if (m_ParentArea != NULL)
		m_ParentArea->SendAdj(&Msg, sizeof(SAActSpeedChangeMsg), -1);
}
void CActiveObject::SendBuffFightOutPos(WORD endx,WORD endy,DWORD objID)
{
	SABuffFightOutMsg Msg;
	Msg.dwGlobalID = objID;
	Msg.wEndX = endx;
	Msg.wEndY = endy;

	if (m_ParentArea != NULL)
		m_ParentArea->SendAdj(&Msg, sizeof(SABuffFightOutMsg), -1);
}
///////////////////////////////////////////////////

bool CActiveObject::GetStopAblePos(WORD &xTarget, WORD &yTarget)
{
    if (m_ParentRegion == NULL) return false;

    WORD x = xTarget;
    WORD y = yTarget;

    BYTE dircheck = 0;

    while (dircheck != 0xff)
    {
        WORD dx = x;
        WORD dy = y;

        switch (RandDir(dircheck))
        {
        case 0:
            dy--;
            break;
        case 1:
            dx++;
            dy--;
            break;
        case 2:
            dx++;
            break;
        case 3:
            dx++;
            dy++;
            break;
        case 4:
            dy++;
            break;
        case 5:
            dx--;
            dy++;
            break;
        case 6:
            dx--;
            break;
        case 7:
            dx--;
            dy--;
            break;
        }

        if (m_ParentRegion->isTileStopAble(dx, dy, GetGID()))
        {
            xTarget = dx;
            yTarget = dy;
            return true;
        }
    }

    return false;
}

BOOL CActiveObject::CheckAction(int iCA)
{
    return  TRUE;
}

void CActiveObject::Synchro(void *data, size_t size)
{
	if (!data)
	{
		rfalse(4, 1, "ActiveOjbect.cpp - Synchro() - !data");
		return;
	}

    if (!m_ParentArea || !m_ParentRegion)
        return;

    m_ParentArea->SendAdj((SMessage*)data, (WORD)size, INVALID_DNID);
}

bool CActiveObject::IsCanMovePostion( float x,float y )
{
	MY_ASSERT(m_ParentRegion);
	return m_ParentRegion->IsCanMove(x,y);
}

bool CActiveObject::Synchrostates( BYTE states )
{
	if(states>= EA_MAX)return false;
	SASynPosMsg	msg;
	msg.m_GID		= GetGID();
	msg.m_Action	= states;
	msg.m_X			= m_curX;
	msg.m_Y			= m_curY;
	msg.m_Z			= m_curZ;

	// ���켣ת������Ч������������
	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea)
		pArea->SendAdj(&msg, sizeof(SASynPosMsg), -1);

	m_NodeCount = m_CurNode = 0;
	memset(&m_NodeData, 0, sizeof(m_NodeData));

	SetCurActionID((EActionState)states);
	SetBackupActionID((EActionState)states);

	return TRUE;
}

void CActiveObject::syneffects(DWORD  dwgidatter, WORD weffectsindex, float feffectsPosX, float feffectsPosY)
{
	SASynEffectsMsg  aeffectsmsg;
	aeffectsmsg.dwgid = GetGID();
	aeffectsmsg.dwgidatter = dwgidatter;
	aeffectsmsg.weffectsindex = weffectsindex;
	aeffectsmsg.feffectPosX = feffectsPosX;
	aeffectsmsg.feffectPosY = feffectsPosY;


	CArea *pArea = (CArea*)m_ParentArea->DynamicCast(IID_AREA);
	if (pArea)
	{
		pArea->SendAdj(&aeffectsmsg, sizeof(SASynEffectsMsg), -1);
	}
}
