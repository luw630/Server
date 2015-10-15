#include "StdAfx.h"
#include "mounts.h"
#include <fstream>
#include "pub/GBK_VisibleTest.h"
using namespace Mounts;
std::string CMountTemplates::m_root = "./data/";
CMountTemplates::CMountTemplates()
{
	InitQualityTable();
	InitQualityAddAttribTable();	
	//初始化道具-坐骑映射表
	InitItemMountTable( );
	InitItemHungerTable( );
	InitMounts( );
	LoadMountSkill( );
}
//初始化道具坐骑表
void	CMountTemplates::InitItemMountTable( )
{
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s坐骑/道具坐骑表.txt",m_root.c_str());
	infile.open(buf);
	
	if( infile.is_open() )
	{		
		//infile.clear();
		infile.getline(buf,1024);
		int itemId;
		_Item_Mount mount;
		while( !infile.eof() )
		{
			infile.getline(buf,1024);
			if(buf[0]==0) break;

			std::strstream stream( buf, (std::streamsize)strlen( buf ) );			
			stream >> itemId >> mount.mountid >> mount.quality;
			m_itemMountTable[itemId] = mount;
			itemId = mount.mountid = mount.quality = 0;
		}			
		infile.close();		
	}
}

//初始化坐骑饲料表
void	CMountTemplates::InitItemHungerTable( )
{
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s坐骑\\坐骑饲料表.txt",m_root.c_str());
	infile.open(buf);

	if( infile.is_open() )
	{		
		//infile.clear();
		infile.getline(buf,1024);
		int itemId;
		while( !infile.eof() )
		{
			infile.getline(buf,1024);
			if(buf[0]==0) break;

			std::strstream stream( buf, (std::streamsize)strlen( buf ) );
			_Mount_Feed feed;
			stream >> itemId >> feed.Hunger >> feed.exp;
			m_itemFeed[itemId] = feed;
			itemId = 0;
		}			
		infile.close();		
	}
}

//初始化品质加成表
void	CMountTemplates::InitQualityAddAttribTable( )
{
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s坐骑/品质加成表.txt",m_root.c_str());
	infile.open(buf);
	ZeroMemory(&m_QualityAddition,sizeof(m_QualityAddition));
	if( infile.is_open() )
	{		
		//infile.clear();
		infile.getline(buf,1024);
		int dwTemp = 0;
		float fTemp=0.0f;
		for( int i=0; i<QualityMax && !infile.eof(); i++ )
		{
			infile.getline(buf,1024);
			if(buf[0]==0) break;

			std::strstream stream( buf, (std::streamsize)strlen( buf ) );			
			stream >> dwTemp;	//品质
			m_QualityAddition[i].Modulus  = ( ( stream >> fTemp ), fTemp ); fTemp = 0.0f;		//系数
			m_QualityAddition[i].InHunger = ( ( stream >> fTemp ), fTemp ); fTemp = 0.0f;		//每秒减少饥饿度
			m_QualityAddition[i].DeHunger = ( ( stream >> fTemp ), fTemp ); fTemp = 0.0f;		//每秒减少饥饿度
			m_QualityAddition[i].UpdateTimer = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//时间计数器
		}	
		infile.close();		
	}
}
//初始化品质概率表
void	CMountTemplates::InitQualityTable( )
{
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s坐骑/品质机率表.txt",m_root.c_str());
	//初始化品质概率表
	infile.open(buf);
	ZeroMemory(&m_Quality,sizeof(m_Quality));
	if( infile.is_open() )
	{		
		//infile.clear();
		infile.getline(buf,1024);
		int dwTemp = 0;
		for( int i=0; i<QualityMax && !infile.eof(); i++ )
		{
			infile.getline(buf,1024);
			if(buf[0]==0) break;

			std::strstream stream( buf, (std::streamsize)strlen( buf ) );			
			stream >> dwTemp >> dwTemp;
			m_Quality[i] = dwTemp;
			dwTemp = 0;
		}			
		infile.close();		
	}
}
const STMount * CMountTemplates::GetTemplateMounts( int mountId )
{
	TMountMap::iterator _Where = m_tMounts.lower_bound(mountId);
	if (_Where == m_tMounts.end() || mountId!=_Where->first)
	{//
		return NULL;
	}
	return &_Where->second;
}
//初始化坐骑数据 
void	CMountTemplates::InitMounts( )
{
	//加载与等级相关的数据
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s坐骑/坐骑总表.txt",m_root.c_str());
	infile.open(buf);
	if( !infile.is_open() ) return;

	//打开文件
	infile.getline(buf,1024);
	STMount m;
	std::string strBuf;

	while( !infile.eof() )
	{
		infile.getline(buf,1024);
		if(buf[0]==0) break;
		
		ZeroMemory(&m,sizeof(m));

		std::strstream stream( buf, (std::streamsize)strlen( buf ) );
		int dwTemp = 0;
		stream >> dwTemp; m.ID = (BYTE)dwTemp;	dwTemp = 0;					//ID
		stream>>m.Name;														//名称
		m.ResID  =  (( stream >> dwTemp ), dwTemp); dwTemp = 0;				//资源
		m.Hunger  =  (( stream >> dwTemp ), dwTemp); dwTemp = 0;			//饥饿(体力)
		stream>>m.RidingAct;												//使用骑乘动作名
// 		m.mSkill[0].ID = (( stream >> dwTemp ), dwTemp); dwTemp = 0;		//附加技能1编号
// 		m.mSkill[1].ID = (( stream >> dwTemp ), dwTemp); dwTemp = 0;		//附加技能2编号
		m.Sound1  =  (( stream >> dwTemp ), dwTemp); dwTemp = 0;			//音效1
		m.Sound2  =  (( stream >> dwTemp ), dwTemp); dwTemp = 0;			//音效2
		stream>>m.Introduction;												//简介

		LoadMounts(m);

		m_tMounts[m.ID] = m;
	}		
	infile.close();
}
//加载模板数据
void	CMountTemplates::LoadMounts( STMount& m )
{
	//加载与等级相关的数据
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s坐骑/坐骑/坐骑_%d.txt",m_root.c_str(),m.ID);
	infile.open(buf);
	if( infile.is_open() )
	{//打开文件
		infile.getline(buf,1024);
		int level(1);
		while( !infile.eof() && level<MaxLevel )
		{
			infile.getline(buf,1024);
			if(buf[0]==0) break;

			std::strstream stream( buf, (std::streamsize)strlen( buf ) );
			int dwTemp = 0;
			stream >> dwTemp;	//等级
			m.Level[level].Exp = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;		//经验
			m.Level[level].Speed = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//速度
			m.Level[level].HP = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;		//血量
			m.Level[level].OutAtt = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//外功
			m.Level[level].InnerAtt = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//内功
			m.Level[level].OutDef = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//外防
			m.Level[level].InnerDef = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//内防
			m.Level[level].Amuck = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//杀气

			level++;
		}		
		infile.close();	
	}
}
//初始化角色坐骑属性
bool	CMountTemplates::InitMounts( SPlayerMounts::Mounts & m, int mountId, BYTE quality )
{
// 	const Mounts::STMount *tMount = GetTemplateMounts(mountId);
// 	if(!tMount)
// 		return false;
// 	int index(0);
// 
// 	ZeroMemory(&m,sizeof(SPlayerMounts::Mounts));
// 	m.ID = tMount->ID;
// 	m.Level = 1;
// 	m.Quality = quality?(quality>QualityMax?QualityMax:quality):GetRandomQuality();
// 	m.Hunger = (float)tMount->Hunger;
// 	m.Exp = 0;
// 	m.mSkill[0] = tMount->mSkill[0];
// 	m.mSkill[1] = tMount->mSkill[1];
// 	m.State = MountState_None;
// 	dwt::strcpy( m.Name,tMount->Name.c_str(),sizeof(m.Name) );	//给一下初始名字,以后可以改名一次.
 	return true;
}
//得到一个随机品质
BYTE	CMountTemplates::GetRandomQuality()
{
	int q = rand()%100000;
	BYTE i=QualityMax-1;
	int r = 0;
	// 如果随机数据脚本有错,即各品质机率相加总和不为100000.有可能会随机不到品质,这时返回一个最差的.
	for( i; i>0; i-- )
	{
		if( q>=r && q<(r+m_Quality[i]) ) break;
		r+=m_Quality[i];
	}
	return i+1;
}
//得到角色坐骑的全属性
bool	CMountTemplates::GetMountsAttrib( SMount& m, SPlayerMounts::Mounts& ptMounts )
{
// 	const STMount *pTm = GetTemplateMounts( ptMounts.ID );
// 	if( !pTm ) return false;
// 	m.ID = pTm->ID;						//编号
// 	strcpy(m.Name,ptMounts.Name);		//名字
// 	m.State = ptMounts.State;			//状态
// 	m.Level = ptMounts.Level;			//等级
// 	m.Quality = ptMounts.Quality;		//品质	
// 	m.Hunger = ptMounts.Hunger;			//饥饿
// 	m.Exp = ptMounts.Exp;				//经验
// 	if( m.Level<=MaxLevel )
//         m.ExpNext = pTm->Level[m.Level].Exp;	//升级所需要的经验值		
// 	else
// 		m.ExpNext = 0xffffffff;
// 	m.ResID = pTm->ResID;//图档ID
// 	memcpy(m.mSkill,ptMounts.mSkill,sizeof(m.mSkill));	//技能
// 	m.Introduction = pTm->Introduction;					//简介 
// 	m.Speed = pTm->Level[m.Level].Speed;				//速度
// 
// 	m.HP = pTm->Level[m.Level].HP;						//生命	
// 	m.Amuck = pTm->Level[m.Level].Amuck;				//杀气	
// 	m.InnerAtt = pTm->Level[m.Level].InnerAtt;			//内功
// 	m.OutAtt = pTm->Level[m.Level].OutAtt;				//外功
// 	m.InnerDef = pTm->Level[m.Level].InnerDef;			//内防
// 	m.OutDef = pTm->Level[m.Level].OutDef;				//外防
// 	
// 	int q = ptMounts.Quality-1;
// 
// 	float nmount =  m.HP * m.Level * m_QualityAddition[q].Modulus / 10;		//生命
// 	if ( nmount > 0.0f && nmount < 1.0f)
// 		m.HP = 1;
// 	else
// 		m.HP = (WORD)nmount;
// 	nmount = m.Amuck * m.Level * m_QualityAddition[q].Modulus / 10 ;		//杀气
// 	if ( nmount > 0.0f && nmount < 1.0f)
// 		m.Amuck = 1;
// 	else
// 		m.Amuck = (WORD)nmount;
// 	nmount = m.InnerAtt * m.Level * m_QualityAddition[q].Modulus / 10;		//内功
// 	if ( nmount > 0.0f && nmount < 1.0f)
// 		m.InnerAtt = 1;
// 	else
// 		m.InnerAtt = (WORD)nmount;
// 	nmount = m.OutAtt * m.Level * m_QualityAddition[q].Modulus / 10;		//外功
// 	if ( nmount > 0.0f && nmount < 1.0f)
// 		m.OutAtt = 1;
// 	else
// 		m.OutAtt = (WORD)nmount;
// 	nmount =  m.InnerDef * m.Level * m_QualityAddition[q].Modulus / 10;		//内防
// 	if ( nmount > 0.0f && nmount < 1.0f)
// 		m.InnerDef = 1;
// 	else
// 		m.InnerDef = (WORD)nmount;
// 	nmount =  m.OutDef * m.Level * m_QualityAddition[q].Modulus / 10;		//外防
// 	if ( nmount > 0.0f && nmount < 1.0f)
// 		m.OutDef = 1;
// 	else
// 		m.OutDef = (WORD)nmount;
	return true;
}
bool	CMountTemplates::GetMountsQualityAddition( _QualityAddition& add, SPlayerMounts::Mounts& ptMounts )
{
// 	int q = ptMounts.Quality-1;
// 
// 	add.Modulus  = m_QualityAddition[q].Modulus;		//系数
// 	add.InHunger = m_QualityAddition[q].InHunger;		//每秒减少饥饿度
// 	add.DeHunger = m_QualityAddition[q].DeHunger;		//每秒减少饥饿度
// 	add.UpdateTimer = m_QualityAddition[q].UpdateTimer;	//时间计数器
	return true;
}
const char*		CMountTemplates::GetQualityName( BYTE q )
{
	static char* szName[QualityMax] = {
		"次品",
		"普通",
		"良品",
		"优品",
		"极品",
		"神品"	
	};
	if( q<=QualityMax ) return szName[q-1];
	else	return " ";
}
DWORD	CMountTemplates::GetQualityColor( BYTE q )
{
	static DWORD color[QualityMax] = {	
			0x0000ad55,		//灰色
			0x0000ffff,		//白色
			0x000007e0,		//绿色
			0x0000001f,		//蓝色
			0x0000f81f,		//紫色
			0x0000fc08		//橙色
	};
	if( q<=QualityMax ) return color[q-1];
	else	return 0;
}
int		CMountTemplates::GetMountId( int itemId )
{
	std::map<int,_Item_Mount>::iterator i = m_itemMountTable.find(itemId);
	if(i!=m_itemMountTable.end())
		return i->second.mountid;
	else
		return -1;
}

int		CMountTemplates::GetMountQuality( int itemId )
{
	std::map<int,_Item_Mount>::iterator i = m_itemMountTable.find(itemId);
	if(i!=m_itemMountTable.end())
		return i->second.quality;
	else
		return 0;
}
int		CMountTemplates::GetInHunger( int itemId )
{
	std::map<int,_Mount_Feed>::iterator i = m_itemFeed.find(itemId);
	if(i!=m_itemFeed.end())
		return i->second.Hunger;
	else
		return 0;
}
int		CMountTemplates::GetInExp( int itemId )
{
	std::map<int,_Mount_Feed>::iterator i = m_itemFeed.find(itemId);
	if(i!=m_itemFeed.end())
		return i->second.exp;
	else
		return 0;
}

bool	CMountTemplates::IsValidName( LPCSTR szName )
{
	static dwt::cStringMatchDict mdict;
	if ( dwt::IsBadStringPtr( szName, CONST_USERNAME ) )
		return false;

	int len = dwt::strlen(szName, CONST_USERNAME);

	for (int i=0; i<len; i++)
	{   //空格之前的都不行
		if (((BYTE)szName[i]) <= 32)
			return FALSE;
		//不能注册中包含#号，因为表情符中是以#来区分的
		if (((BYTE)szName[i]) == '#')
			return FALSE;
		// 不能有单引号，数据库会出错。。。。。
		if (((BYTE)szName[i]) == '\'')
			return FALSE;

		// 不能有单引号，数据库会出错。。。。。
		if (((BYTE)szName[i]) == '?')
			return FALSE;

		if (((BYTE)szName[i]) == '%')
			return FALSE;

		if (((BYTE)szName[i]) == ':')
			return FALSE;

		if (((BYTE)szName[i]) == '\\')
			return FALSE;

		if (((BYTE)szName[i]) == '/')
			return FALSE;

		if (mdict.Contain(&szName[i], len-i))
			return FALSE;

		// 双字节字符不必检测第2个字节
		if (((BYTE)szName[i]) > 128) 
		{
			// 是否为不可见字符
			if (!isVisibleCharacterW(*(wchar_t*)&szName[i]))
				return FALSE;

			i++;

			// 现在需要检测第二个字符是否被截断，不然会出错的！
			if (szName[i] == 0)
				return FALSE;
		}
		else
		{
			// 是否为不可见字符
			if (!isVisibleCharacterA((BYTE)szName[i]))
				return FALSE;
		}
	}

	return TRUE;
}
int		CMountTemplates::AddMountExp( SPlayerMounts::Mounts* ptMounts, DWORD dwExp ,WORD maxleve )
{
// 	if(!ptMounts) return -1;
// 	const STMount *pmt = this->GetTemplateMounts( (int)ptMounts->ID );
// 	if(!pmt) return -1;
// 	if(maxleve > MaxLevel) 
// 		maxleve = MaxLevel;
// 	DWORD nextExp = 0;
// 	DWORD oldExp = ptMounts->Exp;
// 
// 	nextExp = pmt->Level[ptMounts->Level].Exp;	//升级所需要的经验值
// 	if(ptMounts->Level<maxleve && pmt->Level[ptMounts->Level+1].Exp==0)
// 		maxleve = ptMounts->Level;
// 	
// 	ptMounts->Exp += dwExp;
// 	if( ptMounts->Level>= (maxleve) )
// 	{//如果已经是满级了,则不再升级
// 		ptMounts->Level = (BYTE)maxleve;
// 		nextExp = pmt->Level[ptMounts->Level].Exp;
// 		if( ptMounts->Exp >= nextExp )
// 			ptMounts->Exp = nextExp;
// 		if( ptMounts->Exp == oldExp )
// 			return -1;	//没有加经验
// 	}
// 	else if( ptMounts->Exp >= nextExp )
// 	{
// 		ptMounts->Hunger = 100.0f;
// 		ptMounts->Exp = 0;
// 		ptMounts->Level++;
// 		return 1;
// 	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////
// add by xj
void	Mounts::CMountTemplates::LoadMountSkill()
{
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s坐骑/坐骑技能表.txt",m_root.c_str());
	infile.open(buf);
	char tmpBuffer[256];
	int iID;
	if( infile.is_open() )
	{		
		//infile.clear();
		infile.getline(buf,1024);
		while( !infile.eof() )
		{
			infile.getline(buf,1024);
			if(buf[0]==0) break;
			SMountSkill stTempSkill;

			std::strstream stream( buf, (std::streamsize)strlen( buf ) );
			stream >> iID;
			stream >> tmpBuffer ;
			stTempSkill.dwSkillID = iID;
			strncpy(stTempSkill.szName, tmpBuffer, 256);
			stTempSkill.szName[255] = 0;

			stream >> stTempSkill.dwSkillIcon >> stTempSkill.wSkillType >> 
			stTempSkill.iActive >> stTempSkill.wUseMPNum >> stTempSkill.wSkillCDTime ;
			stream >> tmpBuffer;
			strncpy(stTempSkill.Introduction, tmpBuffer, 256);
			stTempSkill.szName[255] = 0;

			m_skillTable[stTempSkill.dwSkillID] = stTempSkill;	//装入map
		}			
		infile.close();
	}
}

SMountSkill* Mounts::CMountTemplates::GetMountSkill( int skillId )
{
	std::map<int,SMountSkill>::iterator i = m_skillTable.find(skillId);
	if(i!=m_skillTable.end())
		return &i->second;
	else
		return 0;
}