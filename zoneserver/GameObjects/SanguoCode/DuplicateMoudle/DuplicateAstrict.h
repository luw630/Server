// ----- CDuplicateAstrict.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/28
//   --  Desc:   萌斗三国的副本的限制判断
// --------------------------------------------------------------------
//   --  管理某一个副本是否可以挑战的逻辑判断，主要管理某一段时间的挑战次数是否达到了上限
//---------------------------------------------------------------------   
#pragma once

class CBaseDataManager;
struct ChapterConfig;
enum CheckResult;

class CDuplicateAstrict
{
public:
	CDuplicateAstrict();
	~CDuplicateAstrict();

	///@brief 初始化某一个副本的限制信息
	void Init(const ChapterConfig* config);
	///@brief 更新挑战的CD时间，暂时无用
	void UpdateCD();
	///@brief 挑战成功后的一些结算操作
	void Resolves(int clearTime = 1);
	///@brief 开始挑战CD的计时，暂时无用
	void Action();
	///@brief 检查是否可以挑战
	CheckResult Check(const CBaseDataManager& dataManger, int clearTime = 1);

	///@brief 获取当前已挑战的次数
	int GetCurChallengeNum();

	///@brief 设置当前的票数
	void SetCurChallengeNum(int num);
	///@brief 设置当前的挑战次数
	void ReSetCurTicketNum(int num);

private:
	int m_iOneDayChanllengeCount;   //每日可挑战的次数 <0表示无限制//
	int m_iChanllengedTimes;   //已被挑战的次数//
	float m_fCurCD;         //下次可挑战的CD,暂时无用//
};

