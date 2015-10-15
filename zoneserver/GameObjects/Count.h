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

// ͳ��Ǯ���������ݽṹ
struct _MONEYDATA {
    DWORD   dwGroundMoney;
    DWORD   dwBuyMoney;
    DWORD   dwSaleMoney;
    DWORD   dwExchangeMoney;
    DWORD   dwGetMoneyFromNpc;
    DWORD   dwSubMoneyFromNpc;

};
// ��Ҫͳ�Ƶ�������ݽṹ
struct _PLAYEREQUIPDATA {
    WORD	wEquipDAM;	    // װ���书������
    WORD	wEquipPOW;	    // װ���ڹ�������
    WORD	wEquipDEF;	    // װ��������
    WORD	wEquipAGI;	    // װ��������

};

struct _PLAYERDATA {
//    BYTE    byPoint;        // ���еĵ���
    DWORD   dwLevel; 
    DWORD   dwExper;   
    DWORD   dwEN;           // ��̫��
    DWORD   dwIN;           // ������
    DWORD   dwST;           // ��̫��
    DWORD   dwAG;           // ������
    DWORD   dwMoney;        // ���ϵ�Ǯ��
    DWORD   dwStoreMoney;   // �ֿ��Ǯ��

    SSkill  stSkill[12];

    _PLAYEREQUIPDATA    stEquipData;
};

// ͳ����
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
    // �����������������; Really = false ��ʱ�� ����ɾ�� ֻ���ж�����Ƿ�����б���
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

    //����
    _MONEYDATA  m_stMoneyData;
    _PLAYERDATA m_stPlayerData;
    // �����ҵ��б�
    std::list<std::string> m_PlayerList;

};

extern CCount g_Count;