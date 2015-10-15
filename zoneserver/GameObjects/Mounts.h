#pragma once

#include <map>
#include "networkmodule/playertypedef.h"
#include "pub/Singleton.h"
namespace Mounts
{
	enum{ QualityMax=6, MaxLevel=30, MaxSkillNum=16, };	//定义常量
	enum MountState{
		MountState_None,	/*普通状态*/
		MountState_Mounts,	/*骑乘状态*/ 
	};
	//坐骑模板数据,从文件中读取
	struct STMount
	{
		BYTE	ID;				//编号,标识坐骑的类型	
		BYTE	Hunger;			//饥饿
		std::string	Name;		//名称
		DWORD	ResID;			
		DWORD	Sound1;			//音效1（上马）
		DWORD	Sound2;			//音效2（跑步）
//		SPlayerMounts::Mounts::MSkill mSkill[2];	//技能
		std::string RidingAct;	//骑乘动作名
		std::string Introduction;	//简介 
		struct _Level
		{
			//DWORD	ResID;
			DWORD	Exp;		//经验
			WORD	Amuck;		//杀气
			WORD	Speed;		//加速 1/100 -> 150/100 
			DWORD	HP;			//血量
			DWORD	InnerAtt;	//内功
			DWORD	OutAtt;		//外功
			DWORD	InnerDef;	//内防
			DWORD	OutDef;		//外防			
		}Level[MaxLevel+1];		//等级数据
	};
	//将动态属性与模板数据合成后的数据.方便程序中使用
	struct SMount
	{
		BYTE ID;				//编号,标识坐骑的类型
		char Name[13];			//名称
		BYTE State;				//状态		
		BYTE Level;				//等级	
		BYTE Quality;			//品质			
		float Hunger;			//饥饿		
		WORD Speed;				//加速 1/100 -> 150/100 
		WORD Amuck;				//杀气
		DWORD Exp;				//当前经验点
		DWORD ExpNext;			//升级所需要的经验
		DWORD ResID;			//图档ID
//		SPlayerMounts::Mounts::MSkill mSkill[MaxSkillNum];	//技能

		DWORD HP;				//血量		
		DWORD InnerAtt;			//内功
		DWORD OutAtt;			//外功
		DWORD InnerDef;			//内防
		DWORD OutDef;			//外防
		std::string Introduction;		//简介 
	};

	//add by xj
	//宠物技能
	struct SMountSkill
	{
		DWORD	dwSkillID;			//技能ID
		char    szName[256];		//名称
		DWORD	dwSkillIcon;		//技能图标
		WORD	wSkillType;			//技能类型
		BYTE	iActive;			//是否主动技能
		WORD	wUseMPNum;			//消耗魔法量
		WORD	wSkillCDTime;		//技能CD时间
		char    Introduction[256];	//简介 
	};

	class CMountTemplates : public CSingleton<CMountTemplates>
	{
	private:
		CMountTemplates();
		SINGLETON(CMountTemplates);
	public:
		//初始化角色坐骑属性
		bool			InitMounts( SPlayerMounts::Mounts & m, int mountId, BYTE quality );

		//加载模板数据
		const STMount * GetTemplateMounts( int mountId );

		//得到随机品质
		BYTE			GetRandomQuality();

		//得到角色坐骑的全属性
		//参数: ptMounts 坐骑的动态加点属性方案
		//返回: 成功返回true,失败返回false
		bool			GetMountsAttrib( SMount& m, SPlayerMounts::Mounts& ptMounts );

		//得到道具对应的坐骑编号
		int				GetMountId( int itemId );
		//得到道具对应的坐骑品质
		int				GetMountQuality( int itemId );
		//得到道具对应的增加饥饿度
		int				GetInHunger( int itemId );
		//得到道具对应的增加经验
		int				GetInExp( int itemId );

		//增加坐骑经验值
		//返回: 1表示升了1级, 0表示没有升级
		int				AddMountExp( SPlayerMounts::Mounts* ptMounts, DWORD dwExp, WORD maxleve=MaxLevel );

		SMountSkill*		GetMountSkill( int skillId );
	public:	//品质
		const char*		GetQualityName( BYTE q );
		DWORD			GetQualityColor( BYTE q );
		struct _QualityAddition
		{
			float	Modulus;		//系数
			float	InHunger;		//每次减少饥饿度
			float	DeHunger;		//每次减少饥饿度
			WORD	UpdateTimer;	//计数器（每多少时间改变一次）
		};
		bool			GetMountsQualityAddition( _QualityAddition& add, SPlayerMounts::Mounts& ptMounts );
		static void		SetRoot( const char* path ) { if( path ) m_root = path; else m_root="./"; }
		static bool		IsValidName( LPCSTR szName );
	protected:
		void			LoadMounts( STMount& m );
		void			InitMounts( );
		void			InitItemMountTable( );			//初始道具坐骑表
		void			InitItemHungerTable( );			//初始道具坐骑表
		void			InitQualityAddAttribTable( );	//初始化品质加成表
		void			InitQualityTable( );			//初始化品质概率表
		void			LoadMountSkill();
	private:
		struct _Item_Mount
		{
			int mountid;
			int quality;
		};
		struct _Mount_Feed	//坐骑饲料表
		{
			int Hunger;		//加体力
			int exp;		//加经验
		};
	private:
		typedef std::map< int,STMount > TMountMap;	//坐骑模板数据表
		TMountMap	m_tMounts;						//模板数据
		
		std::string	m_QualityName[QualityMax];	//品质名字
		DWORD		m_QualityColor[QualityMax];	//品质颜色
		int			m_Quality[QualityMax];		//品质机率表

		_QualityAddition	m_QualityAddition[QualityMax];		//品质附加属性表		
		std::map<int,_Item_Mount>	m_itemMountTable;	//道具-坐骑对应表
		std::map<int,_Mount_Feed>	m_itemFeed;			//道具-饥饿度对应表
		std::map<int,SMountSkill> m_skillTable;			//宠物技能

	private:
		static std::string	m_root;				//保存工作目录,默认为: "./data/"
	};

};
