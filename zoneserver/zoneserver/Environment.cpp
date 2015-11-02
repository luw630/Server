#include "stdafx.h"

#include "Environment.h"
#include "networkmodule/playertypedef.h"
#include <list>
#include <string>

static float fExpScale = 1.0;
static float fDropMoneyScale = 1.0;
static float fDropItemScale = 1.0;

static BYTE byMulHours =0;	// ˫��ʱ��
static BOOL useAttribAnti = false;  // �Ƿ�ʹ���µ��������ϵͳ��

std::list< std::string > extraNetLimitedInfo;
std::string sidname;

// ������ʣ���ʹ�ú������ƣ�ֱ�ӷų��������ӵ�ַ����߷���Ч��
int maxParry = 93;

// ����ʱ��Ԫ������(!=0)��������������(0)��
int	sellType = 0;

// �Ƿ�����ҿ۵�������һ��ʱ���Զ�ȡ��������
int	autoUnlock = 0;

// ������Ʒ�ĵȼ������ƣ�Ĭ���趨Ϊÿ30���������½�20%
int dropitem_levelMargin  = 30;  // ����׶�
int dropitem_reduceRating = 20;  // ÿ�׶εĸı����

// �������װ��ʱȥ���������������ֵ�ļ��ʣ�Ĭ��Ϊ������120�����£�10%������1%
int randomatt_levelLimit = 120; // ����ȼ�����
int randomatt_below = 1000;     // �ȼ������µ��������
int randomatt_above = 10;       // �ȼ������ϵ��������

int cacheEnabled = 0;           // �Ƿ�򿪻��湦��
int backupMargin = 30 * 1000;           // ���濪������Զ�����ʱ����
int backupNum = 1000;           // ���濪������Զ����ݿռ��С

// �Ƿ�ʹ���¹���[�Ժ���ӿ��ܴ����������¹��ܶ�����ͨ������趨��ȡ������]
BOOL newly_ability_enabled = FALSE;
DWORD rcdMoneyRank = 100000;     // Ĭ�ϳ���1w�Ľ�Ǯ�仯���Ž�����־��¼��
DWORD macLimit = 0;

BOOL afLikeHungup = true;        // �Զ�ս����ͳ��ʱ�����Ϊ�һ�״̬
BOOL g_bHangup = FALSE;
DWORD g_dwLockArea = 0;
extern BOOL wgLimit;             // �Ƿ��������, 0 ��ȫ�����ƣ� 1 ����ײ����ƣ� 2 ��Ϸ�߼������ƣ� 4 ֪ͨ����¼�ű��� 8 ֪ͨ���ʺŷ�����
extern BOOL isLimitWG;           // ����������Ծ�ͳ�ҩ����
//extern DWORD g_dwEquipmentWearScale; // ���ڽ�װ���;ý������ţ�2000�;����ųɱ���2.

BOOL bCheckGMCmd = FALSE;
extern int MAXPLAYERLEVEL;

// ��־���ֵ�����趨
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

// �����Ǯ�������趨
float GetDropMoneyScale()
{
    return fDropMoneyScale;
}

void SetDropMoneyScale(float moneyScale)
{
	fDropMoneyScale = moneyScale;
}
// ������Ʒ���ʱ����趨
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
        rfalse(2, 1, "�޷�����Ϸ���������ļ�������");
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
    IniFile.GetInteger( "ENVIRONMENT", "LOCKDISTANCE", 0, ( int* )&g_dwLockArea ); // Ĭ���ǰ뾶Ϊ9�ķ�Χ

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
        "[����ģʽ = %s]��[�����е��¹��� = %s]%s\r\n"
        "�µ��������=%d���������=%d\r\n"
        "[��Ʒ����ȼ�����]ÿ���%d���½�%d%%\r\n"
        "[����װ���������]�ȼ��� %d ���� %.2f%% ���� %.2f%%\r\n"
        "[��Ǯ�仯��־]��Ƕ�� %d\r\n"
        "[PK��Ϣ��������] %s\r\n"
		"[�����ߵȼ����� = %d]",
        fExpScale, fDropMoneyScale, fDropItemScale, byMulHours, 
        ( sellType ? "Ԫ������" : "��������" ), ( newly_ability_enabled ? "�ѿ���" : "����" ), ( autoUnlock ? "��[�Զ�ȡ������״̬]" : "" ),
        useAttribAnti, maxParry, dropitem_levelMargin, dropitem_reduceRating,
        randomatt_levelLimit, randomatt_below / 100.f, randomatt_above / 100.f, rcdMoneyRank, g_dwLockArea ? "����" : "�ر�",MAXPLAYERLEVEL );

    if ( !cacheEnabled )
        rfalse( 2, 1, "[�����汸�ݹ���] δ����\r\n" );
    else
        rfalse( 2, 1, "[�����汸�ݹ���] ����������ʱ���ݼ��Ϊ %d ��\r\n", backupMargin / 1000 );

    if ( macLimit > 2 ) macLimit = 2;
    rfalse( 2, 1, "һ���࿪�����趨 = %s\r\n", macLimit ? ( ( macLimit == 1 ) ? "����" : "˫��" ) : "������" );
    
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