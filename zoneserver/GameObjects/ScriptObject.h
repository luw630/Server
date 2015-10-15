#pragma once

#include "ActiveObject.h"
#include "networkmodule/playertypedef.h"

enum ETaskType { FUSE = 3, FORGE };
enum EPLAYER_TASKTYPE { ETT_BYKILLER = 60001, ETT_KILLER };           // 追杀令的特殊任务的编号

// typedef std::list<SITEMINFO>::iterator  siit;
// typedef std::list<SITEMINFO>  SIIL;

class CScriptObject : public CActiveObject
{
public:
    CScriptObject(void);
    ~CScriptObject(void);

    DWORD OnTriggerActivated(const class CTrigger *pTrigger);

    // 消息相关
    void OnClickMenu(struct SQChoseMenuMsg *pMsg);
	void OnPlayCG(struct SQPlayCG *pMsg);
	void OnClickForgeMenu(struct SQForgeGoodsMsg *pMsg);
    void RecvSRecvItemInfo(struct SQSendSItemInfoMsg *pMsg);
	void RecvOpenScoreList (struct SQUpdateScoreListMsg *pMsg);
    void RecvCustomWndResult( struct SACustomWndMsg *pMsg );
    void RecvScriptTrigger( struct SQScriptTriggerMsg *pMsg );
    void OnDecreasePointResult( DWORD verifier, int result, int type, int remaining );


    void SynAllTask();
    void TalkBubble( LPCSTR info );
    void SendShowTaskMsg(WORD wTaskID, DWORD wState, BYTE byOperate, BYTE taskType);

    //炼制、融合相关
	BOOL SetForgeData(SPackageItem *pItem);
	BOOL SetFuseData(SPackageItem *pItem);
	void GetForgeData(int &nGoodsID, int &nDrinkNum, int &nSuccess, DWORD &nMoney);

public:
    BOOL DelItemByFlag(void);
    BOOL RestoreItemByFlag(WORD wPosX, WORD wPosY,WORD wID);
    void RestoreAllItemByFlag(void);
    bool CheckSOneItemFlag(WORD wID);
    WORD GetSOneItemNum(WORD wID);

//     void GetSIITBegin(siit &it) { it = m_RecvItemList.begin(); } 
//     void GetSIITEnd(siit &it) { it = m_RecvItemList.end(); }
//     void DelSIIT(siit &it) { m_RecvItemList.erase(it); }

    int CheckTaskPoints();
    void RecvLuaCustomMsg( struct SQALuaCustomMsg *pMsg );

protected:
    friend void PrintScriptVariables(LPCSTR name);
};
