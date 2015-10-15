#pragma once
#include <time.h>
#include <memory.h>
#include <math.h> 
///如果左值大于右值返回大于0的数，如果相等返回0，如果小于返回小于0的数
static int CompareTimeOfDay(const tm& lTime, const tm& rTime)
{
	if (lTime.tm_hour > rTime.tm_hour)
		return 1;
	else if (lTime.tm_hour == rTime.tm_hour)
		if (lTime.tm_min > rTime.tm_min)
			return 1;
		else if (lTime.tm_min == rTime.tm_min)
			if (lTime.tm_sec > rTime.tm_sec)
				return 1;
			else if (lTime.tm_sec == rTime.tm_sec)
				return 0;

	return -1;
}

///如果左值大于右值返回大于0的数，如果相等返回0，如果小于返回小于0的数
static int CompareTimeOfYear(const tm& lTime, const tm& rTime)
{
	if (lTime.tm_year > rTime.tm_year)
		return 1;
	else if (lTime.tm_year == rTime.tm_year)
		if (lTime.tm_mon > rTime.tm_mon)
			return 1;
		else if (lTime.tm_mon == rTime.tm_mon)
			if (lTime.tm_mday > rTime.tm_mday)
				return 1;
			else if (lTime.tm_mday == rTime.tm_mday)
				return 0;

	return -1;
}

static int Days_Distance(__time64_t lt, __time64_t rt)
{
	double seconds = difftime(lt, rt);
	return (int)(seconds / (3600 * 24));
}

static int Days_Distance_tm(tm& lTime, tm& rTime)
{
	__time64_t lt = mktime(&lTime);
	__time64_t rt = mktime(&rTime);
	return Days_Distance(lt, rt);
}

static __time64_t RoundTimeOfHour(const tm& time, int hour)
{
	tm temp = time;
	temp.tm_hour = hour;
	temp.tm_min = 0;
	temp.tm_sec = 0;
	return mktime(&temp);
}

static tm GobackPreDay(tm& time)
{
	__time64_t t = mktime(&time);
	t -= 86400;	///减去一天的秒数
	tm pre;
	_localtime64_s(&pre, &t);
	return pre;
}

///@ breif 获取earlyTm和laterTm之间的间隔天数
static int GetIntervalDays(__time32_t earlyTm, __time32_t laterTm)
{
	return ceil(_difftime32(earlyTm, laterTm) / 86400); //秒数时间差/ 24小时秒数  最后向上取整
}

///@brief 刷新判断
struct RefreshJudgement
{
public:
	static bool JudgeCrossed(__time64_t lastTime, tm judgedTime)
	{
		__time64_t juagedTime64 = _mktime64(&judgedTime);
		return JudgeCrossed(lastTime, juagedTime64);
	}

	///@brief 判断当前时间到lastTime这一段时间内是否跨过了judgedTime
	static bool JudgeCrossed(__time64_t lastTime, __time64_t judgedTime)
	{
		static tm curTimeStruct;
		static tm lastTimeStruct;
		static tm judgeTimeStruct;
		__time64_t curLocalTime = _time64(nullptr);
		errno_t tempCurErr = _localtime64_s(&curTimeStruct, &curLocalTime);
		errno_t tempLastErr = _localtime64_s(&lastTimeStruct, &lastTime);
		errno_t tempJudgeErr = _localtime64_s(&judgeTimeStruct, &judgedTime);

		if (tempCurErr || tempLastErr || tempJudgeErr)
			return false;

		if (curTimeStruct.tm_year > lastTimeStruct.tm_year)
			return true;
		else if (curTimeStruct.tm_year == lastTimeStruct.tm_year)
		{
			if (curTimeStruct.tm_mon > lastTimeStruct.tm_mon)
				return true;
			else if (curTimeStruct.tm_mon == lastTimeStruct.tm_mon)
			{
				if (curTimeStruct.tm_mday > lastTimeStruct.tm_mday)
				{
					///大了2天及2天以上的情况,毫不犹豫的PASS
					if (curTimeStruct.tm_mday > lastTimeStruct.tm_mday + 1)
						return true;

					///接下来判断只大了一天的情况
					int compareResult = CompareTimeOfDay(lastTimeStruct, judgeTimeStruct);
					if (compareResult < 0)///judgeTimeStruct时间点在晚上24点以前，或者24点以后得情况
						return true;
					else if (compareResult > 0 && CompareTimeOfDay(curTimeStruct, judgeTimeStruct) >= 0)///judgeTimeStruct时间点在晚上24点以后的情况， 或者24点以前的情况
						return true;
				}
				else if (curTimeStruct.tm_mday == lastTimeStruct.tm_mday)
				{
					if (CompareTimeOfDay(lastTimeStruct, judgeTimeStruct) < 0 && CompareTimeOfDay(curTimeStruct, judgeTimeStruct) >= 0)
						return true;
				}
			}
		}

		return false;
	}
};

class ConverTool
{
public:
	///@breif 转换工具
	static void ConvertInt64ToBytes(__int64 soure, char* byteBuff)
	{
		memset(byteBuff, 0, sizeof(byte) * 8);
		byteBuff[0] = (byte)(0xff & soure);
		byteBuff[1] = (byte)((0xff00 & soure) >> 8);
		byteBuff[2] = (byte)((0xff0000 & soure) >> 16);
		byteBuff[3] = (byte)((0xff000000 & soure) >> 24);
		byteBuff[4] = (byte)((0xff00000000 & soure) >> 32);
		byteBuff[5] = (byte)((0xff0000000000 & soure) >> 40);
		byteBuff[6] = (byte)((0xff000000000000 & soure) >> 48);
		byteBuff[7] = (byte)((0xff00000000000000 & soure) >> 56);
	}

};