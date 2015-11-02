#include "stdafx.h"

#include "Environment.h"
#include "networkmodule/playertypedef.h"
#include <list>
#include <string>

static float fExpScale = 1.0;
static float fDropMoneyScale = 1.0;
static float fDropItemScale = 1.0;

static BYTE byMulHours =0;	// 双倍时间
static BOOL useAttribAnti = false;  // 是否使用新的属性相克系统！

std::list< std::string > extraNetLimitedInfo;
std::string sidname;

// 最大躲避率，不使用函数控制，直接放出对外链接地址，提高访问效率
int maxParry = 93;

// 消费时以元宝优先(!=0)还是以赠宝优先(0)！
int	sellType = 0;

// 是否在玩家扣点锁定后一定时间自动取消锁定！
int	autoUnlock = 0;

// 掉落物品的等级差限制！默认设定为每30级，掉率下降20%
int dropitem_levelMargin  = 30;  // 级差阶段
int dropitem_reduceRating = 20;  // 每阶段的改变比率

// 怪物掉落装备时去尝试设置随机附加值的几率！默认为，怪物120级以下，10%，以上1%
int randomatt_levelLimit = 120; // 怪物等级限制
int randomatt_below = 1000;     // 等级线以下的随机比率
int randomatt_above = 10;       // 等级线以上的随机比率

int cacheEnabled = 0;           // 是否打开缓存功能
int backupMargin = 30 * 1000;           // 缓存开启后的自动备份时间间隔
int backupNum = 1000;           // 缓存开启后的自动备份空间大小

// 是否使用新功能[以后添加可能存在隐患的新功能都可以通过这个设定来取消功能]
BOOL newly_ability_enabled = FALSE;
DWORD rcdMoneyRank = 100000;     // 默认超过1w的金钱变化，才进行日志记录！
DWORD macLimit = 0;

BOOL afLikeHungup = true;        // 自动战斗在统计时被标记为挂机状态
BOOL g_bHangup = FALSE;
DWORD g_dwLockArea = 0;
extern BOOL wgLimit;             // 是否限制外挂, 0 完全不限制， 1 网络底层限制， 2 游戏逻辑层限制， 4 通知到登录脚本， 8 通知到帐号服务器
extern BOOL isLimitWG;           // 用于限制跳跃和吃药。。
//extern DWORD g_dwEquipmentWearScale; // 用于将装备耐久进行缩放：2000耐久缩放成比如2.

BOOL bCheckGMCmd = FALSE;
extern int MAXPLAYERLEVEL;

// 打怪经验值倍率设定
float GetExpScale()
{

    return fExpScale;
}

void SetExpScale(float expScale)
{
	fExpScale = expScale;
}

//int GetVersionType()
//{
//	return nVersionType;
//}

// 掉落金钱量倍率设定
float GetDropMoneyScale()
{
    return fDropMoneyScale;
}

void SetDropMoneyScale(float moneyScale)
{
	fDropMoneyScale = moneyScale;
}
// 掉落物品几率倍率设定
float GetDropItemScale()
{
    return fDropItemScale;
}

void SetDropItemScale(float itemScale)
{
	fDropItemScale = itemScale;
}

BOOL UseAttribAnti()
{
    return useAttribAnti;
}
int change(int *a)
{
	*a = (*a) + 1; 
	 return *a;
}

void LoadEnvironment()
{
	IniFile IniFile;

    if (!IniFile.Load("environment.ini")) 
    {
        rfalse(2, 1, "无法打开游戏环境配置文件！！！");
        return;
    }
    int nTemp;
    IniFile.GetFloat("ENVIRONMENT", "EXPSCALE", 1.0f, &fExpScale);
    IniFile.GetFloat("ENVIRONMENT", "DROPMONEYSCALE", 1.0f, &fDropMoneyScale);
    IniFile.GetFloat("ENVIRONMENT", "DROPITEMSCALE", 1.0f, &fDropItemScale);
	IniFile.GetInteger( "ENVIRONMENT", "SELLTYPE", 0, &sellType );
	IniFile.GetInteger( "ENVIRONMENT", "ANTIATTRIB", false, &useAttribAnti );
	IniFile.GetInteger( "ENVIRONMENT", "MUILTETIME", 16, &nTemp );
	IniFile.GetInteger( "ENVIRONMENT", "MAXPARRY", 93, &maxParry );
    IniFile.GetInteger( "ENVIRONMENT", "NEWLYABILITY", 0, &newly_ability_enabled );
    IniFile.GetInteger( "ENVIRONMENT", "AUTOUNLOCK", 0, &autoUnlock );
    IniFile.GetInteger( "ENVIRONMENT", "RCDMONEYRANK", 100000, ( int* )&rcdMoneyRank );
    IniFile.GetInteger( "ENVIRONMENT", "MACLIMIT", 2, ( int* )&macLimit );

    IniFile.GetInteger( "SHAREDCACHE", "ENABLED", 0, &cacheEnabled );
    IniFile.GetInteger( "SHAREDCACHE", "BACKUPMARGIN", 30*1000, &backupMargin );
	IniFile.GetInteger("SHAREDCACHE", "BACKUPNUM", 1000, &backupNum);

	IniFile.GetInteger( "DROPLIMIT", "LEVELMARGIN", 30, &dropitem_levelMargin );
	IniFile.GetInteger( "DROPLIMIT", "REDUCERATING", 20, &dropitem_reduceRating );

	IniFile.GetInteger( "RANDATT", "LEVELLIMIT", 120, &randomatt_levelLimit );
	IniFile.GetInteger( "RANDATT", "BELOW", 1000, &randomatt_below );
	IniFile.GetInteger( "RANDATT", "ABOVE", 100, &randomatt_above );

    IniFile.GetInteger( "ENVIRONMENT", "HANGUP", 0, ( int* )&g_bHangup );
    IniFile.GetInteger( "ENVIRONMENT", "WGLIMIT", 0, ( int* )&wgLimit );
    IniFile.GetInteger( "ENVIRONMENT", "ISLIMITWG", 0, ( int* )&isLimitWG );

    IniFile.GetInteger( "ENVIRONMENT", "CHECKGMCMD", 0, ( int* )&bCheckGMCmd );
	
	int maxlevel = 0;
	IniFile.GetInteger( "ENVIRONMENT", "MAXPLAYERLEVEL", 75, ( int* )&maxlevel );
	MAXPLAYERLEVEL = maxlevel ;


    extern BOOL buyItemBind;
    IniFile.GetInteger( "ENVIRONMENT", "BUYITEMBIND", 1, ( int* )&buyItemBind );
    IniFile.GetInteger( "ENVIRONMENT", "AFLIKEHUNGUP", 1, ( int* )&afLikeHungup );
    //IniFile.GetInteger( "ENVIRONMENT", "WEARSCALE", 1000, ( int* )&g_dwEquipmentWearScale );
    IniFile.GetInteger( "ENVIRONMENT", "LOCKDISTANCE", 0, ( int* )&g_dwLockArea ); // 默认是半径为9的范围

	char temp[ 64 ] = { 0 };
	IniFile.GetString( "ZONESERVER", "SIDNAME", "DEF", temp, 64 );
	sidname = temp;

    extern DWORD unionMaxExp;
    extern WORD unionMax;
    int tempUnion = 0;
    int tempUnion2 = 0;
    IniFile.GetInteger( "UNION", "MAXEXP", 4000000, &tempUnion );
    IniFile.GetInteger( "UNION", "UNIONMAX", 2000, &tempUnion2 );
    unionMaxExp = ( DWORD )tempUnion;
    unionMax = ( WORD  )tempUnion2;

	byMulHours = static_cast<BYTE>(nTemp);
    IniFile.Clear();


    rfalse( 2, 1, "exp scale = %f\r\ndrop money scale = %f\r\ndrop item scale = %f\r\nMuilteTime = %d\r\n"
        "[消费模式 = %s]，[测试中的新功能 = %s]%s\r\n"
        "新的属性相克=%d，最大躲避率=%d\r\n"
        "[物品掉落等级限制]每相差%d级下降%d%%\r\n"
        "[掉落装备随机概率]等级线 %d 以下 %.2f%% 以上 %.2f%%\r\n"
        "[金钱变化日志]起记额度 %d\r\n"
        "[PK信息锁定功能] %s\r\n"
		"[玩家最高等级限制 = %d]",
        fExpScale, fDropMoneyScale, fDropItemScale, byMulHours, 
        ( sellType ? "元宝优先" : "赠宝优先" ), ( newly_ability_enabled ? "已开放" : "禁用" ), ( autoUnlock ? "，[自动取消锁定状态]" : "" ),
        useAttribAnti, maxParry, dropitem_levelMargin, dropitem_reduceRating,
        randomatt_levelLimit, randomatt_below / 100.f, randomatt_above / 100.f, rcdMoneyRank, g_dwLockArea ? "开放" : "关闭",MAXPLAYERLEVEL );

    if ( !cacheEnabled )
        rfalse( 2, 1, "[共享缓存备份功能] 未启动\r\n" );
    else
        rfalse( 2, 1, "[共享缓存备份功能] 已启动，定时备份间隔为 %d 秒\r\n", backupMargin / 1000 );

    if ( macLimit > 2 ) macLimit = 2;
    rfalse( 2, 1, "一机多开限制设定 = %s\r\n", macLimit ? ( ( macLimit == 1 ) ? "单开" : "双开" ) : "无限制" );
    
	extraNetLimitedInfo.clear();
	std::ifstream stream( "extranli.inf" );
	if ( !stream.is_open() )
		return;

	char line[1024];
	while ( !stream.eof() )
	{
		stream.getline( line, 1024 );
		if ( ( BYTE )line[0] <= ' ' )
			break;

		extraNetLimitedInfo.push_back( line );
	}
}

BYTE GetMultiTimeCount()
{
	return byMulHours;
}

void SetMultiTimeCount(BYTE multiTime)
{
	byMulHours = multiTime;
}