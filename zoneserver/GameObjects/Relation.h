//--------------------------------------//
//Dreamwork soft Ltd.					//
//Luou July 22nd 2004					//
//--------------------------------------//
//好友功能说明：
//好友关系分为一对一的关系，既双方互为好友
//和一对多的关系，即某人单方面是另外一人的好友，而对方并不是他的好友
//《侠义道》中的好友上线通知在一对一的好友间互发，而一对多的好友间则不用
// 好友下线没有提示
//保存时，根据 SPlayerRelates::byRelation 来判断该名字玩家与自己的关系，具体数值的意义参见相应注释
//m_szFriendNameList为游戏过程中用以查询好友map，可以提供快捷并方便的查询，并避免指针越界等情况


#pragma once 
#include "networkmodule/PlayerTypedef.h"
#include "NETWORKMODULE/HeroTypeDef.h"
#include "BaseObject.h"
#include <list>
#include <map>
#include "NETWORKMODULE/RelationMsgs.h"

//----------------------------------
#include <set>
#include <string>



//-----------------------------------------------------
class CRelations 
{
public:
	struct tRelationData
	{
		dwt::stringkey<char[CONST_USERNAME]> sName;
		dwt::stringkey<char[CONST_USERNAME]> sHeadIcon;
		WORD					  wLevle;				//等级
		tRelationData::tRelationData():wLevle(0){}
	};


public:

//新好友列表
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_mapFriendList;	//好友
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_mapEnemieList;//仇人
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_mapMateList;// 结拜
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_mapBlockList;// 黑名单
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_mapTempList;//临时好友

	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_FriendListArr[5];


	inline size_t GetFriendNum() {return m_mapFriendList.size();};
	inline size_t GetBlockNum()  {return m_mapBlockList.size();};
	inline size_t GetEnemiesNum(){return m_mapEnemieList.size();};
	inline size_t GetMateNum()   {return m_mapMateList.size();};
	inline size_t GetTempNum()   {return m_mapTempList.size();};

    //byRealtion 
    //0 -- 好友
	//1 -- 黑名单
    //2 -- 仇人名单
    //3 -- 结拜名单
	BOOL AddRelation_new(BYTE byRelation,tRelationData relationdata,BOOL bSend=TRUE);//添加关系
	BOOL AddRelation_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation, BOOL bSend = TRUE, int nDegree = 1);//添加关系

	BOOL RemoveRelation_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation, BOOL bSend = TRUE);//取消关系

	BOOL FindRelation_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation);

	BOOL IncHateDegree(DNID dnidClient, dwt::stringkey<char[CONST_USERNAME]> strName); // 加1点仇恨值
	BOOL DecHateDegree(DNID dnidClient, dwt::stringkey<char[CONST_USERNAME]> strName); // 减1点仇恨值

	BOOL ChangeDegree(DNID dnidClient, dwt::stringkey<char[CONST_USERNAME]> strName, int &nDegree); // 改变亲密度
	


    int Init(SFixProperty *pData);// 初始化人际关系列表
	int Init(SFixData *pData);
	int GetFriends(char szFriends[MAX_FRIEND_NUMBER][CONST_USERNAME]); // 返回组员的数量
	int GetFriends_new(char szFriends[MAX_PEOPLE_NUMBER][CONST_USERNAME]); // 返回组员的数量
    //int GetBlocks(char ** szBlocks);  // 返回黑名单的数量
    //int GetMates(char ** szMates);   // 返回结拜的数量

    // 好友上线通知
	void FriendsOnline(struct SFriendOnlineMsg  *pFriendOnlineMsg);
	void FriendsOnline_new(struct SFriendOnlineMsg  *pFriendOnlineMsg,BYTE byRelation = RT_FRIEND);

	// 同步所有的好友在线状态（包含黑名单仇人等）
	 void RefreshFriends_new();
    // 同步所有关系的数据

	void SynAllRelations_new(DNID dnidClient);

    // 定时刷新好友的状态和增加亲密度

	int GetFriendDegree_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation = 0);
    // 通过一个结构，设置玩家的关系列表
	BOOL SetPlayerRelations_new(SPlayerRelation *lpPR);
    // 将玩家的关系列表保存到一个结构中

	BOOL SavePlayerRelations_new(SPlayerRelation *lpPR);
	///获取在线好友个数
	int GetOnlineFriendsNum();

	BOOL ProcessRelationMsg(DNID dnidClient, struct SRelationBaseMsg *pRelationMsg, CPlayer *pPlayer);
	BOOL OnRecvAddFriendMsg(DNID dnidClient, struct SQAddFriendMsg *pRelationMsg, CPlayer *pPlayer);
	BOOL OnInitFriendData(CPlayer *pPlayer);
public:
    CRelations();
    ~CRelations();
};
