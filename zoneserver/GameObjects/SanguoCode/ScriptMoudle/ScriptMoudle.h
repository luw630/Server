// ----- ScriptMoudle.h -----
//
//   --  Author: wk
//   --  Date:   15/01/22
//   --  Desc:   �Ƚ����� �ű�������ͷ�ļ�
// --------------------------------------------------------------------
// �����Ľű���غ�����������,��Ϯ�����Ľű�����汾̫��,�л�������������
//---------------------------------------------------------------------  

/*
��ǰ�����޸ĵĵط�:
E:\SanguoServer\trunk\ServerProjects\���������\GameObjects\PlayerData.cpp
m_Property �滻Ϊ m_FixData �����������
*/
#pragma once
struct lua_State;
class CSSanGuoFunc
{
public:
	static int L_GetPlayerDataSG(lua_State *L);	//ȡ�����������
	static int L_GetGoods(lua_State *L);///ȥ��ҵĵ�������
	static int L_AddGoods_SG(lua_State *L);	//������ҵ�������
	static int L_AddGoodsArray_SG(lua_State *L);///����������ҵĵ�������
	static int L_AddSkillPoint(lua_State *L);///�Ӽ��ܵ���
	static int L_DecGoods_SG(lua_State *L);	//�۳���ҵ�������
	static int L_CheckGoods_SG(lua_State *L);	//�����ҵ�������
	static int L_SendLoginGMRes(lua_State *L);	//GMtool��½��֤��������ͻ���

	static int L_SendCheckinData(lua_State *L);	//ͬ��ǩ����Ϣ��ǰ̨
	static int L_GiveCheckinAwards(lua_State *L);	//��ǩ������
	static int L_TipMsg(lua_State *L);	//����ʾ��Ϣ
	static int L_SetPlayerDataSG(lua_State *L);	//�޸������������
	static int L_SynPlayerDataSG(lua_State *L);	//����״ε�½ͬ���ű�����
	static int L_SynHeroExtendsData(lua_State *L); ///����佫����չ���淨������ͬ��
	static int L_SynNotificationDataSG(lua_State *L); ///<��ҵ�½ͬ��ϵͳ�������ݸ����
	static int L_SynMissionDataSG(lua_State *L);///<ͬ����ҵ��������ݸ����
	static int L_UpdateMissionStateSG(lua_State* L);///<ͬ����ҵ�ĳһ����������ݵ��ͻ���
	static int L_SendMsg_SG(lua_State *L);	//�ű�������Ϣ���ͻ���
	static int L_ActiveLeagueOfLegend(lua_State *L);///�����˵��ĳЩ����
	static int L_gm_registerNum(lua_State *L);	//gmȡע������
	static int L_gm_stayAll(lua_State *L);	//gmȡ30������
	static int L_gm_stayOne(lua_State *L);	//gmȡ��������
	static int L_gm_online(lua_State *L);	//gmȡ��������
	static int L_gm_recharge(lua_State *L);	//gmȡ��ֵ��Ϣ
	static int L_gm_leave(lua_State *L);	//gmȡ��ʧ����
	static int L_gm_pointuse(lua_State *L);	//gmȡ������Ϣ
	static int L_gm_operation(lua_State *L);	//gmȡ��̨������Ϣ

	static int L_UpdateVIPLevel(lua_State *L);	//�������VIP�ȼ�
	static int L_SetDessertDispachInfor(lua_State *L);///����С����ķ������
	static int L_ComsumableUsedInfor(lua_State *L);///����Ʒʹ�ú�������Ϣ�����ӿ�
	static int L_ChangeName(lua_State *L);///�޸����ƹ���
	static int L_SendActResult(lua_State *L); ///������ȡ��������
	static int L_SendChargeSumResult(lua_State * L); ///�����ۼƳ�ֵ��������
	static int L_SendFirstRechargeRes(lua_State *L);//��ȡ�׳影�����
	static int L_WeekReMoney(lua_State *L);//����
	static int L_SendMail(lua_State *L);//�ű����ʼ�

	static int L_GetHeroInfo(lua_State *L);//ȡ����佫��Ϣ
	static int L_WarMsg(lua_State *L);//��ս��ǰ̨��Ϣ�������
	static int L_WarMail(lua_State *L);//��ս�ʼ�����

	static int L_RankMsg(lua_State *L);//���а���Ϣ
	static int L_SynMembershipToClient(lua_State * L); //ͬ���¿���Ա��ֹ���ڵ��ͻ���

	static int L_exchangeActRes(lua_State *L);//�һ�����
	static int L_exchangeActSendData(lua_State *L);//�һ���������ͬ��

public:
	static void CreateShadow(void);

};