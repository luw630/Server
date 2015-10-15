#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////
// ���ݶ���ԭ��
// ��������Ķ��ǻ�ȫ���õ������ݽṹ
///////////////////////////////////////////////////////////////////////////////////////////////

enum    E_EXTRA_STATE       // ս�����ӵ�����״̬
{
    EES_DOUBLEBLOOD=1,        // ˫����Ѫ 1 2 4 8...
};

enum E_WOUNDTYPEEX      // �������͵���չ���
{
    EWT_MISS = 10,      // ����
    EWT_GUARD,          // ����
    EWT_NOTATACK,       // ��������
};

enum E_EXTRASTATUS   // ���˸���Ч�� ����λȡ��
{
    EES_NONE =0,    // û�и���Ч��
//    EES_DIZZY=1,    // ѣ��
    EES_XXX  =2,
};

enum    E_ATTACK_CTRL   // ��ҹ���������
{
    EAC_NONE,
    EAC_NOPLAYER,       // ����Ctrl�����ܿ���Χ�ڵ����
    //...
};

enum    E_VENATION_LIST // ��������
{
    VENALIST_DAM,       // ��̫��
    VENALIST_POW,       // ������
    VENALIST_DEF,       // ��̫��
    VENALIST_AGI,       // ������
    VENALIST_NONE,      // �޾���
};


// ���𹥻�������
struct SAttack
{
	DWORD	dwGlobalID;			// �����ߵ�ID
	WORD	wPosX;  			// ��ǰս���������λ��(S->C)���߹�����Ŀ���(C->S)
	WORD	wPosY;  			// ...
    BYTE    byDirection;        // (S->C)�������� ($1�б�Ҫ�ֿ�����������)

	BYTE    byWaitFrames;		// �ȴ�֡�����Է������ĵ�ǰ֡��Ϊ׼������Ҫ����֡���ߵ�[wPosX/wPosY]

	//BYTE	byAttackType;		// �����ķ�ʽ
    BYTE	byFightID;	        // ������ʽ��� 0=��ͨ����

    BYTE    byLevel;            // ��ǰ������ʽ�ȼ�
    DWORD   dwProficiency;      // ���ܵ�������
    DWORD   dwMaxProficiency;   // ���ܵ����������ֵ
    BYTE    byCtrlByte;         // ������
};

// ������������
struct SWound
{
	DWORD	dwFightGlobalID;	// �����ߵ�ID
	WORD	wPosX;  			// ��ǰս���������λ��
	WORD	wPosY;  			// ...

    BYTE    byAttackerSex;      // �������Ա�
    WORD    wAttackWeapon;      // �����ߵ����������ţ���������������������Ч����Ϊ����
    BYTE	byFightID;          // ��������ʽID

	BYTE	byWoundType;		// ���˵�����
    BYTE    byDizzyTime;        // ѣ��ʱ��
	BYTE	byExtraStatus;		// ���ӵ�����״̬���������е����Զ������ݣ�һ��ֻ����һ��״̬�����ӣ�
//	BYTE	byIsCritical;		// �Ƿ�Ϊ����һ�������Է�����
	BYTE	byIsDead;			// Ŀ���Ƿ�����
	DWORD	dwDamage;			// ��ɵ��˺�
    DWORD	dwLastHP;			// ��ʣ�µ�HP
    DWORD	dwMaxHP;			// HP���ֵ
    WORD	wLastMP;			// ��ʣ�µ�HP
    WORD	wMaxMP;			    // HP���ֵ
    WORD	wLastSP;			// ��ʣ�µ�HP
    WORD	wMaxSP;			    // HP���ֵ
    BYTE    byWhichVena;        // ���˾���
    BYTE    byVenaState;        // ��������״̬
};

// ����� SWound��36 -> 16 �ֽ�
struct SWoundTiny
{
	enum
	{
		Hit = 0,				// ����
		Not_Hit,				// δ����
		BAOJI_HIT,			//����
		UNCRIT_HIT,			//����
		WRECK_HIT,			//�ƻ�
		UNWRECK_HIT,		//����
		PUNCTURE_HIT,	//����
		UNPUNCTURE_HIT, //����
		NONE,					// �߽��ǣ�ʲôҲ����
	};

	BYTE	mHitState;			// ��������(���У�δ���У�������˺��������˺�)
	WORD	mDamage;			// ��ɵ���ͨ�˺�
    DWORD	mDefenserGID;		// ��������ID
    float	mDefenserX;			// ��������ս���������λ��
	float	mDefenserY;
// 	WORD	mDamageExtra;		// ��ɵĶ����˺�
// 	WORD	mIgnoreDefDamage;	// ��ɵ��Ʒ��˺�
// 	WORD	mRebound;			// �����˺�
// 	BYTE	isCritical;			// �Ƿ��ػ�
	DWORD	mMaxHp;				// ���Ѫ
	DWORD	mCurHp;				// ��ǰѪ
};
