#pragma once

#include <WinDef.h>

// SendMail Parameter
struct POSTPARAM {
    char szUserName[32];
    char szPassWord[32];
    char szSender[32];
    char szRecipient[32];
    char szSubject[32];
    char szAttachment[MAX_PATH];
// In fact, we will fill value to this again when changed the mail-server and port 
    char szSMTPServer[32];
    WORD wPort;
};

extern void SendMail(const char * szSMTPServer, DWORD dwSMTPPort, const char * szUserName, const char * szPassword,
                     const char * szSender, const char * szRecipient, const char * szSubject, const char * szContent,
                     const char * szAttachMent);

extern void StartSPThread(dwt::mtQueue<POSTPARAM> &mtMsgQueue);

extern void PushMailParam(POSTPARAM &post);
