//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   NetModule.h
//FileAuthor		:	luou
//FileCreateDate	:	15:09 2003-4-3
//FileDescription	:	�ͻ��˼����������ڲ��������ĳ������

//ģ��˵����
//���� KNetServer��һ��ģ�飬�����������������
//Initialize ��Ϊ��ʼ������

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once 

//#ifndef _NETMODULE_H_
//#define _NETMODULE_H_

// �����������ľ�����Ϣ���Ͷ�����SQ����SA����ͷ��
// S��struct�ı�ʶ
// Q&A����ʾΪͬһ�����͵�������Ӧ��
// �ֱ�ʾ����Ϣ����������
// ���磺
// SQLoginMsg	�ڿͻ������ɣ��������������ɷ������������Ϣ
// SALoginMsg	�ڷ��������ɣ������ͻ��ˣ��ɿͻ��˴������Ϣ

typedef unsigned __int64 QWORD;
typedef QWORD DNID;

#define DECLARE_MSG(name, BaseMessage, MessageType) struct name : public BaseMessage { name() {BaseMessage::_protocol = MessageType; } };
#define DECLARE_MSG_MAP(name, BaseMessage, MessageType) struct name : public BaseMessage { name() { BaseMessage::_protocol = MessageType; /*ProtocolIndex=32; ClientIndex=0xffff;*/ } enum {

#define END_MSG_MAP() }; BYTE _protocol; };
#define END_MSG_MAP_WITH_ROTOC() }; BYTE _protocol; DNID dnidClient; QWORD qwSerialNo; }; // R.O.T.O.C mean is respon operation time out check
#define END_MSG_MAP_WITH_ROTOC_GMLEVEL() }; BYTE _protocol; DNID dnidClient; QWORD qwSerialNo; WORD wGMLevel;}; // R.O.T.O.C mean is respon operation time out check


struct SMessage
{
    enum 
    {
        EPRO_MOVE_MESSAGE = 32,		// �ƶ���Ϣ
        EPRO_CHAT_MESSAGE,			// �Ի���Ϣ
        EPRO_FIGHT_MESSAGE,			// ս����Ϣ
        EPRO_SCRIPT_MESSAGE,		// �ű���Ϣ
        EPRO_REGION_MESSAGE,        // ������Ϣ
        EPRO_ITEM_MESSAGE,          // ������Ϣ
        EPRO_SYSTEM_MESSAGE,		// ϵͳ������Ϣ
        EPRO_UPGRADE_MESSAGE,       // ������Ϣ��������Ա仯��
        EPRO_TEAM_MESSAGE,          // �����ص���Ϣ  �������������Ϣ�� 
        EPRO_TONG_MESSAGE,          // �����ص���Ϣ  �������������Ϣ��
        EPRO_MENU_MESSAGE,          // �˵�ѡ�����
        EPRO_NAMECARD_BASE,         // ��Ƭ
        EPRO_RELATION_MESSAGE,      // ����,�������ȵ�
        EPRO_SPORT_MESSAGE,         // �˶�������
		EPRO_BUILDING_MESSAGE,		// ��̬���������Ϣ
        EPRO_PLUGINGAME_MESSAGE,	// ����ʽС��Ϸ
		ERPO_MOUNTS_MESSAGE,		// ���������Ϣ
		EPRO_Center_MESSAGE,		//���Ĺ���������Ϣ
		EPRO_TRANSFORMERS_MESSAGE,	// ������Ϣ
		EPRO_DYARENA_MESSAGE,	// ��������Ϣ

        EPRO_COLLECT_MESSAGE = 0xD0,// ���ݲɼ���������Ϣ

        EPRO_GAMEMNG_MESSAGE = 0xe0,// GMָ�������������֤�ˣ�
        EPRO_GMM_MESSAGE,           // GMģ��ָ�������
        EPRO_GMIDCHICK_MSG,         // ����GM�����֤��Ϣ
		EPRO_GMTOOL_MSG,         // GM�������

        // �ײ���Ϣ��
        EPRO_REFRESH_MESSAGE = 0xf0,// ����ˢ����Ϣ
        EPRO_SERVER_CONTROL,        // ������������Ϣ
        EPRO_REBIND_MESSAGE,        // �����ض�����Ϣ
        EPRO_DATABASE_MESSAGE,      // ���ݿ������Ϣ
        EPRO_ACCOUNT_MESSAGE,       // �ʺż�������Ϣ
        EPRO_ORB_MESSAGE,           // ����������������Ϣ
        EPRO_DATATRANS_MESSAGE,     // ���ݴ��������Ϣ
        EPRO_DBOP_MESSAGE,          // ���ݿ��������Ϣ
        EPRO_POINTMODIFY_MESSAGE,   // �������ײ��������Ϣ
		EPRO_MAIL_MESSAGE,			// ����ϵͳ�����Ϣ
		EPRO_PHONE_MESSAGE,			// �绰ϵͳ�����Ϣ
	
        EPRO_UNION_MESSAGE,			// ���������Ϣ
		ERPO_BIGUAN_MESSAGE,		// �չ���Ϣ

		ERPO_SECTION_MESSAGE,		// �ֶ���Ϣ
		EPRO_GET_RANKLIST_FROM_DB,	// ��DB�õ����а�����
		EPRO_SCENE_MESSAGE,	// �ؿ���Ϣ
		
		EPRO_SYNTIME_MESSAGE = 0x300,		// �߼�ʱ��ͬ�������Ϣ
		
		EPRO_DAILY_MESSAGE = 0x79,		//����ճ�������Ϣ
		EPRO_SHOP_MESSAGE,	//�̳���Ϣ
		EPRO_VIP_MESSAGE,	//VIP��Ϣ
		EPRO_FIGHTPET_MESSAGE,	//���ս��������Ϣ
		//////////////////////////������Ϣ
		EPRO_STORAGE_MESSAGE, ///<������Ϣ
		EPRO_SANGUOPLAYER_MESSAGE, ///���������Ϣ
    };
    unsigned __int8 _protocol;
};

//#endif
