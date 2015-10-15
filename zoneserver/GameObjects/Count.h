#pragma once

#include <fstream>
#include "networkmodule/playertypedef.h"

static const int  TIMECOUNT_HOURS =  60;
static const int  TIMECOUNT_HALFHOURS  = 30;
//////////////////////////////////////////////////////////////////////////////

enum TimeType {  
    GTT_YEAR, GTT_MONTH, GTT_MDAY, GTT_WDAY,  
    GTT_HOURS, GTT_MINUTES, GTT_SECONDS, GTT_YDAY 
};

// 统计钱数量的数据结构
struct _MONEYDATA {
    DWORD   dwGroundMoney;
    DWORD   dwBuyMoney;
    DWORD   dwSaleMoney;
    DWORD   dwExchangeMoney;
    DWORD   dwGetMoneyFromNpc;
    DWORD   dwSubMoneyFromNpc;

};
// 需要统计的玩家数据结构
struct _PLAYEREQUIPDATA {
    WORD	wEquipDAM;	    // 装备武功攻击力
    WORD	wEquipPOW;	    // 装备内功攻击力
    WORD	wEquipDEF;	    // 装备防御力
    WORD	wEquipAGI;	    // 装备灵敏度

};

struct _PLAYERDATA {
//    BYTE    byPoint;        // 还有的点数
    DWORD   dwLevel; 
    DWORD   dwExper;   
    DWORD   dwEN;           // 手太阳
    DWORD   dwIN;           // 手少阳
    DWORD   dwST;           // 足太阴
    DWORD   dwAG;           // 足少阴
    DWORD   dwMoney;        // 身上的钱数
    DWORD   dwStoreMoney;   // 仓库的钱数

    SSkill  stSkill[12];

    _PLAYEREQUIPDATA    stEquipData;
};

// 统计类
class CCount 
{
public:
    enum eCountType {
        CCT_MONEYDATA,
        CCT_PLAYERDATA,

        //......
        CTT_ALL
    };

    enum eMoneyCType {
        CT_GROUND,
        CT_BUY,
        CT_SALE,
        CT_EXCHANGE,
        CT_GETFROMNPC,
        CT_SUBFROMNPC
    };
   
public:
    CCount() :  m_dwCountMoneyTime(0), m_dwCountPlayerTime(0), m_wUpdateMoneyTime(1),
                    m_wUpdatePlayerTime(1) { ResetCount(CTT_ALL); };
    ~CCount() { if(m_outFile) m_outFile.close(); }

    void ResetCount(eCountType type);  
    void StartTime(eCountType countType);

    void SetUpdateTime(eCountType type, int nTime);
    int  GetUpdateTime(eCountType type);

    void SetMoneyCount(eMoneyCType type, int nNum); 
    void SetPlayerDataCount(_PLAYERDATA *data);
    void GetPlayerEquipCount(_PLAYEREQUIPDATA *data);

    void UpdateMoneyDataCount(int nTime);
    void UpdatePlayerDataCount(_PLAYERDATA *pData, const char*filename, int nTime, int nSaveType = std::ios_base::app);

    void PrintMoneyDataCount(int type = 3);
    void PrintPlayerDataCount(_PLAYERDATA *pData, const char*filename,  int nSaveType = std::ios_base::app);
    void PrintPlayerDataCount(_PLAYERDATA *pData, int type);

    void AddPlayerToList(std::string name);
    // -----------------------------------------------------------
    // 这个函数用来两个用途 Really = false 的时候 不用删除 只是判断玩家是否存在列表中
    bool DelPlayerFromList(std::string name, bool Really = true);
    // -----------------------------------------------------------
    bool SavePlayerList(const char*filename);
    bool FillPlayerList(const char*filename);

private:
    DWORD       m_dwCountMoneyTime;
    DWORD       m_dwCountPlayerTime;

    WORD        m_wUpdateMoneyTime;
    WORD        m_wUpdatePlayerTime;
    
    std::ofstream m_outFile;

    //数据
    _MONEYDATA  m_stMoneyData;
    _PLAYERDATA m_stPlayerData;
    // 标记玩家的列表
    std::list<std::string> m_PlayerList;

};

extern CCount g_Count;