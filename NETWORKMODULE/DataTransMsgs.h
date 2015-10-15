#pragma once
#include "NetModule.h"

// �˺����ݿ��������
// ��½ʱ�����˺� [EPRO_CHECK_ACCOUNT] �����ڽ����˺ſ�ʼ�Ƿ�ʱ����Ϊ�ͻ���������Ч�����Լ��ɹ���ͬʱ�˺ſ�ʼ�Ƿѣ�ʵ�����Ǽ�¼��½ʱ�䣬�ȵ��˳���ʱ������ݿ�۵㣩
// �Ƿѹ����У���ʱ���Է���Ϣ [EPRO_REFRESH_CARD_POINT] ���˺ŷ�������������ˢ�£�Ҳ���Ǹ�����Ϸʱ����п۵㣬ͬʱ��Ϸʱ�����㣬������˺Ż�δ�����Ƿѣ����Զ���ʼ�Ƿ�
// �Ƿѹ����У��˺ŷ�������ʱ����ѯ����Ϸ������ [ ] ĳ���˺��Ƿ���Ȼ���ڣ����ѯ��ʧ�ܣ���Ϸ���������ϻ���˺Ų���������Ϸ�����������Ƿ�ֹͣ
// ����˳�ʱ����Ϸ������֪ͨ�˺ŷ����� [EPRO_ACCOUNT_LOGOUT] �˺��˳��� �Ƿ�ֹͣ

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// �˺ŷ����������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SDataTransMsg, SMessage, SMessage::EPRO_DATATRANS_MESSAGE)
//{{AFX
EPRO_SRESET,
EPRO_SCRIPT_BULLETIN,
EPRO_TONGONE
//}}AFX
END_MSG_MAP()


// ���÷������Ͻű���
DECLARE_MSG(_SResetScriptMsg, SDataTransMsg, SDataTransMsg::EPRO_SRESET)
struct SResetScriptMsg : _SResetScriptMsg { BYTE flag; };

// �ű��Ĺ�����Ϣ
DECLARE_MSG(SScriptBulletinMsg, SDataTransMsg, SDataTransMsg::EPRO_SCRIPT_BULLETIN)
#define MAX_SSYSCALL_LEN 500
struct SAScriptBulletinMsg  : public SScriptBulletinMsg
{
    char cChatData[MAX_SSYSCALL_LEN];
    WORD GetMySize()
    {
        _GetSSize(SAScriptBulletinMsg,cChatData);
    }

};

// ͨ���һ�ﵽ����������
DECLARE_MSG(STongOneMsg, SDataTransMsg, SDataTransMsg::EPRO_TONGONE)
struct SQTongOneMsg : public STongOneMsg
{
	char szTongName[CONST_USERNAME];
	DWORD dwFactionTitleID;
};