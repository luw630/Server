
#ifndef     _VENAPOINTDATE_H_
#define     _VENAPOINTDATE_H_

////////////////////////////////////// ����

const   int     LEN_VENAPOINTNAME   =   16;     // ����Ѩλ
const   int     VENA_COUNT          =   9;      // 9������
const   int     POINT_COUNT         =   32;     // ÿ�����������Ѩλ��

typedef struct  _SVenapointData
{
	enum VENA_PROP_DEF
	{
		MAX_HP = 0,							// �������� ->m_wMaxHPOnMad
		MAX_MP,								// ��������
		MAX_SP,								// ��������
		RE_HP,								// �����ָ�����ÿ�λظ�������
		RE_MP,								// �����ָ�����ÿ�λظ�������
		RE_SP,								// �����ָ�����ÿ�λظ�������
		OUT_ATTA_BASE,						// �⹥��������λ1/1000��
		OUT_ATTA_MASTERY,					// �⹥��ͨ
		INNER_ATTA_BASE,					// �ڹ�����
		INNER_ATTA_MASTERY,					// �ڹ���ͨ
		OUT_DEF_BASE,						// �������
		OUT_DEF_MASTERY,					// �����ͨ
		INNER_DEF_BASE,						// �ڷ�����
		INNER_DEF_MASTERY,					// �ڷ���ͨ
		AGILE,								// ��
		AMUCK,								// ɱ�� ->m_Property.m_byAmuck

		VENA_PROPERTY_MAX,
	};

	char		szName[LEN_VENAPOINTNAME];  // ����
	WORD		wNum;                       // ���߱��
	WORD		wID;                        // Ѩλ���
	int			data[VENA_PROPERTY_MAX];
	DWORD		dwUseTelergy;               // ��Ҫ���ĵ��ķ�����
}SVenapointData,*LPSVenapointData;

typedef struct  _SVenapointDataEx : public _SVenapointData
{
    WORD    wAddIN;                         // ����
    WORD    wAddHitPro;                     // ���мӲ���ֱ�
    WORD    wAddDAMPro;                     // �⹦�Ӳ���ֱ�
    WORD    wAddPOWPro;                     // �ڹ��Ӳ���ֱ�
    WORD    wAddAGIPro;                     // ���ݼӲ���ֱ�
    WORD    wAddDodgePro;                   // �����Ӳ���ֱ�
    WORD    wAddDefPro;                     // �����Ӳ���ֱ�
    WORD    wAddSkillAdd[6];                // ���ܼӲ�[0����ɱ�� 1�������� 2��ս���� 3�������� 4������� 5�����]
}SVenapointDataEx,*LPSVenapointDataEx;


////////////////////////////////////// ��������
class   DVenapointData
{
    BYTE            m_byVenapointCount[VENA_COUNT];					// ÿ��������Ѩλ����
    SVenapointData  m_Data[VENA_COUNT][POINT_COUNT];				// ÿ���������ÿ��Ѩλ�����ݣ�ÿ�����������һ��������
																	// ����������ÿ��Ѩλ����ͨ�˺󣬸��ӵ�����
    SVenapointData  m_TotalData;									// ���о���ȫ��ͨ�˺�ĸ�������

    SVenapointDataEx  m_DataEx[VENA_COUNT][POINT_COUNT];
    BYTE            m_byVenapointCountEx[VENA_COUNT];
    SVenapointDataEx  m_TotalDataEx;


public:
    DVenapointData(void);
    ~DVenapointData(void);

    BOOL    LoadData( char* szPath  );
    LPSVenapointData    GetVenapoint( BYTE byVenaNum, BYTE byPointNum );
    BYTE    GetVenapointCount( BYTE byVenaNum );
	std::string GetAddDataName( int data);

    // ������
    BOOL LoadDataEx(char* szPath = "Data\\VenapointDataEx.def");
    LPSVenapointDataEx GetVenapointEx( BYTE byVenaNum, BYTE byPointNum );
    BYTE    GetVenapointCountEx( BYTE byVenaNum );
};

extern  DVenapointData  *g_pVenapointData;

#endif  //_VENAPOINTDATE_H_
