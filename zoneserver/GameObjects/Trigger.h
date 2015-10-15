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
		TT_MONSTER_DROP_ITEM,	// �������
		TT_USEITEM_ON_NPC,		// ��Npcʹ����Ʒ
		TT_USEITEM_ON_MONSTER,	// ��Monsterʹ����Ʒ
    };
    
    DWORD m_dwType;				// �����������б�
    
    union
    {
        struct
        {
            WORD	wNewRegionID;			
            WORD	wReserve;
            WORD	wStartX;
            WORD	wStartY;
			DWORD	dwNewRegionGID;		// ����֧����ת����̬����
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
			QWORD			buildingId;   // ���ڰ󶨵Ľ�����ȫ��Ψһ���
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
