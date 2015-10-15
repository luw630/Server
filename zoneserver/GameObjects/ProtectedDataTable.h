#pragma once

class CProtectedDataTable
{
public:
	enum{ E_N_MAX_NAME_LEN=128,
		E_N_MAX_INFOR_LEN = 1024,
		};

	enum {
		MaxLevel,					//最大等级
		DiffcultBase,				//难度基数
		DiffcultFactor,				//难度系数
		RemainTimeBase,				//持续时间基数
		RemainTimeFactor,			//持续时间系数
		OutDefenceBase,				//外防基数
		OutDefenceFactor,			//外防系数
		InnerDefenceBase,			//内防基数
		InnerDefenceFactor,			//内防系数
		HPLimitedBase,				//生命上限基数
		HPLimitedFactor,			//生命上限系数
		ReduceHurtRateBase,			//减免伤害比例基数
		ReduceHurtRateFactor,		//减免伤害比例系数
		ReduceAbsoluteHurtBase,		//减免绝对伤害基数
		ReduceAbsoluteHurtFactor,	//减免绝对伤害系数
        CountAttackRateBase,		//反弹伤害比例基数
		CountAttackRateFactor,		//反弹伤害比例系数
		ConsumHPBase,				//消耗生命值基数
		ConsumHPFactor,				//消耗生命值系数
		ConsumMPBase,				//消耗内力值基数
		ConsumMPFactor,				//消耗内力值系数
		ConsumSPBase,				//消耗真气值基数
		ConsumSPFactor,				//消耗真气值系数
		ColdTime,					//冷却时间

		PDT_MAX,
	};

	struct ProtectedData
	{
		char name[E_N_MAX_NAME_LEN];
		int  school;					//门派
		DWORD  data[PDT_MAX];
		int  type;
		char information[E_N_MAX_INFOR_LEN];		//技能说明
	};
	typedef map<int, ProtectedData> TPDMap;

	static bool Load(const char* filename);
	static void Destroy();

	static const ProtectedData* GetData( int id );
protected:
	static TPDMap m_mProtectedData;
};