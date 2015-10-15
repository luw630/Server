#pragma once

#include "BaseObject.h"
#include "networkmodule\movetypedef.h"

typedef	struct _SWayNode
{
	int x;
	int y;
}SWayNode,*LPSWayNode;

const __int32 IID_ACTIVEOBJECT = 0x117c95d6; 

const DWORD TILETYPE_MOVE   = 0x01;			// �ر���ƶ�
const DWORD TILETYPE_JUMP   = 0x02;			// �ر����Ծ
const DWORD TILETYPE_LIMIT	= 0x04;			// ��������
const DWORD TILETYPE_STOP   = 0x10;			// �ر�������

const int MAX_MOVEOFFSET	= 2;			// ͬ���ƶ�������������
const int MSEL_UPDATE		= 200;			// ���ݸ��µ�ʱ����� 20��/��

typedef unsigned short ASHORT;

// ��Ҷ�����⣬�����ж��Ƿ������Ϣ�Ĵ���
enum E_CHECKACT
{
    ECA_CANWALK,
    ECA_CANRUN,
    ECA_CANFIGHT,
    ECA_CANWOUND,
    ECA_CANJUMP,
    ECA_CANSKILL,
	ECA_CANUSEITEM,
	ECA_CANWEAPON,		//�ɷ�ʹ������
	ECA_CANARMOUR,		//�ɷ�ʹ�÷���
	ECA_CANZAZEN,
	ECA_CANMUTATE,
	ECA_ISVITALS,		//�Ƿ񱻵�Ѩ

    MAX_CHECKACT
};

// ����״̬��⣬�����ж��ܷ������Ϣ����
enum CHECKSTATUS
{
	CST_CANMOVE,		// �ɷ��ƶ�
	CST_CANJUMP,		// �ɷ���Ծ
	CST_CANUSTITEM,		// �ɷ�ʹ����Ʒ
	CST_CANSTALL,		// �ɷ��̯
	CST_CANATTACK,		// �ɷ񹥻�
	CST_CANSKILLATK,	// �ɷ�ʹ�ü��ܹ���
	CST_WUDI,			// �Ƿ��޵�״̬
	CST_ADDBUFF,			// �Ƿ��ֹĿ�����BUFF
	CST_MAX,
};

// ��Ҷ���
enum EActionState
{
	EA_STAND = 0,
	EA_WALK,
	EA_RUN,
	EA_JUMP,
	EA_ZAZEN,
	EA_ATTACK,	
	EA_WOUND,
	EA_DEAD,
	EA_SKILL_ATTACK,		// �������磺���߼�ʹ�õĹ���״̬
	EA_USEITEM,				// ������һ����������Ʒ����
	EA_COLLECT,				// �ɼ�
	EA_DIZZY,						//ѣ��
	EA_REPEL,						// ���ˣ��ܿ�
	EA_MAX,	
};

const int TILE_BITW = 5;
const int TILE_BITH = 5; 

const int TILE_WIDTH	= 32;
const int TILE_HEIGHT	= 32;

// ����ƶ��Ļ����ٶȣ�1���ƶ�360������
#define  SRC_MOVE_SPEED (((float)(380.0f / 1000.0f)))
#define  BASIC_RUN_SPEED (((float)450) / 1000)

const int PLAYER_BASIC_SPEED = 450;

// ������Ϊ����Ĳ���
class CActiveObject : public IObject
{
public:
	int AddObject(LPIObject pChild) { return 1; }
	int DelObject(LPIObject pChild) { return 1; }
	void OnClose() {}

protected:
	int OnCreate(_W64 long pParameter) { return 1; }
	void OnRun(void);

public:
	// ��һ��tick����ʱ��ǰ״̬��ʱ�䡣
	// ��ʱ�䵽�˾͵��ô˺�������������һЩ����
    virtual BOOL EndPrevAction();

	// �����������һ��״̬����ô����һ��״̬��ʼ����ǰ��״̬
	void BackupActionID2CurActionID();

	 // ���õ�ǰ��״̬
    virtual BOOL SetCurAction();   

	// ִ�е�ǰ��״̬
    virtual BOOL DoCurAction();   

	// ��鵱ǰ�Ķ���
    virtual BOOL CheckAction(int iCA);

	virtual void OnRunEnd() { return; }

public:
    BOOL CheckDist(CActiveObject *pDest, BYTE byDist,BOOL bCheckWay,int x=-1,int y=-1);
    bool CheckJump(int x,int y,int x1,int y1,BOOL bCheckEndPos);
	BOOL CheckPath(struct SQSynWayTrackMsg *pMsg);
	
public:
    bool SetJump(struct SQASynWayJumpMsg *pMsg);

	// �ö������STAND��״̬
	bool Stand();
    bool SetZazen( bool bZazen );
	bool Synchrostates(BYTE states);
	bool Stand(EActionState eaction);

    void SendErrorMsg( BYTE byMsgId );
    void SendMsg( LPVOID msg, size_t size );

	void FillSynPos(struct SPath *pSSP);
	void GetCurPos(WORD &x, WORD &y);
	void GetOffPos(WORD &x, WORD &y, char len = -1);
    void GetMarkOffPos(WORD &x, WORD &y);
	void ClearMarkStop();

    void SendActSpeed(void);				// �㲥�����ݾ������ٶ� SAActSpeedChangeMsg

    void SetWalkSpeedCoef(float speedCoef)     
	{   
		m_fWalkSpeedCoef = speedCoef;          
	}

    void Synchro( void *data, size_t size );

	void AddUpSpeedProficiency( int nMoveTiles );

	void SendBuffFightOutPos(WORD endx,WORD endy,DWORD objID);	// �㲥���󱻻��˵���Ϣ

	/************************************************************************/
	/* �µ��ƶ�����															*/
	/************************************************************************/
	
	// ����·��
	bool SetWay(struct SQSynPathMsg *pMsg);

	// ����·��
	BOOL StorePathXYSJ(struct SQSynWayTrackMsg* pMsg);
	BOOL StorePath(struct SQSynPathMsg *pMsg);

	// ��track��·��ת����node���͵�·��
	void ConverWayTrack2WayNode(SSynWay* pSynWay,SWayNode* pNodeTemp);

	BOOL UpdateWalking();
	void InitPlayerMove(struct SQSynPathMsg *pMsg);
	
	//chenj �ɼ�
	virtual bool UpdateCollect();

	// ��ǰ��·��ָʾ��
	INT32	m_byCurWayIndex;
	INT32	mCurDesNode;
	INT32	mNodeCount;
	INT32	mStartMoveTime;
	INT32	mElapsed;
	float	mMoveSpeed;

	// �˶��켣�ڵ�
	struct 
	{
		SWayNode mNodeData[MAX_TRACK_LENGTH];	
		
		// �����ܴ���MAX_TRACK_SIZE��Tile���ƶ����ݣ�����ǰ�ڵ��len��0��ʱ�򣬱�ʾ�ƶ�����
		SWayTrack m_WayTrack[MAX_TRACK_LENGTH + 1];
	}mMoveNode;

	// �ƶ�����ʱ��ʱ��
	DWORD	m_dwMoveSegment;   

	// ÿ�ƶ�һ���ڵ����ķѵ�ʱ�䣨��Ӧ�ڿͻ��ˣ�
	float	m_fMovePerStep;    

	// ��ǰ���ƶ�����
	BYTE	m_byCurStep;    
	BYTE	m_IsMove2Attack;	// �Ƿ��ƶ�ȥ����
	// �����3���ƶ�����
	WORD		m_curTileX;						// ��ǰ�ĸ�������
	WORD		m_curTileY;						// 
	float		m_curX;							// ��ǰ��X������
 	float		m_curY;							// ��ǰ��Y������
 	float		m_curZ;							// ��ǰ��Z������
 	float		m_Direction;					// ��ǰ�ķ���
	WORD		m_OriSpeed;						// ԭʼ�����ٶ�
 	float		m_Speed;						// ��ǰ���ƶ��ٶ�
 	DWORD		m_MoveStartTime;				// �ƶ�����ʱ��
 	DWORD		m_NodeCount;					// ·����ĸ���
 	DWORD		m_CurNode;						// ��ǰ����Ľڵ�
 	DWORD		m_Elapsed;						// ������ʱ��
	bool		m_isRunEnd;						// ���ڱ���һ���ƶ��Ƿ����
 	SWayPoint	m_NodeData[MAX_TRACK_LENGTH];	// �����·����

public:
	CActiveObject(void);
	~CActiveObject(void);

public:
    // ������Ծ������
	BYTE			m_byJumpMaxTile;				// �����Ծ�ĸ�����
    BYTE            m_byJumpFrame;					// ��Ծ�ƶ�·���Ľ���
    DWORD           m_dwJumpSegment;				// ��Ծ����ʱ��ʱ��
    float           m_fJumpPerStep;					// ÿ��Ծһ���ڵ����ķѵ�ʱ�䣨��Ӧ�ڿͻ��ˣ�

	// ����������Ϊ������
    //!8Test
     float			m_fActionFrame;					// �ö������Ϊ���ʱ�䣬һ��������ѭ��FrameΪ��λ
     float			m_fActFrameDec;					// �����ݾ������ٶȣ�0.500 - 2.000��
     float			m_fAttFrameDec;					// �����ٶȵݼ�ֵ

	EActionState	m_CurAction;					// ��ǰ�Ķ�����Ϊ
	BYTE		    m_byActionBak;					// ����ǵ�ǰ��������������������

	void SetBackupActionID(INT32 newBackupActionID);

	inline INT32 GetBackupActionID()
	{
		return m_byActionBak;
	}

	void SetCurActionID(EActionState newActionID);

	inline EActionState GetCurActionID()
	{
		return m_CurAction;
	}

	virtual void OnChangeState(EActionState newActionID);

	// �����ƶ��Ĳο�����
	BYTE		m_bySendOver;					// �����������ÿһ��Tile�ƶ���ֻ����һ����Ϣ��Ч

    //Ŀǰ·�����յ�
    WORD        m_wEndX;
    WORD        m_wEndY;
    DWORD       m_PrevMark;    
	BYTE        m_byDir;						// ��ǰ��Ҫǰ���ķ���

public:
	class CRegion	*m_ParentRegion;			// �����ĳ���
	class CArea		*m_ParentArea;				// ����������

    // ·����Ѱ
    // ���� <  0 :û���ҵ����ʵ�·��
    //     >= 0 :�ҵ����ʵ�·��
    int  FindPath(ASHORT xend, ASHORT yend,SQSynWayTrackMsg &WayMsg);
	void MakeRadomPath(WORD wMoveArea, const WORD wSrcX, const WORD wSrcY, int &iXEnd, int &iYEnd);
    bool GetStopAblePos(WORD &xTarget, WORD &yTarget);

	// �����3
	bool SetMovePath(SQSynPathMsg *pWayMsg);
	void MakeRadomPath(DWORD moveArea, float srcX, float srcY, SQSynPathMsg &msg);

	virtual bool IsCanMovePostion(float x,float y);//�ܷ��ƶ���Ŀ���

	void syneffects(DWORD  dwgidatter,WORD weffectsindex, float feffectsPosX, float feffectsPosY);//����һЩ��Ч��ͬ��
private:
    WORD        m_wIdleTime;
    BOOL        m_bStopToStand;
    float       m_fWalkSpeedCoef;			// �ٶ����ϵ��������>1.0��

public:
	bool		m_bPlayerAction[CST_MAX];	// ���������Ϊ
};
