#pragma once

//轻功数据表
class CFlyDataTable
{
public:
	enum {
		N_MAX_NAME_LEN = 128,
		N_MAX_INFOR_LEN = 1024,
	};

	enum{
		MaxLevel,					//最大等级
		DiffcultBase,				//难度基数
		DiffcultFactor,				//难度系数
		RemainTimeBase,				//持续时间基数
		RemainTimeFactor,			//持续时间系数
		JumpTileBase,				//跳跃格数基数
		UpSpeedBase,				//移动速度提升基数
		UpSpeedFactor,				//移动速度提升系数
		AGBase,						//增加身法基数
		AGFactor,					//增加身法系数
		ReduceCDBase,				//降低公共冷却时间基数
		ReduceCDFactor,				//降低公共冷却时间系数
		ReduceDelayBase,			//降低冷却时间基数
		ReduceDelayFactor,			//降低冷却时间系数
		AbsoluteParryBase,			//增加绝对躲闪基数
		AbsoluteParryFactor,		//增加绝对躲闪系数
		ConsumHPBase,				//消耗生命值基数
		ConsumHPFactor,				//消耗生命值系数
		ConsumMPBase,				//消耗内力值基数
		ConsumMPFactor,				//消耗内力值系数
		ConsumSPBase,				//消耗真气值基数
		ConsumSPFactor,				//消耗真气值系数
		ColdTime,					//冷却时间


		E_FLY_MAX,
	};
	
	struct SFlyData
	{
		char name[N_MAX_NAME_LEN];
		int  school;				// 门派
		DWORD  data[E_FLY_MAX];
		int	 type;
		char information[N_MAX_INFOR_LEN];		//技能说明
	};	
	typedef map<int, SFlyData>	TFLyMap;

	static bool Load( const char* filename);
	static void Destroy();

	static const SFlyData* GetData(int nId);

private:
	static TFLyMap	m_mFlyData;
};