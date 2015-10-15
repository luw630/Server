#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////
// 数据定义原则
// 放在这里的都是会全局用到的数据结构
///////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_TRACK_LENGTH        128

#define MY_WALK_FRAMESKIP       8
#define MY_RUN_FRAMESKIP        5
#define MY_JUMP_FRAMESKIP       3   // 相当于客户端在空中飞过的3贞
#define MY_FLY_FRAMESKIP        4

// comment by chenj 最大跳跃距离不再是固定的了
//const   int     MAX_JUMPOFFSETX = 10;       // 跳跃的最长距离（半个屏幕）x
//const   int     MAX_JUMPOFFSETY = 13;       // 跳跃的最长距离（半个屏幕）y

// 路径节点
struct SWayTrack
{
    WORD    dir:3;  // 该节点方向
    WORD    len:13;  // 本方向上的距离
};

// 用于网络传递的路径节点（按照迪卡尔坐标）
struct  SNWayNode 
{
    WORD    x;
    WORD    y;
};

// 同步移动路径：
struct SSynWay
{
    WORD wSegX, wSegY;                      // 移动起点，以客户端的当前位置为准，如果是站立状态，为当前主角坐标，如果是移动状态，为主角将要移动到的下一个点为坐标
    BYTE byAction;                          // 行为方式，决定当前的移动方式以及标准速度
    SWayTrack Track[MAX_TRACK_LENGTH];      // 移动路径，以移动起点决定的移动路径，包含了方向和长度
};

// 侠义道3路径点结构
struct SWayPoint
{
	float	m_X;							// 路径的X坐标
	float	m_Y;							// 路径的Y坐标
};

// 侠义道3同步移动路径结构
// 为了优化网络的数据流量，不要每次都发送整个结构（包含MAX_TRACK_LENGTH个路径点信息）
// 而是应该发送最多m_wayPoints个SWayPoint结构就行了。在定义同步移动消息的时候，SPath
// 结构成员务必是最后一个成员。
struct SPath
{
	WORD		m_wMapID;					// 场景地图编号
	BYTE		m_wayPoints;				// 移动的路径点个数
	BYTE		m_moveType;					// 移动的方式（走/跑），决定移动的速度
	SWayPoint	m_path[MAX_TRACK_LENGTH];	// 移动路径，包含了路径上的每一个路径点
};											// （第一个点为起点：如果是站立状态，为当前主角坐标，如果是移动状态，为主角将要移动到的下一个点为坐标）

///////////////////////////////////////////////////////////////////////////////////////////////
// 玩家衣服的颜色信息
// 衣服列表（外衣，内衣，腰带，鞋子）
enum E_EQUIP_COL 
{
    EEC_FROCK = 0,
    EEC_SHIRT,
    EEC_GIRDLE,
    EEC_SHOES,
    //...
    EEC_MAX
};
