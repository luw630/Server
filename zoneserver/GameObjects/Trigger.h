#pragma once

#include "networkmodule\playertypedef.h"

class CTrigger
{
public:
    CTrigger(void);
    ~CTrigger(void);

public:
    enum 
    {
        TT_CHANGE_REGION = 1,
        TT_CLICK_ON_NPC,
        TT_MONSTER_HALF_HP,
        TT_MONSTER_NEWLIFE,
		TT_MONSTER_AI,
        TT_MONSTERDEAD,
        TT_MAPTRAP_EVENT,
        TT_CLICK_ON_PLAYER,
        TT_PLAYERDEAD,
        TT_MAPTRAP_BUILDING,
        TT_CLICK_ON_BUILDING,
		TT_MONSTER_DROP_ITEM,	// 怪物掉落
		TT_USEITEM_ON_NPC,		// 对Npc使用物品
		TT_USEITEM_ON_MONSTER,	// 对Monster使用物品
    };
    
    DWORD m_dwType;				// 触发器类型列表
    
    union
    {
        struct
        {
            WORD	wNewRegionID;			
            WORD	wReserve;
            WORD	wStartX;
            WORD	wStartY;
			DWORD	dwNewRegionGID;		// 增加支持跳转到动态场景
        }SChangeRegion;

        struct
        {
            class CPlayer	*pPlayer;
            class CNpc		*pNpc;
        }SClickOnNpc;
		
		struct
        {
            class CPlayer	*pPlayer;
            class CNpc		*pNpc;
			DWORD			dwItemID;
        }SUseItemOnNpc;

		struct
        {
            class CPlayer	*pPlayer;
            class CMonster	*pMonster;
			DWORD			dwItemID;
        }SUseItemOnMonster;

        struct
        {
            class CMonster	*pMonster;
            DWORD			dwScriptID;
        }SMonsterHalfHP;

        struct
        {
            class CMonster	*pMonster;
            DWORD			dwScriptID;
        }SMonsterNewLife;

		struct 
		{
			class CMonster	*pMonster;
			class CPlayer	*pPlayer;
			DWORD			dwScriptID;
		}SMonsterAI;

        struct 
        {
			class CMonster	*pMonster;
			class CPlayer	*pPlayer;
			DWORD			dwScriptID; 
        }SMonsterDead;

        struct 
        {
            WORD			wCurX;
            WORD			wCurY;
			DWORD			dwScriptID; 
			QWORD			_variant;
			lite::Variant const& GetVariant() const { return reinterpret_cast< lite::Variant const & >( _variant ); }
			lite::Variant& GetVariant() { return reinterpret_cast< lite::Variant& >( _variant ); }
        }SMapEvent;

        struct 
        {
			class CPlayer	*pPlayer;
		    DWORD			dwScriptID; 
            DWORD			dwPPlayerID;
        }SClickOnPlayer;

        struct 
        {
            WORD			wCurX;
            WORD			wCurY;
			DWORD			reserve; 
			QWORD			buildingId;   // 用于绑定的建筑物全局唯一编号
        }SMapEvent_Building;

        struct 
        {
			class CBuilding	*pBuilding;
			class CPlayer	*pPlayer;
        }SClickOnBuilding;

		struct
		{
			class CMonster	*pMonster;
			class CPlayer	*pPlayer;
			DWORD			dwDropItemID; 
		}SMonster_DropItem;
    };
};
