#pragma once

//#define USEMAPCOLLISION 

#include "BaseObject.h"
#include "networkmodule/BuildingTypedef.h"
#include "networkmodule/UpgradeMsgs.h"
#include "player.h"
#include "trigger.h"
#include "MapServer.h"

const __int32 IID_REGION = 0x11030636; 

enum
{
	LimitItem = 0,		
	UnDealItem,		
	LimitSkill,		
	TrigerBuffMonster,	
	TrigerBuffPlayer,	
	Limit_max,
};

#define MY_MAP_VERSION	4

#define GetCurArea(pos, len) ((pos)/(len)) // (((pos+1)/len) + (((pos+1)%len)?1:0) - 1)

// ���η����еĳ����ṹ˵��
// Region�ǳ��������壬��һ�ŵ�ͼΪ��λ��ʹ��.map�ļ�
// ��Region�д���һϵ�е�Area
// Area�Ծ���ķ�ʽ���У���Ϊһ������С�������Խ���󳡾���ɵĸ���
// AddObject������ӽ�ɫ�ȴ���AI�Ķ���������Ҳ���Կ���һ�¶�̬�ı䳡��
class CMonster;
class CNpc;
class CBuilding;

class CRegion : public IObject, public CObjectManager
{
public:
	typedef SAUpdateNpcListMsg::NpcInfo NpcList;

private:
	IObject *CreateObject(const __int32 IID);

public:
    IObject *VDC(const __int32 IID) { return (IID == IID_REGION) ? this : IObject::VDC(IID); }

public:
	CRegion() : m_MutateMonsterNum(0) {}

	virtual int AddObject(LPIObject pChild);
	virtual int DelObject(LPIObject pChild);
	void OnClose();

protected:
	void OnRun(void);
	int OnCreate(_W64 long pParameter); // ���ﴫ���ֻ��һ��ID�ţ����ݾ����ID������Ӧ�ĳ���

private:
	int LoadTileMark(WORD ID);			// װ�ص�ͼ�赲��Ϣ
	int LoadNpc(LPCSTR filename);		// װ�س���NPC
	int LoadMonster(LPCSTR filename);	// װ�س�������
	int LoadTrigger(LPCSTR filename);	// װ�س������崥����
	int LoadTimer(LPCSTR filename);		// װ�س���ʱ�䴥����
    int LoadMonsterBR(LPCSTR filename); // װ�س�����������ˢ������
	///�µ�ͼ��ײ
	 int LoadRegincollision(WORD ID); // װ�ص�ͼ�赲��ײ
	 
public:
	LPIObject &GetArea(WORD x, WORD y);
    const class CTrigger *GetTrapTrigger(WORD index);
	BOOL Move2Area(LPIObject pObj, WORD x, WORD y, bool bUseTile = false);	// ��һ������ת�Ƶ�Ŀ������
	void GetAreaNpcList(std::vector<CRegion::NpcList>& npcList);
	bool IsCanMove(float x,float y);
	bool		m_bUseCollision;
	void SetUseMapCollision(bool use){m_bUseCollision=use;}
public:
	LPIObject SearchNpcByScriptID(DWORD dwID);
	LPIObject SearchObjectListByRegion(DWORD dwGID);
	LPIObject SearchObjectListInArea(DWORD dwGID, WORD x, WORD y);
	LPIObject SearchObjectListInAreas(DWORD dwGID, WORD xCenter, WORD yCenter);

    LPIObject SearchObjectByPos(WORD xCenter, WORD yCenter);

	// ������������º�������չ�棬���ֵľ��巽ʽ�Ǹö����ų�����һ������λ�ý��й�����������һ���ӿڻ��ǳ����ĳ�Ա
	// �����������˵�ǵ��幥������ʵ���֣��������������ӳ٣������ò���Ӧ���幥����������Ϊ��Χ������һ����Ҫ�ӿ�
	// pFStyle�ǹ����ķ�ʽ��SFightStyle����ṹ���������п��ܵĹ�����ʽ���Լ��ù�����ʽ�������Ĺ�������
	// ptPosition�ǹ����ľ��峡�����꣬����ֵֻ��һ���ɹ����ı�־��û��ʵ�ʵ����壬�����Ժ���չ
	DWORD AttackPositon(struct SFightStyle *pFStyle, POINT ptPosition);

    int Broadcast(struct SMessage *pMsg, WORD wSize, DNID dnidExcept);

	LPIObject GetObjectInRegin(DWORD dwGID);///�ڵ�ͼ�в������

public:
    DWORD GetTileMark(WORD CurX, WORD CurY);
    void MarkTileStop(WORD CurX, WORD CurY, DWORD &PrevMark, DWORD GID);
    bool isTileWalkAble(WORD CurX, WORD CurY);
    bool isTileStopAble(WORD CurX, WORD CurY, DWORD GID);

private:
	LPIObject	*AreaTable;		// �ܵ������б��ʾ�������

public:
	WORD		m_nAreaW;		// ��AreaΪ��λ�ĳ������
	WORD		m_nAreaH;				

public:
	WORD		m_wRegionID;		// ���������
	WORD		m_wMapID;			// ��������ͼ���

	WORD		m_RegionW;			// ��TileΪ��λ�ĳ������
	WORD		m_RegionH;			
	DWORD		*m_pdwTiles;		// ����������Tile��������
	DWORD		*m_pdwTileMarks;	// ����������Tile�����������
#ifdef USEMAPCOLLISION
	MapServer   *m_pmapsever;		//������ײ����
#endif
public:
	// ���ﻹ���Դ��һЩ
	// ����б�ȣ���Ҫʹ�ó��������������
	check_list<LPIObject>		m_PlayerList;		// ���ɶ�����б�
    std::map<QWORD, LPIObject>	m_BuildingMap;		// ���ɽ�������б����û������������ı�����鷳��
    int							prevBuildingNumber;	// �ϴα���ʱ����Ч�Ľ���������

	DWORD	m_MapProperty;			// ��������
	WORD	m_wReLiveRegionID;		// �����㳡�����
	POINT	m_ptReLivePoint;		// ����������
	WORD	m_BgMusic;				// ����WAV���
	int		m_bLoopPlay;			// �����Ƿ�ѭ��
	DWORD	m_Weather;				// �����б�
	DWORD   m_dwNeedLevel;			// �ȼ�����
	WORD	m_dwPKAvailable;		// PK����
    DWORD   m_dwPKValueLimit;		// PKֵ����
    DWORD   m_dwSpecialAccess;		// ���⼼������
	WORD	m_wSkillAccess;			// ��������
	BOOL	m_bAllowedMount;		// �Ƿ���������
	BYTE		m_nSafeZoneInfo;			//PK����1Ϊ��ȫ����0Ϊ����PK����

    BOOL    m_IsHangup;				// �һ�����
    BOOL    m_IsChildProtect;		// ���ֱ���
    BOOL    m_IsDamageVenation;		// ������������
    BOOL    m_IsBlock;				// �Ƿ�����赲

    BOOL    m_IsBatchrRefresh;		// ����ˢ��
    float   m_wRegionMultExp;		// �����౶

	WORD	m_MutateMonsterNum;		// ɫ����

	// �������縱���������ⳡ���������
	std::vector<WORD>	m_vLimitItemID;			// ������ʹ�õ�ID�б�
	std::vector<WORD>	m_vUnDealItemID;		// �����ƽ����Լ���������Ʒ�б�
	std::vector<WORD>	m_vLimitSkillID;		// ������ʹ�õļ���ID�б�
	std::vector<BYTE>	m_vTrigerBuffMonster;	// ���ﴥ����BUFF�б�
	std::vector<BYTE>	m_vTrigerBuffPlayer;	// ��Ҵ�����BUFF�б�
	WORD				m_wLimitMaxHP;			// �����������ޣ�0Ϊ������
	WORD				m_wLimitMaxMP;			// ����ħ�����ޣ�0Ϊ������
	WORD				m_wLimitMaxSP;			// �����������ޣ�0Ϊ������
	BOOL				m_bIsLimitChangeEquip;	// �Ƿ����Ƹ���װ��
	BOOL				m_bIsLimitAllItem;		// �Ƿ��������е�Item
	WORD				m_wPracticeSkillType;	// �õ�ͼ�����������书����	

    // ��������Ч��
    struct SRegionSkillEffect
    {
        enum
        {
            SkillEffectMAX = 16,
        };

        WORD id;    // �书ID
        WORD odds;  // ǧ�ֱ�
    };
    SRegionSkillEffect skillEffect[SRegionSkillEffect::SkillEffectMAX];

    // [��ͨPK] [�����ȼ�PKֵ��Ȩ] [����PK] [����PK] [PK���������ˣ�������PKֵ������װ��]
	enum PKA_TYPE { PAT_REGIONLEVEL, PAT_NORMAL, PAT_NULL, PAT_PKFREE, PAT_PKCHAN }; 

	enum ZONE_INFO
	{
		PK_SAFE,		///��ȫ����
		PK_FREE,		///����PK
		PK_NOTPKVALUE, //����ģʽ
	};

	// �������͹�ϵ��
	// ��ϵ�� { û�У� ����ϵPKֵ�� ����ϵ����ֵ, ����2��3�� }
	enum PKX_TYPE { PXT_NULL, PXT_NOTPKVALUE, PXT_NOTXVALUE, PXT_BOTHPKX };
	DWORD	m_dwPKX;			

    // ���崥�����б�
    BYTE					m_byMaxTrap;
    class CTrigger			*m_pTrap;
    std::map<int, CTrigger> trapMap;
    std::set<DWORD>			m_LimitItemList;	// ����������ʹ�õ���Ʒ�б�
    DWORD					m_dwAIRefCount;

private:
    std::vector<BYTE>		m_TrapList;			// ��̬����������������б�
    WORD					m_ExperMul;			// ���鱶��

public:
    //class AstarPathFind *m_pAstar;
    
    WORD        GetExperMul(void) const { return m_ExperMul; }
    void        SetExperMul(WORD wNum) { m_ExperMul = wNum; }
    // ���崥���� ��ز���
    BYTE        GetMaxTrap() const { return m_byMaxTrap; }
    class       CTrigger *GetTrapPtr() { return m_pTrap; }
    void        DestroyTrap();  
	bool		SetTrap( WORD byOrder, WORD x, WORD y, CTrigger *pTrap );
    class       CTrigger *MakeNewTrap(BYTE byMax);

    struct DummyFunctor { void operator () ( ... ) {} };
	
	// ���ɶ���
	CFightPet			*CreateFightPet(PVOID pParameter);
	CMonster			*CreateMonster(PVOID pParameter, DWORD controlId = 0);
	virtual CNpc        *CreateNpc(PVOID pParameter, DWORD controlId = 0);
    virtual CBuilding   *CreateBuilding(PVOID pParameter, CPlayer *pPlayer, BOOL isGenerate = FALSE, DWORD controlId = 0); 

    // ɾ������
	int					DelFightPet(DWORD dwGID);
    virtual int			DelMonster(DWORD dwGID);
    virtual int         DelNpc(DWORD dwGID);
    virtual int         DelBuilding(DWORD dwGID); 

	//add by yuntao.liu
	check_list<LPIObject>* GetMonsterListByAreaIndex(INT32 x,INT32 y);
	check_list<LPIObject>* GetPlayerListByAreaIndex(INT32 x,INT32 y);

	//add by ab_yue
	void SetParamToList(WORD type, char* str);		//�������ͽ����������ݴ���ʽ������Ӧ������
	void ClearVectorByType(WORD type);
	void PushItemInVevtor(WORD type,const char* str);
	bool CanUseItem(WORD id);
	bool CanDealItem(WORD id);
	bool CanUseSkill(WORD id);
	//end

private:
    std::map< DWORD, DWORD > controlIdMap;

public:
    DWORD       CheckObjectByControlId( DWORD controlId );
    LPIObject   GetObjectByControlId( DWORD controlId );
    BOOL        RemoveObjectByControlId( DWORD controlId );
    BOOL        RemoveBuilding( QWORD uniqueId );
	BOOL		RemoveBuilding( CBuilding *pBuilding, BOOL isDfRemove = TRUE );
    BOOL        SaveBuildings();
    BOOL        LoadBuildings( struct SABuildingGetMsg *pMsg );
	int			GetMonsterCount(bool CountDead = false);
	int			GetLiveMonster(WORD monsterID);//����ID
	int			GetNpcCount();
	int			GetPlayerCount(int state);
	void			KillMonster(CPlayer *player);

    DWORD       m_dwTickTime;

    void        SetRegionBRType( DWORD dwBRTeamNumber, WORD wRefreshStyle );
};

