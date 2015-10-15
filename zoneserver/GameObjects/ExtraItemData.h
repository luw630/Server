#pragma once


#define MAX_EXITEMDATA 5
#define MAX_EQUIPNUMBER 8

typedef struct _tagEXTRADATA{
    BYTE byType;
    BYTE byNum;
}EXTRADATA, *LPEXTRADATA;

enum DATA_TYPE { DT_DAM = 1, DT_POW, DT_DEF, DT_AGI, DT_LUA, DT_AMU,  
                 DT_IN, DT_MHP, DT_MMP, DT_MSP, DT_RHP, DT_RMP, DT_RSP };
struct EXTRA_ITEMDATA {
    WORD wDAMAD;        // 加攻击力
    WORD wPOWAD;        // 加内功攻击力
    WORD wDEFAD;        // 加防御力
    WORD wAGIAD;        // 加灵敏度
    WORD wAmuckAD;      // 加杀气
    WORD wLUAD;         // 加气运
    WORD wRHPAD;        // 加生命自动恢复速度
    WORD wRSPAD;        // 加体力自动恢复速度
    WORD wRMPAD;        // 加内力自动恢复速度

    WORD wSkillAD;      // 加技能
    WORD wPointAD;      // 加能力点
    WORD wLevelAD;      // 加等级

    WORD wHPAD;         // 增加生命最大值
    WORD wSPAD;         // 增加体力最大值
    WORD wMPAD;         // 增加内力最大值

    WORD wHPR;          // 恢复生命百分比
    WORD wSPR;          // 恢复体力百分比
    WORD wMPR;          // 恢复内力百分比
   
    WORD wIN;           // 悟性
    WORD wClear;        // 消除异常状态
    
    WORD wValidTime;    // 增加效果的持续时间，如果这个时间是0的话，说明是永久性的增加效果

};

class CExtraItemData
{
public:
    CExtraItemData(void) {  
       ZeroData();
    }
    ~CExtraItemData(void);

    void ZeroData(void);
    void UpdateItemData(void);
    bool AddExtraData(EXTRADATA *pData);
    EXTRA_ITEMDATA *GetItemData(void) { return &m_stItemData; }
    EXTRADATA *GetExtraItemData(void)  { return &m_stExtraData[0]; }
    WORD GetExtraDataNum(void) const { return m_wCount; }

private:
    EXTRADATA m_stExtraData[MAX_EXITEMDATA];
    EXTRA_ITEMDATA m_stItemData;
    WORD m_wCount;
};


#include "common.h"
// alone item
#include "orbframework/orbframework.h"

struct ALONEITEM {
    ALONEITEM()
    {
        dwID = 0;
        dwRate = 0;
        bInList = false;  
    }

    DWORD dwID;
    DWORD dwRate;
    bool  bInList;  
};


#define _theAloneItem (CAloneItem::Instance())

class CAloneItem
{
    static CAloneItem *_instance;
    CTSet<DWORD> _set;

    tForceReadMostlyOrbUser<ALONEITEM> m_OrbUser;
    tObjectService<DWORD, ALONEITEM> m_ItemInfoMap;

public:
    ~CAloneItem(void);

    bool AddItem(ALONEITEM &item);
    bool DelItem(DWORD dwKey);
    bool Change(DWORD dwKey, ALONEITEM &item);

    bool Init(void);
    bool isAloneItem(DWORD dwID) { return _set.Locate(dwID); }
    
    static CAloneItem *Instance(void);

protected:
    CAloneItem(void);
};


//CAloneItem *CAloneItem::_instance = NULL;
//CAloneItem *CAloneItem::Instance(void)
//{
//    if(!_instance)
//        _instance = new CAloneItem;
//
//    return _instance;
//}
//
//static BOOL CALLBACK s_net_operation_end_callback_aloneiteminfo(const DWORD &dwKey,
//                                                               ALONEITEM &itme, 
//                                                               CAloneItem *pAItem)
//{
//
//
//    return TRUE;
//}
//
//bool CAloneItem::Init(void)
//{
//    if(_set.Size())
//    {
//        std::set<DWORD>::iterator it = _set.it_begin;
//        for(; it != _set.it_end; )
//        {
//            ALONEITEM item;
//            item.dwID = *it;
//            item.bInList = true;
//            AddItem(item);
//            it++;
//        }
//
//        return true;
//    }
//
//    return false;
//}
//
//CAloneItem::CAloneItem()
//{
//    extern char ORBCADDR[256];
//
//    m_OrbUser.SetNetOpEndCallback((tForceReadMostlyOrbUser<ALONEITEM>::NET_OPEND_CALLBACK)
//        s_net_operation_end_callback_aloneiteminfo, (LPARAM)this);
//
//    if (m_OrbUser.ConnectToServer(ORBCADDR, 22345))
//        m_OrbUser.RequestRefreshAll();
//
//    Init();
//}
//
//CAloneItem::~CAloneItem()
//{
//    if(_instance)
//        delete _instance;
//}
//
//bool CAloneItem::AddItem(ALONEITEM &item)
//{
//    if(item.dwID == 0)
//        return false;
//
//    if (m_ItemInfoMap.LocateObject(item.dwID) != NULL)
//        return false;
//
//    if (!m_OrbUser.RequestCreateObject((LPCSTR)item.dwID, item))
//        return false;
//
//    return true;
//}
//
//bool CAloneItem::DelItem(DWORD dwKey)
//{
//    if (!m_OrbUser.RequestDeleteObject((LPCSTR)dwKey))
//        return false;
//
//    return true;
//}
//
//bool CAloneItem::Change(DWORD dwKey, ALONEITEM &item)
//{
//    if (m_ItemInfoMap.LocateObject(item.dwID) == NULL)
//        return false;
//
//    if(!DelItem(dwKey))
//        return false;
//
//    return AddItem(item);
//}

//#define F_MEMBER 4
//#define BASE_SUBFNUMBER 2
//
//enum FFrameLevel { FF_CHIEF = 1, FF_ZHANG, FF_TANG, FF_FUFA, FF_XIANG };
//
//
//class CFactionFrame;
//
//class CSubFrame
//{
//public:
//    CSubFrame(void);
//    virtual ~CSubFrame(void);
//
//    virtual void Update(CFactionFrame *pPubFrame);
//    virtual bool Add(ustring name);
//    virtual bool Del(ustring name);
//    
//    ustring GetFrameName(void) const { return m_uFrameName; }
//    void SetFrameName(ustring name) { m_uFrameName = name; }
//
//private:
//    CTSet<ustring> _set;
//
//    ustring m_uFrameName;
//    BYTE    m_byFrameLevel;
//    DWORD   m_FrameMoney;
//};
//
//class CFactionFrame 
//{
//public:
//    CFactionFrame(void);
//    virtual ~CFactionFrame(void);
//
//    virtual void Notify(void);
//    virtual void Attach(CSubFrame *pSubFrame);
//    virtual void Detach(CSubFrame *pSubFrame);
//    
//    bool Detach(ustring FrameName);
//    void GetSubFrame(CSubFrame &subFrame);
//
//    void AddSubFrameNum(BYTE byNumber) { m_bySubFrameNum = byNumber; }
//    WORD GetSubFrameNum(void) const { return m_bySubFrameNum; }
//
//private:
//    typedef std::map<ustring, CSubFrame> SUBLIST;
//    SUBLIST  _list;
//
//    BYTE m_bySubFrameNum;
//    CTSet<ustring> _set;
//};
//
//
//class CFactionCenter
//{
//public:
//    void RecvChangeInfoMsg(struct SQFrameChangeInfoMsg &msg);
//
//    CFactionCenter *Instance(void);
//
//private:
//    static CFactionCenter *_instance;
//    std::map<ustring, CFactionFrame>  _factionList;
//};


//void CFactionFrame::Attach(CSubFrame *pSubFrame)
//{
//    if(pSubFrame)
//    {
//        _list[pSubFrame->GetFrameName()] =  *pSubFrame;
//        m_bySubFrameNum++;
//    }
//
//}
//
//bool CFactionFrame::Detach(ustring FrameName)
//{
//    if(FrameName.size() > 0)
//    {
//        std::map<ustring, CSubFrame>::iterator  it = _list.find(FrameName);        
//        if(it != _list.end())
//        {
//            _list.erase(it);
//            m_bySubFrameNum--;
//            return true;
//        }
//    }
//
//    return false;
//}
//
//void CFactionFrame::Notify()
//{
//    std::map<ustring, CSubFrame>::iterator  it = _list.begin();
//    for(; it != _list.end(); )
//    {  
//        it->second.Update(this);
//        it++;
//    }
//
//}