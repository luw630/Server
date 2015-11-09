// ------ EnduranceManager.h -----
//	--	 Author : LiuWeiWei	  --
//	--	 Date : 03/18/2015    --
//	--	 Desc : 体力回复管理类 --
// -------------------------------

#pragma once
#include "stdint.h"

struct SEnduranceData;

class CEnduranceManager
{
public:
	CEnduranceManager();
	~CEnduranceManager();
	/**@brief 初始化体力数据
	@param [in] 体力数据
	@param [in] bGMMark GM玩家标志
	*/
	void Init(SEnduranceData* pData, bool bGMMark);
	/**@brief 计算体力上线后的体力数据
	@param [in] newPlayer 新玩家标志
	@param [in] playerLevel 玩家等级
	@param [in] sid 玩家SID
	*/
	void InitEnduranceData(bool newPlayer, const int32_t playerLevel, const DWORD sid);
	/**@brief 添加体力值
	@param [in] value 添加的体力值 可为负数 为负则体力取与0比较的最大值
	@param [in] playerLevel 玩家等级
	@param [in] globalLimmitted 是否用全局配置的体力上限来限制体力，为false就用君主等级所对应的体力限制来限制君主的体力
	*/
	void PlusEndurance(const int32_t value, const int32_t playerLevel, bool globalLimit = true);
	/**@brief 检测体力值
	@note 先计算应该回复的体力值
	@param [in] playerLevel 玩家等级,如果是GM号则体力最大值为全局配置的体力上限,否则为对应君主等级的体力上限
	@param [in] compareValue 体力比较值
	@return 如果计算后的体力值大于比较值则返回true 否则false
	*/
	bool Check(const int32_t playerLevel, int32_t compareValue = 0);
	/**@brief 获取体力值
	@note 调用此接口不会自动计算回复的体力值
	*/
	DWORD GetEndurance() const;
	/**@brief 获取体力上限
	@param [in] playerLevel 玩家等级
	@param [in] globalLimit 是否为全局上限
	*/
	int32_t GetEnduranceLimit(const int32_t playerLevel, bool globalLimit) const;
	/**@升级VIP时更新体力上限值
	@param [in] sid 玩家SID
	@param [in] playerLevel 玩家君主等级
	*/
	void UpdateEnduranceForVIP(const DWORD sid, const int32_t playerLevel);
private:
	/**@brief 计算此时距离上次计算所需回复的体力值
	@return 计算后可回复的体力值
	*/
	int32_t Recover();
private:
	SEnduranceData* m_pEnduranceData;	///<体力数据/
	__time64_t m_tLastCheckRecoverTime;		///<上一次检测体力回复的时间/
	__time64_t m_tCurTime;				///<用于获取当前时间
	int32_t m_iRecoverSeconds;			///<体力回复所需秒数/
	int32_t m_iRecoverPoint;			///<当前回复的体力点/
	int32_t m_iEnduranceLimit;			///<体力上限/
	int32_t m_iExEnduranceLimit;		///<VIP扩展体力上限
	bool m_bGMMark;						///<GM玩家标志/
	//bool m_bInitFinished;				///<初始化完成标志/
};

