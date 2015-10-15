//BuildingTypedef.h
#pragma once
#include "itemtypedef.h"
#include "datamsgs.h"

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// �����������Ϣ��, Ϊ�˲�Ӱ����Ŀ֮��İ�����ϵ�߼�,
//=============================================================================================
DECLARE_MSG_MAP(SBuildingBaseMsg, SMessage, SMessage::EPRO_BUILDING_MESSAGE)
//{{AFX
EPRO_BUILDSTRUCTURE,            // ��ʼ���콨����
EPRO_SYN_BUILDING_INFO,          // ͬ�����������Ϣ
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

// ������Ļ�������
//struct SBuildingBaseData
//{
//    WORD index;         // ������������ţ�ֱ���Ա�ŵĴ�С���ж�ͼ�㣬1w����Ϊ��̺�㣬1~2wΪ����㣬1~3wΪ������������
//    WORD graph;         // ���������ͼƬ���
//
//    QWORD tileMask;     // 8x8�赲��Ϣ��
//
//    // ������ͼƬ�����������Ӧ�����赲����
//    BYTE centerX, centerY; 
//
//    BYTE buildType;     // �������ͣ���ʱ���Զ���������ʱ���������ĳɳ�����Ҫ����Ͷ������������ģ�
//    BYTE buildSpeed;    // �����ٶȣ���λΪ���ӣ�
//    BYTE material;      // ���󽨲����ͣ�ͬʱҲ�����˸ò��ʵĿ�������������������ֻ�趨���Ĵ����ͣ�������ͬ�����͵Ĳ��ʻᵼ�½��������ĸı䣩
//    BYTE maintenance;   // �ɳ�ֵ�ٶȣ�Ϊ�����ٶȵı��ʣ�10λ�ģ�
//
//    DWORD maxHP;        // �;ö����ֵ��
//
//    union
//    {
//        struct Defender
//        {
//            BYTE type;      // ������������
//            BYTE range;     // ��������
//            BYTE atkSpeed;  // ����Ƶ��
//            BYTE reserve;   // ����
//            WORD damage;    // ���������ɵ��˺�ֵ
//        };
//
//        struct Scripter
//        {
//            DWORD scriptId; // ����ű�
//        };
//
//        struct Decker
//        {
//        };
//    };
//};
//
//// ���ڱ���Ľ��������� 40BYTE
//struct SBuildingSaveData : 
//    public SItemBase    // ������������Ϊ��������
//{
//    // ---------- 8byte -----------
//    // WORD wIndex;         ��������
//    // BYTE size  : 6;      ��Ҫ��������ݴ�С!
//    // BYTE flags : 2;      ������״̬�������������У�����У�ͣ�ã�
//    // QWORD uniqueId;      Ψһ��ʶ��,��������������!
//
//    // ---------- 8byte -----------
//   
//    DWORD curHP;            // ��ǰ�;öȡ�
//
//    // ---------- 12byte -----------
//    WORD scriptValue[6];    // 6����������!
//
//    // ---------- 12byte -----------
//    BYTE ownerType : 2;     // ӵ�������ͣ����⡢���ˡ����ɡ�������
//    BYTE openType : 1;      // �Ƿ���⿪�ţ�δ���š�������
//    char owner[CONST_USERNAME];         // ӵ��������!
//};
//
struct WPOINT
{
    short x, y;
};

// һ��16 + 12���ֽڣ�
struct SaveBlock_fixed
{
    QWORD reserve : 24;
    QWORD uniqueId : 40;
    WORD index;         // ��������������ݱ��
    WORD parentId;      // ������ID
    WORD x, y;

    BYTE ownerType;     // ӵ��������
	char ownerName[CONST_USERNAME]; // ӵ������

    inline QWORD UniqueId() const 
    { 
        return reinterpret_cast< const QWORD& >( reinterpret_cast< const char* >( this )[3] ) & 0x000000ffffffffff; 
    }
};

struct WLINE
{
    WPOINT seg, end;
};

//�����ñ��ж�ȡ����������
struct SBuildingConfigData
{
    WORD index;				        // �������������
    BYTE centerx, centery;          // �����������ڵ�λ��
    DWORD image;				    // ���������ͼƬ���	
    WORD totalBuildingState;		// �ܵ��޽�����
    WORD ctrlScript;				// Ĭ�ϴ���/ɾ���ű� �������½�ʱ���õĴ����ű���ţ���������ң�Ҳ������ϵͳ�ű���
    WORD initScript;				// Ĭ�ϼ��ؽű�
    //WORD clickScriptId;             // ����ű���ţ����ܽű���
    //WORD deadScriptId;              // �����ű���ţ����ٽű��أ�
    DWORD hpMax;                    // ����;öȣ���ĳЩ����¿ɸ��ģ�
	WLINE line;						// �ڵ���
    union
    {
        QWORD qwtileMask;			//�赲
        DWORD dwtileMask;
    };

	union
    {
        QWORD qwtileLimit;			// ��������
        DWORD dwtileLimit;
    };


    inline void reset()
    {
        memset( this, 0, sizeof *this );		
    }
};

//// ������Ϸ�еĽ��������ݵĹ̶��ṹ
struct SBuildingFixedData
{
    union
    {
        struct
        {
            DWORD inverse           : 1;    // �Ƿ���ʾ��ת
            DWORD usingState        : 2;    // �������Ӧ��״̬
            DWORD ownerType         : 2;    // �������ӵ��������[0:ϵͳ][1:����][2:����][3:����]
			DWORD showTitle		: 1;	// �Ƿ���ʾ����
            DWORD state_Ex          : 18;   // ��δʹ�õ�bit��
            DWORD uniqueId_Hi       : 8;

            DWORD uniqueId_Lo;
        };

        struct
        {
            QWORD _state             : 24;       // Bit״̬��ʶ״̬��ʶռ��24��bit��������չ �޽���������ά�������� 

            // Ψһ��ʶ��,��������������!40��bit���͵��ߵĸ�ʽһ��
            // 24 : 14 : 2 ��λ������˳����Ϊ���ܹ������ֱ�ӽ���������Ŷ�λ�ڸ�2bit���������ݿ���ֱ�Ӳ������Ͳ�ѯ��
            QWORD uniqueTimestamp   : 24;       // [ʱ���      ] 24bit����30��Ϊ��λ���м�¼����������ƿɴﵽ15.96��
            QWORD uniqueCreateIndex : 14;       // [10�����ۼ�ֵ] 14bit��ÿ30���ڵ��������Ͽ�����16384������
            QWORD uniqueServerId    : 2;        // [���������  ] 2bit����¼���ߵĲ���λ�ã��Ա����ڲ�ͬ�������ϵı���ظ�
        };
    };

    WORD index;                         // �������������
    WORD curX, curY;				    // �����������ڵ�λ��
    WORD initScriptId;                  // ��ʼ���ű���ţ������������ڳ����ϵ�ͬʱ�ʹ�����ʼ���ű���
    WORD clickScriptId;                 // ����ű���ţ����ܽű���
    WORD deadScriptId;                  // �����ű���ţ����ٽű��أ�
    DWORD hpMax;                        // ����;öȣ���ĳЩ����¿ɸ��ģ�
    std::string viewTitle;			    // ��������
    std::string owner;			        // ӵ�������ƣ������ǰ��ɣ�Ҳ�����Ǹ��ˣ�

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
    WORD buildProgress;         // ��ǰ���޽�����
    WORD functionScipt;         // ���ܽű�
    DWORD hp;                   // ��ǰ�;öȡ�

    inline void reset()
    {
        memset( this, 0, sizeof *this);		
    }
};

//�������������������¼������

DECLARE_MSG(SBuildingSaveMsg,SBuildingBaseMsg,SBuildingBaseMsg::EPRO_BUILDING_SAVE )
struct SQBuildingSaveMsg : public SBuildingSaveMsg
{
	WORD	wParentID;		//������ID
	DWORD   dwServerID;		//������ID
	WORD	wNumbers;		//�����������ݳ�Ա����
	WORD	wBufSize;		//�������л���������С		
	BYTE	Buffers[1];		//����Ӧ�ã���ջ�ϵľ�̬�ڴ���������л����ݱ���		
};

//struct SABuildingSaveMsg: public SBuildingSaveMsg
//{
//	BYTE retMsg;
//};

DECLARE_MSG(SBuildingGetMsg,SBuildingBaseMsg,SBuildingBaseMsg::EPRO_BUILDING_GET )
struct SQBuildingGetMsg : public SBuildingGetMsg
{
	WORD	wParentID;		//������ID
	DWORD	dwServerID;		//��½������ID
	DNID	dwZoneID;		//���������ID���������ûش�Ŀ�꣡��
};

struct SABuildingGetMsg: public SBuildingGetMsg
{
	enum
	{
		RET_SUCCESS,
		RET_FAIL
	};
	BYTE	retMsg;
	DNID	dwZoneID;		//���������ID���������ûش�Ŀ�꣡��
	WORD	wParentID;
	WORD	wNumbers;
	WORD	WBufSize;
	BYTE	Buffers[1];
};



