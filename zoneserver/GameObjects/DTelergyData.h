
#ifndef     _DTELERGYDATA_H_
#define     _DTELERGYDATA_H_

// ��̬�ķ����ݵ����ֵ
const int MAX_TELETGY = 256;
 // �ķ���������
const int LEN_TELERGYNAME = 16;          

#define MAX_INFO_LENTH		1024
enum E_TOTAL_TELERGY			// �ķ����ݶ�ȡͳһ�ӿڱ�ʶ���
{
    TELERGYR_NONE,
    TELERGYR_wDamCoef,           // �⹥��ϵ��	
    TELERGYR_wPowCoef,           // �ڹ���ϵ��	

    TELERGYR_wDamAddCoef,        // �⹦����	
    TELERGYR_wPowAddCoef,        // �ڹ�����	
    TELERGYR_wDefAddCoef,        // ��������	
    TELERGYR_wAgiAddCoef,        // ��������	
    TELERGYR_wHPMaxAddCoef,      // Ѫ������	
    TELERGYR_wMPMaxAddCoef,      // ��������	
    TELERGYR_wSPMaxAddCoef,      // ��������	
    TELERGYR_wKnockOdds,         // ���ɼ���ϵ��	
    TELERGYR_wKnockDefCoef,      // ����ϵ��	
    TELERGYR_wOnVitalsTimeCoef,  // ��Ѩʱ��ϵ��	
    TELERGYR_wOffVitalsTimeCoef, // ��Ѩʱ��ϵ��	
    TELERGYR_wOnLmtAtkTimeCoef,  // ����ʱ��ϵ��	
    TELERGYR_wOffLmtAtkTimeCoef, // ����ʱ��ϵ��	
    TELERGYR_wOnDizzyTimeCoef,   // ѣ��ʱ��ϵ��	
    TELERGYR_wOffDizzyTimeCoef,  // ����ʱ��ϵ��	
    TELERGYR_wOnToxicTimeCoef,   // �ж�ʱ��ϵ��	
    TELERGYR_wOffToxicTimeCoef,  // �ⶾʱ��ϵ��	
    TELERGYR_wDamAddTimeCoef,    // �ṥʱ��ϵ��	
    TELERGYR_wDefAddTimeCoef,    // ���ʱ��ϵ��	
    TELERGYR_wAgiAddTimeCoef,    // ����ʱ��ϵ��	
    TELERGYR_wAmuAddTimeCoef,    // ��ɱʱ��ϵ��	

    TELERGYR_wDamDam,            // ��̫���˺�	
    TELERGYR_wPowDam,            // �������˺�	
    TELERGYR_wDefDam,            // ��̫���˺�	
    TELERGYR_wAgiDam,            // �������˺�	
    TELERGYR_wDamDef,            // ��̫������	
    TELERGYR_wPowDef,            // ����������	
    TELERGYR_wDefDef,            // ��̫������	
    TELERGYR_wAgiDef,            // ����������	
    TELERGYR_wDamMed,            // ��̫������	
    TELERGYR_wPowMed,            // ����������	
    TELERGYR_wDefMed,            // ��̫������	
    TELERGYR_wAgiMed,            // ����������	

    TELERGYR_wOffZeroSPTimeCoef, // �ֿ���������Ϊ0ʱ��ϵ��
    TELERGYR_wOffZeroMPTimeCoef, // �ֿ���������ʱ��ϵ��
    TELERGYR_wOffZeroHPTimeCoef, // �ֿ���������ʱ��ϵ��

    TELERGYR_MAX,
};


struct TelergyDataXiaYiShiJie
{
	/*
	����������ķ������������Ļ��������˸ı䣬����Ϊ�˲�Ӱ����ǰ���߼����ܣ�Ŀǰ��ȫ���޸ķ�����
	��ԭ�����ķ����ݽṹ�Ļ���������µ����ݽṹ---yuntao.liu
	*/
	enum TELE_PROP_DEF
	{
		addBlood = 0,					// ������������	
		addOutAttackBase,				// �⹥��������
		addOutAttackMastery,			// �⹥��ͨ����
		addInnerAttackBase,				// �ڹ���������
		addInnerAttackMastery,			// �ڹ���ͨ����
		addOutDefenseBase,				// �����������
		addOutDefenseMastery,			// �����ͨ����
		addInnerDefenseBase,			// �ڷ���������
		addInnerDefenseMastery,			// �ڷ���ͨ����
		addBodyWork,					// ������
		addKillForce,					// ɱ������
		addStaafType,					// ����ϵ
		addKungfuBaseForceByFistType,	// ȭϵ�乥��������������
		addSwordType,					// ��ϵ
		addPenType,						// ��ϵ
		addStabType,					// ��ϵ
		addQinType,						// ��ϵ
		addKnifeType,					// ��ϵ
		addHalberdType,					// �ϵ
		addHookType,					// ��ϵ
		addRingType,					// �ֻ�ϵ
		addHandTaiYang,					// ��̫�����ˣ����ָ�����һ�����������岻ͬ�ˣ�
		addHandShaoYang,				// ����������
		addFootTaiYin,					// ��̫������
		addFootShaoYin,					// ����������
		addImmunityBuffFirst,			//����buff 1
		addImmunityBuffSecond,			//����buff 2

		MAX_TELERGY_XIA_YI_SHI_JIE
	};
	INT32 m_index;							// ���
	char  m_name[ LEN_TELERGYNAME ];		// ����
	int	  m_nSchool;						// ���� 

	INT32 m_data[ MAX_TELERGY_XIA_YI_SHI_JIE ];
	DWORD m_hardBase;						//�ķ��ѶȻ���
	DWORD m_hardCoefficient;				// �ķ��Ѷ�ϵ��
	DWORD m_maxLevel;						// �ķ��������� 
	char  information[MAX_INFO_LENTH];		//����˵��
};

////////////////////////////////////////////////////////////////////// �ķ��������
struct  TelergyData : TelergyDataXiaYiShiJie
{
	// Zeb 2009-11-19
    //WORD wIndex;						// ���	
    //char szName[LEN_TELERGYNAME];       // ����	
    //WORD wDamCoef;           			// �⹥��ϵ��	
    //WORD wPowCoef;           			// �ڹ���ϵ��	
    //WORD wDamAddCoef;        			// �⹦����	
    //WORD wPowAddCoef;        			// �ڹ�����	
    //WORD wDefAddCoef;        			// ��������	
    //WORD wAgiAddCoef;        			// ��������	
    //WORD wHPMaxAddCoef;      			// Ѫ������	
    //WORD wMPMaxAddCoef;      			// ��������	
    //WORD wSPMaxAddCoef;      			// ��������	
    //WORD wKnockOdds;         			// ���ɼ���ϵ��	
    //WORD wKnockDefCoef;      			// ����ϵ��	
    //WORD wOnVitalsTimeCoef;  			// ��Ѩʱ��ϵ��	
    //WORD wOffVitalsTimeCoef; 			// ��Ѩʱ��ϵ��	

    //WORD wOnLmtAtkTimeCoef;  			// ����ʱ��ϵ��	
    //WORD wOffLmtAtkTimeCoef; 			// ����ʱ��ϵ��	

    //WORD wOnDizzyTimeCoef;   			// ѣ��ʱ��ϵ��	
    //WORD wOffDizzyTimeCoef;  			// ����ʱ��ϵ��	
    //WORD wOnToxicTimeCoef;   			// �ж�ʱ��ϵ��	
    //WORD wOffToxicTimeCoef;  			// �ⶾʱ��ϵ��	
    //WORD wDamAddTimeCoef;    			// �ṥʱ��ϵ��	
    //WORD wDefAddTimeCoef;    			// ���ʱ��ϵ��	
    //WORD wAgiAddTimeCoef;    			// ����ʱ��ϵ��	
    //WORD wAmuAddTimeCoef;    			// ��ɱʱ��ϵ��	
    //WORD wDamDam;            			// ��̫���˺�	
    //WORD wPowDam;            			// �������˺�	
    //WORD wDefDam;            			// ��̫���˺�	
    //WORD wAgiDam;            			// �������˺�	
    //WORD wDamDef;            			// ��̫������	
    //WORD wPowDef;            			// ����������	
    //WORD wDefDef;            			// ��̫������	
    //WORD wAgiDef;            			// ����������	
    //WORD wDamMed;            			// ��̫������	
    //WORD wPowMed;            			// ����������	
    //WORD wDefMed;            			// ��̫������	
    //WORD wAgiMed;            			// ����������
    //WORD wOffZeroSPTimeCoef; 			// �ֿ���������Ϊ0ʱ��ϵ��
    //WORD wOffZeroMPTimeCoef; 			// �ֿ���������ʱ��ϵ��
    //WORD wOffZeroHPTimeCoef; 			// �ֿ���������ʱ��ϵ��

    //WORD wDiffCoef;          			// �ķ��Ѷ�ϵ��
    //WORD wMaxLevel;          			// �ķ���������
    //int nMutexTelergy;      			// ���⼼��
	// Zeb 2009-11-19
};

class DTelergyData
{
    TelergyData m_Data[MAX_TELETGY];
    int m_iMaxTelergy;

public:
    DTelergyData(void);
    ~DTelergyData(void);

    BOOL LoadData(char* szPath);
    TelergyData *GetTelergy( WORD wIndex ); 
    int GetCount() {return m_iMaxTelergy;};

	static DWORD GetCurLevelMax(TelergyData* pTempData,int iLevel);	//�õ������ķ��ȼ�����Ҫ�����������
	static DWORD GetCurPractice(WORD wTime);
private:
	// Ŀǰ��������ķ����ݱ���ϵ��ķ��������ڴ����Ƿ���һ��ģ��������ϵ��ķ����ݺ�
	// ���ô˺��������µ��ķ����ݱ�
	BOOL LoadDataXiaYiShiJie(char* szPath);
};

extern DTelergyData* g_pTelergyData;

#endif  //_DTELERGYDATA_H_