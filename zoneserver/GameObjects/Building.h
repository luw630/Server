#pragma once

#include "BaseObject.h"
#include "fightobject.h"
#include "networkmodule/netmodule.h"
#include "networkmodule/BuildingTypedef.h"
#include "liteserializer/lite.h"

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

const __int32 IID_BUILDING = 0x1a3eb7c8; 

// 建筑物的基本属性

//struct SBuildingBaseData
//{
//    WORD wIndex;        // 建筑物索引编号
//    WORD wGraphIndex;   // 建筑物外观图片编号
//    BYTE byLevel;       // 属于那一层（地毯和遮盖暂时不计入这里边）
//    BYTE byChangeTA;    // 如何改变地图阻挡属性，或者不改变（Change Tile Attrib）
//    BYTE byCanClick;    // 是否以可交互对象的形式存在
//    WORD wMaxHP;        // 耐久度最大值。
//};

// 用于保存的建筑物属性
//struct SBuildingSaveData
//{
//    WORD wIndex;            // 建筑物索引编号
//    WORD wCurX;             // 建筑物所处于的位置
//    WORD wCurY;             // 。。。
//    WORD wCurHP;            // 当前耐久度。
//    WORD wClickScriptID;    // 点击脚本编号
//    WORD wDeadScriptID;     // 死亡脚本编号
//};


class CBuilding : public CFightObject
{
    IObject *VDC(const __int32 IID) { if ((IID == IID_ACTIVEOBJECT) || (IID == IID_FIGHTOBJECT) ) return this; return NULL; }

public:
	static int LoadBuildingData();                                  // 载入建筑物的数据
    static const SBuildingConfigData* GetBuildingData( WORD index );

	static BOOL MoveBuilding( LPCSTR name, WORD regionId, WORD x, WORD y,  BOOL checkBlk = TRUE );
	static BOOL MoveBuilding( WORD curregionId, QWORD uniqueId, WORD regionId, WORD x, WORD y, BOOL checkBlk = TRUE );
public:
    static void OnRecvBuildingMsgs( struct SBuildingBaseMsg *pMsg, class CPlayer *sender );

private:
	int OnCreate( _W64 long pParameter );
	void OnClose();

public:
    int AfterCreate( CPlayer *pPlayer, BOOL isGenerate );
    int DeforeRemove();
    BOOL GetStateMsg( void *data, WORD &size );
    QWORD GetUniqueId() { return property.uniqueId(); };
    WORD GetIndex() { return property.index; }
    WORD GetCurX() { return property.curX; }
    WORD GetCurY() { return property.curY; }

	BOOL CheckAreaBlock( CPlayer *pPlayer, CRegion *region, BOOL checkLmt = TRUE, BOOL checkMask = TRUE );
	BOOL RelationBlock( BOOL relation );		// 与场景阻挡相关联(  )
    
	virtual void OnDamage(INT32 *nDamage, CFightObject *pFighter){}

	BOOL MoveBuilding( WORD regionId, WORD x, WORD y, BOOL bCheckBlk = TRUE );
protected:
	void OnRun(void);

public:
    // 将整个建筑物的数据转换为lua表，压入lua环境中！（在这个过程中，部分数据应该会进行更新）
	int lua_PushTable( struct lua_State *L );

    // 使用脚本中的数据来更新建筑物的数据
	int lua_TabUpdate( struct lua_State *L, int idx ); // 直接使用表进行数据更新
	int lua_VarUpdate( struct lua_State *L, int idx ); // 使用 变量名、数值 进行更新。。。（所以 idx = string | idx+1 = value ）

public:
    // 针对于 svarMap 的操作，取出对应键的值！
    BOOL GetInteger( LPCSTR key, int &value );
    LPCSTR GetString( LPCSTR key, size_t *size = NULL );

	size_t OnSave( LPVOID pBuf, size_t bufsize );

public:
    CBuilding(void);
    ~CBuilding(void);

public:
    struct CreateParameter
    {
        // 建筑物类型,放置模式是否反向,建筑物坐标
        WORD index, inverse, x, y;
        BYTE ownerType;
		char ownerName[CONST_USERNAME];
        LPCVOID loadData;   // 这个是用于读取时直接指定数据用的！
    } param;

    WORD checkId;   // 用于更新网络同步的标记！
    DWORD controlId;    // 脚本用控制ID

	BOOL ModifyBuilding( int type, int value, BOOL isSyn = TRUE );

private:
    SBuildingDynamicData property;

    // 用于脚本扩展的变量列表...
    // 注: 由于我们的脚本系统不是一个可连续持久化的环境( 使用reset之类的操作可直接重置所有脚本数据... )
    // 所以直接暂时把这些数据转移到系统内存中, 在合适的时机再次返回脚本环境中进行数据读写...
    std::map< std::string, lite::Variant > svarMap; 
};
