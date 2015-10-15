#pragma once

#include <map>
#include "networkmodule/playertypedef.h"
#include "pub/Singleton.h"
namespace Mounts
{
	enum{ QualityMax=6, MaxLevel=30, MaxSkillNum=16, };	//���峣��
	enum MountState{
		MountState_None,	/*��ͨ״̬*/
		MountState_Mounts,	/*���״̬*/ 
	};
	//����ģ������,���ļ��ж�ȡ
	struct STMount
	{
		BYTE	ID;				//���,��ʶ���������	
		BYTE	Hunger;			//����
		std::string	Name;		//����
		DWORD	ResID;			
		DWORD	Sound1;			//��Ч1������
		DWORD	Sound2;			//��Ч2���ܲ���
//		SPlayerMounts::Mounts::MSkill mSkill[2];	//����
		std::string RidingAct;	//��˶�����
		std::string Introduction;	//��� 
		struct _Level
		{
			//DWORD	ResID;
			DWORD	Exp;		//����
			WORD	Amuck;		//ɱ��
			WORD	Speed;		//���� 1/100 -> 150/100 
			DWORD	HP;			//Ѫ��
			DWORD	InnerAtt;	//�ڹ�
			DWORD	OutAtt;		//�⹦
			DWORD	InnerDef;	//�ڷ�
			DWORD	OutDef;		//���			
		}Level[MaxLevel+1];		//�ȼ�����
	};
	//����̬������ģ�����ݺϳɺ������.���������ʹ��
	struct SMount
	{
		BYTE ID;				//���,��ʶ���������
		char Name[13];			//����
		BYTE State;				//״̬		
		BYTE Level;				//�ȼ�	
		BYTE Quality;			//Ʒ��			
		float Hunger;			//����		
		WORD Speed;				//���� 1/100 -> 150/100 
		WORD Amuck;				//ɱ��
		DWORD Exp;				//��ǰ�����
		DWORD ExpNext;			//��������Ҫ�ľ���
		DWORD ResID;			//ͼ��ID
//		SPlayerMounts::Mounts::MSkill mSkill[MaxSkillNum];	//����

		DWORD HP;				//Ѫ��		
		DWORD InnerAtt;			//�ڹ�
		DWORD OutAtt;			//�⹦
		DWORD InnerDef;			//�ڷ�
		DWORD OutDef;			//���
		std::string Introduction;		//��� 
	};

	//add by xj
	//���＼��
	struct SMountSkill
	{
		DWORD	dwSkillID;			//����ID
		char    szName[256];		//����
		DWORD	dwSkillIcon;		//����ͼ��
		WORD	wSkillType;			//��������
		BYTE	iActive;			//�Ƿ���������
		WORD	wUseMPNum;			//����ħ����
		WORD	wSkillCDTime;		//����CDʱ��
		char    Introduction[256];	//��� 
	};

	class CMountTemplates : public CSingleton<CMountTemplates>
	{
	private:
		CMountTemplates();
		SINGLETON(CMountTemplates);
	public:
		//��ʼ����ɫ��������
		bool			InitMounts( SPlayerMounts::Mounts & m, int mountId, BYTE quality );

		//����ģ������
		const STMount * GetTemplateMounts( int mountId );

		//�õ����Ʒ��
		BYTE			GetRandomQuality();

		//�õ���ɫ�����ȫ����
		//����: ptMounts ����Ķ�̬�ӵ����Է���
		//����: �ɹ�����true,ʧ�ܷ���false
		bool			GetMountsAttrib( SMount& m, SPlayerMounts::Mounts& ptMounts );

		//�õ����߶�Ӧ��������
		int				GetMountId( int itemId );
		//�õ����߶�Ӧ������Ʒ��
		int				GetMountQuality( int itemId );
		//�õ����߶�Ӧ�����Ӽ�����
		int				GetInHunger( int itemId );
		//�õ����߶�Ӧ�����Ӿ���
		int				GetInExp( int itemId );

		//�������ﾭ��ֵ
		//����: 1��ʾ����1��, 0��ʾû������
		int				AddMountExp( SPlayerMounts::Mounts* ptMounts, DWORD dwExp, WORD maxleve=MaxLevel );

		SMountSkill*		GetMountSkill( int skillId );
	public:	//Ʒ��
		const char*		GetQualityName( BYTE q );
		DWORD			GetQualityColor( BYTE q );
		struct _QualityAddition
		{
			float	Modulus;		//ϵ��
			float	InHunger;		//ÿ�μ��ټ�����
			float	DeHunger;		//ÿ�μ��ټ�����
			WORD	UpdateTimer;	//��������ÿ����ʱ��ı�һ�Σ�
		};
		bool			GetMountsQualityAddition( _QualityAddition& add, SPlayerMounts::Mounts& ptMounts );
		static void		SetRoot( const char* path ) { if( path ) m_root = path; else m_root="./"; }
		static bool		IsValidName( LPCSTR szName );
	protected:
		void			LoadMounts( STMount& m );
		void			InitMounts( );
		void			InitItemMountTable( );			//��ʼ���������
		void			InitItemHungerTable( );			//��ʼ���������
		void			InitQualityAddAttribTable( );	//��ʼ��Ʒ�ʼӳɱ�
		void			InitQualityTable( );			//��ʼ��Ʒ�ʸ��ʱ�
		void			LoadMountSkill();
	private:
		struct _Item_Mount
		{
			int mountid;
			int quality;
		};
		struct _Mount_Feed	//�������ϱ�
		{
			int Hunger;		//������
			int exp;		//�Ӿ���
		};
	private:
		typedef std::map< int,STMount > TMountMap;	//����ģ�����ݱ�
		TMountMap	m_tMounts;						//ģ������
		
		std::string	m_QualityName[QualityMax];	//Ʒ������
		DWORD		m_QualityColor[QualityMax];	//Ʒ����ɫ
		int			m_Quality[QualityMax];		//Ʒ�ʻ��ʱ�

		_QualityAddition	m_QualityAddition[QualityMax];		//Ʒ�ʸ������Ա�		
		std::map<int,_Item_Mount>	m_itemMountTable;	//����-�����Ӧ��
		std::map<int,_Mount_Feed>	m_itemFeed;			//����-�����ȶ�Ӧ��
		std::map<int,SMountSkill> m_skillTable;			//���＼��

	private:
		static std::string	m_root;				//���湤��Ŀ¼,Ĭ��Ϊ: "./data/"
	};

};
