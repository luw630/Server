// ----- TimerEvent_SG.h -----
//
//   --  Author: wk
//   --  Date:   15/01/23
//   --  Desc:   �Ƚ����� ��ʱ�¼���ͷ�ļ�
// --------------------------------------------------------------------
// ��Ҫ����Ϊע�ᶨʱ�¼�,��ÿ��24��ˢ������(����������,���������������(c++�����Լ�lua����))
//---------------------------------------------------------------------  
#pragma once
#include "..\..\EventBase.h"
#include <functional>
#include <map>
typedef std::function<void(int)> FunctionPtr;





enum EventTypeSG
{
	none,
	EveryDay5 = 1,	///ÿ��5���¼�
	EveryDay12,		///ÿ��12���¼�
	EveryDay14,		///ÿ��14���¼�
	EveryDay18,		///ÿ��18���¼�
	EveryDay20,		///ÿ��20���¼�
	EveryDay21,		///ÿ��21���¼�
	EveryDay24,		///ÿ��24���¼�
	pRefresh,//�������ˢ������,11:59:50
	Function, // ������
	hourly, // ÿСʱ
	EachTenMinutes, //ÿ10����
	EachOneMinute, //ÿ1����
	VarietyShopEvent,		///�ӻ����¼�
	ArenaShopEvent,			///�������̵��¼�
	ExpeditionShopEvent,		///Զ���̵��¼�
	LegionShopEvent,				///�����̵��¼�
	MiracleMerchantEvent,	///��Ե�����¼�
	GemMerchantEvent,		///�䱦�����¼�
	SoulExchangeShopEvent,	///����һ��̵��¼�
	WarOfLeagueShopEvent,	///��ս�̵��¼�
};

class TimerEvent_SG : public EventBase
{
public:
	static int FristReg();
	static TimerEvent_SG* SetCallback(FunctionPtr &func, int timePar);

private:								
	virtual void SelfDestory();		// �����ٷ���
	virtual void OnCancel();	// �¼����ж�
	virtual void OnActive();	// �¼�������

	int EveryDay_5();
	int EveryDay_12();
	int EveryDay_14();
	int EveryDay_18();
	int EveryDay_20();
	int EveryDay_21();
	int EveryDay_24();
	int EveryHour();
	int ActiveTenMinEvent();//����ʮ����ʱ��
	int ActiveOneMinEvent();//ÿ���Ӽ���һ��
	///@brief �ӻ���
	int VarietyShop();
	///@brief �������̵�
	int ArenaShop();
	///@brief Զ���̵�
	int ExpeditionShop();
	///@brief �����̵�
	int LegionShop();
	///@brief ��Ե����
	int MiracleMerchant();
	///@brief �䱦����
	int GemMerchant();
	///@brief �����̵�
	int SoulExchangeShop();
	///@brief ��ս�̵�
	int WarOfLeagueShop();
private:
	int eType;
	int playerSid; //��ҵľ�̬ID����ID������з�����ȺΨһ
	FunctionPtr m_Function;
};

