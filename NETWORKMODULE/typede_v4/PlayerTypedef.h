#pragma once
#include "../pub/ConstValue.h"
#include "../NETWORKMODULE/NewPetTypedef.h"
#include "../NETWORKMODULE/ConstValueSG.h"

#define GMMODULEON
//wk 20150205 �Ż���Ҫ������
//
//const int MAX_FRIEND_NUMBER			= 200;			//����
//const int MAX_BLOCK_NUMBER			= 100;			//������
//const int MAX_MATE_NUMBER			= 100;			//����
//const int MAX_ENEMIE_NUMBER			= 100;			//����
//
////const int MAXPLAYERLEVEL			= 75;			// ��ҵ���ߵȼ�
//
//const int MAX_WAREHOUSE_ITEMNUMBER	= 72;			// �ֿ�ĵ����������
//const int MAX_TASK_NUMBER			= 20;			// ���������������
//const int MAX_WHOLE_TASK			= 8192 * 8;		// ���������
//
//const int MAX_TELEGRYNUM			= 50;			// �ķ�����
//const int MAX_SKILLCOUNT			= 50;			// ��������
//
//const int MAX_JM_XUE				= 195;			// ��ǰϵͳѨ������
//const int MAX_JM_XUE_MEMORY			= 32;			// ����洢��
//const int MAX_MAI_NUMBER			= 9;			// ��������
//
//const int MAX_FIGHTPET_NUM			= 6;			// Я�����������
//const int MAX_NEWPET_NUM = 99;		//���Я������������
//
//const int MAX_ROSERECOD_NUM			= 100;			// ����¼���ͻ���¼��
//const int ALL_RANK_NUM				= 3000;			// �����ݿ�����ȡ����������а�����
//const int SizeTask					= 100;
//#define MAX_TITLE (16+1)							// �����ӵ�еĳƺŵ��������16��ϵͳ+1����ң�
//const int MAX_ONLINEGIFT_NUM		= 50;			// ���߽����������
//#define TASK_MAX	8
//#define TASK_MAX_TYPE	8


//const int MAX_FRIEND_NUMBER = 100;			//����
const int MAX_BLOCK_NUMBER = 1;			//������
const int MAX_MATE_NUMBER = 1;			//����
const int MAX_ADD_NUMBER = 50;			//������ӵĺ����б�
const int MAX_ENEMIE_NUMBER = 1;			//����

//const int MAXPLAYERLEVEL			= 75;			// ��ҵ���ߵȼ�

const int MAX_WAREHOUSE_ITEMNUMBER = 1;			// �ֿ�ĵ����������
const int MAX_TASK_NUMBER = 1;			// ���������������
const int MAX_WHOLE_TASK = 1;		// ���������

const int MAX_TELEGRYNUM = 1;			// �ķ�����
const int MAX_SKILLCOUNT = 1;			// ��������

const int MAX_JM_XUE = 1;			// ��ǰϵͳѨ������
const int MAX_JM_XUE_MEMORY = 1;			// ����洢��
const int MAX_MAI_NUMBER = 1;			// ��������

const int MAX_FIGHTPET_NUM = 1;			// Я�����������
const int MAX_NEWPET_NUM = 1;		//���Я������������

const int MAX_ROSERECOD_NUM = 1;			// ����¼���ͻ���¼��
const int ALL_RANK_NUM = 1;			// �����ݿ�����ȡ����������а�����
const int SizeTask = 1;
#define MAX_TITLE (1)							// �����ӵ�еĳƺŵ��������16��ϵͳ+1����ң�
const int MAX_ONLINEGIFT_NUM = 1;			// ���߽����������
#define TASK_MAX	1
#define TASK_MAX_TYPE	1
// �����3���ɶ���ְҵ
enum XYD3_SCHOOL
{
	XS_NONE = 0,
	XS_WUYUE = 1,		// ����
	XS_SHUSHAN,			// ����
	XS_SHAOLIN,			// ��Ѫ��
	XS_CIHANG,			// ��ʹ
	XS_RIYUE,			// ����

	XS_MAX,				// Max
};

// ��Ǯ����
enum TPlayerMoneyType
{
	EMoneyType_NonBind = 1, // �ǰ󶨽��
	EMoneyType_Bind = 2, // �󶨽��	
};

// �����3���״̬���
enum PLAYER_SHOW_EXTRASTATE
{
	PSE_NONE		= 0x00000000,	// ��״̬
	PSE_STALL_SALE	= 0x00000001,	// ��̯��
};

// �����3�ķ����ݽṹ
struct SXYD3Telergy
{
	WORD	m_TelergyID;		// �ķ�ID
	WORD	m_TelergyLevel;		// �ķ��ȼ�
};

struct SFixBaseData
{
    char	m_szTongName[11];	// ������ƣ�ͬʱΪ���ڲ��Ҹð��ɵĹؼ���
	DWORD	m_dwPowerVal;		// ս����

	SFixBaseData() { memset(m_szTongName, 0, sizeof(m_szTongName)); }
};

struct SFixPackage				// ��������������+����+��չ1+��չ2��96�����ӣ�
{
	SPackageItem	m_BaseGoods[PackageAllCells];
};

struct SFixStorage1
{
    SPackageItem    m_pStorageGoods[MAX_WAREHOUSE_ITEMNUMBER];		// ���ߵĲֿ�
	DWORD			m_whMoney;										// �ֿ�ǰ󶨻���
	DWORD			m_whBindMoney;									// �ֿ�İ󶨻���
	// ------------- ���Ҫ���뵽�ֿ⣬�õ���ʱ���ڴ�---------------------------
// 	DWORD			m_whZengBao;									// �ֿ��е�����
// 	DWORD			m_whYuanBao;									// �ֿ��е�Ԫ��
	BYTE			m_ActiveTimes;									// ����Ĵ���
};

// *************************** �����3����ϵͳ ***************************
enum TaskStatus
{
	TS_FAIL,								//����ʧ��
	TS_NOTACCEPT = 1,				// δ��
	TS_ACCEPTED,					// �ѽӣ�����У�
	TS_FINISHED,					// ����ɣ�δ����
	TS_COMPLETE,					// �ѽ�
	TS_TALKIMME,					// �Ի����н飩
	TS_GIVE,                        // ��NPC���ߣ��н飩
	TS_GET,                         // ��NPC����õ��ߣ��н飩
	TS_FUTUREACCEPT,				// δ���ɽӣ���������Ͽ�ʱ����ڣ�
};

enum TaskFlagType
{
	TT_ITEM  = 1,					// ��Ʒ
	TT_KILL,						// ɱ��
	TT_TALK,						// �Ի�
	TT_GIVE,						// ����Ʒ
	TT_GET,                         // ��ȡ����
	TT_USEITEM,						// ʹ�õ���
	TT_REGION,						// �ܵ�ͼ
	TT_SCENE,						// ��ɹؿ�
	TT_ADD,						// ��������
	TT_EMPTY = 100,					// �����
};

// ���ڴ���������Ϣ�����NPC��
struct TaskInfo
{
	WORD	m_TaskID;
	BYTE	m_TaskStatus;			// ����״̬
};

#pragma pack (push)
#pragma pack (1)

// ����������Ϣ������
typedef struct STaskFlagImpl
{
	union
	{
		// ���ڶԻ����
		struct
		{
			DWORD	NpcID;
			WORD	Times;
			WORD	CurTimes;
		}Talk;

		// ���ڵ������
		struct
		{
			DWORD	ItemID;
			WORD	ItemNum;
			WORD	CurItemNum;
		}Item;

		// ���ڸ�/��ȡ���
		struct  
		{
			DWORD	NpcID;
			DWORD	ItemID;
			WORD	ItemNum;
			WORD	CurItemNum;
		}GiveGet;

		// ����ɱ�����
		struct
		{
			DWORD	MonsterID;
			WORD	KillNum;
			WORD	CurKillNum;
		}Kill;

		// ����ʹ�õ������
		struct  
		{
			DWORD	ItemID;
			WORD	UseTimes;
			WORD	CurUseTimes;
		}UseItem;

		// �����ܵ�ͼ���
		struct
		{
			WORD	DestMapID;
			WORD	DestX;
			WORD	DestY;
		}Region;

		// ���ڳ����ؿ����
		struct
		{
			WORD	DestSceneID;
			WORD	DestX;
			WORD	DestY;
		}Scene;

		//��������
		struct 
		{
			enum 
			{
				TASK_EINTENSIFY,  //װ��ǿ��
				TASK_EJDING,  //װ������
				TASK_ERISE,  //װ������
				TASK_EKEYIN,  //װ����ӡ
				TASK_EQUALITY,  //װ������
				TASK_XINYANG, //��������
				TASK_TRANSFORMERS, //��������
				TASK_SKILL, //�������� ��������
				TASK_SCENECOMPLETE, //�ؿ����
			};
			BYTE btype;//��������
			WORD wCount; //��ǰ����
			WORD wAllCount;//�ܴ���
			DWORD dIndex;	//��Ӧװ�������ǵ���ID�����ڼ�������Ϊ����ID
		}TaskADD;

	};
}STFImpl;

// �����������ݽṹ
struct STaskFlag
{
	STaskFlag() 
	{
		memset(this, 0, sizeof(STaskFlag));
	}
	BYTE		m_Type;				// �������
	BYTE		m_Complete;			// �Ƿ����
	DWORD		m_Index;			// ��16λ�������񣬵�16λ���������
	STFImpl		m_TaskDetail;		// ���Ľ�����
};

// ����������ӵ�����
struct SPlayerTask
{
	WORD		m_TaskID;			// ����ID
	BYTE		m_FlagCount;		// ���������Ŀ
	QWORD		m_CreateTime;		// ���񴴽�ʱ��
	STaskFlag	m_Flags[8];			// �����������
};

struct SPlayerCycTask
{
	WORD	m_TaskID;				//����ID�����һ����ɵ��ճ���������ID
	char	m_CompleteTime[30];		//�����һ���������ʱ��
	BYTE	m_type;					//taskType = 1(��������)  taskType = 2(�������)(������д)
	BYTE    m_cyctype;				//cyctype  = 1(�ճ�����),	2(�ܻ�(��ʱ)����),	3(�ܳ�����)
	SPlayerCycTask() : m_TaskID(0),m_type(0),m_cyctype(0)
	{
		memset(m_CompleteTime, 0, 30);
	}
};
struct SPlayerRdTaskInfo //������� ����ṹ��ֻ�Ǽ�¼������������
{
	WORD m_NpcID;									  //NpcID
	WORD m_RdTaskNum;								  //������������ ������ɵ���������
	WORD m_TaskID;									  //�Ҽ�������   ���������������ID
	SPlayerRdTaskInfo():m_NpcID(0),m_RdTaskNum(0){}
};
struct SPlayerLimtTime
{
	char m_CompleteTime[30];		                  //��һ�ε�����ʱ��
	SPlayerRdTaskInfo m_SPlayerRdTaskInfo[80];
	BYTE m_RdNum;
	BYTE flag;
	SPlayerLimtTime():flag(0),m_RdNum(0)
	{
		memset(m_CompleteTime,0,30);
	}
	void UpdataLimtTimeData()
	{
		m_RdNum = 0;
		memset(m_CompleteTime,0,30);
		memset(m_SPlayerRdTaskInfo,0,sizeof(SPlayerRdTaskInfo));
	}
	bool IsExitNpcIDWithIndex(WORD NpcID,long &index)
	{
		bool flag = false;
		for (long i = 0; i < m_RdNum; ++i)
		{
			 if ( m_SPlayerRdTaskInfo[i].m_NpcID == NpcID)
			 {
				index = i;
				flag = true;
				break;
			 }
		}
		return flag;
	}
};
#pragma pack (pop)

// ��ҵ�������Ϣ�����֧�ּ�¼MAX_WHOLE_TASK������
struct SPlayerTasks
{
	SPlayerTask		m_PlayerTask[MAX_TASK_NUMBER];		// ��������
	BYTE			m_flags[MAX_WHOLE_TASK / TASK_MAX_TYPE];			// ������ɱ��
	SPlayerCycTask  m_PlayerCyc[20];
	SPlayerLimtTime m_SPlayerLimtTime;
	int MarkComplete(WORD taskID)
	{
		if (0 == taskID)
			return -1;
		
		DWORD group = (taskID - 1) / TASK_MAX_TYPE;
		DWORD idx = (TASK_MAX_TYPE -1)-(taskID - 1) % TASK_MAX_TYPE;

		m_flags[group] |= (1 << idx); 
		return 1;
	}
	
	int MarkUnComplete(WORD taskID)
	{
		if (0 == taskID)
			return -1;

		DWORD group = (taskID - 1) / TASK_MAX_TYPE;
		DWORD idx = (TASK_MAX_TYPE -1)-(taskID - 1) % TASK_MAX_TYPE;
		
		m_flags[group] &= ~(1 << idx); 
		return 1;
	}

	int IsComplete(WORD taskID)
	{
		if (0 == taskID)
			return -1;

		DWORD group = (taskID - 1) / TASK_MAX_TYPE;
		DWORD idx = (TASK_MAX_TYPE -1)-(taskID - 1) % TASK_MAX_TYPE;

		return (m_flags[group] & (1 << idx)) ? 1 : 0;
	}

	void ClearCompleteMark(WORD taskID)
	{
		if (taskID == 0)
		{
			return;
		}
		DWORD group = (taskID - 1) / TASK_MAX_TYPE;
		m_flags[group] = 0;
	}
};
// *************************** �����3����ϵͳ ***************************

// ��ҵľ�����Ϣ��Ŀǰ���֧��256��Ѩλ
struct SPlayerJingMai
{
	BYTE m_JingMai[MAX_JM_XUE_MEMORY];
	BYTE m_byShareExpTimes; //gw+ ���з��������
	//INT64 m_countShareExpLoginTime; //gw+�������¼ʱ�䣨����ÿ��Ĵ�����

	//gw+init
	SPlayerJingMai()
	{
		memset(this, 0, sizeof(*this));
	}
	// ���þ�����Ѩ���������
	void SetJingMaiShareExpTimes(BYTE byShareTimes)
	{
		m_byShareExpTimes = byShareTimes;
	}
	BYTE GetJingMaiShareExpTimes()
	{
		return m_byShareExpTimes;
	}

	int MarkVenPoint(WORD VenPointID)
	{
		if (0 == VenPointID || VenPointID > MAX_JM_XUE)
			return -1;

		DWORD group = (VenPointID-1) / 8;
		DWORD idx	= 7 - (VenPointID-1) % 8;

		m_JingMai[group] |= (1 << idx);
		return 1;
	}

	int IsVenMarked(WORD VenPointID)
	{
		if (0 == VenPointID || VenPointID > MAX_JM_XUE)
			return -1;

		DWORD group = (VenPointID-1) / 8;
		DWORD idx	= 7 - (VenPointID-1) % 8;

		return (m_JingMai[group] & (1 << idx)) ? 1 : 0;
	}
};
// *************************** �����3����ϵͳ ***************************

struct SPlayerGM
{
    DWORD  m_GMLevel;
};

// ��ɫ����������
struct SPlayerMounts
{
	//wk 20150205 �Ż���Ҫ������
	//enum { MAX_MOUNTS = 8, };
	enum { MAX_MOUNTS = 1, };
	enum { State_None, State_Equip, State_Riding, };
	enum 
	{
		SPM_MHP,			// ��������
		SPM_MMP,			// ��������
		SPM_MTP,			// ��������
		SPM_GONGJI,			// ����
		SPM_FANGYU,			// ����
		SPM_BAOJI,			// ����
		SPM_SHANBI,			// ����
		SPM_SPEED,			// �ٶȣ������棩

		SPM_MAX,
	};

	struct Mounts
	{
		Mounts() { ID = 0; }

		WORD	ID;						// ���
		DWORD	GrowPoint;				// ����
		BYTE	Level;					// �ȼ�
		BYTE	State;					// ״̬
		WORD	Points[SPM_SPEED];		// ����
	} mount[MAX_MOUNTS];
};

// ��ɫ�ķ�ս����������
struct SPlayerPets
{
	//wk 20150205 �Ż���Ҫ������
	//enum { MAX_PET = 5, };							// ��������
	enum { MAX_PET = 1, };							// ��������
	enum { Pet_State_None, Pet_State_Equip, };		// ״̬
	enum PetAttriType 
	{
		PAT_WAIGONG,
		PAT_NEIGONG,
		PAT_TIZHI,
		PAT_WUXING,
		PAT_SHENFA,

		PAT_MAX,
	};

	struct PetAttri
	{
		WORD	type	: 4;						// ֧��15������
		WORD	value	: 8;						// ���ֵ255
		WORD	reserve	: 4;						// ����
	};

	struct Pets
	{
		BYTE		ID;					// ���
		BYTE		Sex;				// �Ա�	
		BYTE		Level;				// �ȼ�
		BYTE		State;				// ״̬
		BYTE		LeftPoint;			// ʣ�����
		char		Name[11];			// ����
		WORD		Attri[PAT_MAX];		// ��������
		WORD		Quality[PAT_MAX];	// ��������
		WORD		Happyness;			// ���ֶ�
		WORD		FuseDegree;			// �ں϶�
		DWORD		UpdateTimer;		// ���¼�ʱ��
	}pets[MAX_PET];
};

// �����3����ϵͳ
struct SFightPetExt : public SFightPetBase
{
	SEquipment	m_Equip[EQUIP_P_MAX];	// ����Ŀǰ��װ���ĵ���
};

// �����3����һ�������
struct SXYD3FixData
{
	DWORD		m_version;				// �汾�ţ����ڶ�����������չ
	DWORD		m_LeaveTime;			// �������ʱ��ʱ��, ��ǰ���ڱ�����Ҳ����ߵ���ʱ�䣨ͨ��m_OnlineTime��m_LeaveTime���Լ������ҵ�ע��ʱ�䣩
	DWORD		m_LeaveIP;				// �������ʱ��IP
	DWORD		m_OnlineTime;			// ����ʱ�䣬 ��ǰ���ڱ���������ߵ���ʱ��
	BYTE		m_Sex:1;				// �Ա�
	BYTE		m_BRON:3;				// �������ʾͼƬ����1��ʾ��ʦ
	BYTE		m_School:4;				// ����
	BYTE		m_CurTitle;				// ��ǰ��ʾ��ͷ��
	BYTE		m_FaceID;				// ��ģ��
	BYTE		m_HairID;				// ͷ��ģ��
	WORD		m_Level;				// �ȼ�
	WORD		m_TurnLife;				// ת������
	DWORD		m_TongID;				// ��������ID
	DWORD		m_ShowState;			// ��ҵı���״̬0 ԭʼ 1��2��Ӧ����״̬
	
	DWORD		m_BindMoney;			// �󶨻��ң����ɽ��ף�����˰��//����
	DWORD		m_Money;				// �ǰ󶨻��ң��ɽ��ף�����˰��//����
	DWORD		m_ZengBao;				// ���������ɽ��ף�XYD3--	   //��ȯ
	QWORD		m_ZBTotalReplenish;		// ��һ�õ���������
	DWORD		m_YuanBao;				// Ԫ������ʵ���ң��ɽ��ף���˰��
	DWORD		m_YBTranOut;			// Ԫ�������ܳ�����
	DWORD		m_YBTranIn;				// Ԫ�������ܽ�����
	DWORD		m_YBTranTax;			// Ԫ��������˰����
	QWORD		m_YBTotalReplenish;		// ��ҳ�ֵ��������
	
	DWORD		m_CurHp;				// ��ǰ����
	DWORD		m_CurMp;				// ��ǰ����
	DWORD		m_CurSp;				// ��ǰ����
	DWORD		m_CurTp;				// ��ǰ����
	DWORD		m_CurJp;				// ��ǰ����
	
	WORD		m_JingGong;				// ����
	WORD		m_FangYu;				// ����
	WORD		m_QingShen;				// ����
	WORD		m_JianShen;				// ����

	QWORD		m_Exp;					// ��ҵ�ǰ����
	WORD		m_RemainPoint;			// ���ʣ�����
	WORD		m_MingWang;				// ����
	WORD		m_PKValue;				// PKֵ
	WORD		m_XYValue;				// ����ֵ
	WORD		m_EngValue;				// ���ͼ���ֵ
	
	WORD		m_CurRegionID;			// ��ǰ��ͼID
	float		m_X;					// ��ǰ��������
	float		m_Y;
	float		m_Z;
	float		m_dir;					// ����

	WORD		m_wExtGoodsActivedNum;	// ��չ���������������(�ܱ����������=��ʼ����+��չ������)

	bool		m_bWarehouseLocked;		// �Ƿ������˲ֿ�
	BYTE		m_FightPetActived;		// ��ǰ���ڱ�������Ƿ��ڱ���״̬������ǣ���������������0-99����������ǣ���Ϊ0xff

	char		m_Name[CONST_USERNAME];					// m_Name[CONST_USERNAME-1] == 0
	char		m_Title[MAX_TITLE][CONST_USERNAME];		// ��ҵĳƺţ�0Ϊ����Զ���ġ�2014/3/27 lyĬ�ϵ�һ��Ԫ��Ϊ��ǰʹ���еĳƺ�
	SEquipment	m_Equip[EQUIP_P_MAX];					// ���Ŀǰ��װ���ĵ���
	SSkill		m_pSkills[MAX_SKILLCOUNT];				// Ŀǰ��ѧϰ���书
	SXYD3Telergy	m_Xyd3Telergy[MAX_TELEGRYNUM];		// Ŀǰ��ѧϰ���ķ�
	SFightPetExt	m_FightPets[MAX_FIGHTPET_NUM];		// ������ϵ�����

	INT64		m_lastBiguanTime;
	INT64		m_dayOnlineTime;	// ÿ�������ۼ�ʱ��
	BYTE		m_bStopTime;		// �Ƿ�ֹͣ��ʱ,���Ѿ���Ϊ�Ƿ�Ϊ���δ������
	BYTE		m_onlineGiftStates[MAX_ONLINEGIFT_NUM];	// ���߽�����ȡ״̬
	BYTE		m_FashionMode;		// ��ǰ�ķ�װģʽ����ʱװ������ͨ
	BYTE		m_bVipLevel;		// ���VIP�ȼ�
	INT64		m_dLoginTime64;		// ������ҵ�½��Ϸʱ��¼���ĵ�½ʱ��
	INT64		m_dLeaveTime64;		// ��������뿪��Ϸʱ��¼�����뿪ʱ��

	INT64		m_CountDownBeginTime;		//����ʱ���-��ʼʱ��
	BYTE     	m_dTimeCountDownGiftState;	//����ʱ���-��ǰӦ��ȡ�ĵ���ʱ�������


	DWORD		m_KillBossNum;		// ��ɱBOSS��Ŀ

	////��Ҽ�����ص�����
	BYTE      m_PlayerPattern;		//��ǰ���ڱ�������Ƿ�Ϊ�״γ�ֵ��ʶ(0Ϊ���һ��Ҳû�г�ֵ����1Ϊ��ҵ�ǰ��ֵ��һ�Σ���û����ȡ������2Ϊ�����ȡ�״γ�ֵ�������״̬)
	DWORD		m_CurUsedSkill[10];		//��ǰ���ʹ�õļ��ܡ�0-2Ϊ��ͨ��̬����ʹ�õļ��ܣ�3-5Ϊ��̬1�£�6-8Ϊ��̬2�µģ�9Ϊ����ʹ�õļ���
	////�������ص����ݣ����ÿ���õ���������ݣ�24�����������
	BYTE m_bySilerCoinUsedNum;	//����ʹ�õ�������
	BYTE m_byAnimaUsedNum;		//����ʹ�õ�������
	BYTE m_byExpUsedNum;		//����ʹ�õľ�����
	time_t m_dwPreReqTime;		//����ʹ�õ�ʱ�䡣0��ʾ����û��ʹ�ã�����ʱ���ʾ�Ѿ�ʹ��

	////���Ԫ������������
	BYTE m_IsBuyed;	//�Ƿ��Ѿ�����
	DWORD m_YuanBaoGiftPos;		//���Ԫ�������λ��

	//add by ly 2014/5/24 �³���ϵͳ�ĳ�������
	BYTE m_CurPetNum;	//��ǰ���ӵ�еĳ�����
	SNewPetData m_NewPetData[MAX_NEWPET_NUM];
	DWORD m_TransPetIndex;		//�����ս��������
	DWORD m_FollowPetIndex;	//�����ս��������
};
//���������
struct SPlayerKylinArm
{
	BYTE	byKylinArmLevel;
	DWORD	wYuanqi;

	const static BYTE MaxLevel = 9;
};
//��������
struct SPlayerTiZhi
{
	enum
	{
		TZ_MAX_HP = 0,				//����
		TZ_MAX_MP,					//����
		TZ_MAX_TL,					//����
		TZ_MAX_AT,					//����
		TZ_MAX_DF,					//����
		TZ_MAX_BJ,					//����
		TZ_MAX_SB,					//����
		TZ_MAX_SH,					//�˺�
		TZ_MAX_MS,					//����
		TZ_MAX_RS,					//�ƶ��ٶ�
		TZ_MAX_AS,					//����
		//TZ_MAX_HA,				//���ֹ�����		
		TZ_MAX_QL,					//����ۻ���		
		TZ_MAX,
	};
	BYTE byTiZhiLevel;
	DWORD dwTiZhiAttrs[TZ_MAX];
	const static BYTE MaxLevel = 10;
};
// ���ѣ�������
// struct SPlayerRelates
// {
// 	char		szName[CONST_USERNAME];
// 	BYTE		byRelation	:4;		// ��ϵ
// 	BYTE		wLevle;					// �ȼ�                                                              
// };

// struct SPlayerRelation
// {
// 	SPlayerRelates m_PlayerFriends[MAX_FRIEND_NUMBER];	// ����
// };

struct sRoseRecod
{
	char		szName[CONST_USERNAME];
	WORD		wYear;				// ��¼������
	BYTE		wMonth		:4;
	BYTE		wDay		:5;            
	DWORD		wIndex;				// ���ĵ���ID
	WORD		wRosenum;			// ����
};

struct SPlayerRoseRecod //��¼�����ͻ��ļ�¼
{
	sRoseRecod m_PlayerRoseRecod[MAX_ROSERECOD_NUM];
	DWORD	m_PlayerRoseNum;			//��ҵ�ǰ�Ļ�����
	UINT64		m_uPlayerRoseUpdata;		//��ҵ�ǰ�ͻ�״ֵ̬�����ֵ�����ж��Ƿ���¿ͻ���
};

// ���ڱ���
struct SPlayerUnionBase
{
};

// ���ڴ������Ϸ�ڴ���
struct SPlayerUnionData : public SPlayerUnionBase
{
};
// 
struct GKLUpLevelLimt //�����ʹ������
{
	char  GKLtime[30];  //��¼ʱ��
	short m_GKLUseNum;	//����
	BYTE  m_GKLFlag;
	GKLUpLevelLimt():m_GKLUseNum(0),m_GKLFlag(0)
	{
		memset(GKLtime,0,30);
	}
	void AddUseNum(short Num)
	{
		m_GKLUseNum += Num;
		if (m_GKLUseNum < 0)
		{
			m_GKLUseNum = 0;
		}
		if (m_GKLUseNum > 2)
		{
			m_GKLUseNum = 2;
		}
	}
};
struct BlessLevelLimt
{
	char  Blesstime[30];  //��¼ʱ��
	short m_BlessUseNum;	//����
	BYTE  m_BlessFlag;
	BYTE  m_BlessOnce;
	BYTE  m_HightiTEM;
	SPackageItem m_BlessSPackageitem;
	BlessLevelLimt():m_BlessUseNum(0),m_BlessFlag(0),m_BlessOnce(0),m_HightiTEM(0)
	{
		memset(Blesstime,0,30);
	}

};

struct DayPartDataRecord//�ճ�������¼
{
	WORD m_RegionID;	//EventRegion[...]�¼�����ID[...]
	BYTE m_EnterNum;	//���뼸��
	BYTE m_NumLimit;	//���븱������
	DayPartDataRecord():m_RegionID(0),m_EnterNum(0),m_NumLimit(0){}
};
struct DayRecordArray//һ����ҵ�����
{
	char  m_DayRecordtime[30];  //��¼ʱ��
	BYTE  m_DayRecordFlag;	  //0û�м�¼1�Ѿ���¼
	DayPartDataRecord m_DayRecordArray[30];//��Ƹ�������30��
	DayRecordArray():m_DayRecordFlag(0)
	{
		memset(m_DayRecordtime,0,30);
	}
	void InitRecordArray()
	{
		memset(m_DayRecordArray,0,sizeof(m_DayRecordArray));
		memset(m_DayRecordtime,0,30);
		m_DayRecordFlag = 0;
	}
};

// ����BUFF����
struct TSaveBuffData
{
	DWORD	m_dwBuffID; // Buff ID
	int		m_curStep;	// ��ǰ�Ľ׶�
	WORD	m_ActedTimes;// Buff�Ѿ�����Ĵ���
	bool	m_CanDropBuff;// �Ƿ�ɾ��
};
struct SPlayerSaveBuff
{
	SPlayerSaveBuff()
	{
		memset(this, 0, sizeof(*this));
	}
	const static BYTE msc_byMaxSaveBuffCount = 5; //��������Buff����
	TSaveBuffData m_tSaveBuffData[msc_byMaxSaveBuffCount];
};

//20150121 wk ��ǰ����,�Ƚ��������ݵ�������5K
//#define LUABUFFERSIZE 32768			//lua�����ݴ洢�ռ� 




#define MAX_BUFFERSIZE 67848        //Ԥ�������ռ�
// �̶���������ԣ��ᱣ������������	
struct SFixProperty : 
    public SFixBaseData,
    public SFixPackage,
    public SFixStorage1,
    public SPlayerTasks,
	public SPlayerJingMai,
    public SPlayerGM,
//    public SPlayerRelation,
	public SPlayerMounts,
	public SPlayerPets,
	public SXYD3FixData,
	public SPlayerTiZhi,
	public SPlayerKylinArm,
	public GKLUpLevelLimt,
	public BlessLevelLimt,
	public SPlayerRoseRecod,
	public DayRecordArray,
	public SPlayerSaveBuff
{
	enum SAVESTATE		// ��ɫ�������ϵ�״̬
	{
		ST_LOGIN =1,	// �ոյ���
		ST_LOGOUT,		// �˳�
		ST_HANGUP,		// ���߹һ�
	};

    DWORD   m_dwStaticID;					// ��ҶԷ�������ȺΨһ��̬��ID�������ݿ����������
    char    m_UPassword[CONST_USERPASS];	// ��Ҷ�������
	BYTE    m_byStoreFlag;					// ��ҵĵ�ǰ״̬���������ֱ���ʱ����Ϣ
	//wk 20150205 �Ż���Ҫ������
//	BYTE	m_bcacheBuffer[MAX_BUFFERSIZE]; // Ԥ�����������
	//BYTE	m_bluaBuffer[LUABUFFERSIZE];	// ������lua �����ݵĴ洢
	BYTE	m_bluaBuffer[10];	// ������lua �����ݵĴ洢
	// �����3�İ汾��
    static const int VERSIONID = 5;
    static DWORD GetVersion() { /*rfalse("sizeof(SfixProperty) %d", sizeof(SFixProperty)); */return (sizeof(SFixProperty) << 16 | VERSIONID); };
};


struct SPlayerTempData				// ��ҵ���ʱ���ݣ������棬����ʹ���ڿ糡��������ʱ������ݴ���
{
    DWORD	m_dwTeamID;				// ��ӵ�����
    bool	m_IsRegionChange;		// �ǳ�ʼ�����ǳ���ת��

    struct 
	{
        DWORD GID;                  // pk����
        DWORD lasttime;             // ʣ�µ�pkʱ��
    } m_PkList[30];                 // PK�����б�

    DWORD	m_dwTimeLeaving;		// ��ʱ���ݵĸ���
    DWORD	m_dwOneHourLeaving;		// ��ʱ���ݵĸ��£�Сʱ��
    DWORD   m_dwExtraState;         // ��ҵ�һЩ����״̬����̯=0x01��û�ã������ƾ���=0x02�������ƾ���=0x04
	WORD	m_wMutateID;		    // �����ͼ�����
    WORD    m_wScapegoatID;         // �����ͼ����� =0 ������
    WORD    m_wGoatEffectID;        // �����Ч�����
    WORD    m_wMonsterCountOnGoat;  // ��װ����ɱ������ļ���
    WORD    m_wGoatMutateID;        // ��ǰ����������װ���(1-11)
    DWORD   m_TVOnGoatMutate_dwTime;// ��������ʱ��
    DWORD   m_dwSaveTime;           // ����ʱ�䣡
    WORD    m_wCheckID;             // ˢ���ñ�ţ�
    BYTE    m_bQuestFullData;       // �Ƿ���Ҫ��ͻ��˷��ͳ�ʼ������
	WORD	m_wTeamSkillID;			// ���μ�ID
	WORD	m_wTeamMemberCount;		// ��Ա����
    DWORD   checkPoints[4];			// 0-����[�����ں�����չ]�� 1-Ԫ���� 2-������ 3-ͨ��
};

// �����������
struct SRegion
{
    char    szName[17];             // ����������
    WORD    ID;                     // �ó����ı��
    WORD    MapID;                  // �ó����ĵ�ͼ���
    WORD    NeedLevel;              // �ó�������ĵȼ�
};

static const int MAX_SCORE_NUMBER		= 10;
static const int MAX_SCHOOL_NUMBER		= 5;
static const int MAX_FIVESCORE_NUMBER	= 5;

// ���а��������
struct SScoreTable
{
    enum ScoreType {
                                     //  �����ơ�              ���ж�������
        ST_ALEVEL,                   // ����10��            [�ȼ�]
        ST_AMONEY,                   // ....10�󸻺�        [�����]
        ST_RFAME,                    // ....10������        [����ֵ]
        ST_LFAME,                    // ....10��ħͷ        ...
        ST_MEDICALETHICS,            // ....10��ҽ��        [ҽ��ֵ]
        ST_KILLVAL,                  // ....10�����        [PKֵ]      [�ȼ�]

        ST_SLEVEL,                   // ����10��            [����]      [�ȼ�]
        ST_SXVALUE,                  // [��ʱû��]          ...         [����ֵ]
        ST_SMONEY,                   // ����10�󸻺�        ...         [�����]	

        //new
        ST_ROSENUM,                  // ���Ӽ���            [õ����]
        ST_BAOTU,                    // ��ͼ��ʦ            [�ڱ���]
        ST_ZHENFA,                   // ����ʦ            [���ػ���]
        ST_HUNTER,                   // ��������            [���Ի���]
        ST_MIGONG,                   // �Թ�Уξ            [�Թ�����]   

		ST_FACTION,					 // ��ǿ����

        ST_MAX,
    };

    struct SScoreRecode
    {
        DWORD dwValue;  // ��ǰֵ
        char Name[11];  // ��ɫ����
    };

    struct SScoreRecodeEx
    {
        DWORD dwValue;   // ��ǰֵ
        char  Name[11];  // ��ɫ����
        DWORD sid;       // ���SID
        WORD  level;     // ��ҵȼ�
    };

    // ��������
    SScoreRecode Level[MAX_SCHOOL_NUMBER][MAX_SCORE_NUMBER];     // �ȼ����а�
    SScoreRecode XValue[MAX_SCHOOL_NUMBER][MAX_SCORE_NUMBER];    // �������а�
    SScoreRecode Money[MAX_SCHOOL_NUMBER][MAX_SCORE_NUMBER];     // ��Ǯ���а�

    // ��������
    SScoreRecode ALevel[MAX_SCORE_NUMBER];
    SScoreRecode AMoney[MAX_SCORE_NUMBER];
    SScoreRecode FAME[2][MAX_SCORE_NUMBER];                      // ����������

	// ս��������
    SScoreRecode PowerValue[2][MAX_SCORE_NUMBER];

	// ҽ������
	SScoreRecode MedicalEthics[MAX_SCORE_NUMBER];		         // ҽ������

    // ����
    SScoreRecode PKValue[MAX_SCORE_NUMBER];                      // ����

    SScoreRecodeEx RoseNum[MAX_FIVESCORE_NUMBER];                // ���Ӽ���
    SScoreRecodeEx BaoTu[MAX_FIVESCORE_NUMBER];                  // ��ͼ��ʦ
    SScoreRecodeEx ZhenFa[MAX_FIVESCORE_NUMBER];                 // ����ʦ
    SScoreRecodeEx HuntScore[MAX_FIVESCORE_NUMBER];              // ��������
    SScoreRecodeEx MazeScore[MAX_FIVESCORE_NUMBER];              // �Թ�Уξ
};

struct SSimplePlayer
{
    DWORD   dwStaticID;         // ����������ݿ��е�Ψһ���
    DWORD   dwGlobalID;         // ����ұ��ε�¼��ȫ�ֹؼ���

    WORD    wServerID;          // ��������������ID
    QWORD	dnidClient;			// ����������ϸ���ҵ�DNID

    WORD    wCurRegionID;       // ���ڳ�����ID
    DWORD   dwGMLevel;          // GM�ȼ�

	BYTE	bOnlineState;		// 0:����  1:����  2:�һ� 
};

// ������Ϣ�б�����ݽṹ
const int MAXPUBLICINFO		= 128;
const int MAXPUBLICINFONUM	= 10;

struct SPublicInfo
{
	SPublicInfo() { nTimeAdd=0; }

	char	szSay[MAXPUBLICINFO];	// ����
	long	lClr;					// ��ɫ
	int		nTime;					// ���ʱ��
	int     nTimeAdd;				// ʱ���ۼ�
	WORD	wType;

	enum
	{
		TP_CHANNEL,					// ����Ƶ����ʾ
		TP_ROLL,					// ������ʾ
	};
};

class CPublicInfoTab
{
public:
	CPublicInfoTab() { m_listInfo.clear(); }

	std::list<SPublicInfo> m_listInfo;

	void AddPInfo(SPublicInfo pInfo)
	{
		if (m_listInfo.size() >= MAXPUBLICINFONUM)
			m_listInfo.pop_front();
		m_listInfo.push_back(pInfo);
	}
};

// �̶���������ԣ��ᱣ������������
typedef SFixProperty SFixPlayerDataBuf;

// ������������
struct SSpanPlayerDataBuf : public SFixBaseData, public SXYD3FixData
{
    static const int VERSIONID = 0;

    SSpanPlayerDataBuf()
    {
        m_version = GetVersion(); 
    }

    static DWORD GetVersion() { return (sizeof( SSpanPlayerDataBuf ) << 16 | VERSIONID); };
};

// �°汾�����˺Ź�������
struct SWareHouses
{
};

// 
// struct SRankList
// {
// 	struct SRankContent
// 	{
// 		char	name[11];	// ����
// 		BYTE	school;		// ����
// 		DWORD	value;		// ��ǰֵ
// 	};
// 
// 	SRankContent Level[MAX_RANDLIST_NUM];		// �ȼ�����
// 	SRankContent Money[MAX_RANDLIST_NUM];		// ��Ǯ����
// };

// ============================XYD3���а�====================
//20150427 wk 100 ��Ϊ1
//static const int MAX_RANKLIST_NUM	= 100;		// ���͸��ͻ���������а���Ŀ
static const int MAX_RANKLIST_NUM = 1;		// ���͸��ͻ���������а���Ŀ

struct NewRankList
{
	DWORD Level;				// �ȼ�
	DWORD Money;				// �ǰ󶨻���
	DWORD BossNum;				// ��ɱBOSS��Ŀ
	char name[CONST_USERNAME];	// �ǳ�
	BYTE  School;				// ����

	NewRankList():Level(0),Money(0),BossNum(0)
	{
		memset(name,0,CONST_USERNAME);
	}
};

// ���ڷ��͸��ͻ��˵����а�ṹ
enum RANK_TYPE
{
	RT_LEVEL,
	RT_MONEY,
	RT_BOSSKILL,

	RT_MAX,
};

struct Rank4Client
{
	Rank4Client() {}
	Rank4Client(char* pName, BYTE rt, DWORD val, BYTE sc) : RankType(rt), RankValue(val), School(sc)
	{
		memcpy(name, pName, sizeof(char) * CONST_USERNAME);
	}


	BYTE	RankType;				// ���а�����
	DWORD	RankValue;				// ���а���ֵ	
	char	name[CONST_USERNAME];	// �������
	BYTE	School;					// ����
};

//20150821 �������а�,��ͨ����
//������ֵ,sid,�ȼ�,ͷ��,vipLV,����,�����,heroID,heroStar,heroStep,heroLv  --50��,1Ϊ������,��5������,�佫id �佫�Ǽ� �佫Ʒ�� �佫�ȼ�,������û��
struct RankList_SG
{
	DWORD Num;				// ������ֵ
	DWORD sid;				// sid
	DWORD lv;				// �ȼ�
	DWORD icon;				// ͷ��
	char name[CONST_USERNAME];	// �ǳ�
	char fname[CONST_USERNAME];	//�����

	RankList_SG() :Num(0), sid(0), lv(0), icon(0)
	{
		memset(name, 0, CONST_USERNAME);
		memset(fname, 0, CONST_USERNAME);
	}
};
//20150821 �������а�,��������
struct RankList_hero_SG
{
	DWORD Num;				// ������ֵ
	DWORD sid;				// sid
	DWORD lv;				// �ȼ�
	DWORD icon;				// ͷ��
	char name[CONST_USERNAME];	// �ǳ�
	char fname[CONST_USERNAME];	//�����

	DWORD heroID;				// �佫id
	DWORD heroStar;				// �佫�Ǽ�
	DWORD heroStep;				// �佫Ʒ��
	DWORD heroLv;				// �佫�ȼ�

	RankList_hero_SG() :Num(0), sid(0), lv(0), icon(0),  heroID(0), heroStar(0), heroStep(0), heroLv(0)
	{
		memset(name, 0, CONST_USERNAME);
		memset(fname, 0, CONST_USERNAME);
	}
};

//�������а�ö��
enum RankType_SG
{
	MaxHero=1,// ������, 
	BattleTeam=2,// ���ݰ�, 
	HeroAllStar=3,// ���ǰ�, 
	PlayerLevel=4,// �ȼ���, 
	Overcome=5 ,//����ն����, 
	HeroNum=6,// �佫����, 
	PlayerMoney = 7,// ��Ҹ�����
};
////////////////////////////////////////////////////////


