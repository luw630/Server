//--------------------------------------//
//Dreamwork soft Ltd.					//
//Luou July 22nd 2004					//
//--------------------------------------//
//���ѹ���˵����
//���ѹ�ϵ��Ϊһ��һ�Ĺ�ϵ����˫����Ϊ����
//��һ�Զ�Ĺ�ϵ����ĳ�˵�����������һ�˵ĺ��ѣ����Է����������ĺ���
//����������еĺ�������֪ͨ��һ��һ�ĺ��Ѽ以������һ�Զ�ĺ��Ѽ�����
// ��������û����ʾ
//����ʱ������ SPlayerRelates::byRelation ���жϸ�����������Լ��Ĺ�ϵ��������ֵ������μ���Ӧע��
//m_szFriendNameListΪ��Ϸ���������Բ�ѯ����map�������ṩ��ݲ�����Ĳ�ѯ��������ָ��Խ������


#pragma once 
#include "networkmodule/PlayerTypedef.h"
#include "NETWORKMODULE/HeroTypeDef.h"
#include "BaseObject.h"
#include <list>
#include <map>
#include "NETWORKMODULE/RelationMsgs.h"

//----------------------------------
#include <set>
#include <string>



//-----------------------------------------------------
class CRelations 
{
public:
	struct tRelationData
	{
		dwt::stringkey<char[CONST_USERNAME]> sName;
		dwt::stringkey<char[CONST_USERNAME]> sHeadIcon;
		WORD					  wLevle;				//�ȼ�
		tRelationData::tRelationData():wLevle(0){}
	};


public:

//�º����б�
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_mapFriendList;	//����
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_mapEnemieList;//����
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_mapMateList;// ���
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_mapBlockList;// ������
	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_mapTempList;//��ʱ����

	std::map<dwt::stringkey<char[CONST_USERNAME]>, tRelationData> m_FriendListArr[5];


	inline size_t GetFriendNum() {return m_mapFriendList.size();};
	inline size_t GetBlockNum()  {return m_mapBlockList.size();};
	inline size_t GetEnemiesNum(){return m_mapEnemieList.size();};
	inline size_t GetMateNum()   {return m_mapMateList.size();};
	inline size_t GetTempNum()   {return m_mapTempList.size();};

    //byRealtion 
    //0 -- ����
	//1 -- ������
    //2 -- ��������
    //3 -- �������
	BOOL AddRelation_new(BYTE byRelation,tRelationData relationdata,BOOL bSend=TRUE);//��ӹ�ϵ
	BOOL AddRelation_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation, BOOL bSend = TRUE, int nDegree = 1);//��ӹ�ϵ

	BOOL RemoveRelation_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation, BOOL bSend = TRUE);//ȡ����ϵ

	BOOL FindRelation_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation);

	BOOL IncHateDegree(DNID dnidClient, dwt::stringkey<char[CONST_USERNAME]> strName); // ��1����ֵ
	BOOL DecHateDegree(DNID dnidClient, dwt::stringkey<char[CONST_USERNAME]> strName); // ��1����ֵ

	BOOL ChangeDegree(DNID dnidClient, dwt::stringkey<char[CONST_USERNAME]> strName, int &nDegree); // �ı����ܶ�
	


    int Init(SFixProperty *pData);// ��ʼ���˼ʹ�ϵ�б�
	int Init(SFixData *pData);
	int GetFriends(char szFriends[MAX_FRIEND_NUMBER][CONST_USERNAME]); // ������Ա������
	int GetFriends_new(char szFriends[MAX_PEOPLE_NUMBER][CONST_USERNAME]); // ������Ա������
    //int GetBlocks(char ** szBlocks);  // ���غ�����������
    //int GetMates(char ** szMates);   // ���ؽ�ݵ�����

    // ��������֪ͨ
	void FriendsOnline(struct SFriendOnlineMsg  *pFriendOnlineMsg);
	void FriendsOnline_new(struct SFriendOnlineMsg  *pFriendOnlineMsg,BYTE byRelation = RT_FRIEND);

	// ͬ�����еĺ�������״̬���������������˵ȣ�
	 void RefreshFriends_new();
    // ͬ�����й�ϵ������

	void SynAllRelations_new(DNID dnidClient);

    // ��ʱˢ�º��ѵ�״̬���������ܶ�

	int GetFriendDegree_new(dwt::stringkey<char[CONST_USERNAME]> strName, BYTE byRelation = 0);
    // ͨ��һ���ṹ��������ҵĹ�ϵ�б�
	BOOL SetPlayerRelations_new(SPlayerRelation *lpPR);
    // ����ҵĹ�ϵ�б��浽һ���ṹ��

	BOOL SavePlayerRelations_new(SPlayerRelation *lpPR);
	///��ȡ���ߺ��Ѹ���
	int GetOnlineFriendsNum();

	BOOL ProcessRelationMsg(DNID dnidClient, struct SRelationBaseMsg *pRelationMsg, CPlayer *pPlayer);
	BOOL OnRecvAddFriendMsg(DNID dnidClient, struct SQAddFriendMsg *pRelationMsg, CPlayer *pPlayer);
	BOOL OnInitFriendData(CPlayer *pPlayer);
public:
    CRelations();
    ~CRelations();
};
