#pragma once

#include "NetModule.h"

// �ͻ��˲˵�������Ϣ
//=============================================================================================
DECLARE_MSG(SMenuBaseMsg, SMessage, SMessage::EPRO_MENU_MESSAGE)
struct SQMenuMsg : public SMenuBaseMsg
{
    enum EMenuItem
    {
        EMI_EXCHANGE,
        EMI_CREATETEAM,
        EMI_JOINTEAM,
        EMI_INVITEJOINTEAM,
        EMI_WHISPER,
        EMI_JOINFACTION,
        EMI_OTHEREQUIPMENT,
		EMI_JOKINGWITHOTHER,
		EMI_WIFE_REMIT,				// ���޴���
        EMI_LAKELOG,				// ��������־���
        EMI_BOSSINFO,				// BOSS ��Ϣ
		EMI_ASSOCIATION,			// ����
        EMI_UNIONRECVEXP,			// ��ȡ���徭��
        EMI_OTHEREQUIPMENTSID,		// ͨ��Sid�鿴���װ��
        EMI_DOCTORADDBUFF,			// ҽ��BUFF
        EMI_XYBAPJIAN,				// ���屦��
        EMI_TASKINF,				// ����
        EMI_ASK_ADDDOCTOR,          // ѯ���Ƿ�Ը�⸽��ҽ��BUFF
        EMI_AGREE_ADD_DOCTORBUFF,   // ͬ�⸽��ҽ��BUFF
        EMI_REFUSE_ADD_DOCTORBUFF,  // �ܾ�����ҽ��BUFF
        EMI_NEWS,                   // �°����
        EMI_REQUESTDATA,            // ������չһ��������������������ݵĹ�������
        EMI_FLOWER,                 // �ͻ�
        EMI_WULINRENMING,           // ��������
        EMI_WULINJIEPING,           // ���ֽ�Ƹ
        EMI_CLICKOTHER,             // ���ĳ��
        EMI_TRACKINGCANCEL,         // ȡ��׷��ĳ��
        EMI_STANDINGSQUEST,         // ս��ͳ���������
    };

    BYTE byMenuItem;    // ѡ��Ĳ˵���
    DWORD dwDestGID;    // ���Ŀ���GID
};
//---------------------------------------------------------------------------------------------

struct SQClickPlayerMsg:
    public SQMenuMsg
{
    BYTE type;  // 0 ��ʾ��һ������1��ʾȷ������
};

struct SQJokingWitheOtherMsg :
	public SQMenuMsg
{
	WORD wItemIdx;
};

struct SQDoctorAddBuffMsg :
    public SQMenuMsg
{
    DWORD dwValue;  // 0x000000FF:DEF, 0x0000FF00:DAM, 0x00FF0000:POW, 0xFF000000:AGI, �������
};

struct SQInviteJoinTeamMsg : public SQMenuMsg
{
	DWORD dwDestGID;		// Ŀ��ID
};

struct SQPresentFlowerMsg :
    public SQMenuMsg
{
    char szDestName[ CONST_USERNAME ];
};