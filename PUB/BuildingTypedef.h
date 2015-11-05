#pragma once

#include "networkmodule/itemtypedef.h"

/*/ 
建造系统初步：
    新的对象类型，旧的原有对象管理模式，综合成为建造系统！
    建筑物大概会拥有以下几个特点：
    1）外观
    2）属于物件层（地毯和遮盖暂时不计入这里边）
    3）如何改变地图阻挡属性，或者不改变（实现台阶，围墙，门框，出入口。。。）
    4）是否以可交互对象的形式存在
    5）根据4的设定决定这个对象是否可以破坏！耐久度。

    建筑对象的同步机制：
    基本和其他对象的同步方式相同！
    基本传送数据为：全局编号，XY坐标，建筑物编号，耐久度
    （剩下的数据根据查表决定）同步消息大小基本在16个字节以内
/*/

// 建筑物的基本属性
struct SBuildingBaseData
{
    WORD index;         // 建筑物索引编号（直接以编号的大小来判断图层，1w以下为地毯层，1~2w为物件层，1~3w为组合物件需另查表）
    WORD graph;         // 建筑物外观图片编号

    QWORD tileMask;     // 8x8阻挡信息！

    // 建筑物图片的中心坐标对应到的阻挡坐标
    BYTE centerX, centerY; 

    BYTE buildType;     // 建造类型（随时间自动生长，随时间消化建材成长，需要玩家劳动辅助消化建材）
    BYTE buildSpeed;    // 建造速度（单位为分钟）
    BYTE material;      // 需求建材类型，同时也决定了该材质的抗击打能力基数（这里只设定建材大类型，消化不同子类型的材质会导致建筑质量的改变）
    BYTE maintenance;   // 成长值速度（为建造速度的倍率，10位的）

    DWORD maxHP;        // 耐久度最大值。

    union
    {
        struct Defender
        {
            BYTE type;      // 攻击类型特性
            BYTE range;     // 攻击距离
            BYTE atkSpeed;  // 攻击频率
            BYTE reserve;   // 保留
            WORD damage;    // 建筑物可造成的伤害值
        };

        struct Scripter
        {
            DWORD scriptId; // 点击脚本
        };

        struct Decker
        {
        };
    };
};

// 用于保存的建筑物属性 40BYTE
struct SBuildingSaveData : 
    public SItemBase    // 借用其数据作为基本属性
{
    // ---------- 8byte -----------
    // WORD wIndex;         建筑物编号
    // BYTE size  : 6;      需要保存的数据大小!
    // BYTE flags : 2;      建筑物状态（正常，建筑中，拆除中，停用）
    // QWORD uniqueId;      唯一标识符,建筑物索引数据!

    // ---------- 8byte -----------
    WORD curX, curY;        // 建筑物所处于的位置
    DWORD curHP;            // 当前耐久度。

    // ---------- 12byte -----------
    WORD scriptValue[6];    // 6个附加数据!

    // ---------- 12byte -----------
    BYTE ownerType : 2;     // 拥有者类型（特殊、个人、帮派、公共）
    BYTE openType : 1;      // 是否对外开放（未开放、公共）
    char owner[11];         // 拥有者名称!
};

// 用于游戏中的建筑物数据结构
struct SBuildingData :
    public SBuildingBaseData,
    public SBuildingSaveData
{
};
