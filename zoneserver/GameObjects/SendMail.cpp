#include "StdAfx.h"
#include <WinDef.h>
#include <tchar.h>
#include <windows.h>
#include "SendMail.h"

#import "C:\Program Files\Common Files\System\ado\msado15.dll" no_namespace rename("EOF", "ADOEOF")
#import "cdosys.dll" no_namespace

void SendMail(const char * szSMTPServer, DWORD dwSMTPPort, const char * szUserName, const char * szPassword,
              const char * szSender, const char * szRecipient, const char * szSubject, const char * szContent,
              const char * szAttachMent)
{
    CoInitialize(0);

    try
    {
        IMessagePtr PMessage( __uuidof(Message) );
        IConfigurationPtr PConfiguration( __uuidof(Configuration) );

        PMessage->Configuration = PConfiguration;

        FieldsPtr PFields = PConfiguration->Fields;
        PFields->Item[cdoSendUsingMethod]->Value = cdoSendUsingPort;
        PFields->Item[cdoSMTPServer]->Value = szSMTPServer;
        PFields->Item[cdoSMTPServerPort]->Value = dwSMTPPort;
        PFields->Item[cdoSMTPAuthenticate]->Value = cdoBasic;
        PFields->Item[cdoSendUserName]->Value = szUserName;
        PFields->Item[cdoSendPassword]->Value = szPassword;
        PFields->Update();

        PMessage->From = szSender;
        PMessage->To = szRecipient;
        PMessage->Subject = szSubject;
        PMessage->TextBody = szContent;
        if ( szAttachMent ) PMessage->AddAttachment(szAttachMent, "", "");
        PMessage->Send();
    }

    catch (_com_error&)
    {
    }

    CoUninitialize();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// sendmail 
/////////////////////////////
dwt::mtQueue<POSTPARAM> g_PostParamQueue;

DWORD _stdcall t_SPRoutine(LPVOID lpParameter)
{
    dwt::mtQueue<POSTPARAM> *pQueue = reinterpret_cast<dwt::mtQueue<POSTPARAM> *>(lpParameter);
    if(!pQueue)
        return 0;

    POSTPARAM postParam;
    while(!pQueue->isEnd())
    {
        Sleep(10);
        if(pQueue->empty())
            continue;
        
        postParam = pQueue->front();
        pQueue->pop();

        SendMail(postParam.szSMTPServer, postParam.wPort, postParam.szUserName, 
            postParam.szPassWord, postParam.szSender, postParam.szRecipient,
            postParam.szSubject, "ScriptInfo", postParam.szAttachment);

    }

    return 0;
}

void StartSPThread(dwt::mtQueue<POSTPARAM> &mtMsgQueue)
{
    mtMsgQueue.PushExitEvent(CreateThread(0, 0, t_SPRoutine, &mtMsgQueue, 0, 0)); 
}

void PushMailParam(POSTPARAM &post)
{
    g_PostParamQueue.push(post);
}
