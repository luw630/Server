#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////
// ���ݶ���ԭ��
// ��������Ķ��ǻ�ȫ���õ������ݽṹ
///////////////////////////////////////////////////////////////////////////////////////////////

static const int MAX_ITEM_NUMBER = 64;
static const int MAX_TASK_NUMBER = 200;

static const int ITEMNAME_MAXLEN = 40;

static const int REDUCETIME = 946569600;	// �󶨵��ߵ�ƫ��ʱ�� 1999-12-31 00:00:00
// �����б�
enum    E_SCHOOL
{
    ES_SHAOLIN=0,                   // ����  (0-15)
    ES_WUDANG,                      // �䵱  (16-31)
    ES_EMEI,                        // ����  (32-47)
    ES_TANGMEN,                     // ����  (48-63)
    ES_MOJIAO,                      // ħ��  (64-79)
    //...

    ES_MAX
};

enum EItemType { 
//  ��Ǯ
    MONEY = 0, 
//  �����ص�_��ʹ�ã�  ���ص�_��ʹ�ã�     �����ص�_��ʹ�ã�           �ű���_��ʹ�ã�
    NORMAL_USABLE = 2, OVERLAP_USABLE = 4, SPECIAL_OVERLAP_USABLE = 6, SCRIPT_USABLE = 8,
//  �����ص�_����ʹ�ã����ص�_����ʹ�ã�   �����ص�_����ʹ�ã�         �ű���_����ʹ��
    NORMAL_UNABLE = 3, OVERLAP_UNABLE = 5, SPECIAL_OVERLAP_UNABLE = 7, SCRIPT_UNABLE = 9,
//  ��Ƕ��_�����㣬  װ����_������        , ��������[�ֿ�][����]
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
inline BOOL ISROSE( int idx )               { return ( ( idx >= 51520 ) && ( idx <= 51529 ) ); } // õ��

static const int BASE_PACKAGE_GOOD = 10000;
static const int BASE_PACKAGE_WAREHOUSE = 10100;
//static const int BASE_GOOD_NUMBER = 24;     // ��չ������
static const int BASE_PACKAGE_NUMBER = 64;     // ��չ�ֿ�

//typedef unsigned __int64 QWORD;

#pragma pack (push)
#pragma pack (1)

// ������ģ������ã��������ݽṹ
//  һ��ItemBuffer �ṹʾ�� 
// 0x14002008000BCD0200000000000000002D2D2D2D2D0000000000000000000000
// UniqueID = 0x02cd0b0008    �ܹ�10*4  λ
// 20 �ϲ�����2*4���� ��Ʒ����ռ���Ƿ�����
//1400  �ܹ�4*4 ������ƷItemIndex  0x0014

struct SItemBase 
{
    enum { 
        F_BINDED        = 0x1,       // �Ѿ����޷�[����]
        F_DISABLED      = 0x2,       // �Ѿ���ֹ�޷�[����/ʹ��/�϶�/����/����]
    };

    // ȫ��Ψһ��ţ���40bit
    // ############################
    // ע�⣺������������
    // ############################
    // ��Ϊ�������ݿ�1�϶����������������������ſ����ظ���Ŀǰ3������ֱ�ռ��һ����
    // ���Բ��������ֵ��Ϊ�ؼ����ж����ݣ�����
    // ��Ҫע�⣡����
    // ----------------------------
    // ������ݿ�1�϶�Ľ��������������uniqueTimestamp��bit�����������������
    // ʹ��һ����̬���ռλ��������ô��1��2����֧��8.98�꣬1��4����֧��4.49�꣬Ӧ�ù��˰ɡ���
    // ____________________________
    static const YEAR_SEGMENT       = 2007;         // ʱ�������ʼʱ��Ϊ2007��
    static const MAX_ACCUMULATOR    = 0x3fff;       // 30�����ۼ�ֵ�����ֵΪ0x3fff == 16383

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
            QWORD   wIndex            : 16;     // ����Ʒ�б��еı�ţ�ͨ�������Բ�����й��ڸõ��ߵ�����
            QWORD   size              : 6;      // ��������ռ�õ��ڴ�ռ䣬������SItemBase��8�ֽڣ�����Ч���ݴ�С��
            QWORD   flags             : 2;      // �����ߵĵ������Ա�־��

            // 24 : 14 : 2 ��λ������˳����Ϊ���ܹ������ֱ�ӽ���������Ŷ�λ�ڸ�2bit���������ݿ���ֱ�Ӳ������Ͳ�ѯ��
            QWORD   uniqueTimestamp   : 24;     // [ʱ���      ] 24bit����30��Ϊ��λ���м�¼����������ƿɴﵽ15.96��
            QWORD   uniqueCreateIndex : 14;     // [10�����ۼ�ֵ] 14bit��ÿ30���ڵ��������Ͽ�����16384������
            QWORD   uniqueServerId    : 2;      // [���������  ] 2bit����¼���ߵĲ���λ�ã��Ա����ڲ�ͬ�������ϵı���ظ�

        } details;

        // ����details�ڲ�ʹ�õ���λ������ֱ�ӷ��ʷ��صĶ���64λ���ݣ�����Ӵ������㸺����
        // ʹ�øýṹ�������ϣ���Ҫ��ʹ���ڴ����ⲿ��������ʱ������ֱ��ȡ��һ����Ϊ���㴦�������
        struct
        {
            WORD wIndex;
            BYTE size  : 6;
            BYTE flags : 2;
        };
    };
};

// ԭʼitem���ݿռ��С
struct SRawItemBuffer : 
    public SItemBase
{
    BYTE buffer[56]; // max dummy buffer
};

// �ű�����ߣ���Ҫ���ڴ���ű�����
// 8�ֽڻ����������ݣ�1�ֽ���Чλ�жϣ�54�ֽڵĽű�������
// ��Ϊsizeֻ��6bit���ܱ�������ֵΪ63������
struct SScriptItem : 
    public SItemBase
{
    BYTE declare;       // λ����[ͬʱҲ�����˽ű��������ֻ����8��]����bit���ж��Ƿ���Ч
    BYTE streamData[54];
};

// ���ص�����ߣ��ص���������wIndex����������ݾ���
struct SOverlap : 
    public SItemBase
{
    union {
        struct { BYTE  number; }    normal;     // ��ͨ�ص�����ߣ�����ص�����Ϊ0xff
        struct { WORD  number; }    special;    // �����ص�����ߣ�����ص�����Ϊ0xffff
        struct { DWORD number; }    money;      // ��Ǯ�����0xffffffff
    };
};

// ��װ���ṹ 60 BYTE
struct SEquipment :
    public SItemBase
{
    static const MAX_SCRIPTS            = 2;
    static const MAX_SLOTS              = 5;
    static const MAX_ACTIVATES          = 3;
    static const MAX_QUALITYS           = 6;
    static const MAX_INHERITS           = 6;
    static const EQUIPMENT_MAXNUMBER    = 16; // ��ɫ�������ͬʱװ��8��װ��

    // ʹ�� 5BYTE
    struct JewelSlot {
        BYTE type   : 3;                    // ��ʯ���� 0->7  ��3λ
        BYTE value  : 5;                    // ��ʯ�ȼ� 0->31 ��5λ

        // �� type == 0��Ϊ�������ͣ� ��ʱ��value == 0 ��ʾ�ò����Ч�� value == 1 ��ʾ�ò��Ϊ��
        BOOL isEmpty()   const { return *( LPBYTE )this == 8; }
        BOOL isInvalid() const { return *( LPBYTE )this == 0; }
        BOOL isJewel()   const { return !isInvalid() && ( type != 0 ); }
    } slots[ MAX_SLOTS ]; 

    // ʹ��40BYTE
    struct  Attribute 
    {
        struct Unit
        {
            WORD type   : 6;        // װ����������0-63
            WORD value  : 10;       // װ������ֵ
        };

        Unit activates[3];          // ��������3��[ǿ������]
        Unit naturals;              // �츳1��[��������]
        Unit qualitys[6];           // ����6��[����δ����] �����ʱ���Ѿ�����[���丽��]��
									// �������ٻ����һ�����ԣ��������һ�����Զ�û�У����ʶδ����
        Unit inherits[6];           // �̳�����
        Unit reserves[2];           // δʹ�õ�2��


        // �书���ķ��ӵȼ�����
        // ����ʹ��4��BYTE�Ա���� �书���ķ�ʹ��3BYTE�Ŀ�
        struct UnitSkill
        {
            DWORD skillType : 7;    // 0- 127
            DWORD teletgyType : 7;  // 0- 127
            DWORD skillValue : 7;   // 0- 127
            DWORD teletgyValue : 7; // 0- 127
            DWORD reserve : 4;      // ����
        } unitSkill;

    }attribute;

    WORD isDevelop: 1;              // �����ж�qualitys�е������Ǽ��������Ļ��ǵ��������
    WORD isRefine : 1;              // �Ƿ��Ѿ�Ѫ��
    WORD developCount : 10;         // ����ͳ��
    WORD level : 4;                 // ��װ����ǿ���ȼ�
    BYTE reserve;                   // װ����

    WORD scriptDatas[ MAX_SCRIPTS ]; // ���ڽű��趨�����ݣ����ڹ�������
};

//// װ������� 32BYTE
//struct SEquipment : 
//    public SItemBase
//{
//    static const MAX_SCRIPTS            = 2;
//    static const MAX_SLOTS              = 5;
//    static const MAX_ATTRIBUTES         = 7;
//    static const EQUIPMENT_MAXNUMBER    = 8; // ��ɫ�������ͬʱװ��8��װ��
//
//    BYTE prefix;                            // ( �������ڰ�ʱ������ )ǰ׺ [���/����/����/�Լ��������趨��ǰ׺ֵ]��Ӱ���װ���ļӲ�Ч��
//    BYTE level;                             // ( �������ڱ��ְ�ʱ�� BYTE + WORD )�ȼ�
//    WORD experience;                        // ( �������ڱ��ְ�ʱ�� BYTE + WORD, )������/����ֵ 
//                                            // ��������level �� experience 
//											// ע�⣺���ﱣ��ʱ�䲢δʹ��DWORD����Ϊ�ṹ��С�����ƣ�����ʹ��ƫ��������ռ䲻�������,
//											// ȡ��ʱ�� ( ��ǰ���� - 1999-12-31 00:00:00[946569600] ) = �������ڣ� ��Ҫʹ�õ�ʱ�� + 1999-12-31 00:00:00 ����
//
//    struct Durance {
//        WORD cur;                           // ��ǰ�;ö� 0~65535
//        BYTE max;                           // ����;ö� ( 0~255 ) * 255
//
//        // �;öȱ�ʾ������/255
//    } durance;
//
//    BYTE reserve;                           // �������ݣ�8bit
//
//    struct JewelSlot {
//        BYTE type   : 3;                    // ��ʯ���� 0->7  ��3λ
//        BYTE value  : 5;                    // ��ʯ�ȼ� 0->31 ��5λ
//
//        // �� type == 0��Ϊ�������ͣ� ��ʱ��value == 0 ��ʾ�ò����Ч�� value == 1 ��ʾ�ò��Ϊ��
//        BOOL isEmpty()   const { return *( LPBYTE )this == 8; }
//        BOOL isInvalid() const { return *( LPBYTE )this == 0; }
//        BOOL isJewel()   const { return !isInvalid() && ( type != 0 ); }
//    } slots[ MAX_SLOTS ];                    // 5����
//
//    struct ExtraAttribute {
//        BYTE type   : 5;                    // �������� 0->31
//        BYTE value  : 3;                    // ���Եȼ� 0->7
//
//        // ���Ե���Ч�жϷ���ͬ��
//        BOOL isInvalid() const { return *( LPBYTE )this == 0; }
//    } attributes[ MAX_ATTRIBUTES ];          // ���7����������
//
//    WORD scriptDatas[ MAX_SCRIPTS ];         // ���ڽű��趨�����ݣ����ڹ�������
//};

// ���ӵ�����
struct SCellPos 
{ 
    BYTE byCellX;
    BYTE byCellY; 
};

// Tile������
struct STilePos 
{ 
    WORD wTileX;
    WORD wTileY; 
};

// ���ڵ������еĵ���
struct SPackageItem : 
    public SCellPos,
    public SRawItemBuffer
{
};

// ���ڵ��ϵĵ���
struct SGroundItem : 
    public STilePos,
    public SRawItemBuffer
{
};

// �������
struct SSkill
{
    BYTE byTypeID;			// �ڼ����б��еı��
    BYTE byLevel;			// ���ܵĵȼ�
	DWORD dwProficiency;	// ���ܵ�������
};

// �����������ݣ� �����������Ҫ������ʵʱ���ݴ����ˣ�������Ҫ�����С��
struct ClientVisible
{
    // �������
    DWORD dwBuy;            // ����۸�
    DWORD dwSell;           // �����۸�
	WORD wSellPrice;	    // ��Ʒ����Ԫ�������۸�
	WORD wScorePrice;	    // ��Ʒ�۸�

	WORD wItemID;           // ���߱��
    WORD byGraph1;          // ͼƬ��ţ�����������Ļ���Ҳʹ����һ��IDȥ�Ҿ��������������
	WORD wGraph2;           // ������ʾ�õ�ͼƬ���

	WORD wSound1;           // ����Ʒ���䣨������Ʒ����װ��������Ч���w
	WORD wSpecial;          // ����Ӧ�ã���Ŀǰ���������ص�������
    WORD byLevelEM;         // �ȼ�����
    WORD wENEM;             // ��������
    WORD wINEM;             // ��������
    WORD wSTEM;             // ��������
    WORD wAGEM;             // ������

    WORD wHPAD;            // �����������ֵ
    WORD wSPAD;            // �����������ֵ
    WORD wMPAD;            // �����������ֵ
    WORD wHPR;             // �ָ�������
    WORD wSPR;             // �ָ�������
    WORD wMPR;             // �ָ�������

    BYTE byType;            // �õ��ߵ�����
	BYTE byHoldSize;        // �õ���ռ�õ���Ʒ������
    BYTE byGroupEM;         // ��������

    // ʹ��/װ���������
    WORD wDAMAD;            // -�ӹ�����
    WORD wPOWAD;            // ���ڹ�������
    WORD wDEFAD;            // �ӷ�����
    WORD wAGIAD;            // ��������
    WORD wAmuckAD;          // ��ɱ��
    WORD wLUAD;             // ������
    WORD wRHPAD;            // �������Զ��ָ��ٶ�
    WORD wRSPAD;            // �������Զ��ָ��ٶ�
    WORD wRMPAD;            // �������Զ��ָ��ٶ�
    WORD wClear;            // �����쳣״̬

    WORD wPointAD;          // �����Ե�
    WORD wLevelAD;          // �ӵȼ�

    WORD material;          // ���ߵĲ������ͣ������趨�;öȵĿ۳���

    WORD wSubLevel;         // װ�����ӵȼ����ͻ�����Ҫ�õġ���
	BYTE mBindType;         // ������
    WORD wShineLEM;         // ����ȼ�����  0������ 
    WORD wGraph3;           // ����ͼƬ���

    char szName[ITEMNAME_MAXLEN];    //ԭΪ11->20
};

struct ClientInvisible
{
    DWORD dwScripID;        // �ű����
	WORD wEffect;           // Ч��ͼ��Ŀǰ������Ƥ��߱���

    BYTE nMainLevel;        // ����4�����ݺ���װ��أ�ֻ��Ҫ�ڷ������жϣ�
    BYTE nAddType;
	BYTE wAddScale;         // �ӳɱ��� 

	BYTE wGestID;           // ϰ���书����μ����
	BYTE wGestLevel;        // �书����ȼ�����

    BYTE durance;           // ���ߵ�Ĭ���;öȣ����ڳ�ʼ��ʱ�趨�;öȵ�����

    WORD reserve1;          // Ӱ��ĳɹ��ٷֱ�
    WORD reserve2;          // ��������
    WORD restrict;          // �Ƿ�������ʾ�ĵ��ߣ��ڰ�̯�������Ƿ���ʾ��
    WORD reserve4;          // ����Ч���ĳ���ʱ�䣬������ʱ����0�Ļ���˵���������Ե�����Ч��
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

// ���ڼ�¼��־�Ľṹ��
struct LogInfo
{
    LogInfo( DWORD rcdtype, LPCSTR description = NULL ) : rcdtype( rcdtype ), description( description ) {}

    DWORD   rcdtype;        // ��¼��� 
    LPCSTR  description;    // ��¼˵��
};

// itemBuffer : �����߻��棬 index : ���߱�ţ� param : �ص�����/������ rcdtype : ��¼��ţ� description : ��¼˵��
struct GenItemParams
{
    GenItemParams( WORD index, DWORD number = 1 ) : slotNumber( 0 ), index( index ), number( number ) {}
    GenItemParams( WORD index, DWORD number, WORD slotNumber ) : slotNumber( slotNumber ), index( index ), number( number ) {}

    WORD    slotNumber;
    WORD    index;
    DWORD   number;
};

/// ���������Ʒ�ṹ
struct SpecialItem
{
	DWORD mID;                // ID
	DWORD mLevelAdd[4];       // �ȼ�����
	DWORD mIntensifyAdd[3];   // ǿ������
	DWORD mHoleAdd[4];        // ������
	DWORD mInnetenessAddOddsAndFlag[2]; // �������Ӽ��� �� ��ʶ
	DWORD mAptitudeCheckupOdds[2]; // ���ʼ�������
	DWORD mDropAddsOddsAndFlag[3]; // ���丽�ӱ�ʶ ���� �ͱ�ʶ
};

struct SGemData
{
    // ����λ�ò�Ҫ�����ƶ�����Ϊ����ı�ʶ���ڶ�λʹ��
    enum 
    {
        GEMDATA_EN = 1,			            // ��̫��
        GEMDATA_ST,		                    // ��̫��
        GEMDATA_IN,			                // ������
        GEMDATA_AG,			                // ������
        GEMDATA_DAMAD,                      // �⹦
        GEMDATA_POWER,                      // �ڹ�
        GEMDATA_DEFENCE,                    // ����
        GEMDATA_AGILITY,                    // ����
        GEMDATA_AMUCK,                      // ɱ��
        GEMDATA_LU,			                // ����
        GEMDATA_SAVVY,			        	// ����
        GEMDATA_ADDHPMAXP,                  // ������������ǧ�ֱ�
        GEMDATA_ADDMPMAXP,                  // ����MP����ǧ�ֱ�
        GEMDATA_ADDSPMAXP,                  // ����SP����ǧ�ֱ�
        GEMDATA_HPSPEED,                    // HP�ָ��ٶ�(��)
        GEMDATA_MPSPEED,                    // MP�ָ��ٶ�(��)
        GEMDATA_SPSPEED,                    // SP�ָ��ٶ�(��)   
        GEMDATA_DAMADCTR,                   // �⹦�˺�����   
        GEMDATA_POWERCTR,                   // �ڹ��˺�����   
        GEMDATA_ALLHURTCTR,                 // �����˺�����   
        GEMDATA_DODGE,                      // ��������   
        GEMDATA_ABSDEFENCE,                 // ���Է���   
        GEMDATA_REBOUNDP,                   // ��������   
        GEMDATA_TRANSLIFEP,                 // ��Ѫ����
        GEMDATA_TRANSMP,                    // ���ڼ���
        GEMDATA_TRANSSP,                    // ���弸��
        GEMDATA_HIT,                        // ���м���
        GEMDATA_DAMADHURT,                  // �⹦�˺�
        GEMDATA_POWERHURT,                  // �ڹ��˺�
        GEMDATA_IGNOREDEF,                  // ���Է���
        GEMDATA_CSX8,                       // ����X8
        GEMDATA_RESISTVITALSP,              // �ֿ���Ѩ����
        GEMDATA_RESISTTOXICP,               // �ֿ��ж�����
        GEMDATA_RESISLIMITSKILLP,           // �ֿ����м���
        GEMDATA_RESISDIZZYP,                // �ֿ�ѣ�μ���
        GEMDATA_RESISLIMITHP,               // �ֿ����0����������
        GEMDATA_RESISLIMITMP,               // �ֿ����0��������
        GEMDATA_RESISLIMITSP,               // �ֿ����0�������� 
        GEMDATA_VITALSP,                    // ��Ѩ���м���
        GEMDATA_TOXICP,                     // �ж�����
        GEMDATA_LIMITSKILLP,                // ���м���
        GEMDATA_DIZZYP,                     // ѣ�μ���
        GEMDATA_ZEROHP,                     // ���0����������
        GEMDATA_ZEROMP,                     // ���0�ڼ���
        GEMDATA_ZEROSP,                     // ���0��������
        GEMDATA_EXP,                        // ���⾭��Ӳ�   
        GEMDATA_SKILLEXP,                   // �书����Ӳ�   
        GEMDATA_SKILLLEVEL,                 // ���ܵȼ��Ӳ�  
        GEMDATA_TELERGYLEVEL,               // �ķ��ȼ��Ӳ�
        GEMDATA_ACTIONSPEED,                // �����ٶȰٷֱ�
        GEMDATA_MOVESPEED,                  // �ƶ��ٶȰٷֱ�
        GEMDATA_RETARMOVESPEED,             // �ٻ�

        GEMDATA_MAX                         // ���
    };

    // ��������
    __int16	en;			    //��̫��
    __int16	st;		        //��̫��
    __int16	in;			    //������
    __int16	ag;			    //������ 
    __int16	damad;          // �⹦
    __int16	power;          // �ڹ�
    __int16	defence;        // ����
    __int16	agility;        // ����
    __int16	amuck;          // ɱ��
    __int16	lu;			    // ����
    __int16	savvy;			// ����
    __int16 addHPMaxP;      // ������������ǧ�ֱ�
    __int16 addMPMaxP;      // ����MP����ǧ�ֱ�
    __int16 addSPMaxP;      // ����SP����ǧ�ֱ�

    // ��������
    __int16 HPSpeed;        // HP�ָ��ٶ�(��)
    __int16 MPSpeed;        // MP�ָ��ٶ�(��)
    __int16 SPSpeed;        // SP�ָ��ٶ�(��)
    __int16 damadCtr;       // �⹦�˺�����
    __int16	powerCtr;       // �ڹ��˺�����
    __int16 allHurtCtr;     // �����˺�����
    __int16 dodge;          // ��������
    __int16 absDefence;     // ���Է���
    __int16 reboundP;       // ��������

    // ս������
    __int16 transLifeP;     // ��Ѫ����
    __int16 transMP;        // ���ڼ���
    __int16 transSP;        // ���弸��
    __int16 hit;            // ���м���
    __int16 damadHurt;      // �⹦�˺�
    __int16	powerHurt;      // �ڹ��˺�
    __int16 ignoreDef;      // ���Է���
    __int16 csX8;           // ����X8

    // �ֿ�Ч����
    __int16 resistVitalsP;    // �ֿ���Ѩ����
    __int16 resisTtoxicP;     // �ֿ��ж�����
    __int16 resisLimitskillP; // �ֿ����м���
    __int16 resisDizzyP;      // �ֿ�ѣ�μ���
    __int16 resislimitHP;     // �ֿ����0����������
    __int16 resislimitMP;     // �ֿ����0��������
    __int16 resislimitSP;     // �ֿ����0��������

    // Ч����
    __int16 vitalsP;        // ��Ѩ���м���
    __int16 toxicP;         // �ж�����
    __int16 limitskillP;    // ���м���
    __int16 dizzyP;         // ѣ�μ���
    __int16 zeroHP;         // ���0����������
    __int16 zeroMP;         // ���0�ڼ���
    __int16 zeroSP;         // ���0��������

    // ����Ч��
    __int16 exp;            // ���⾭��Ӳ�
    __int16 skillExp;       // �书����Ӳ�

    // ���ܼӲ�
    __int16 skillLevel;     // ���ܵȼ��Ӳ� ��8λ�漼��ID ��8λ��ȼ�
    __int16 telergyLevel;   // �ķ��ȼ��Ӳ� ��8λ�漼��ID ��8λ��ȼ�

    // ����׷��
    __int16 actionSpeed;    // �����ٶȰٷֱ�
    __int16 moveSpeed;      // �ƶ��ٶȰٷֱ�
    __int16 retarMoveSpeed; // �ٻ�

    // ˥����100ǧ�ֱ�
    const SGemData operator * ( int value )
    {
        SGemData data;    
        memset( &data, 0, sizeof( data ) );
        //data.addHPMaxP           = ( addHPMaxP * value ) / 100;
        return data;
    }

    // ��+=Խ���ʱ��
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
        damadHurt           = _CHECK( data.damadHurt + damadHurt );// �⹦�˺�
        powerHurt           = _CHECK( data.powerHurt + powerHurt );// �⹦�˺�
#undef _CHECK
    }
};

struct SGemAtt
{
	DWORD att[57][3];
};

