#pragma once
#include"stdint.h"
#include<memory>
#include "Networkmodule\SanguoPlayerMsg.h"
#include"../PUB/Singleton.h"

class CBlessDataMgr;
class CBlessRandomPrize;
struct CGlobalConfig;
class BlessMoudle : public CSingleton<BlessMoudle>
{
public:
	BlessMoudle();
	~BlessMoudle();
	void DispatchBlessMsg(CBlessDataMgr *pBlessDataMgr, SBlessMsg *pMsg);
	bool Release();
private:


	void _ProcessFreeMoneyBless(CBlessDataMgr *pBlessDataMgr);
	void _ProcessSingleMoneyBless(CBlessDataMgr *pBlessDataMgr);
	void _ProcessMutipleMoneyBless(CBlessDataMgr *pBlessDataMgr);

	void _ProcessFreeDiamondBless(CBlessDataMgr *pBlessDataMgr);
	void _ProcessSingleDiamondBless(CBlessDataMgr *pBlessDataMgr);
	void _ProcessMutipleDiamondBless(CBlessDataMgr *pBlessDataMgr);

	int32_t _FirstMoneyBless();
	int32_t _FirstDiamondBless();
	const int32_t _GetHeroStarLevel(const int32_t heroID);
	void _ProcessMutipleDiamondResult(DWORD *prizeArray); //专门处理多次元宝祈福的返回结果
	std::unique_ptr<CBlessRandomPrize > m_upRandomObject;
	CGlobalConfig& globalConfig;
};											


