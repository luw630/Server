#pragma once

#include "BaseObject.h"
#include "networkmodule\movetypedef.h"

typedef	struct _SWayNode
{
	int x;
	int y;
}SWayNode,*LPSWayNode;

const __int32 IID_ACTIVEOBJECT = 0x117c95d6; 

const DWORD TILETYPE_MOVE   = 0x01;			// 地表可移动
const DWORD TILETYPE_JUMP   = 0x02;			// 地表可跳跃
const DWORD TILETYPE_LIMIT	= 0x04;			// 建筑限制
const DWORD TILETYPE_STOP   = 0x10;			// 地表上有人

const int MAX_MOVEOFFSET	= 2;			// 同步移动所容许的最长距离
const int MSEL_UPDATE		= 200;			// 数据更新的时间节奏 20次/秒

typedef unsigned short ASHORT;

// 玩家动作检测，用于判断是否接受消息的处理
enum E_CHECKACT
{
    ECA_CANWALK,
    ECA_CANRUN,
    ECA_CANFIGHT,
    ECA_CANWOUND,
    ECA_CANJUMP,
    ECA_CANSKILL,
	ECA_CANUSEITEM,
	ECA_CANWEAPON,		//可否使用武器
	ECA_CANARMOUR,		//可否使用防具
	ECA_CANZAZEN,
	ECA_CANMUTATE,
	ECA_ISVITALS,		//是否被点穴

    MAX_CHECKACT
};

// 生物状态检测，用于判断能否接受消息处理
enum CHECKSTATUS
{
	CST_CANMOVE,		// 可否移动
	CST_CANJUMP,		// 可否跳跃
	CST_CANUSTITEM,		// 可否使用物品
	CST_CANSTALL,		// 可否摆摊
	CST_CANATTACK,		// 可否攻击
	CST_CANSKILLATK,	// 可否使用技能攻击
	CST_WUDI,			// 是否无敌状态
	CST_ADDBUFF,			// 是否禁止目标添加BUFF
	CST_MAX,
};

// 玩家动作
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
	EA_SKILL_ATTACK,		// 侠义世界：新逻辑使用的攻击状态
	EA_USEITEM,				// 新增加一个用来做物品处理
	EA_COLLECT,				// 采集
	EA_DIZZY,						//眩晕
	EA_REPEL,						// 击退，受控
	EA_MAX,	
};

const int TILE_BITW = 5;
const int TILE_BITH = 5; 

const int TILE_WIDTH	= 32;
const int TILE_HEIGHT	= 32;

// 玩家移动的基本速度，1秒移动360个像素
#define  SRC_MOVE_SPEED (((float)(380.0f / 1000.0f)))
#define  BASIC_RUN_SPEED (((float)450) / 1000)

const int PLAYER_BASIC_SPEED = 450;

// 控制行为节奏的部分
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
	// 有一个tick来计时当前状态的时间。
	// 当时间到了就调用此函数，做结束的一些操作
    virtual BOOL EndPrevAction();

	// 如果设置了下一个状态，那么用下一个状态初始化当前的状态
	void BackupActionID2CurActionID();

	 // 设置当前的状态
    virtual BOOL SetCurAction();   

	// 执行当前的状态
    virtual BOOL DoCurAction();   

	// 检查当前的动作
    virtual BOOL CheckAction(int iCA);

	virtual void OnRunEnd() { return; }

public:
    BOOL CheckDist(CActiveObject *pDest, BYTE byDist,BOOL bCheckWay,int x=-1,int y=-1);
    bool CheckJump(int x,int y,int x1,int y1,BOOL bCheckEndPos);
	BOOL CheckPath(struct SQSynWayTrackMsg *pMsg);
	
public:
    bool SetJump(struct SQASynWayJumpMsg *pMsg);

	// 让对象进入STAND的状态
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

    void SendActSpeed(void);				// 广播由敏捷决定的速度 SAActSpeedChangeMsg

    void SetWalkSpeedCoef(float speedCoef)     
	{   
		m_fWalkSpeedCoef = speedCoef;          
	}

    void Synchro( void *data, size_t size );

	void AddUpSpeedProficiency( int nMoveTiles );

	void SendBuffFightOutPos(WORD endx,WORD endy,DWORD objID);	// 广播对象被击退的消息

	/************************************************************************/
	/* 新的移动代码															*/
	/************************************************************************/
	
	// 设置路径
	bool SetWay(struct SQSynPathMsg *pMsg);

	// 保存路径
	BOOL StorePathXYSJ(struct SQSynWayTrackMsg* pMsg);
	BOOL StorePath(struct SQSynPathMsg *pMsg);

	// 把track的路径转换成node类型的路径
	void ConverWayTrack2WayNode(SSynWay* pSynWay,SWayNode* pNodeTemp);

	BOOL UpdateWalking();
	void InitPlayerMove(struct SQSynPathMsg *pMsg);
	
	//chenj 采集
	virtual bool UpdateCollect();

	// 当前的路径指示点
	INT32	m_byCurWayIndex;
	INT32	mCurDesNode;
	INT32	mNodeCount;
	INT32	mStartMoveTime;
	INT32	mElapsed;
	float	mMoveSpeed;

	// 运动轨迹节点
	struct 
	{
		SWayNode mNodeData[MAX_TRACK_LENGTH];	
		
		// 最坏情况能储存MAX_TRACK_SIZE格Tile的移动数据，当当前节点的len是0的时候，表示移动结束
		SWayTrack m_WayTrack[MAX_TRACK_LENGTH + 1];
	}mMoveNode;

	// 移动启动时的时间
	DWORD	m_dwMoveSegment;   

	// 每移动一个节点所耗费的时间（对应于客户端）
	float	m_fMovePerStep;    

	// 当前的移动步数
	BYTE	m_byCurStep;    
	BYTE	m_IsMove2Attack;	// 是否移动去攻击
	// 侠义道3的移动数据
	WORD		m_curTileX;						// 当前的格子坐标
	WORD		m_curTileY;						// 
	float		m_curX;							// 当前的X轴坐标
 	float		m_curY;							// 当前的Y轴坐标
 	float		m_curZ;							// 当前的Z轴坐标
 	float		m_Direction;					// 当前的方向
	WORD		m_OriSpeed;						// 原始整形速度
 	float		m_Speed;						// 当前的移动速度
 	DWORD		m_MoveStartTime;				// 移动启动时间
 	DWORD		m_NodeCount;					// 路径点的个数
 	DWORD		m_CurNode;						// 当前处理的节点
 	DWORD		m_Elapsed;						// 滑过的时间
	bool		m_isRunEnd;						// 用于表明一次移动是否完成
 	SWayPoint	m_NodeData[MAX_TRACK_LENGTH];	// 保存的路径点

public:
	CActiveObject(void);
	~CActiveObject(void);

public:
    // 用于跳跃的数据
	BYTE			m_byJumpMaxTile;				// 最大跳跃的格子数
    BYTE            m_byJumpFrame;					// 跳跃移动路径的节奏
    DWORD           m_dwJumpSegment;				// 跳跃启动时的时间
    float           m_fJumpPerStep;					// 每跳跃一个节点所耗费的时间（对应于客户端）

	// 用于所有行为的数据
    //!8Test
     float			m_fActionFrame;					// 该对象的行为完成时间，一般是以主循环Frame为单位
     float			m_fActFrameDec;					// 由敏捷决定的速度（0.500 - 2.000）
     float			m_fAttFrameDec;					// 攻击速度递减值

	EActionState	m_CurAction;					// 当前的对象行为
	BYTE		    m_byActionBak;					// 这个是当前动作结束后用来调整的

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

	// 用于移动的参考数据
	BYTE		m_bySendOver;					// 这个用来限制每一个Tile移动中只能有一个消息有效

    //目前路径的终点
    WORD        m_wEndX;
    WORD        m_wEndY;
    DWORD       m_PrevMark;    
	BYTE        m_byDir;						// 当前需要前进的方向

public:
	class CRegion	*m_ParentRegion;			// 所属的场景
	class CArea		*m_ParentArea;				// 所属的区块

    // 路径搜寻
    // 返回 <  0 :没有找到合适的路径
    //     >= 0 :找到合适的路径
    int  FindPath(ASHORT xend, ASHORT yend,SQSynWayTrackMsg &WayMsg);
	void MakeRadomPath(WORD wMoveArea, const WORD wSrcX, const WORD wSrcY, int &iXEnd, int &iYEnd);
    bool GetStopAblePos(WORD &xTarget, WORD &yTarget);

	// 侠义道3
	bool SetMovePath(SQSynPathMsg *pWayMsg);
	void MakeRadomPath(DWORD moveArea, float srcX, float srcY, SQSynPathMsg &msg);

	virtual bool IsCanMovePostion(float x,float y);//能否移动到目标点

	void syneffects(DWORD  dwgidatter,WORD weffectsindex, float feffectsPosX, float feffectsPosY);//处理一些特效的同步
private:
    WORD        m_wIdleTime;
    BOOL        m_bStopToStand;
    float       m_fWalkSpeedCoef;			// 速度提高系数（高速>1.0）

public:
	bool		m_bPlayerAction[CST_MAX];	// 玩家限制行为
};
