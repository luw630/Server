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
/*�������ݿ�õ���������а���Ϣ���뵽һ��map���ͣ�KEY�����йؼ��֣�Ҳ���ǵȼ�����Ǯ��BOSS��ɱ��
 *�ǽ������У�Ȼ��Ѹ��������Ľ����������뵽Rank4Client���棬���ݿͻ��˵����󣬰��跢�͡�
 *--------------------------------------------------------------------------------
 *���ͻ��˵�����а�ť�������а�˵�ʱ�����ʱ����Ҫ������Ϣ��RegionServer�����ʱ�������
 *��DB����ȡ���ݡ�Ȼ����ݿͻ��˵����ѡ�ͬ��������Ӧ�����а����ݡ����һ�ٸ���
 *--------------------------------------------------------------------------------
 *������Ժ���һ�����������Ǵ����ݿ������ȡ������ݵ�ʱ�򣬱�֤��������������������ȡ��
 *���ԣ������������棬���Ӧ�ý�������ȡ��
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
	
	void Initialized();											// ������������Ҫ�ĳ�ʼ����Ϣ
	bool CheckTime(INT64 CurTime);								// ���ʱ�䣬��ʱ����	

	void SendToDB_2_GetRankData(const char *pName);				// ��DB�������а�����

	void GetRankData_From_ZoneServer(PLAYERLIST playerList);	// ��ȡ��ǰ��������������а�����
	void GetRankData_From_DB(NewRankList* pRankList, int num);	// ��ȡDB������а�����

	bool ProcessTheRankList();									// �������а�����
	void GetTheRankList(CPlayer *player);

	void SetRankCondition(bool isGet) { IsGetNewRank = isGet;}
	bool GetRankCondition() const { return IsGetNewRank; }
	bool CanProcessTheRankList() { return IsGetTheRawDataFromDB && IsGetCurSvrData && !IsProcess; }

protected:
	CSingleRank() {}
	~CSingleRank() {}

	int  GetTheLevelLimitFromScript();			// �ӽű��еõ��ϰ�ȼ�����
	void RankList_4_Client(CPlayer* player, RankMap &rankDataMap, BYTE rankType);		// ���а�����ɸѡ������

private:
	NewRankList RawRankList[ALL_RANK_NUM];		// ���ڱ����DBȡ����ԭʼ���а�����
	NewRankList *pCurSvrRankList;				// ���浱ǰ��������ҵ����а�����,һֱ������REGION SERVER��һ�������Բ��ÿ��Բ���DELETE
	
	int			DBNum;							// ���ݿ���ȡ������������
	int			SvrNum;							// ��ǰ������ȡ������������

	INT64		oldTime;						// ����ʱ���ж�	

	bool		IsGetTheRawDataFromDB;			// �Ƿ��Ѿ��õ�DB�����а�����
	bool		IsGetCurSvrData;				// �Ƿ��Ѿ���õ�ǰ����������
	bool		IsProcess;						// �Ƿ��Ѿ����������
	bool		IsGetNewRank;					// �Ƿ��Ѿ�������а�����
	//bool		IzUpdate;						// �Ƿ����
};