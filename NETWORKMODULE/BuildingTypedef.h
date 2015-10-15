//BuildingTypedef.h
#pragma once
#include "itemtypedef.h"
#include "datamsgs.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// 建筑物相关消息类, 为了不影响项目之间的包含关系逻辑,
//=============================================================================================
DECLARE_MSG_MAP(SBuildingBaseMsg, SMessage, SMessage::EPRO_BUILDING_MESSAGE)
//{{AFX
EPRO_BUILDSTRUCTURE,            // 开始建造建筑物
EPRO_SYN_BUILDING_INFO,          // 同步建筑物的信息
EPRO_BUILDING_SAVE,
EPRO_BUILDING_GET
//}}AFX
END_MSG_MAP()
//---------------------------------------------------------------------------------------------

DECLARE_MSG( SBuildStructureMsg, SBuildingBaseMsg, SBuildingBaseMsg::EPRO_BUILDSTRUCTURE )
struct SQBuildStructureMsg : public SBuildStructureMsg
{
    WORD index, inverse, x, y;
};

DECLARE_MSG( SSynBuildingMsg, SBuildingBaseMsg, SBuildingBaseMsg::EPRO_SYN_BUILDING_INFO )
struct SASynBuildingMsg : public SSynBuildingMsg 
{
    DWORD gid;
    //SBuildingSaveData data;
    char append[1];
};

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
//struct SBuildingBaseData
//{
//    WORD index;         // 建筑物索引编号（直接以编号的大小来判断图层，1w以下为地毯层，1~2w为物件层，1~3w为组合物件需另查表）
//    WORD graph;         // 建筑物外观图片编号
//
//    QWORD tileMask;     // 8x8阻挡信息！
//
//    // 建筑物图片的中心坐标对应到的阻挡坐标
//    BYTE centerX, centerY; 
//
//    BYTE buildType;     // 建造类型（随时间自动生长，随时间消化建材成长，需要玩家劳动辅助消化建材）
//    BYTE buildSpeed;    // 建造速度（单位为分钟）
//    BYTE material;      // 需求建材类型，同时也决定了该材质的抗击打能力基数（这里只设定建材大类型，消化不同子类型的材质会导致建筑质量的改变）
//    BYTE maintenance;   // 成长值速度（为建造速度的倍率，10位的）
//
//    DWORD maxHP;        // 耐久度最大值。
//
//    union
//    {
//        struct Defender
//        {
//            BYTE type;      // 攻击类型特性
//            BYTE range;     // 攻击距离
//            BYTE atkSpeed;  // 攻击频率
//            BYTE reserve;   // 保留
//            WORD damage;    // 建筑物可造成的伤害值
//        };
//
//        struct Scripter
//        {
//            DWORD scriptId; // 点击脚本
//        };
//
//        struct Decker
//        {
//        };
//    };
//};
//
//// 用于保存的建筑物属性 40BYTE
//struct SBuildingSaveData : 
//    public SItemBase    // 借用其数据作为基本属性
//{
//    // ---------- 8byte -----------
//    // WORD wIndex;         建筑物编号
//    // BYTE size  : 6;      需要保存的数据大小!
//    // BYTE flags : 2;      建筑物状态（正常，建筑中，拆除中，停用）
//    // QWORD uniqueId;      唯一标识符,建筑物索引数据!
//
//    // ---------- 8byte -----------
//   
//    DWORD curHP;            // 当前耐久度。
//
//    // ---------- 12byte -----------
//    WORD scriptValue[6];    // 6个附加数据!
//
//    // ---------- 12byte -----------
//    BYTE ownerType : 2;     // 拥有者类型（特殊、个人、帮派、公共）
//    BYTE openType : 1;      // 是否对外开放（未开放、公共）
//    char owner[CONST_USERNAME];         // 拥有者名称!
//};
//
struct WPOINT
{
    short x, y;
};

// 一共16 + 12个字节！
struct SaveBlock_fixed
{
    QWORD reserve : 24;
    QWORD uniqueId : 40;
    WORD index;         // 建筑物的配置数据编号
    WORD parentId;      // 父场景ID
    WORD x, y;

    BYTE ownerType;     // 拥有者类型
	char ownerName[CONST_USERNAME]; // 拥有名称

    inline QWORD UniqueId() const 
    { 
        return reinterpret_cast< const QWORD& >( reinterpret_cast< const char* >( this )[3] ) & 0x000000ffffffffff; 
    }
};

struct WLINE
{
    WPOINT seg, end;
};

//从配置表中读取出来的数据
struct SBuildingConfigData
{
    WORD index;				        // 建筑物索引编号
    BYTE centerx, centery;          // 建筑物所处于的位置
    DWORD image;				    // 建筑物外观图片编号	
    WORD totalBuildingState;		// 总的修建进度
    WORD ctrlScript;				// 默认创建/删除脚本 建筑物新建时调用的创建脚本编号（可能是玩家，也可能是系统脚本）
    WORD initScript;				// 默认加载脚本
    //WORD clickScriptId;             // 点击脚本编号（功能脚本）
    //WORD deadScriptId;              // 死亡脚本编号（销毁脚本呢）
    DWORD hpMax;                    // 最大耐久度（在某些情况下可更改）
	WLINE line;						// 遮挡线
    union
    {
        QWORD qwtileMask;			//阻挡
        DWORD dwtileMask;
    };

	union
    {
        QWORD qwtileLimit;			// 放置限制
        DWORD dwtileLimit;
    };


    inline void reset()
    {
        memset( this, 0, sizeof *this );		
    }
};

//// 用于游戏中的建筑物数据的固定结构
struct SBuildingFixedData
{
    union
    {
        struct
        {
            DWORD inverse           : 1;    // 是否显示反转
            DWORD usingState        : 2;    // 建筑物的应用状态
            DWORD ownerType         : 2;    // 建筑物的拥有者类型[0:系统][1:个人][2:帮派][3:特殊]
			DWORD showTitle		: 1;	// 是否显示名字
            DWORD state_Ex          : 18;   // 还未使用的bit数
            DWORD uniqueId_Hi       : 8;

            DWORD uniqueId_Lo;
        };

        struct
        {
            QWORD _state             : 24;       // Bit状态标识状态标识占用24个bit，留于扩展 修建、正常、维护、警戒 

            // 唯一标识符,建筑物索引数据!40个bit，和道具的格式一样
            // 24 : 14 : 2 的位域排序顺序是为了能够对齐后直接将服务器编号定位在高2bit，便于数据库中直接操作，和查询！
            QWORD uniqueTimestamp   : 24;       // [时间戳      ] 24bit，以30秒为单位进行记录，最大数限制可达到15.96年
            QWORD uniqueCreateIndex : 14;       // [10秒内累加值] 14bit，每30秒内单服务器上可生成16384个道具
            QWORD uniqueServerId    : 2;        // [服务器编号  ] 2bit，记录道具的产生位置，以避免在不同服务器上的编号重复
        };
    };

    WORD index;                         // 建筑物索引编号
    WORD curX, curY;				    // 建筑物所处于的位置
    WORD initScriptId;                  // 初始化脚本编号（将建筑放置于场景上的同时就触发初始化脚本）
    WORD clickScriptId;                 // 点击脚本编号（功能脚本）
    WORD deadScriptId;                  // 死亡脚本编号（销毁脚本呢）
    DWORD hpMax;                        // 最大耐久度（在某些情况下可更改）
    std::string viewTitle;			    // 可视名称
    std::string owner;			        // 拥有者名称（可能是帮派，也可能是个人）

    inline void reset()
    {
        memset( this, 0, sizeof *this );		
    }

    inline QWORD uniqueId() const 
    { 
        return reinterpret_cast< const QWORD& >( reinterpret_cast< const char* >( this )[3] ) & 0x000000ffffffffff; 
    }

    inline void uniqueId( QWORD v ) 
    { 
        reinterpret_cast< DWORD& >( reinterpret_cast< LPBYTE >( this )[3] ) = reinterpret_cast< DWORD& >( v );
        reinterpret_cast< LPBYTE >( this )[7] = reinterpret_cast< LPBYTE >( &v )[4]; 
    }
};

struct SBuildingDynamicData:
    public	SBuildingFixedData
{	
    WORD buildProgress;         // 当前的修建进度
    WORD functionScipt;         // 功能脚本
    DWORD hp;                   // 当前耐久度。

    inline void reset()
    {
        memset( this, 0, sizeof *this);		
    }
};

//区域服务器《——》登录服务器

DECLARE_MSG(SBuildingSaveMsg,SBuildingBaseMsg,SBuildingBaseMsg::EPRO_BUILDING_SAVE )
struct SQBuildingSaveMsg : public SBuildingSaveMsg
{
	WORD	wParentID;		//父场景ID
	DWORD   dwServerID;		//服务器ID
	WORD	wNumbers;		//数据流中数据成员个数
	WORD	wBufSize;		//整个序列化数据流大小		
	BYTE	Buffers[1];		//特殊应用，用栈上的静态内存块来做序列化数据保存		
};

//struct SABuildingSaveMsg: public SBuildingSaveMsg
//{
//	BYTE retMsg;
//};

DECLARE_MSG(SBuildingGetMsg,SBuildingBaseMsg,SBuildingBaseMsg::EPRO_BUILDING_GET )
struct SQBuildingGetMsg : public SBuildingGetMsg
{
	WORD	wParentID;		//父场景ID
	DWORD	dwServerID;		//登陆服务器ID
	DNID	dwZoneID;		//区域服务器ID（用于设置回传目标！）
};

struct SABuildingGetMsg: public SBuildingGetMsg
{
	enum
	{
		RET_SUCCESS,
		RET_FAIL
	};
	BYTE	retMsg;
	DNID	dwZoneID;		//区域服务器ID（用于设置回传目标！）
	WORD	wParentID;
	WORD	wNumbers;
	WORD	WBufSize;
	BYTE	Buffers[1];
};



