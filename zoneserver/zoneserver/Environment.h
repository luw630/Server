// Environment.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once
// 打怪经验值倍率
float GetExpScale();
void SetExpScale(float expScale);
// 掉落金钱量倍率
float GetDropMoneyScale();
void SetDropMoneyScale(float moneyScale);
// 掉落物品几率倍率
float GetDropItemScale();
void SetDropItemScale(float itemScale);

BYTE  GetMultiTimeCount();
void  SetMultiTimeCount(BYTE multiTime);

// 是否使用新的属性克制系统
BOOL UseAttribAnti();

// 读取环境设置数据
void LoadEnvironment();
