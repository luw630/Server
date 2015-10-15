#include "stdafx.h"
#include "DataTransfer.h"


CDataTransfer::CDataTransfer(void)
{
    m_pNetServer = new CCollecterServer;
}

CDataTransfer::~CDataTransfer(void)
{
    m_pNetServer->Destroy();

    if( m_pNetServer )
    {
        delete  m_pNetServer;
    }
}
//////////////////////////////////////////////////////////////////
bool    CDataTransfer::InitializeNet( char *szPort )
{
	return  InitializeServer((*m_pNetServer), szPort, &CDataTransfer::OnDispatchMsg, this);
}

void    CDataTransfer::UpdateNetMsg(void)
{
    m_pNetServer->Execution();
}

DWORD   CDataTransfer::GetClientNum()
{

		return  m_pNetServer ? m_pNetServer->GetClientNum():0;

}

void    CDataTransfer::OnDispatchMsg(DNID dnidClient, LINKSTATUS enumStatus, SMessage *pMsg)
{
    //switch( enumStatus )
    //{
    //case LinkStatus_UnConnected:    // ��Ч������״̬
    //    __asm nop;
    //    break;
    //case LinkStatus_Waiting_Login:  // ���Ӻ�ȴ���֤��Ϣ��״̬
    //    __asm nop;
	   // break;
    //case LinkStatus_Login:          // ���Ӻ�ȴ���֤�����״̬����Ϊ�п�����Ҫ�ύ��֤���ݵ������ط���
    //    __asm nop;
	   // break;
    //case LinkStatus_Connected:      // ���Ӻ���֤ͨ������������ʹ�õ�״̬
    //    __asm nop;
	   // break;
    //case LinkStatus_Disconnected:   // �����ѶϿ���״̬
    //    __asm nop;
    //    break;
    //default:
    //    break;
    //}

    if( pMsg==NULL )
    {
        return  ;
    }
    if( pMsg->_protocol!=SMessage::EPRO_COLLECT_MESSAGE )
    {
        return  ;
    }

    SCollectBaseMsg *pCllctMsg = (SCollectBaseMsg*)pMsg;

    switch( pCllctMsg->_protocol )
    {
    case SCollectBaseMsg::EPRO_COLLECT_LOGIN:
        break;
    case SCollectBaseMsg::EPRO_COLLECT_TRANSFER:
        SQCollectTransferMsg *pRequestMsg = (SQCollectTransferMsg*)pMsg;
        OnRecvRequestData( pRequestMsg->wDataID, dnidClient );
        break;
    }
}

void    CDataTransfer::DestroyNet(void)
{
    m_pNetServer->Destroy();
}

BOOL    CDataTransfer::SendMessage(DNID dnidClient, const void *pvBuf, WORD wSize)
{
    return  m_pNetServer->SendMessage( dnidClient, pvBuf, wSize );
}

void    CDataTransfer::OnRecvRequestData( int iDataType, DNID dnid )
{
    switch( iDataType )
    {
    case DATAID_TOTLEMONEY:
        break;

    case DATAID_TOTLEMONEY_OT:
        break;
    }
}

