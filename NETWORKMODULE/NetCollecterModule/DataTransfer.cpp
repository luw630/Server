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
    //case LinkStatus_UnConnected:    // 无效的连接状态
    //    __asm nop;
    //    break;
    //case LinkStatus_Waiting_Login:  // 连接后等待认证消息的状态
    //    __asm nop;
	   // break;
    //case LinkStatus_Login:          // 连接后等待认证结果的状态（因为有可能需要提交认证数据到其他地方）
    //    __asm nop;
	   // break;
    //case LinkStatus_Connected:      // 连接后认证通过，可以正常使用的状态
    //    __asm nop;
	   // break;
    //case LinkStatus_Disconnected:   // 连接已断开的状态
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

