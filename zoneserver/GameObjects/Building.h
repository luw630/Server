#pragma once

#include "BaseObject.h"
#include "fightobject.h"
#include "networkmodule/netmodule.h"
#include "networkmodule/BuildingTypedef.h"
#include "liteserializer/lite.h"

/*/ 
����ϵͳ������
    �µĶ������ͣ��ɵ�ԭ�ж������ģʽ���ۺϳ�Ϊ����ϵͳ��
    �������Ż�ӵ�����¼����ص㣺
    1�����
    2����������㣨��̺���ڸ���ʱ����������ߣ�
    3����θı��ͼ�赲���ԣ����߲��ı䣨ʵ��̨�ף�Χǽ���ſ򣬳���ڡ�������
    4���Ƿ��Կɽ����������ʽ����
    5������4���趨������������Ƿ�����ƻ����;öȡ�

    ���������ͬ�����ƣ�
    ���������������ͬ����ʽ��ͬ��
    ������������Ϊ��ȫ�ֱ�ţ�XY���꣬�������ţ��;ö�
    ��ʣ�µ����ݸ��ݲ�������ͬ����Ϣ��С������16���ֽ�����
/*/

const __int32 IID_BUILDING = 0x1a3eb7c8; 

// ������Ļ�������

//struct SBuildingBaseData
//{
//    WORD wIndex;        // �������������
//    WORD wGraphIndex;   // ���������ͼƬ���
//    BYTE byLevel;       // ������һ�㣨��̺���ڸ���ʱ����������ߣ�
//    BYTE byChangeTA;    // ��θı��ͼ�赲���ԣ����߲��ı䣨Change Tile Attrib��
//    BYTE byCanClick;    // �Ƿ��Կɽ����������ʽ����
//    WORD wMaxHP;        // �;ö����ֵ��
//};

// ���ڱ���Ľ���������
//struct SBuildingSaveData
//{
//    WORD wIndex;            // �������������
//    WORD wCurX;             // �����������ڵ�λ��
//    WORD wCurY;             // ������
//    WORD wCurHP;            // ��ǰ�;öȡ�
//    WORD wClickScriptID;    // ����ű����
//    WORD wDeadScriptID;     // �����ű����
//};


class CBuilding : public CFightObject
{
    IObject *VDC(const __int32 IID) { if ((IID == IID_ACTIVEOBJECT) || (IID == IID_FIGHTOBJECT) ) return this; return NULL; }

public:
	static int LoadBuildingData();                                  // ���뽨���������
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
	BOOL RelationBlock( BOOL relation );		// �볡���赲�����(  )
    
	virtual void OnDamage(INT32 *nDamage, CFightObject *pFighter){}

	BOOL MoveBuilding( WORD regionId, WORD x, WORD y, BOOL bCheckBlk = TRUE );
protected:
	void OnRun(void);

public:
    // �����������������ת��Ϊlua��ѹ��lua�����У�������������У���������Ӧ�û���и��£�
	int lua_PushTable( struct lua_State *L );

    // ʹ�ýű��е����������½����������
	int lua_TabUpdate( struct lua_State *L, int idx ); // ֱ��ʹ�ñ�������ݸ���
	int lua_VarUpdate( struct lua_State *L, int idx ); // ʹ�� ����������ֵ ���и��¡����������� idx = string | idx+1 = value ��

public:
    // ����� svarMap �Ĳ�����ȡ����Ӧ����ֵ��
    BOOL GetInteger( LPCSTR key, int &value );
    LPCSTR GetString( LPCSTR key, size_t *size = NULL );

	size_t OnSave( LPVOID pBuf, size_t bufsize );

public:
    CBuilding(void);
    ~CBuilding(void);

public:
    struct CreateParameter
    {
        // ����������,����ģʽ�Ƿ���,����������
        WORD index, inverse, x, y;
        BYTE ownerType;
		char ownerName[CONST_USERNAME];
        LPCVOID loadData;   // ��������ڶ�ȡʱֱ��ָ�������õģ�
    } param;

    WORD checkId;   // ���ڸ�������ͬ���ı�ǣ�
    DWORD controlId;    // �ű��ÿ���ID

	BOOL ModifyBuilding( int type, int value, BOOL isSyn = TRUE );

private:
    SBuildingDynamicData property;

    // ���ڽű���չ�ı����б�...
    // ע: �������ǵĽű�ϵͳ����һ���������־û��Ļ���( ʹ��reset֮��Ĳ�����ֱ���������нű�����... )
    // ����ֱ����ʱ����Щ����ת�Ƶ�ϵͳ�ڴ���, �ں��ʵ�ʱ���ٴη��ؽű������н������ݶ�д...
    std::map< std::string, lite::Variant > svarMap; 
};
