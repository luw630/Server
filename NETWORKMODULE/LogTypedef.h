#pragma once
#include "../pub/ConstValue.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// 数据定义原则
// 放在这里的都是会全局用到的数据结构
///////////////////////////////////////////////////////////////////////////////////////////////

// 玩家角色列表信息
struct SCharListData
{
    BYTE    m_bySex:1;					// 玩家角色性别
    BYTE    m_byBRON:3;					// 玩家所显示图片级别，1表示出师
    BYTE    m_bySchool:4;				// 玩家所属门派
	BYTE	m_FaceID;					// 脸模型
	BYTE	m_HairID;					// 头发模型
    char	m_szName[CONST_USERNAME];	// 玩家名称
    DWORD   m_dwStaticID;				// 数据库ID
    BYTE    m_byIndex;					// 角色索引
    WORD    m_wLevel;					// 玩家等级
	DWORD	m_w3DEquipID[13];			// 3D装备挂件物品号,包含现有的2件时装
	DWORD	m_wWeaponID;				// 武器号
	BYTE			bFashionMode;		//时装模式还是普通模式
	DWORD   dShowState;//玩家当前变身状态

};

// 客户端与登录服务器之间的创建角色的信息
// 为了节省网络流量，去除了多余的数据。
struct SC2LCreateFixProperty
{
	char    m_szName[CONST_USERNAME];	// 名字
	BYTE    m_bySex:1;					// 性别
	BYTE    m_byBRON:3;					// 玩家所显示图片级别，1表示出师
	BYTE    m_bySchool:4;				// 门派
	BYTE	m_FaceID;					// 脸模型
	BYTE	m_HairID;					// 头发模型
	DWORD	m_w3DEquipID[13];			// 3D装备挂件物品号
	DWORD   m_dwStaticID;				// 客户端到登陆服务器无意义
};

// 创建角色信息
struct SCreateFixProperty
{
    char    m_szName[CONST_USERNAME];	// 名字
    BYTE    m_byBRON:3;					// 玩家所显示图片级别，1表示出师
    BYTE    m_bySex:1;					// 性别
    BYTE    m_bySchool:4;				// 门派
	BYTE	m_FaceID;					// 脸模型
	BYTE	m_HairID;					// 头发模型
	DWORD   m_dwStaticID;
	WORD    m_wCurRegionID;				// 当前所在地图的ID
};

// GMLog信息
struct SGMLog
{
	char szAccount[MAX_ACCOUNT];
	WORD wOperType; // 操作类型，如改密码1 ，查在线时间2...
	char szOperObject[MAX_ACCOUNT]; //操作的对象，一般为玩家的名字，（某些操作可能是帐号）
	char szOperPara1[33];  //操作的参数1，可以为数字或者符号
	char szOperPara2[33];  //操作的参数2，可以为数字或者符号
	int  OperTime;         //操作的时间， TIME 为待定结构
};

