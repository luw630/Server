#pragma once

#include "FightObject.h"

const int PMAXLEVEL = 70;
// ÿһ���ȼ���Ҫ�ı�ĸ�������
typedef	struct	_SLevelUnit
{
	DWORD	dwMaxHP;					// �������е��������ֵ
	WORD	wMaxMP;					// �������е��������ֵ
	WORD	wMaxSP;					// �������е��������ֵ
	WORD	wAmuck;				    // �����ۼ�ɱ��
	WORD	wPoint;					// �����ۼƷ������
    UINT64	dwExp;					// �������辭��ֵ
}SLevelUnit,*LPSLevelUnit;

// ÿһ�����ɵĻ��������Լ�ÿһ���ȼ���Ҫ�ı�ĸ�������
typedef	struct	_SLevelData
{
	// ��������
    WORD	wBaseEN;				// ���� �����ʣ�
    WORD	wBaseST;				// ���� ��ǿ׳��
    WORD	wBaseIN;				// ���� ���ǻۣ�
    WORD	wBaseAG;				// �� ��������
    WORD	wBaseLU;				// ���� ��������
    // ��������
	WORD	wBaseDAM;				// �书��������ʼֵ��4λ���ڣ������ɣ�װ�������������й�
	WORD	wBasePOW;				// �ڹ���������ʼֵ��4λ����
	WORD	wBaseDEF;				// ��������ʼֵ��4λ����
	WORD	wBaseAGI;				// �����ȳ�ʼֵ��4λ����
    // �������Ա仯��
    WORD	wHPVal;					// ������������
    WORD	wMPVal;					// ������������
    WORD	wSPVal;					// ������������
    // ������λ����ϵ��
    WORD    wWound[3];              // ���������α�������ϵ��
    
    SLevelUnit	Data[PMAXLEVEL];			// ÿһ���ľ�������
}SLevelData,*LPSLevelData;

class   DUpgradeSys
{
    SLevelData  SchoolData[5];
public: 
    DUpgradeSys(void);
    ~DUpgradeSys(void);

    BOOL    LoadLevelData(void);
    SLevelData *GetLevelData(BYTE bySchool);

    //void    SendAddPlayerExp( IObject *pObj, WORD wExpVal );
};

extern  DUpgradeSys     *g_pUpGradeData;
