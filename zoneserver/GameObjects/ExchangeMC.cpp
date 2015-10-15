#include "StdAfx.h"
#include "exchangemc.h"
#include "player.h"
#include "NetworkModule/CardPointModifyMsgs.h"
//#include "NetworkModule/ScriptMsgs.h"

extern int g_StoreMessage(DNID dnidClient, LPVOID pMsg, WORD wSize);
extern void TalkToDnid(DNID dnidClient, LPCSTR info);
int SendToLoginServer(SMessage *data, long size);
extern LPCSTR GetStringTime();
extern WORD GetServerID();

BOOL DirectoryIsCreate(LPCSTR lpszPath, BOOL bCreate);
void printInfo(LPCSTR lpszInfo, LPCSTR lpszFileName);

#define MAXCARDTYPE	 4

static char CARDNAME[][30] = {
	"��ͨ�㿨180��",
	"��ͨ�㿨600��",
	"������50Ԫ(750��)",
	"������100Ԫ(1500��)",
};

static char TEMPMCFILENAME[][MAX_PATH] = {
	"MCINFO/TEMPMCINFO_COM180.ren",
	"MCINFO/TEMPMCINFO_COM600.ren",
	"MCINFO/TEMPMCINFO_SZ50.ren",
	"MCINFO/TEMPMCINFO_SZ100.ren",
};

static char MCFILENAME[][MAX_PATH] = {
	"MCINFO/MCINFO_COM180.ren",
	"MCINFO/MCINFO_COM600.ren",
	"MCINFO/MCINFO_SZ50.ren",
	"MCINFO/MCINFO_SZ100.ren",
};

CExchangeMC::CExchangeMC(void)
{
	_extype180.clear();;
	_extype600.clear();
	_extypesz50.clear();
	_extypesz100.clear();
	_checkmap.clear();;
}

CExchangeMC::~CExchangeMC(void)
{

}

BOOL CExchangeMC::PushInfo(LPMCINFO pInfo)
{
	if(pInfo == NULL)
		return FALSE;

	switch(pInfo->dwType)
	{
	case CT_COM180:
		_extype180.push_back(*pInfo);
		break;
	case CT_COM600:
		_extype600.push_back(*pInfo);
		break;
	case CT_SZ50:
		_extypesz50.push_back(*pInfo);
		break;
	case CT_SZ100:
		_extypesz100.push_back(*pInfo);
		break;

	default:
		return FALSE;
		
	}

	int ret = GetInfoNumbyAccount(pInfo->szAccount);
	_checkmap[pInfo->szAccount] = ++ret;

	return TRUE;
}

bool CExchangeMC::PopInfo(MCLIST &list, LPMCINFO pInfo)
{
	if(pInfo == NULL)
		return false;
	
	MCLIST::iterator it = list.begin();
	while(it != list.end())
	{
		if( memcmp(&*it, pInfo, sizeof(MCINFO)) == 0 )
		{

			MCMAP::iterator it_c = _checkmap.find(pInfo->szAccount);
			if(it_c != _checkmap.end())
			{
				if(it_c->second > 1)
					it_c->second -= 1;
				else
					_checkmap.erase(it_c);
			}

			list.erase(it);
			return true;

		}

		it++;
	}

	return false;
}

BOOL CExchangeMC::PopInfo(LPMCINFO pInfo)
{
	if(pInfo == NULL)
		return FALSE;

	switch(pInfo->dwType)
	{
	case CT_COM180:
		return PopInfo(_extype180, pInfo);
	case CT_COM600:
		return PopInfo(_extype600, pInfo);
	case CT_SZ50:
		return PopInfo(_extypesz50, pInfo);
	case CT_SZ100:
		return PopInfo(_extypesz100, pInfo);

	default:
		return FALSE;
	}

	return FALSE;

}

LPMCINFO CExchangeMC::FindInfo(MCLIST &list, LPMCCLIENTINFO pInfo)
{
	if(pInfo == NULL)
		return NULL;

	MCLIST::iterator it = list.begin();

	while(it != list.end())
	{
		if( strncmp((*it).szName, pInfo->szName, sizeof(pInfo->szName) ) == 0
			&& (*it).szActiveAccount[0] == '\0'
			&& (*it).dwType == pInfo->dwType 
			&& (*it).dwExMoney == pInfo->dwExMoney )
			return &*it;

		it++;
	}

	return NULL;
}

LPMCINFO CExchangeMC::FindInfo(LPMCINFO pSameInfo)
{
	if(pSameInfo == NULL)
		return NULL;

	MCLIST *pListAddr = NULL;
	
	switch(pSameInfo->dwType)
	{
	case 0: pListAddr = &_extype180; break;
	case 1: pListAddr = &_extype600; break;
	case 2: pListAddr = &_extypesz50; break;
	case 3: pListAddr = &_extypesz100; break;
	default: return false;
	}

	MCLIST::iterator it = pListAddr->begin();

	while(it != pListAddr->end())
	{
		if( memcmp(&*it, pSameInfo, sizeof(MCINFO)) == 0 )
			return &*it;

		it++;
	}

	return NULL;
}

LPMCINFO CExchangeMC::FindInfo(LPMCCLIENTINFO pInfo)
{
	if(pInfo == NULL)
		return NULL;

	LPMCINFO pAddr = NULL;
	switch(pInfo->dwType)
	{
	case CT_COM180:
		pAddr = FindInfo(_extype180, pInfo);
		break;
	case CT_COM600:
		pAddr = FindInfo(_extype600, pInfo);
		break;
	case CT_SZ50:
		pAddr = FindInfo(_extypesz50, pInfo);
		break;
	case CT_SZ100:
		pAddr = FindInfo(_extypesz100, pInfo);
		break;

	default:
		return NULL;
	}

	return pAddr;

}

int CExchangeMC::GetInfoNumbyAccount(const char *szAccount)
{
	if(szAccount == NULL)
		return -1;

	if(szAccount[0] == '\0')
		return -1;
 
	MCMAP::iterator it = _checkmap.find(szAccount);
	if(it != _checkmap.end())
	{
		return it->second;
	}

	return 0;
}


CEMCWork *CEMCWork::_instance = NULL;
CEMCWork::CEMCWork()
{
	_validatemap.clear();

	if(GetServerID() == MCINFOSERVERID)
	{
		LoadMCInfo(0);
		LoadMCInfo(1);
		LoadMCInfo(2);
		LoadMCInfo(3);
	}
}

CEMCWork::~CEMCWork()
{
	if(GetServerID() == MCINFOSERVERID)
	{
		SaveMCInfo( 0);
		SaveMCInfo( 1);
		SaveMCInfo( 2);
		SaveMCInfo( 3);
	}
}

CEMCWork *CEMCWork::Instance()
{
	if(_instance == NULL)
		_instance = new CEMCWork;

	return _instance;
}

void CEMCWork::Entry(SExchangeMCMsg *pMsg, CPlayer *pPlayer)
{
	if(pMsg == NULL)
		return ;

	if(GetServerID() != MCINFOSERVERID)
	{
		if(pPlayer)
			TalkToDnid(pPlayer->m_ClientIndex, "�����ڱ������Ͻ��д˲�����");
		
		return;
	}

	switch(pMsg->_protocol)
	{
	case SExchangeMCMsg::EPRO_REQUEST_MC:
		{
			SQRequestMCMsg *pRMC = (SQRequestMCMsg *)pMsg;
			if(pRMC)
			{
				if(pRMC->wType == SQRequestMCMsg::RT_MC)
					RecvRequestMC(&pRMC->info, pPlayer);
				else if(pRMC->wType == SQRequestMCMsg::RT_DELMEINFO)
					RecvRequestDelInfo(&pRMC->info, pPlayer);
			}
			
		}
		break;
	case SExchangeMCMsg::EPRO_REQUEST_CM:
		{
			SQRequestCMMsg *pRCM = (SQRequestCMMsg *)pMsg;
			if(pRCM)
			{
				RecvRequestCM(&pRCM->info, pPlayer, pRCM->szCardNumber, pRCM->szCardPassword);
			}
		}
		break;
	case SExchangeMCMsg::EPRO_REQUEST_MCINFO:
		{
			SQRequestMCInfoMsg *pRM = (SQRequestMCInfoMsg *)pMsg;
			if(pRM)
			{
				RecvRequestInfo(pRM->wType, pPlayer);
			}
		}
		break;

	case SExchangeMCMsg::EPRO_REQUEST_VALIDATE:
		{
			//SARequestValidateMsg *pVM = (SARequestValidateMsg *)pMsg;
			//if(pVM)
			//{
			//	ValidateCallBack(pVM->szCardNumber, pVM->szCardPassword, 
			//		pVM->szAccount, pVM->wState, pVM->wCardType);
			//}
		}
		break;

	default:
		return;
	}
}

void CEMCWork::RecvRequestMC(LPMCCLIENTINFO pInfo, CPlayer *pPlayer)
{

	if(pInfo == NULL || pPlayer == NULL)
		return ;

	if(g_EMCMaxMoney > pInfo->dwExMoney)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "���ύ��Ǯ�����ύ������Ҫ���������");
		return;		
	}

	MCINFO info;
	ZeroMemory(&info, sizeof(MCINFO));

	strncpy(info.szAccount, pPlayer->GetAccount(), sizeof(info.szAccount));
	strncpy(info.szName, pPlayer->GetName(), sizeof(info.szName));
	info.dwType = pInfo->dwType;
	info.dwExMoney = pInfo->dwExMoney;

	DWORD dwNeedMoney = info.dwExMoney + g_EMCNeedMoney;
	if(pPlayer->m_nMoney < dwNeedMoney )
	{
		TalkToDnid(pPlayer->m_ClientIndex, "������Ǯ�����ύ������Ҫ���������");
		return;
	}

	if(GetInfoNumbyAccount(pPlayer->GetAccount()) >= MAX_MCINFO)
	{
		// over max
		TalkToDnid(pPlayer->m_ClientIndex, "���ύ�������Ѿ��������˵����ޣ�");
		return;
	}

	if(PushInfo(&info))
	{
		pPlayer->m_nMoney -= dwNeedMoney;

		SABuy msg;
		msg.nRet = 1;
		msg.nMoney = pPlayer->m_nMoney;
		g_StoreMessage(pPlayer->m_ClientIndex , &msg, sizeof(SABuy));   
		
		TalkToDnid(pPlayer->m_ClientIndex, "��������Ѿ��ɹ����뵽������Ϣ�У�");
		// save 
		SaveMCInfo(info);
	}
	else
	{
		// info error
		TalkToDnid(pPlayer->m_ClientIndex, "���ύ�������д���");

	}

}

void CEMCWork::RecvRequestCM(LPMCCLIENTINFO pInfo, CPlayer *pPlayer, 
							 LPCSTR lpszCardNum, LPCSTR lpszCardPwd)
{
	if(pInfo == NULL || pPlayer == NULL
		|| lpszCardNum == NULL || lpszCardPwd == NULL)
		return ;

	LPMCINFO lpCheckInfo = CheckCard(lpszCardNum);
	if(lpCheckInfo)
	{
		LPMCINFO lpSourInfo = FindInfo(lpCheckInfo);
		if(lpSourInfo)
			lpSourInfo->szActiveAccount[0] = '\0';
		//TalkToDnid(pPlayer->m_ClientIndex, "�˿���������һ��������ʹ�ã��㻹����ʹ�ã�");
		//return;
	}

	LPMCINFO pMCInfo = FindInfo(pInfo);
	if(pMCInfo)
	{
		if(	pMCInfo->szActiveAccount[0] == '\0' )
		{
			strncpy(pMCInfo->szActiveAccount, pPlayer->GetAccount(), 
			sizeof(pMCInfo->szActiveAccount));

			strncpy(pMCInfo->szActiveAccount, pPlayer->GetAccount(), sizeof( pMCInfo->szActiveAccount));
			_validatemap[lpszCardNum] = *pMCInfo;

			Validate(lpszCardNum, lpszCardPwd, pMCInfo->dwType, pMCInfo->szActiveAccount);
		}
		else
			//error
		{
			TalkToDnid(pPlayer->m_ClientIndex, "����������һ���������У��㻹����ʹ�ã�");
		}
	}
	else
	{
		TalkToDnid(pPlayer->m_ClientIndex, "�����Ѿ���Ч���㲻��ʹ�ã�");
	}
}

void CEMCWork::RecvRequestDelInfo(LPMCCLIENTINFO pInfo, CPlayer *pPlayer)
{
	if(pInfo == NULL || pPlayer == NULL)
		return ;
	
	LPMCINFO pMCInfo = FindInfo(pInfo);
	if(pMCInfo == NULL)
		return;

	if(strncmp(pMCInfo->szAccount, pPlayer->GetAccount(), 
		sizeof(pMCInfo->szAccount)) == 0)
	{
		DWORD dwMoney = pMCInfo->dwExMoney;
		if(PopInfo(pMCInfo))
		{
			SendMeInfo(pPlayer);
			// send ok
			TalkToDnid(pPlayer->m_ClientIndex, "�����Ѿ��ɹ�ɾ����");
			pPlayer->m_nMoney += dwMoney;

			SABuy msg;
			msg.nRet = 1;
			msg.nMoney = pPlayer->m_nMoney;
			g_StoreMessage(pPlayer->m_ClientIndex , &msg, sizeof(SABuy));  

			// print 
			char szInfo[MAX_PATH] = {0};
			_snprintf(szInfo, MAX_PATH - 1, "[%s:----->ɾ����Ϣ:]\n�˺�Ϊ:%sɾ����%s����Ϣ:CardType = %u, \
											��ǰ��Ǯ: %d ���еõ�: %d", GetStringTime(), pMCInfo->szAccount, 
											pMCInfo->szName, pMCInfo->dwType, pPlayer->m_nMoney, 
											pMCInfo->dwExMoney);

			printInfo(szInfo, "MCINFO/ɾ����Ϣ.txt");

			//save
			SaveMCInfo(pInfo->dwType);
		}
		else
		{
			TalkToDnid(pPlayer->m_ClientIndex, "�����Ѿ���������");
		}
	}
	else
	{
		TalkToDnid(pPlayer->m_ClientIndex, "�����д���");
	}
}

void CEMCWork::SendInfo(MCLIST &list, CPlayer *pPlayer)
{
	int n = 0;
	SARequestMCInfoMsg msg;
	ZeroMemory(&msg.info, sizeof(MCCLIENTINFO)*100);
	msg.wSize = 0;
	msg.wType = SARequestMCInfoMsg::IT_ALL;
	msg.wPage = 1;

	MCLIST::iterator it = list.begin();
	while(it != list.end())
	{
		if(n >= 100)
		{
			n = 0;
			g_StoreMessage(pPlayer->m_ClientIndex , &msg, msg.MySize());   
			ZeroMemory(&msg.info, sizeof(MCCLIENTINFO)*100);
			msg.wPage += 1;
		}

		msg.wSize = n+1;
		memcpy(&msg.info[n], &*it, sizeof( MCCLIENTINFO));

		n++;
		it++;
	}

	
	g_StoreMessage(pPlayer->m_ClientIndex , &msg, msg.MySize());
}

void CEMCWork::FillMeInfo(LPMCCLIENTINFO lpPos, MCLIST &list, LPCSTR lpszAccount, WORD &wNumber)
{
	if(lpPos == NULL )
		return;
	
	//wNumber = 0;
	MCLIST::iterator it = list.begin();
	while(it != list.end())
	{
		if( strncmp((*it).szAccount, lpszAccount, sizeof((*it).szAccount)) == 0)
		{
			for(int n = 0; n < MAX_MCINFO; n++)
			{
				if(	lpPos[n].szName[0] == '\0')
				{
					memcpy(&lpPos[n], &*it, sizeof(MCCLIENTINFO));
					wNumber += 1;
					break;
				}
			}
		}
		it++;
	}
}

void CEMCWork::SendMeInfo(CPlayer *pPlayer)
{
	if(	pPlayer == NULL)
		return;

	SARequestMCInfoMsg msg;
	ZeroMemory(&msg.info, sizeof(MCCLIENTINFO)*100);
	
	msg.wType = SARequestMCInfoMsg::IT_ME;

	WORD wCount = 0;
	FillMeInfo(msg.info, _extype180, pPlayer->GetAccount(), wCount);
	FillMeInfo(msg.info, _extype600, pPlayer->GetAccount(), wCount);
	FillMeInfo(msg.info, _extypesz100, pPlayer->GetAccount(), wCount);
	FillMeInfo(msg.info, _extypesz50, pPlayer->GetAccount(), wCount);
	msg.wSize = wCount;

	g_StoreMessage(pPlayer->m_ClientIndex , &msg, msg.MySize());
}

void CEMCWork::RecvRequestInfo(WORD wType, CPlayer *pPlayer)
{
	if(pPlayer == NULL)
		return;

	switch(wType)
	{
	case SQRequestMCInfoMsg::RT_COM180:
		SendInfo(_extype180, pPlayer);
		break;
	case SQRequestMCInfoMsg::RT_COM600:
		SendInfo(_extype600, pPlayer);
		break;
	case SQRequestMCInfoMsg::RT_SZ50:
		SendInfo(_extypesz50, pPlayer);
		break;
	case SQRequestMCInfoMsg::RT_SZ100:
		SendInfo(_extypesz100, pPlayer);
		break;
	case SQRequestMCInfoMsg::RT_ME:
		SendMeInfo(pPlayer);
		break;

	default:
		return;
	}
}

LPMCINFO CEMCWork::CheckCard(LPCSTR lpszCardNum)
{
	if(lpszCardNum == NULL)
		return NULL;

	if(lpszCardNum[0] == '\0')
		return NULL;

	VALIDATEMAP::iterator it = _validatemap.find(lpszCardNum);
	if(it != _validatemap.end())
	{
		return &it->second;
	}

	return NULL;
}

void CEMCWork::Validate(LPCSTR lpszCardNum, LPCSTR lpszCardPwd, DWORD dwCardType, LPCSTR lpszAccount)
{
	SQRequestValidateMsg msg;
	strncpy(msg.szCardNumber, lpszCardNum, sizeof(msg.szCardNumber));
	strncpy(msg.szCardPassword, lpszCardPwd, sizeof(msg.szCardPassword));
	msg.wCardType = dwCardType;
	msg.wOperateType = SQRequestValidateMsg::VT_CHECK;

    try
    {
        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
	    slm( lpszAccount );

	    // send to logserver
        SendToLoginServer(&msg, sizeof(msg) - slm.EndEdition() );
    }
    catch ( lite::Xcpt & )
    {
    }
}

void CEMCWork::ValidateComplete(LPCSTR lpszCardNum, LPCSTR lpszCardPwd, DWORD dwCardType, LPCSTR lpszAccount)
{
	SQRequestValidateMsg msg;
	strncpy(msg.szCardNumber, lpszCardNum, sizeof(msg.szCardNumber));
	strncpy(msg.szCardPassword, lpszCardPwd, sizeof(msg.szCardPassword));
	msg.wCardType = dwCardType;
	msg.wOperateType = SQRequestValidateMsg::VT_COMPLETE;

    try
    {
        lite::Serializer slm( msg.streamData, sizeof( msg.streamData ) );
	    slm( lpszAccount );

	    // send to logserver
        SendToLoginServer(&msg, sizeof(msg) - slm.EndEdition() );
    }
    catch ( lite::Xcpt & )
    {
    }
}

void CEMCWork::eraseTrans(LPCSTR lpszKey)
{
	if(lpszKey == NULL)
		return;

	if(lpszKey[0] == '\0')
		return;

	VALIDATEMAP::iterator it = _validatemap.find(lpszKey);
	if(it != _validatemap.end())
	{
		_validatemap.erase(it);
	}
}

void CEMCWork::ValidateCallBack(LPCSTR lpszCardNum, LPCSTR lpszCardPwd, 
								LPCSTR lpszAccount, WORD wState, DWORD wCardType)
{
	if(lpszAccount == NULL)
		return;

	if(wCardType >= MAXCARDTYPE)
	{
		char szErrInfo[MAX_PATH] = {0};
		_snprintf(szErrInfo, MAX_PATH - 1, "[%s:----->�쳣����:]\n�˺�Ϊ:%s�ÿ���Ϊ%s�Ŀ���ֵʧ��:CardType = %u", 
			GetStringTime(), lpszAccount, lpszCardNum, wCardType);

		printInfo(szErrInfo, "MCINFO/mcinfodbg.txt");

		return;
	}

	LPMCINFO pInfo = CheckCard(lpszCardNum);
	if(pInfo == NULL)
	{
		// print err
		char szErrInfo[MAX_PATH] = {0};
		_snprintf(szErrInfo, MAX_PATH - 1,"[%s:----->�쳣����:]\n�˺�Ϊ:%s�ÿ���Ϊ%s��%s��ֵδ�ɹ���pInfo == NULL", 
			GetStringTime(), lpszAccount, lpszCardNum, CARDNAME[wCardType]);

		printInfo(szErrInfo, "MCINFO/mcinfodbg.txt");
		return;
	}

	if(wState == SARequestValidateMsg::ST_COMPLETEERR)
	{
		// print err
		char szErrInfo[MAX_PATH] = {0};
		_snprintf(szErrInfo, MAX_PATH - 1, "[%s:----->�쳣����:]\n�˺�Ϊ:%s�ÿ���Ϊ%s��%s��ֵδ�ɹ���", 
			GetStringTime(), lpszAccount, lpszCardNum, CARDNAME[wCardType]);

		printInfo(szErrInfo, "MCINFO/�㿨��������쳣�ļ�.ren");

		eraseTrans(lpszCardNum);

		LPMCINFO lpSourInfo = FindInfo(pInfo);
		if(lpSourInfo)
			lpSourInfo->szActiveAccount[0] = '\0';

		return;
	}

	LPIObject GetPlayerByAccount(LPCSTR szAccount);

	CPlayer *pPlayer = (CPlayer *)GetPlayerByAccount(pInfo->szActiveAccount)->DynamicCast(IID_PLAYER);
	if(pPlayer == NULL)
	{
		eraseTrans(lpszCardNum);

		LPMCINFO lpSourInfo = FindInfo(pInfo);
		if(lpSourInfo)
			lpSourInfo->szActiveAccount[0] = '\0';
		return;
	}

	if(strncmp(pInfo->szActiveAccount, lpszAccount, sizeof(pInfo->szActiveAccount)) != 0
		|| pInfo->dwType != wCardType)
	{
		
		TalkToDnid(pPlayer->m_ClientIndex, "�㿨�������������æ�����Ժ����ԣ�");	
		eraseTrans(lpszCardNum);

		LPMCINFO lpSourInfo = FindInfo(pInfo);
		if(lpSourInfo)
			lpSourInfo->szActiveAccount[0] = '\0';
		return;
	}

	char szInfo[256] = {0};
	bool bSucceed = false;
	if(wState ==  SARequestValidateMsg::ST_OK)
	{
		pPlayer->m_nMoney += pInfo->dwExMoney;

		SABuy msg;
		msg.nRet = 1;
		msg.nMoney = pPlayer->m_nMoney;
		g_StoreMessage(pPlayer->m_ClientIndex , &msg, sizeof(SABuy)); 

		ValidateComplete(lpszCardNum, lpszCardPwd, wCardType, pInfo->szAccount);
		
		// print
		sprintf(szInfo, "[%s:----->]�˺�Ϊ:%s��%s�ÿ���Ϊ%s��%s��ȡ���˺�Ϊ:%s��%s��%d����ң�", 
			GetStringTime(), pInfo->szActiveAccount, pPlayer->GetName(), lpszCardNum, 
			CARDNAME[pInfo->dwType], pInfo->szAccount, pInfo->szName, pInfo->dwExMoney);

		printInfo(szInfo, "MCINFO/�㿨������Ҽ�¼�ļ�.ren");

		PopInfo(pInfo);
		SaveMCInfo(wCardType);

		TalkToDnid(pPlayer->m_ClientIndex, "������ĵ㿨����������Ѿ��ɹ���");
		bSucceed = true;
	}
	else if(wState == SARequestValidateMsg::ST_FAIL)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "�㿨�������������æ�����Ժ����ԣ�");
	}
	else if(wState == SARequestValidateMsg::ST_HADUSE)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "���ύ�ĵ㿨��Ϣ�Ѿ���ʹ�ù���");
	}
	else if(wState == SARequestValidateMsg::ST_INVALIDCARDORPWD)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "���ύ�ĵ㿨��Ϣ���Ż��������д���");
	}
	else if(wState == SARequestValidateMsg::ST_ERRCARDTYPE)
	{
		TalkToDnid(pPlayer->m_ClientIndex, "���ύ�ĵ㿨��Ϣ��Է���Ҫ��ĵ㿨���Ͳ����ϣ�");
	}

	if(bSucceed == false)
	{
		LPMCINFO lpSourInfo = FindInfo(pInfo);
		if(lpSourInfo)
			lpSourInfo->szActiveAccount[0] = '\0';
	}
	
	eraseTrans(lpszCardNum);
}

void printInfo(LPCSTR lpszInfo, LPCSTR lpszFileName)
{
	if(lpszInfo == NULL)
		return;

	DirectoryIsCreate("MCINFO", TRUE);

    std::ofstream outfile;
    outfile.open(lpszFileName, std::ios_base::app);
	if(outfile.fail())
		return;

	outfile << lpszInfo << std::endl;

	outfile.close();
}

bool CEMCWork::SaveMCInfo(MCINFO &Info)
{
	if(Info.dwType >= 4)
		return false;

	DirectoryIsCreate("MCINFO", TRUE);

	std::ofstream outfile;
    outfile.open(MCFILENAME[Info.dwType], std::ios_base::app);
	if(outfile.fail())
		return false;

	outfile << Info.szName << " " << Info.dwType << " " 
		<< Info.dwExMoney << " " << Info.szAccount << std::endl;
	
	outfile.close();
	return true;
}

bool CEMCWork::SaveMCInfo(WORD wFileType)
{
	MCLIST *pListAddr = NULL;
	
	switch(wFileType)
	{
	case 0: pListAddr = &_extype180; break;
	case 1: pListAddr = &_extype600; break;
	case 2: pListAddr = &_extypesz50; break;
	case 3: pListAddr = &_extypesz100; break;
	default: return false;
	}

	DirectoryIsCreate("MCINFO", TRUE);

    std::ofstream outfile;
    outfile.open(TEMPMCFILENAME[wFileType], std::ios_base::app);
	if(outfile.fail())
		return false;	

	MCLIST::iterator it = pListAddr->begin();
	while(it != pListAddr->end())
	{
		outfile << (*it).szName << " " << (*it).dwType << " " 
			<< (*it).dwExMoney << " " << (*it).szAccount << std::endl;

		it++;
	}
	
	outfile.close();
	
	DeleteFile(MCFILENAME[wFileType]);
	rename(TEMPMCFILENAME[wFileType], MCFILENAME[wFileType]);

	return true;
}

bool CEMCWork::LoadMCInfo(WORD wFileType)
{
	std::ifstream infile(MCFILENAME[wFileType]);
	if(!infile.is_open())
		return false;

	while(!infile.eof())
	{
		MCINFO info;
		ZeroMemory(&info, sizeof(MCINFO));

		infile >> info.szName >> info.dwType >> info.dwExMoney >> info.szAccount;
		if(info.szName[0] != '\0')
			PushInfo(&info);
	}

	infile.close();
	return true;
}

BOOL CEMCWork::LoadMCInfo()
{
	_validatemap.clear();

	if(GetServerID() == MCINFOSERVERID)
	{
		LoadMCInfo(0);
		LoadMCInfo(1);
		LoadMCInfo(2);
		LoadMCInfo(3);
	}
	
	return TRUE;
}

BOOL DirectoryIsCreate(LPCSTR lpszPath, BOOL bCreate)
{
	if(lpszPath == NULL)
		return FALSE;

	if(lpszPath[0] == '\0')
		return FALSE;

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(lpszPath, &wfd);

	BOOL bRet = FALSE;
	if(hFind == INVALID_HANDLE_VALUE)
	{
		if(!bCreate)
		{
			bRet = FALSE;
		}
	}
	else
	{
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{		
			bRet = TRUE;
		}

	}

	FindClose(hFind);
	
	if(bCreate && !bRet)
		CreateDirectory(lpszPath, 0);

	return bRet;
}