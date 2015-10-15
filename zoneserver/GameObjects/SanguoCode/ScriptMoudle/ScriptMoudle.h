// ----- ScriptMoudle.h -----
//
//   --  Author: wk
//   --  Date:   15/01/22
//   --  Desc:   萌将三国 脚本管理类头文件
// --------------------------------------------------------------------
// 三国的脚本相关函数都放这里,沿袭下来的脚本管理版本太多,有机会再慢慢整理
//---------------------------------------------------------------------  

/*
以前代码修改的地方:
E:\SanguoServer\trunk\ServerProjects\区域服务器\GameObjects\PlayerData.cpp
m_Property 替换为 m_FixData 三国玩家数据
*/
#pragma once
struct lua_State;
class CSSanGuoFunc
{
public:
	static int L_GetPlayerDataSG(lua_State *L);	//取三国玩家数据
	static int L_GetGoods(lua_State *L);///去玩家的点数数据
	static int L_AddGoods_SG(lua_State *L);	//增加玩家点数数据
	static int L_AddGoodsArray_SG(lua_State *L);///批量增加玩家的点数数据
	static int L_AddSkillPoint(lua_State *L);///加技能点数
	static int L_DecGoods_SG(lua_State *L);	//扣除玩家点数数据
	static int L_CheckGoods_SG(lua_State *L);	//检查玩家点数数据
	static int L_SendLoginGMRes(lua_State *L);	//GMtool登陆验证结果发往客户端

	static int L_SendCheckinData(lua_State *L);	//同步签到信息到前台
	static int L_GiveCheckinAwards(lua_State *L);	//给签到奖励
	static int L_TipMsg(lua_State *L);	//发提示信息
	static int L_SetPlayerDataSG(lua_State *L);	//修改三国玩家数据
	static int L_SynPlayerDataSG(lua_State *L);	//玩家首次登陆同步脚本数据
	static int L_SynHeroExtendsData(lua_State *L); ///玩家武将的扩展性玩法的数据同步
	static int L_SynNotificationDataSG(lua_State *L); ///<玩家登陆同步系统推送数据给玩家
	static int L_SynMissionDataSG(lua_State *L);///<同步玩家的任务数据给玩家
	static int L_UpdateMissionStateSG(lua_State* L);///<同步玩家的某一个任务的数据到客户端
	static int L_SendMsg_SG(lua_State *L);	//脚本发送消息给客户端
	static int L_ActiveLeagueOfLegend(lua_State *L);///激活将神传说的某些副本
	static int L_gm_registerNum(lua_State *L);	//gm取注册人数
	static int L_gm_stayAll(lua_State *L);	//gm取30日留存
	static int L_gm_stayOne(lua_State *L);	//gm取单日留存
	static int L_gm_online(lua_State *L);	//gm取在线人数
	static int L_gm_recharge(lua_State *L);	//gm取充值信息
	static int L_gm_leave(lua_State *L);	//gm取流失数据
	static int L_gm_pointuse(lua_State *L);	//gm取消费信息
	static int L_gm_operation(lua_State *L);	//gm取后台功能信息

	static int L_UpdateVIPLevel(lua_State *L);	//更新玩家VIP等级
	static int L_SetDessertDispachInfor(lua_State *L);///设置小额奖励的发放情况
	static int L_ComsumableUsedInfor(lua_State *L);///消耗品使用后的相关信息交互接口
	static int L_ChangeName(lua_State *L);///修改名称功能
	static int L_SendActResult(lua_State *L); ///反馈获取活动奖励情况
	static int L_SendChargeSumResult(lua_State * L); ///发送累计充值奖励数据
	static int L_SendFirstRechargeRes(lua_State *L);//领取首冲奖励结果
	static int L_WeekReMoney(lua_State *L);//基金
	static int L_SendMail(lua_State *L);//脚本发邮件

	static int L_GetHeroInfo(lua_State *L);//取玩家武将信息
	static int L_WarMsg(lua_State *L);//国战和前台消息交互相关
	static int L_WarMail(lua_State *L);//国战邮件奖励

	static int L_RankMsg(lua_State *L);//排行版消息
	static int L_SynMembershipToClient(lua_State * L); //同步月卡会员截止日期到客户端

	static int L_exchangeActRes(lua_State *L);//兑换活动结果
	static int L_exchangeActSendData(lua_State *L);//兑换活动玩家数据同步

public:
	static void CreateShadow(void);

};