#pragma once

//////////////////////////////////////////////////////////////////////
struct lua_State;


// 这个宏只是用于注册一些只需要直接调用的函数
#define MAKE_LFUNC(regname, ptrname)     int regname(lua_State *L) { \
    ptrname; return 0; }

// 怪物相关的脚本函数
class CSMonsterFunc
{
public:
    static int L_getmonsterlistid(lua_State *L);
    static int L_getdynmonstergid(lua_State *L);
    static int L_delmonster(lua_State *L);
    static int L_createmonsterg(lua_State *L);
    static int L_getmonstercurpos( lua_State *L );

public:
    static void CreateShadow(void);
};

//----------------------------------------------------------
// NPC相关的脚本函数
class CSNPCFunc
{
public:   
    static int L_getnpclistid(lua_State *L);
    static int L_addtaskfornpc(lua_State *L);
    static int L_deltaskfornpc(lua_State *L);
    static int L_gettaskdatafromnpc(lua_State *L);
    static int L_checktaskfromnpc(lua_State *L);

    static int L_getdynnpcgid(lua_State *L);
    static int L_delnpc(lua_State *L);
    static int L_createnpcg(lua_State *L);

public:
    static void CreateShadow(void);

};

//-----------------------------------------------------------
// 玩家相关的脚本函数
class CSPlayerFunc
{
public:
    static int L_damagetovenation(lua_State *L);
    static int L_getplayervenationstate(lua_State *L);
    static int L_changeskill(lua_State *L);
    static int L_getrecvmoney(lua_State *L);

    static int L_getbackitemsinfo(lua_State *L);
    static int L_checkrecvitem(lua_State *L);
    static int L_getsoneitemnum(lua_State *L);
    static int L_delallrecvitems(lua_State *L);
    static int L_restoreallrecvitems(lua_State *L);
    static int L_clearallsitems(lua_State *L);

    static int L_getplayercurpos(lua_State *L);
    static int L_getregionretpos( lua_State *L);

	static int L_setbuynpc(lua_State *L);

    static int L_ismonstermutate(lua_State *L);
    static int L_setmutate(lua_State *L);
    static int L_getmutateid(lua_State *L);

    static int L_sendhelpinfo(lua_State *L);

    static int L_addmultipletime(lua_State *L);
    static int L_inviteteamtohome(lua_State *L);

    static int L_setspouse(lua_State *L);
    static int L_delspouse(lua_State *L);

    static int L_getcurmoney(lua_State *L);

    // gaminghouse
    static int L_entergaminghouse(lua_State *L);
    static int L_getroomid(lua_State *L);
    static int L_getfrienddegree(lua_State *L);
    static int L_getmarrydegree(lua_State *L);
    static int L_setmarrydate(lua_State *L);
    static int L_getmarrydate(lua_State *L);

    // 设置武功熟练度 
    static int L_addskillproficiency(lua_State *L);

    static int L_checkroletasks(lua_State *L);
    static int L_checkroletaskn(lua_State *L);
    static int L_addroletaskelements(lua_State *L);
    static int L_addroletaskelementn(lua_State *L);
    static int L_setroletaskelements(lua_State *L);
    static int L_setroletaskelementn(lua_State *L);
    static int L_getroletaskelements(lua_State *L);
    static int L_getroletaskelementn(lua_State *L);
	static int L_cleantask(lua_State *L);
	
	// 侠义道3任务
	static int L_settaskflag(lua_State *L);
	static int L_gettaskstatus(lua_State *L);
	static int L_settaskdata(lua_State *L);
	static int L_syntaskstatus(lua_State *L);
	static int L_canadditems(lua_State *L);
	static int L_istaskflagcomplete(lua_State *L);
	static int L_DelTaskMap(lua_State *L);
	static int L_delroletask(lua_State *L);
	static int L_ClearPlayerInfo(lua_State *L);
	static int L_settaskinfo(lua_State *L);
	static int L_setmenuinfo(lua_State *L);
	static int L_OnPlayerCycTask(lua_State *L);
	static int L_getrdtasksNum(lua_State *L);
	static int L_GetRdNpcNum(lua_State *L);
	static int L_GetCurrentNpcTaskID(lua_State *L);
	static int L_InsertPlayerRdTaskInfo(lua_State *L);
	static int L_OnXKLProcess(lua_State *L);
	static int L_AddXKLUseNum(lua_State * L);
	static int L_GetXKLUseNum(lua_State * L);
	static int L_OnHandleCycTask(lua_State * L);
	static int L_GetHeadIDByStr(lua_State * L);
	static int L_PrintXKLLOG(lua_State * L);
	static int L_SendRoleTaskInfo(lua_State *L);
	static int L_OnUpdateTask(lua_State *L);
	static int L_settaskstatus(lua_State *L);//用于直接设置任务
    // 帮派相关
    static int L_getfactionname(lua_State *L);
    static int L_isfactionmaster(lua_State *L);
	static int L_setfactionsystitleid(lua_State *L);
	static int L_getelapsedtimesFac(lua_State *L);

	// 组队打怪相关
	static int L_sendteaminfotoaround(lua_State *L);
	static int L_putaroundteamto(lua_State *L);
	static int L_putallplayerA(lua_State *L);
	static int L_getregionbackpos(lua_State *L);
	static int L_addsubteamtask(lua_State *L);

	// 玩家洗点
	static int L_addpoint(lua_State *L);
	static int L_subpoint(lua_State *L);
	static int L_getpoint(lua_State *L);
	static int L_getcurpoint(lua_State *L);

	// 元宝的操作
	static int L_getmoneypoint(lua_State *L);
	static int L_notifymoneypoint(lua_State *L);
	static int L_settelergyproficM(lua_State *L);

	// 元宝操作(免费版本)
	static int L_getxydpoint(lua_State *L);
	static int L_getgiftcoupon(lua_State *L);
	static int L_setgiftcoupon(lua_State *L);

	// 侠义币换点卡
	static int L_getmymcinfo(lua_State *L);

	// 更新赠宝
	static int L_updateGiftCoupon(lua_State *L);

public:
    static void CreateShadow(void);

    // 门派相关
    static int L_setwulinchief(lua_State *L);

public:
	static int L_GetPlayerMapInfo(lua_State *L);
};

//-----------------------------------------------------------
// 其他相关的脚本函数

class CSOtherFunc
{
public:
    static int L_makenewtrap(lua_State *L);
    static int L_settrap(lua_State *L);
    static int L_gettrapnum(lua_State *L);

    static int L_seteffectmsg(lua_State *L);

    static int L_setmail(lua_State *L);
    static int L_putallplayer(lua_State *L);

    static int L_setregionexper(lua_State *L);
    static int L_getregionexper(lua_State *L);

    static int L_getlocalserverid(lua_State *L);
    static int L_getserverid(lua_State *L);
    // spouse
    static int L_pushspouse(lua_State *L);
    static int L_setgaminginfo(lua_State *L);

    static int L_tipforbottom(lua_State *L);

    // 新任务
    static int L_loadtaskdesc(lua_State *L);
    static int L_loadtaskdescA(lua_State *L);

    // 帮战相关
    static int L_sendmessagetoallfactionmember(lua_State *L);
	static int L_setregionpkx(lua_State *L);	// 设置场景奖惩关系等

	// 场景税率相关
	static int L_emptyrratemap(lua_State *L);
	static int L_setregionrate(lua_State *L);
	static int L_emptyrmoneymap(lua_State *L);
	static int L_setcountregion(lua_State *L);
	static int L_setcommonrrate(lua_State *L);
	static int L_getregionrmoney(lua_State *L);
	static int L_zerormoneymap(lua_State *L);

public:
    static void CreateShadow(void);
};

// error codes for NotifyMaster
#define ERROR_NONE		0
#define ERROR_PLAYER	1
#define ERROR_REGION	2

#define SEND_OK 3

#define MASTER_LEVEL 50
#define FIND_NUMBER 10

void WriteSpouseToFile();
//string GetSpouseName(DWORD dwID);


// 场景税率相关
static float s_fRegionRate = 0.003f;
//////////////////////////
// 任务 
static unsigned int ROLETASK_NUMBER		= 100;
static unsigned int OFFLINETASK_NUMBER	= 5;
static unsigned int MAX_TASKELEMENT		= 10;

enum RTASK_STATE { RT_NORMAL, RT_OFFLINE = 10000 };

// *************************** 侠义道三任务系统 ***************************
typedef int RTRESULT;
#define R_FAILED		-1
#define R_TOK			0
#define R_TASKFULL		1
#define R_TASKIN		2


class CRoleTask
{
public:
	friend class CRoleTaskManager;
	typedef std::list<STaskFlag> TELIST;

public:
	WORD		m_Index;			// 任务ID
    QWORD		m_CreateTime;		// 任务的创建时间
	WORD		m_timelimit;		//任务时间限制
	RTRESULT PushFlag(STaskFlag &flag);
	RTRESULT DelFlg(STaskFlag *pFlag);
	STaskFlag* GetFlag(DWORD index);

	size_t SizeFlags(void) const
	{
		return m_flagList.size();
	}

    TELIST		m_flagList;
};

class CRoleTaskManager 
{
public:
    typedef std::list<CRoleTask> RTLIST;

public:
    void SendRoleTaskInfo(struct SRoleTaskBaseMsg *pMsg, long Size, class CPlayer *pPlayer);
    void SendAllRoleTaskInfo(class CPlayer *pPlayer);

public:
	void ClearAllRoleTask();
    RTRESULT AddRoleTask(CRoleTask *pTask);
    RTRESULT DelRoleTask(CRoleTask *pTask);
	int IsTaskFinished(CRoleTask *pTask);
    CRoleTask *GetRoleTask(WORD Key);
	RTLIST    *GetRoleTask();
	WORD GetTaskCount()
	{
		return m_taskList.size();
	}
	
public:
    RTLIST  m_taskList;			// 总的任务列表
};

// *************************** 侠义道三任务系统 ***************************

///////////////////////
// 对应的任务描述表
typedef struct _STaskDesc 
{
    std::string Desc1;
    std::string Desc2;
    std::string Desc3;
    std::string Desc4; // 保留
}TASKDESC, *LPTASKDESC;

class CRoleTaskDesc 
{
protected:
    CRoleTaskDesc(void) {};

public:
    enum DescType { DT_TASK, DT_ELEMENT };

public:
    ~CRoleTaskDesc(void);

    void PushTaskDesc(WORD wTypeKey, LPTASKDESC lpDesc, WORD wDescType );
    LPTASKDESC GetTaskDesc(WORD wTypeKey, WORD wDescType);
    
    static CRoleTaskDesc *Instance(void);

private:
    typedef std::map<WORD, TASKDESC> TASKDESCMAP;
    TASKDESCMAP _descMap;
    TASKDESCMAP _taskdescMap;

    static CRoleTaskDesc *_pInstance;
};