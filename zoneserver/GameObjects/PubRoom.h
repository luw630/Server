#pragma once 


// Base and interface
class CRoomContainer;
class CRoom
{
public:
    CRoom(void);
    virtual ~CRoom(void);
    virtual void Update(CRoomContainer *pPubFrame) = 0;
    virtual void Release(void) = 0;
    
    DWORD   GetMasterID(void) const;
    void    SetMasterID(DWORD dwID);

    void    SetRoomID(DWORD dwRoomID);
    DWORD   GetRoomID(void) const;

private:
    DWORD _dwMasterID;
    DWORD _dwRoomID;
};


class CRoomContainer
{
public:
    virtual void Attach(CRoom *pSubRoom);
    virtual void Detach(CRoom *pSubRoom);
    virtual void Notify(WORD wType = 0);

    virtual CRoom *Locate(DWORD dwRoomID);

    size_t  MapSize(void) const { return _subroomList.size(); }

private:
    typedef std::map<DWORD, CRoom*> ROOMMAP; 
    ROOMMAP _subroomList;
};

// dispatch every container's message
#define _theRoomContainerCenter (RoomContainerCenter::Instance())
class RoomContainerCenter
{
protected:
    RoomContainerCenter(void);

public:
    ~RoomContainerCenter(void);
    void DispatchContainerMsg(struct SSportsBaseMsg *pMsg, class CPlayer *pPlayer);

    static RoomContainerCenter *Instance(void);
private:
    static RoomContainerCenter *_pRoomContainerCenter;
};