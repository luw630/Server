#include <time.h>

#include <iterator>
#include <string>
#include <vector>
#include <map>

//#include "BaseObject.h"
#include "networkmodule/playertypedef.h"
#include "Player.h"

using std::map;
using std::string;
using std::vector;

typedef hash_map<string, NewRankList>	RawRankMap;
typedef vector<const CPlayer*>			PLAYERLIST;
typedef vector<const CPlayer*>::iterator PLAYERLIST_ITERATOR;

/********************************************************************************/
/*将从数据库得到的玩家排行榜信息存入到一个map类型，KEY是排行关键字，也就是等级、金钱与BOSS击杀数
 *非降序排列，然后把根据排序后的结果，将其存入到Rank4Client里面，根据客户端的请求，按需发送。
 *--------------------------------------------------------------------------------
 *当客户端点击排行榜按钮弹出排行榜菜单时，这个时候需要发送消息给RegionServer，这个时候服务器
 *从DB中拉取数据。然后根据客户端点击的选项不同，发送相应的排行榜数据。最多一百个。
 *--------------------------------------------------------------------------------
 *这里可以忽略一个条件，就是从数据库里面读取玩家数据的时候，保证将满足条件的所有数据取出
 *所以，如果玩家在里面，则就应该将其排名取出
 */
/********************************************************************************/

class CSingleRank
{
public:
	static CSingleRank& GetInstance()
	{
		static CSingleRank rank;
		return rank;
	}
	
	void Initialized();											// 服务器开启必要的初始化信息
	bool CheckTime(INT64 CurTime);								// 检测时间，定时更新	

	void SendToDB_2_GetRankData(const char *pName);				// 向DB请求排行榜数据

	void GetRankData_From_ZoneServer(PLAYERLIST playerList);	// 获取当前服务器的玩家排行榜数据
	void GetRankData_From_DB(NewRankList* pRankList, int num);	// 获取DB玩家排行榜数据

	bool ProcessTheRankList();									// 处理排行榜数据
	void GetTheRankList(CPlayer *player);

	void SetRankCondition(bool isGet) { IsGetNewRank = isGet;}
	bool GetRankCondition() const { return IsGetNewRank; }
	bool CanProcessTheRankList() { return IsGetTheRawDataFromDB && IsGetCurSvrData && !IsProcess; }

protected:
	CSingleRank() {}
	~CSingleRank() {}

	int  GetTheLevelLimitFromScript();			// 从脚本中得到上榜等级限制
	void RankList_4_Client(CPlayer* player, RankMap &rankDataMap, BYTE rankType);		// 排行榜数据筛选与排序

private:
	NewRankList RawRankList[ALL_RANK_NUM];		// 用于保存从DB取出的原始排行榜数据
	NewRankList *pCurSvrRankList;				// 保存当前服务器玩家的排行榜数据,一直伴随着REGION SERVER的一生，所以不用可以不用DELETE
	
	int			DBNum;							// 数据库里取出的数据条数
	int			SvrNum;							// 当前服务器取出的数据条数

	INT64		oldTime;						// 用于时间判定	

	bool		IsGetTheRawDataFromDB;			// 是否已经得到DB的排行榜数据
	bool		IsGetCurSvrData;				// 是否已经获得当前服务器数据
	bool		IsProcess;						// 是否已经处理过数据
	bool		IsGetNewRank;					// 是否已经获得排行榜数据
	//bool		IzUpdate;						// 是否更新
};