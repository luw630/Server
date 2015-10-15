#pragma once


#include "networkmodule/MoneyPointDef.h"
#include "../pub/ConstValue.h"

struct MCINFO : public _MCCLIENTINFO
{
	char szAccount[MAX_ACCOUNT];
	char szActiveAccount[MAX_ACCOUNT];	
};
typedef MCINFO*	LPMCINFO;

#define MAX_MCINFO	10

class CExchangeMC
{

	typedef std::list<MCINFO> MCLIST;
	typedef std::map<dwt::stringkey<char [MAX_ACCOUNT]>, WORD> MCMAP;

	friend class CEMCWork;
protected:
	CExchangeMC(void);

	bool PopInfo(MCLIST &, LPMCINFO);
	LPMCINFO FindInfo(MCLIST &, LPMCCLIENTINFO);

public:
	~CExchangeMC(void);

	BOOL PushInfo(LPMCINFO pInfo);
	BOOL PopInfo(LPMCINFO pInfo);
	LPMCINFO FindInfo(LPMCCLIENTINFO pInfo);
	LPMCINFO FindInfo(LPMCINFO pSameInfo);
	int	 GetInfoNumbyAccount(const char *szAccout);

private:
	MCLIST _extype180;
	MCLIST _extype600;
	MCLIST _extypesz50;
	MCLIST _extypesz100;

	MCMAP  _checkmap;
};


class CPlayer;
struct SExchangeMCMsg;
#define _EMCWorkInstance	(CEMCWork::Instance())
class CEMCWork :  CExchangeMC
{
	typedef std::map<dwt::stringkey<char [30]>, MCINFO> VALIDATEMAP;

protected:
	CEMCWork() ;
	void RecvRequestMC(LPMCCLIENTINFO pInfo, CPlayer *pPlayer);
	void RecvRequestCM(LPMCCLIENTINFO pInfo, CPlayer *pPlayer, 
		LPCSTR lpszCardNum, LPCSTR lpszCardPwd);
	void RecvRequestInfo(WORD wType, CPlayer *pPlayer);
	void SendInfo(MCLIST &list, CPlayer *pPlayer);
	void SendMeInfo(CPlayer *pPlayer);
	void FillMeInfo(LPMCCLIENTINFO lpPos, MCLIST &list, LPCSTR lpszAccount, WORD &wNumber);
	void RecvRequestDelInfo(LPMCCLIENTINFO pInfo, CPlayer *pPlayer);

	LPMCINFO CheckCard(LPCSTR lpszCardNum);
	void Validate(LPCSTR lpszCardNum, LPCSTR lpszCardPwd, DWORD dwCardType, LPCSTR lpszAccount);
	void ValidateComplete(LPCSTR lpszCardNum, LPCSTR lpszCardPwd, DWORD dwCardType, LPCSTR lpszAccount);
	void ValidateCallBack(LPCSTR lpszCardNum, LPCSTR lpszCardPwd, LPCSTR lpszAccount, 
		WORD wState, DWORD wCardType);

	bool SaveMCInfo(WORD wFileType);
	bool SaveMCInfo(MCINFO &Info);
	bool LoadMCInfo(WORD wFileType);

	void eraseTrans(LPCSTR lpszKey);

public:
	~CEMCWork();
	void Entry(SExchangeMCMsg *pMsg, CPlayer *pPlayer);

	static CEMCWork *Instance(void);

	BOOL LoadMCInfo(void);
private:
	static CEMCWork *_instance;
	VALIDATEMAP _validatemap;
};
