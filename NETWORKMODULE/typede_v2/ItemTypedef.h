#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////
// 数据定义原则
// 放在这里的都是会全局用到的数据结构
///////////////////////////////////////////////////////////////////////////////////////////////

static const int MAX_ITEM_NUMBER = 64;
static const int MAX_TASK_NUMBER = 200;

static const int ITEMNAME_MAXLEN = 40;

static const int REDUCETIME = 946569600;	// 绑定道具的偏移时间 1999-12-31 00:00:00
// 门派列表
enum    E_SCHOOL
{
    ES_SHAOLIN=0,                   // 少林  (0-15)
    ES_WUDANG,                      // 武当  (16-31)
    ES_EMEI,                        // 峨嵋  (32-47)
    ES_TANGMEN,                     // 唐门  (48-63)
    ES_MOJIAO,                      // 魔教  (64-79)
    //...

    ES_MAX
};

enum EItemType { 
//  金钱
    MONEY = 0, 
//  不可重叠_可使用，  可重叠_可使用，     特殊重叠_可使用，           脚本类_可使用，
    NORMAL_USABLE = 2, OVERLAP_USABLE = 4, SPECIAL_OVERLAP_USABLE = 6, SCRIPT_USABLE = 8,
//  不可重叠_不可使用，可重叠_不可使用，   特殊重叠_不可使用，         脚本类_不可使用
    NORMAL_UNABLE = 3, OVERLAP_UNABLE = 5, SPECIAL_OVERLAP_UNABLE = 7, SCRIPT_UNABLE = 9,
//  镶嵌类_标记起点，  装备类_标记起点        , 包裹类型[仓库][背包]
    /*JEWEL_SEGMENT =30, */EQUIPMENT_SEGMENT = 101, EXTERN_PACKAGE = 117 };

enum JewelType { BT_YELLOW = 30, BT_BLUE, BT_GREEN, BT_WHITE, BT_RED, BT_BLACK  };

enum EMoveTo { MT_INPACKAGE, MT_PACKAGE, MT_EQUIP, MT_FORGE, MT_FUSE, MT_EXCHANGE, MT_INEXCHANGE, MT_BUNDLE };

// ( ( ( type & 6 ) == 4 ) && ( type < 30 ) )
// ( ( ( type & 6 ) == 6 ) && ( type < 30 ) )

inline BOOL ISNORMAL( int type )            { return ( ( type == NORMAL_USABLE ) || ( type == NORMAL_UNABLE ) ); }
inline BOOL ISNORMALOVERLAP( int type )     { return ( ( type == OVERLAP_USABLE ) || ( type == OVERLAP_UNABLE ) ); }
inline BOOL ISSPECIALOVERLAP( int type )    { return ( ( type == SPECIAL_OVERLAP_USABLE ) || ( type == SPECIAL_OVERLAP_UNABLE ) ); }
inline BOOL ISSCRIPTITEM( int type )        { return ( ( type == SCRIPT_USABLE ) || ( type == SCRIPT_UNABLE ) ); }
inline BOOL IS_EXTERN_PACKAGE( int type )   { return ( type == EXTERN_PACKAGE ); }

inline BOOL ISUNUSABLE( int type )          { return ( ( type & 1 ) || ( type >= SCRIPT_UNABLE ) ); }
inline BOOL ISOVERLAP( int type )           { return ( ( ( type & 4 ) && ( type < 30 ) ) || ( type == 0 ) ); }
inline BOOL ISEQUIPMENT( int type )         { return ( ( type >= EQUIPMENT_SEGMENT ) && ( type < 117 ) ); }
inline BOOL ISBUNDLE_GOOD( int type )       { return ( (type >= 10001) && (type <= 10040) ); }
inline BOOL ISBUNDLE_WAREHOUSE( int type )  { return ( (type >= 10101) && (type <= 10140) ); }

inline BOOL ISJEWEL( int idx )              { return ( ( idx >= 101 ) && ( idx <= 625 ) && ( ( idx % 100 ) <= 25 ) ); }
inline BOOL ISROSE( int idx )               { return ( ( idx >= 51520 ) && ( idx <= 51529 ) ); } // 玫瑰

static const int BASE_PACKAGE_GOOD = 10000;
static const int BASE_PACKAGE_WAREHOUSE = 10100;
//static const int BASE_GOOD_NUMBER = 24;     // 扩展背包数
static const int BASE_PACKAGE_NUMBER = 64;     // 扩展仓库

//typedef unsigned __int64 QWORD;

#pragma pack (push)
#pragma pack (1)

// 最基本的（保存用）道具数据结构
//  一个ItemBuffer 结构示例 
// 0x14002008000BCD0200000000000000002D2D2D2D2D0000000000000000000000
// UniqueID = 0x02cd0b0008    总共10*4  位
// 20 合并起来2*4代表 物品所点空间和是否锁定
//1400  总共4*4 代表物品ItemIndex  0x0014

struct SItemBase 
{
    enum { 
        F_BINDED        = 0x1,       // 已经绑定无法[交易]
        F_DISABLED      = 0x2,       // 已经禁止无法[交易/使用/拖动/丢弃/消耗]
    };

    // 全局唯一编号，总40bit
    // ############################
    // 注意：！！！！！！
    // ############################
    // 因为存在数据库1拖多的情况，所以区域服务器编号可能重复，目前3个区域分别占用一个号
    // 所以不能以这个值作为关键的判断数据！！！
    // 需要注意！！！
    // ----------------------------
    // 添加数据库1拖多的解决方案：以牺牲uniqueTimestamp的bit数来保存服务器组编号
    // 使用一个动态编号占位来处理，那么，1拖2可以支持8.98年，1拖4可以支持4.49年，应该够了吧……
    // ____________________________
    static const YEAR_SEGMENT       = 2007;         // 时间戳的起始时间为2007年
    static const MAX_ACCUMULATOR    = 0x3fff;       // 30秒内累加值的最大值为0x3fff == 16383

    inline QWORD uniqueId() const 
    { 
        return reinterpret_cast< const QWORD& >( reinterpret_cast< const char* >( this )[3] ) & 0x000000ffffffffff; 
    }

    inline void uniqueId( QWORD v ) 
    { 
        reinterpret_cast< DWORD& >( reinterpret_cast< LPBYTE >( this )[3] ) = reinterpret_cast< DWORD& >( v );
        reinterpret_cast< LPBYTE >( this )[7] = reinterpret_cast< LPBYTE >( &v )[4]; 
    }

    union
    {
        struct
        {
            QWORD   wIndex            : 16;     // 在物品列表中的编号，通过它可以查出所有关于该道具的数据
            QWORD   size              : 6;      // 本道具所占用的内存空间，包含了SItemBase（8字节）的有效数据大小！
            QWORD   flags             : 2;      // 本道具的道具属性标志！

            // 24 : 14 : 2 的位域排序顺序是为了能够对齐后直接将服务器编号定位在高2bit，便于数据库中直接操作，和查询！
            QWORD   uniqueTimestamp   : 24;     // [时间戳      ] 24bit，以30秒为单位进行记录，最大数限制可达到15.96年
            QWORD   uniqueCreateIndex : 14;     // [10秒内累加值] 14bit，每30秒内单服务器上可生成16384个道具
            QWORD   uniqueServerId    : 2;      // [服务器编号  ] 2bit，记录道具的产生位置，以避免在不同服务器上的编号重复

        } details;

        // 由于details内部使用的是位域，所以直接访问返回的都是64位数据，结果加大了运算负担！
        // 使用该结构参与联合，主要就使用于处理外部访问数据时，可以直接取到一个较为方便处理的数据
        struct
        {
            WORD wIndex;
            BYTE size  : 6;
            BYTE flags : 2;
        };
    };
};

// 原始item数据空间大小
struct SRawItemBuffer : 
    public SItemBase
{
    BYTE buffer[56]; // max dummy buffer
};

// 脚本类道具，主要用于储存脚本数据
// 8字节基本道具数据，1字节有效位判断，54字节的脚本数据区
// 因为size只有6bit，能保存的最大值为63。。。
struct SScriptItem : 
    public SItemBase
{
    BYTE declare;       // 位定义[同时也限制了脚本数据最大只能由8项]，按bit来判断是否有效
    BYTE streamData[54];
};

// 可重叠类道具，重叠的上限由wIndex查出来的数据决定
struct SOverlap : 
    public SItemBase
{
    union {
        struct { BYTE  number; }    normal;     // 普通重叠类道具，最大重叠数量为0xff
        struct { WORD  number; }    special;    // 特殊重叠类道具，最大重叠数量为0xffff
        struct { DWORD number; }    money;      // 金钱，最大0xffffffff
    };
};

// 新装备结构 60 BYTE
struct SEquipment :
    public SItemBase
{
    static const MAX_SCRIPTS            = 2;
    static const MAX_SLOTS              = 5;
    static const MAX_ACTIVATES          = 3;
    static const MAX_QUALITYS           = 6;
    static const MAX_INHERITS           = 6;
    static const EQUIPMENT_MAXNUMBER    = 16; // 角色身上最多同时装备8件装备

    // 使用 5BYTE
    struct JewelSlot {
        BYTE type   : 3;                    // 玉石类型 0->7  低3位
        BYTE value  : 5;                    // 玉石等级 0->31 高5位

        // 如 type == 0，为特殊类型， 此时，value == 0 表示该插槽无效； value == 1 表示该插槽为空
        BOOL isEmpty()   const { return *( LPBYTE )this == 8; }
        BOOL isInvalid() const { return *( LPBYTE )this == 0; }
        BOOL isJewel()   const { return !isInvalid() && ( type != 0 ); }
    } slots[ MAX_SLOTS ]; 

    // 使用40BYTE
    struct  Attribute 
    {
        struct Unit
        {
            WORD type   : 6;        // 装备属性类型0-63
            WORD value  : 10;       // 装备属性值
        };

        Unit activates[3];          // 激活属性3条[强化附加]
        Unit naturals;              // 天赋1条[天生附加]
        Unit qualitys[6];           // 资质6条[资质未鉴定] 掉落的时候已经鉴定[掉落附加]，
									// 鉴定至少会产生一条属性，所以如果一条属性都没有，则标识未鉴定
        Unit inherits[6];           // 继承属性
        Unit reserves[2];           // 未使用的2条


        // 武功，心法加等级特殊
        // 这里使用4个BYTE以便对齐 武功和心法使用3BYTE的空
        struct UnitSkill
        {
            DWORD skillType : 7;    // 0- 127
            DWORD teletgyType : 7;  // 0- 127
            DWORD skillValue : 7;   // 0- 127
            DWORD teletgyValue : 7; // 0- 127
            DWORD reserve : 4;      // 保留
        } unitSkill;

    }attribute;

    WORD isDevelop: 1;              // 用于判断qualitys中的属性是鉴定出来的还是掉落出来的
    WORD isRefine : 1;              // 是否已经血炼
    WORD developCount : 10;         // 鉴定统计
    WORD level : 4;                 // 该装备的强化等级
    BYTE reserve;                   // 装备？

    WORD scriptDatas[ MAX_SCRIPTS ]; // 用于脚本设定的数据，便于功能扩充
};

//// 装备类道具 32BYTE
//struct SEquipment : 
//    public SItemBase
//{
//    static const MAX_SCRIPTS            = 2;
//    static const MAX_SLOTS              = 5;
//    static const MAX_ATTRIBUTES         = 7;
//    static const EQUIPMENT_MAXNUMBER    = 8; // 角色身上最多同时装备8件装备
//
//    BYTE prefix;                            // ( 现在用于绑定时间类型 )前缀 [坚固/锋利/精炼/以及各种新设定的前缀值]可影响该装备的加层效果
//    BYTE level;                             // ( 现在用于保持绑定时间 BYTE + WORD )等级
//    WORD experience;                        // ( 现在用于保持绑定时间 BYTE + WORD, )熟练度/经验值 
//                                            // 这里利用level 和 experience 
//											// 注意：这里保持时间并未使用DWORD，因为结构大小的限制，决定使用偏移来处理空间不足的问题,
//											// 取得时间 ( 当前日期 - 1999-12-31 00:00:00[946569600] ) = 保存日期， 在要使用的时候 + 1999-12-31 00:00:00 即可
//
//    struct Durance {
//        WORD cur;                           // 当前耐久度 0~65535
//        BYTE max;                           // 最大耐久度 ( 0~255 ) * 255
//
//        // 耐久度表示方法：/255
//    } durance;
//
//    BYTE reserve;                           // 保留数据，8bit
//
//    struct JewelSlot {
//        BYTE type   : 3;                    // 玉石类型 0->7  低3位
//        BYTE value  : 5;                    // 玉石等级 0->31 高5位
//
//        // 如 type == 0，为特殊类型， 此时，value == 0 表示该插槽无效； value == 1 表示该插槽为空
//        BOOL isEmpty()   const { return *( LPBYTE )this == 8; }
//        BOOL isInvalid() const { return *( LPBYTE )this == 0; }
//        BOOL isJewel()   const { return !isInvalid() && ( type != 0 ); }
//    } slots[ MAX_SLOTS ];                    // 5格插槽
//
//    struct ExtraAttribute {
//        BYTE type   : 5;                    // 属性类型 0->31
//        BYTE value  : 3;                    // 属性等级 0->7
//
//        // 属性的有效判断方法同上
//        BOOL isInvalid() const { return *( LPBYTE )this == 0; }
//    } attributes[ MAX_ATTRIBUTES ];          // 最大7个附加属性
//
//    WORD scriptDatas[ MAX_SCRIPTS ];         // 用于脚本设定的数据，便于功能扩充
//};

// 格子的坐标
struct SCellPos 
{ 
    BYTE byCellX;
    BYTE byCellY; 
};

// Tile的坐标
struct STilePos 
{ 
    WORD wTileX;
    WORD wTileY; 
};

// 放在道具栏中的道具
struct SPackageItem : 
    public SCellPos,
    public SRawItemBuffer
{
};

// 放在地上的道具
struct SGroundItem : 
    public STilePos,
    public SRawItemBuffer
{
};

// 技能相关
struct SSkill
{
    BYTE byTypeID;			// 在技能列表中的编号
    BYTE byLevel;			// 技能的等级
	DWORD dwProficiency;	// 技能的熟练度
};

// 道具配置数据， 现在这个数据要用来做实时数据传输了，所以需要将其减小！
struct ClientVisible
{
    // 交易相关
    DWORD dwBuy;            // 购入价格
    DWORD dwSell;           // 卖出价格
	WORD wSellPrice;	    // 物品侠义元宝卖出价格
	WORD wScorePrice;	    // 赠品价格

	WORD wItemID;           // 道具编号
    WORD byGraph1;          // 图片编号（如果是武器的话，也使用这一个ID去找具体的武器动画）
	WORD wGraph2;           // 地面显示用的图片编号

	WORD wSound1;           // 该物品掉落（放入物品栏，装备）的音效编号w
	WORD wSpecial;          // 特殊应用！（目前用于最大的重叠数量）
    WORD byLevelEM;         // 等级需求
    WORD wENEM;             // 活力需求
    WORD wINEM;             // 悟性需求
    WORD wSTEM;             // 体魄需求
    WORD wAGEM;             // 身法需求

    WORD wHPAD;            // 增加生命最大值
    WORD wSPAD;            // 增加体力最大值
    WORD wMPAD;            // 增加内力最大值
    WORD wHPR;             // 恢复生命量
    WORD wSPR;             // 恢复体力量
    WORD wMPR;             // 恢复内力量

    BYTE byType;            // 该道具的类型
	BYTE byHoldSize;        // 该道具占用的物品栏格子
    BYTE byGroupEM;         // 门派需求

    // 使用/装备相关数据
    WORD wDAMAD;            // -加攻击力
    WORD wPOWAD;            // 加内功攻击力
    WORD wDEFAD;            // 加防御力
    WORD wAGIAD;            // 加灵敏度
    WORD wAmuckAD;          // 加杀气
    WORD wLUAD;             // 加气运
    WORD wRHPAD;            // 加生命自动恢复速度
    WORD wRSPAD;            // 加体力自动恢复速度
    WORD wRMPAD;            // 加内力自动恢复速度
    WORD wClear;            // 消除异常状态

    WORD wPointAD;          // 加属性点
    WORD wLevelAD;          // 加等级

    WORD material;          // 道具的材质类型，用于设定耐久度的扣除量

    WORD wSubLevel;         // 装备的子等级，客户端需要用的……
	BYTE mBindType;         // 绑定类型
    WORD wShineLEM;         // 发光等级需求  0不发光 
    WORD wGraph3;           // 发光图片编号

    char szName[ITEMNAME_MAXLEN];    //原为11->20
};

struct ClientInvisible
{
    DWORD dwScripID;        // 脚本编号
	WORD wEffect;           // 效果图（目前用于人皮面具变身）

    BYTE nMainLevel;        // 以下4个数据和套装相关，只需要在服务器判断！
    BYTE nAddType;
	BYTE wAddScale;         // 加成比例 

	BYTE wGestID;           // 习得武功或队形技编号
	BYTE wGestLevel;        // 武功自身等级需求

    BYTE durance;           // 道具的默认耐久度，用于初始化时设定耐久度的上限

    WORD reserve1;          // 影响的成功百分比
    WORD reserve2;          // 加能力点
    WORD restrict;          // 是否限制显示的道具（在摆摊搜索中是否显示）
    WORD reserve4;          // 增加效果的持续时间，如果这个时间是0的话，说明是永久性的增加效果
};

struct SItemData : 
    public ClientInvisible,
    public ClientVisible
{
};

struct SAutoUseItemSet
{
	SAutoUseItemSet() : bUse(false), wPoint(0), wWhich(0)
	{}

	bool bUse;
	WORD wPoint;
	WORD wWhich;
};

#pragma pack (pop)

// 用于记录日志的结构体
struct LogInfo
{
    LogInfo( DWORD rcdtype, LPCSTR description = NULL ) : rcdtype( rcdtype ), description( description ) {}

    DWORD   rcdtype;        // 记录编号 
    LPCSTR  description;    // 记录说明
};

// itemBuffer : 填充道具缓存， index : 道具编号， param : 重叠数量/孔数， rcdtype : 记录编号， description : 记录说明
struct GenItemParams
{
    GenItemParams( WORD index, DWORD number = 1 ) : slotNumber( 0 ), index( index ), number( number ) {}
    GenItemParams( WORD index, DWORD number, WORD slotNumber ) : slotNumber( slotNumber ), index( index ), number( number ) {}

    WORD    slotNumber;
    WORD    index;
    DWORD   number;
};

/// 特殊掉落物品结构
struct SpecialItem
{
	DWORD mID;                // ID
	DWORD mLevelAdd[4];       // 等级附加
	DWORD mIntensifyAdd[3];   // 强化附加
	DWORD mHoleAdd[4];        // 洞附加
	DWORD mInnetenessAddOddsAndFlag[2]; // 天生附加几率 和 标识
	DWORD mAptitudeCheckupOdds[2]; // 资质鉴定几率
	DWORD mDropAddsOddsAndFlag[3]; // 掉落附加标识 几率 和标识
};

struct SGemData
{
    // 这里位置不要随意移动，因为这里的标识用于定位使用
    enum 
    {
        GEMDATA_EN = 1,			            // 手太阳
        GEMDATA_ST,		                    // 足太阴
        GEMDATA_IN,			                // 手少阳
        GEMDATA_AG,			                // 足少阴
        GEMDATA_DAMAD,                      // 外功
        GEMDATA_POWER,                      // 内功
        GEMDATA_DEFENCE,                    // 防御
        GEMDATA_AGILITY,                    // 敏捷
        GEMDATA_AMUCK,                      // 杀气
        GEMDATA_LU,			                // 运气
        GEMDATA_SAVVY,			        	// 悟性
        GEMDATA_ADDHPMAXP,                  // 增加生命上限千分比
        GEMDATA_ADDMPMAXP,                  // 增加MP上限千分比
        GEMDATA_ADDSPMAXP,                  // 增加SP上限千分比
        GEMDATA_HPSPEED,                    // HP恢复速度(量)
        GEMDATA_MPSPEED,                    // MP恢复速度(量)
        GEMDATA_SPSPEED,                    // SP恢复速度(量)   
        GEMDATA_DAMADCTR,                   // 外功伤害抵消   
        GEMDATA_POWERCTR,                   // 内功伤害抵消   
        GEMDATA_ALLHURTCTR,                 // 所有伤害抵消   
        GEMDATA_DODGE,                      // 躲闪几率   
        GEMDATA_ABSDEFENCE,                 // 绝对防御   
        GEMDATA_REBOUNDP,                   // 反弹几率   
        GEMDATA_TRANSLIFEP,                 // 吸血几率
        GEMDATA_TRANSMP,                    // 吸内几率
        GEMDATA_TRANSSP,                    // 吸体几率
        GEMDATA_HIT,                        // 命中几率
        GEMDATA_DAMADHURT,                  // 外功伤害
        GEMDATA_POWERHURT,                  // 内功伤害
        GEMDATA_IGNOREDEF,                  // 忽略防御
        GEMDATA_CSX8,                       // 暴击X8
        GEMDATA_RESISTVITALSP,              // 抵抗点穴几率
        GEMDATA_RESISTTOXICP,               // 抵抗中毒几率
        GEMDATA_RESISLIMITSKILLP,           // 抵抗封招几率
        GEMDATA_RESISDIZZYP,                // 抵抗眩晕几率
        GEMDATA_RESISLIMITHP,               // 抵抗打出0生命力几率
        GEMDATA_RESISLIMITMP,               // 抵抗打出0内力几率
        GEMDATA_RESISLIMITSP,               // 抵抗打出0体力几率 
        GEMDATA_VITALSP,                    // 点穴命中几率
        GEMDATA_TOXICP,                     // 中毒几率
        GEMDATA_LIMITSKILLP,                // 封招几率
        GEMDATA_DIZZYP,                     // 眩晕几率
        GEMDATA_ZEROHP,                     // 打出0生命力几率
        GEMDATA_ZEROMP,                     // 打出0内几率
        GEMDATA_ZEROSP,                     // 打出0体力几率
        GEMDATA_EXP,                        // 额外经验加层   
        GEMDATA_SKILLEXP,                   // 武功经验加层   
        GEMDATA_SKILLLEVEL,                 // 技能等级加层  
        GEMDATA_TELERGYLEVEL,               // 心法等级加层
        GEMDATA_ACTIONSPEED,                // 攻击速度百分比
        GEMDATA_MOVESPEED,                  // 移动速度百分比
        GEMDATA_RETARMOVESPEED,             // 迟缓

        GEMDATA_MAX                         // 最大
    };

    // 基本属性
    __int16	en;			    //手太阳
    __int16	st;		        //足太阴
    __int16	in;			    //手少阳
    __int16	ag;			    //足少阴 
    __int16	damad;          // 外功
    __int16	power;          // 内功
    __int16	defence;        // 防御
    __int16	agility;        // 敏捷
    __int16	amuck;          // 杀气
    __int16	lu;			    // 运气
    __int16	savvy;			// 悟性
    __int16 addHPMaxP;      // 增加生命上限千分比
    __int16 addMPMaxP;      // 增加MP上限千分比
    __int16 addSPMaxP;      // 增加SP上限千分比

    // 生存属性
    __int16 HPSpeed;        // HP恢复速度(量)
    __int16 MPSpeed;        // MP恢复速度(量)
    __int16 SPSpeed;        // SP恢复速度(量)
    __int16 damadCtr;       // 外功伤害抵消
    __int16	powerCtr;       // 内功伤害抵消
    __int16 allHurtCtr;     // 所有伤害抵消
    __int16 dodge;          // 躲闪几率
    __int16 absDefence;     // 绝对防御
    __int16 reboundP;       // 反弹几率

    // 战斗属性
    __int16 transLifeP;     // 吸血几率
    __int16 transMP;        // 吸内几率
    __int16 transSP;        // 吸体几率
    __int16 hit;            // 命中几率
    __int16 damadHurt;      // 外功伤害
    __int16	powerHurt;      // 内功伤害
    __int16 ignoreDef;      // 忽略防御
    __int16 csX8;           // 暴击X8

    // 抵抗效果类
    __int16 resistVitalsP;    // 抵抗点穴几率
    __int16 resisTtoxicP;     // 抵抗中毒几率
    __int16 resisLimitskillP; // 抵抗封招几率
    __int16 resisDizzyP;      // 抵抗眩晕几率
    __int16 resislimitHP;     // 抵抗打出0生命力几率
    __int16 resislimitMP;     // 抵抗打出0内力几率
    __int16 resislimitSP;     // 抵抗打出0体力几率

    // 效果类
    __int16 vitalsP;        // 点穴命中几率
    __int16 toxicP;         // 中毒几率
    __int16 limitskillP;    // 封招几率
    __int16 dizzyP;         // 眩晕几率
    __int16 zeroHP;         // 打出0生命力几率
    __int16 zeroMP;         // 打出0内几率
    __int16 zeroSP;         // 打出0体力几率

    // 特殊效果
    __int16 exp;            // 额外经验加层
    __int16 skillExp;       // 武功经验加层

    // 技能加层
    __int16 skillLevel;     // 技能等级加层 高8位存技能ID 低8位存等级
    __int16 telergyLevel;   // 心法等级加层 高8位存技能ID 低8位存等级

    // 额外追加
    __int16 actionSpeed;    // 攻击速度百分比
    __int16 moveSpeed;      // 移动速度百分比
    __int16 retarMoveSpeed; // 迟缓

    // 衰减，100千分比
    const SGemData operator * ( int value )
    {
        SGemData data;    
        memset( &data, 0, sizeof( data ) );
        //data.addHPMaxP           = ( addHPMaxP * value ) / 100;
        return data;
    }

    // 当+=越界的时候
    void operator += ( const SGemData &data )
    {
        int temp = 0;
#define  _CHECK( v )   ( v > 1000 ) ? ( 1000 ) :( v )

        absDefence          = _CHECK( data.absDefence + absDefence );
        addHPMaxP           = _CHECK( data.addHPMaxP + addHPMaxP );
        addMPMaxP           = _CHECK( data.addMPMaxP + addMPMaxP );
        addSPMaxP           = _CHECK( data.addSPMaxP + addSPMaxP );
        ag                  = _CHECK( data.ag + ag );
        agility             = _CHECK( data.agility + agility );
        allHurtCtr          = _CHECK( data.allHurtCtr + allHurtCtr );
        amuck               = _CHECK( data.amuck + amuck );
        csX8                = _CHECK( data.csX8 + csX8 );
        damad               = _CHECK( data.damad + damad );
        damadCtr            = _CHECK( data.damadCtr + damadCtr );
        defence             = _CHECK( data.defence + defence );
        dizzyP              = _CHECK( data.dizzyP + dizzyP );
        dodge               = _CHECK( data.dodge + dodge );
        en                  = _CHECK( data.en + en );
        exp                 = _CHECK( data.exp + exp );
        hit                 = _CHECK( data.hit + hit );
        HPSpeed             = _CHECK( data.HPSpeed + HPSpeed );
        ignoreDef           = _CHECK( data.ignoreDef + ignoreDef );
        in                  = _CHECK( data.in + in );
        limitskillP         = _CHECK( data.limitskillP + limitskillP );
        lu                  = _CHECK( data.lu + lu );
        MPSpeed             = _CHECK( data.MPSpeed + MPSpeed );
        power               = _CHECK( data.power + power );
        powerCtr            = _CHECK( data.powerCtr + powerCtr );
        reboundP            = _CHECK( data.reboundP + reboundP );
        resisDizzyP         = _CHECK( data.resisDizzyP + resisDizzyP );
        resislimitHP        = _CHECK( data.resislimitHP + resislimitHP );
        resislimitMP        = _CHECK( data.resislimitMP + resislimitMP );
        resisLimitskillP    = _CHECK( data.resisLimitskillP + resisLimitskillP );
        resislimitSP        = _CHECK( data.resislimitSP + resislimitSP );
        resisTtoxicP        = _CHECK( data.resisTtoxicP + resisTtoxicP );
        resistVitalsP       = _CHECK( data.resistVitalsP + resistVitalsP );
        skillExp            = _CHECK( data.skillExp + skillExp );
        SPSpeed             = _CHECK( data.SPSpeed + SPSpeed );
        st                  = _CHECK( data.st + st );
        toxicP              = _CHECK( data.toxicP + toxicP );
        transLifeP          = _CHECK( data.transLifeP + transLifeP );
        transMP             = _CHECK( data.transMP + transMP );
        transSP             = _CHECK( data.transSP + transSP );
        vitalsP             = _CHECK( data.vitalsP + vitalsP );
        zeroHP              = _CHECK( data.zeroHP + zeroHP );
        zeroMP              = _CHECK( data.zeroMP + zeroMP );
        zeroSP              = _CHECK( data.zeroSP + zeroSP );
        exp                 = _CHECK( data.exp + exp );
        skillExp            = _CHECK( data.skillExp + skillExp );
        skillLevel          = _CHECK( data.skillLevel + skillLevel );
        telergyLevel        = _CHECK( data.telergyLevel + telergyLevel );
        actionSpeed         = _CHECK( data.actionSpeed + actionSpeed );
        moveSpeed           = _CHECK( data.moveSpeed + moveSpeed );
        retarMoveSpeed      = _CHECK( data.retarMoveSpeed + retarMoveSpeed );
        damadHurt           = _CHECK( data.damadHurt + damadHurt );// 外功伤害
        powerHurt           = _CHECK( data.powerHurt + powerHurt );// 外功伤害
#undef _CHECK
    }
};

struct SGemAtt
{
	DWORD att[57][3];
};

