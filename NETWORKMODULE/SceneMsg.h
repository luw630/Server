#pragma once

#include "NetModule.h"
#include "../pub/ConstValue.h"
#include "PlayerTypedef.h"
#define MAX_STAR_SIZE 256				//�������ݴ�С
#define MAX_SCENE_SIZE		1024		//�ؿ����ݴ�С
#define MAX_IREWARD_SIZE	256		//�����������ݴ�С
#define MAX_OBJ	3
#define MAX_LIST_SIZE		128
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// �ؿ������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SSceneMsg, SMessage, SMessage::EPRO_SCENE_MESSAGE)
EPRO_SCENE_INIT,				//���볡����ͼ�ؿ�ͬ��һ������
EPRO_SCENE_SELECT,				//ѡ������ĳһ���ؿ�
EPRO_SCENE_ENTER,				//ѡ�����ĳһ���ؿ�
EPRO_SCENE_WINREWARD,		//ʤ������
EPRO_SCENE_PICKCARDS,		//ʤ���Ժ����
EPRO_SCENE_FAIL,					//����ʧ��
EPRO_SCENE_SDINFO,				//ɨ����Ϣ
EPRO_SCENE_SDREWARD,		//������ɨ����Ľ���
EPRO_SCENE_LEAVE,					//�����뿪�ؿ�
EPRO_SCENE_LIST,					//�½ڵĹؿ��б�
EPRO_SCENE_FINISH,					//���͹ؿ����
EPRO_SCENE_QUESTREWARD,		//����ؿ�����
EPRO_SCENE_DEBRISINFO,		//��Ƭ�ռ���Ϣ
EPRO_SCENE_DEBRISAWARD,		//��Ƭ�������
EPRO_SCENE_BATCHESMONSTER, //����ˢ�����֪ͨ
EPRO_SCENE_SDREWARDLIST,				//ɨ�������б�
EPRO_SCENE_GROUPMONSTERDIE,			//��ǰ����Ĺ����Ѿ�����(���ڵ���ģʽ�ͻ���֪ͨ������)
END_MSG_MAP()


// ����ǰ�Ĺؿ�����
DECLARE_MSG( SSceneInit, SSceneMsg, SSceneMsg::EPRO_SCENE_INIT )
struct SQSceneInit : public SSceneInit
{
	BYTE		bSceneMapIndex;			//������½�����
};

struct SASceneInit : public SSceneInit
{
	WORD	wCurrentSceneMap;	//��ǰ�½ڵ�ͼ
	WORD	wCurrentScene;	//��ǰ�ؿ�
	BYTE		bStarData[MAX_STAR_SIZE]; //��ǰ�ؿ�����������������
};


// ����ѡ��Ĺؿ����ݣ� ���ͻ���ѡ��ĳһ�ؿ���ʱ��
DECLARE_MSG( SSceneSelect, SSceneMsg, SSceneMsg::EPRO_SCENE_SELECT )
struct SQSceneSelect : public SSceneSelect
{
	WORD	wSceneMap;		//ѡ����½ڵ�ͼ
	WORD wsceneIndex;		//ѡ��Ĺؿ�
};
struct SASceneSelect : public SSceneSelect
{
	WORD	wscenemap;		//ѡ����½�
	WORD wsceneIndex;		//ѡ��Ĺؿ�
	BYTE		bsdtime;		//��ǰʣ��ɨ������
	BYTE		bmaxsdtime;		//���ɨ������

	BYTE		bcommontime;	//��ǰʣ����ͨ����
	BYTE		bmaxcommontime;	//�����ͨ����

	BYTE		bsplimit;		//�������ƣ����븱���������ĵ�����
	WORD SDTime;		//ͨ�عؿ�����ʱ����
	WORD  wbesttime;//���ʱ����

	DWORD	dexpreward;	//  ���齱��
	DWORD	dmoneyreward;//��Ǯ����
	BYTE			bitemreward[MAX_IREWARD_SIZE];//װ������ UINT ����
	BYTE		bequip[MAX_IREWARD_SIZE];//װ���Ƽ� UINT ����
	
	char   bestplayername[CONST_USERNAME];//�������������
	
};


//  ���ͻ���ѡ�����ĳһ�ؿ���ʱ��
DECLARE_MSG( SSceneEnter, SSceneMsg, SSceneMsg::EPRO_SCENE_ENTER )
struct SQSceneEnter : public SSceneEnter
{
	WORD	wSceneMap;		//ѡ����½ڵ�ͼ
	WORD wsceneIndex;		//ѡ��Ĺؿ�
	BYTE		bentertype;		//���뷽ʽ��0 ���� 1 ɨ��
	BYTE		bcount;				//ɨ������
};
struct SASceneEnter : public SSceneEnter
{
	WORD  wSDcost;			//ɨ��������ȴ������
	DWORD	dSDTime;		//ɨ��ͨ����ȴʱ����
	BYTE		bresult;			// 0 ��ͨͨ�ؿ�ʼ 1 ɨ��ͨ�ؿ�ʼ 2 ����ؿ�����ɨ���ؿ�ʧ�� �������� 3 �������� 4 ��Ǯ���� 
}; 

//ͨ�عؿ��Ժ�Ľ�����������
DECLARE_MSG( SWinreward, SSceneMsg, SSceneMsg::EPRO_SCENE_WINREWARD )
struct SAWinrewardInfo : public SWinreward				
{
	BYTE	bwintype;		//ͨ������ 0 ��ͨͨ�� 1ɨ�� 
	BYTE bwinstar;		//�����Ǽ�
	BYTE	 bspecialscene;			//�����ؿ� 1 ��ͨ�ؿ� 2�����ؿ�
	WORD	wfinishtime;	//ͨ��ʱ��
	WORD	wkillcount;	//��ɱ����
	WORD	wmaxhit; //�������
	DWORD	dexp[MAX_OBJ];//����ֵ 0  ��� 1 ���� 2 ����
	DWORD	dmoney;//��ҵõ�����������
	DWORD	dmoneyextract[3];		//��Ϸ�ҳ�ȡ
	DWORD	dgoldextract[3];			//��ҳ�ȡ
	BYTE			bitemreward[MAX_IREWARD_SIZE];//װ������ UINT ����		
};

//�ͻ����������ʤ������
DECLARE_MSG( SPickCards, SSceneMsg, SSceneMsg::EPRO_SCENE_PICKCARDS )
struct SQPickCards: public SPickCards		
{
	BYTE		bextracttype; //�������� 0 ��Ϸ�� 1 ���
	BYTE		bextractindex;//��������
};

//���������ͳ���ʤ������
struct SAPickCards: public SPickCards		
{
	BYTE		bextracttype; //�������� 0 ��Ϸ�� 1 ���
	BYTE		bextractindex;//��������
	BYTE		bitemreward[MAX_IREWARD_SIZE];//װ������ UINT ����		
};

//����ʧ����Ϣ
DECLARE_MSG( SPassFail, SSceneMsg, SSceneMsg::EPRO_SCENE_FAIL )
struct SAPassFail:public SPassFail
{
	WORD	wfinishtime;	//ͨ��ʱ��
	WORD	wmaxhit; //�������
};

//�ͻ�������ɨ����ֹ
DECLARE_MSG( SStopSD, SSceneMsg, SSceneMsg::EPRO_SCENE_SDINFO )
struct SQStopSD: public SStopSD		
{
	BYTE bStopType;// ɨ��ֹͣ���� 0 ɨ��ֹͣ 1 ʹ�ý��ֱ�����ɨ��
};

struct SAStopSD: public SStopSD		
{
	WORD	wSceneMap;		//ѡ����½ڵ�ͼ
	WORD wsceneIndex;		//ѡ��Ĺؿ� 
	BYTE		bresult;				// 0 ɨ���ؿ����  1 �Ѿ�ֹͣɨ���Ĺؿ�  2 ֹͣʧ�� 3 ���߿�ʼɨ������ɨ���Ĺؿ�ID
};


//�ͻ�������ɨ���Ľ���
DECLARE_MSG( SSDReward, SSceneMsg, SSceneMsg::EPRO_SCENE_SDREWARD )
struct SQSDReward: public SSDReward		
{
	BYTE bisdouble;//�Ƿ�˫������ 0 1
};

struct SASDReward: public SSDReward		
{
	BYTE		bresult;				// 0 �޷���ȡ���� ��������   1 ������ȡ��ɣ��رս��棩 
};


//�ͻ��������뿪�ؿ�
DECLARE_MSG( SSceneLeave, SSceneMsg, SSceneMsg::EPRO_SCENE_LEAVE )
struct SQSceneLeave: public SSceneLeave		
{
};

//���½��б����
DECLARE_MSG( SSceneList, SSceneMsg, SSceneMsg::EPRO_SCENE_LIST )
struct SASceneList: public SSceneList		
{
	WORD	wSceneMap;		//ѡ����½ڵ�ͼ
	WORD	wCurrentScene;	//�Ѿ����������ؿ�
	BYTE		bhighlight[MAX_LIST_SIZE];//��Ҫ�����Ĺؿ����� ����1Ϊ��ʼ����
};

//���͵�ǰ�ؿ��Ѿ���ɵ���Ϣ 
DECLARE_MSG(SSceneFinish, SSceneMsg, SSceneMsg::EPRO_SCENE_FINISH)
struct SASceneFinish : public SSceneFinish
{
};

//����ؿ�����
DECLARE_MSG(SSceneQuestReward, SSceneMsg, SSceneMsg::EPRO_SCENE_QUESTREWARD)
struct SQSceneQuestReward : public SSceneQuestReward
{
};

//������Ƭ�ռ���Ϣ
DECLARE_MSG(SDebrisinfo, SSceneMsg, SSceneMsg::EPRO_SCENE_DEBRISINFO)
struct SQDebrisinfo : public SDebrisinfo
{
};

struct SADebrisinfo : public SDebrisinfo
{
	BYTE bbadgeindex;//��ǰ�ռ��Ļ�������
	BYTE bDebrisnum;//��ǰ��Ƭ������
	BYTE bDebrisAllnum;//��ǰ����������Ƭ������
};

//��Ƭ����
DECLARE_MSG(SDebrisAward, SSceneMsg, SSceneMsg::EPRO_SCENE_DEBRISAWARD)
struct SADebrisAward : public SDebrisAward   //֪ͨ�ͻ��˿�����ȡ��ǰ���½���
{
};

struct SQDebrisAward : public SDebrisAward   //�ͻ���������ȡ��ǰ���½���
{
};

//����ˢ��֪ͨ
DECLARE_MSG(SBatchesMonster, SSceneMsg, SSceneMsg::EPRO_SCENE_BATCHESMONSTER)
struct SABatchesMonster : public SBatchesMonster   //����ˢ��֪ͨ
{
	BYTE bIndex; //��ǰ�ڼ���
	BYTE bstate;//״̬��0 ���� 1ˢ��
};

// �ؿ�ɨ�����ʱ���͵�ǰɨ�������б�
DECLARE_MSG(SSceneRewardList, SSceneMsg, SSceneMsg::EPRO_SCENE_SDREWARDLIST)
struct SASceneRewardList : public SSceneRewardList   //ɨ�������б�
{
	WORD	wSceneMap;		//ѡ����½ڵ�ͼ
	WORD wsceneIndex;		//ѡ��Ĺؿ� 
	DWORD	dexp;//����ֵ���� 
	DWORD	dmoney;//������ҽ���
	WORD  wbuymoey; //����˫�����ѽ��
	BYTE		bitemreward[MAX_IREWARD_SIZE];//װ������ UINT ����	
};

//��ǰ����Ĺ����Ѿ�����(���ڵ���ģʽ�ͻ���֪ͨ������)
DECLARE_MSG(SGroupMonsterDie, SSceneMsg, SSceneMsg::EPRO_SCENE_GROUPMONSTERDIE)
struct SQGroupMonsterDie : public SGroupMonsterDie
{
};