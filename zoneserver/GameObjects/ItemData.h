#pragma once

#include "networkmodule\itemtypedef.h"

#define MakeLifeTime(m) ((m) * 60000)
#define MAX_ITEMSCOPE   65535

#include <list>
using namespace std;

class CItemData
{
public:
	static int LoadItemData( LPCSTR filename, bool crcInit = true, BOOL refresh = FALSE );        // 载入所有道具的配置数据
	static int LoadSpecialItemData( LPCSTR filename, BOOL refresh = FALSE );                      // 读取又特殊属性的物品
	/*static SpecialItem* GetSpecialItem( DWORD id );*/
    /*static const SItemData *GetItemData( int index );                       // 获取指定道具的配置数据*/
	static int LoadSpecialItemAttribute( LPCSTR filename, BOOL refresh = FALSE );                      // 读取又特殊属性的物品
	/*static SGemAtt* GetSpecialItemAttribute( DWORD id );*/
    // 更新&添加&删除道具的配置数据
    // itemData == NULL 恢复默认设置
    // itemData.wItemID == index 更新设置
    // itemData.wItemID == -1 删除该设置
    /*static BOOL UpdateItemData( int index, const SItemData *itemData = NULL, BOOL fullUpdate = false ); */

    // 主要用于客户端道具数据有效性检测！
    /*static std::pair< DWORD, const SItemData* >* GetRawData( int index );  */
};

