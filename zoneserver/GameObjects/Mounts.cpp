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
	//��ʼ������-����ӳ���
	InitItemMountTable( );
	InitItemHungerTable( );
	InitMounts( );
	LoadMountSkill( );
}
//��ʼ�����������
void	CMountTemplates::InitItemMountTable( )
{
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s����/���������.txt",m_root.c_str());
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

//��ʼ���������ϱ�
void	CMountTemplates::InitItemHungerTable( )
{
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s����\\�������ϱ�.txt",m_root.c_str());
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

//��ʼ��Ʒ�ʼӳɱ�
void	CMountTemplates::InitQualityAddAttribTable( )
{
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s����/Ʒ�ʼӳɱ�.txt",m_root.c_str());
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
			stream >> dwTemp;	//Ʒ��
			m_QualityAddition[i].Modulus  = ( ( stream >> fTemp ), fTemp ); fTemp = 0.0f;		//ϵ��
			m_QualityAddition[i].InHunger = ( ( stream >> fTemp ), fTemp ); fTemp = 0.0f;		//ÿ����ټ�����
			m_QualityAddition[i].DeHunger = ( ( stream >> fTemp ), fTemp ); fTemp = 0.0f;		//ÿ����ټ�����
			m_QualityAddition[i].UpdateTimer = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//ʱ�������
		}	
		infile.close();		
	}
}
//��ʼ��Ʒ�ʸ��ʱ�
void	CMountTemplates::InitQualityTable( )
{
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s����/Ʒ�ʻ��ʱ�.txt",m_root.c_str());
	//��ʼ��Ʒ�ʸ��ʱ�
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
//��ʼ���������� 
void	CMountTemplates::InitMounts( )
{
	//������ȼ���ص�����
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s����/�����ܱ�.txt",m_root.c_str());
	infile.open(buf);
	if( !infile.is_open() ) return;

	//���ļ�
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
		stream>>m.Name;														//����
		m.ResID  =  (( stream >> dwTemp ), dwTemp); dwTemp = 0;				//��Դ
		m.Hunger  =  (( stream >> dwTemp ), dwTemp); dwTemp = 0;			//����(����)
		stream>>m.RidingAct;												//ʹ����˶�����
// 		m.mSkill[0].ID = (( stream >> dwTemp ), dwTemp); dwTemp = 0;		//���Ӽ���1���
// 		m.mSkill[1].ID = (( stream >> dwTemp ), dwTemp); dwTemp = 0;		//���Ӽ���2���
		m.Sound1  =  (( stream >> dwTemp ), dwTemp); dwTemp = 0;			//��Ч1
		m.Sound2  =  (( stream >> dwTemp ), dwTemp); dwTemp = 0;			//��Ч2
		stream>>m.Introduction;												//���

		LoadMounts(m);

		m_tMounts[m.ID] = m;
	}		
	infile.close();
}
//����ģ������
void	CMountTemplates::LoadMounts( STMount& m )
{
	//������ȼ���ص�����
	dwt::ifstream infile;
	char buf[1024];
	sprintf(buf,"%s����/����/����_%d.txt",m_root.c_str(),m.ID);
	infile.open(buf);
	if( infile.is_open() )
	{//���ļ�
		infile.getline(buf,1024);
		int level(1);
		while( !infile.eof() && level<MaxLevel )
		{
			infile.getline(buf,1024);
			if(buf[0]==0) break;

			std::strstream stream( buf, (std::streamsize)strlen( buf ) );
			int dwTemp = 0;
			stream >> dwTemp;	//�ȼ�
			m.Level[level].Exp = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;		//����
			m.Level[level].Speed = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//�ٶ�
			m.Level[level].HP = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;		//Ѫ��
			m.Level[level].OutAtt = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//�⹦
			m.Level[level].InnerAtt = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//�ڹ�
			m.Level[level].OutDef = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//���
			m.Level[level].InnerDef = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//�ڷ�
			m.Level[level].Amuck = ( ( stream >> dwTemp ), dwTemp ); dwTemp = 0;	//ɱ��

			level++;
		}		
		infile.close();	
	}
}
//��ʼ����ɫ��������
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
// 	dwt::strcpy( m.Name,tMount->Name.c_str(),sizeof(m.Name) );	//��һ�³�ʼ����,�Ժ���Ը���һ��.
 	return true;
}
//�õ�һ�����Ʒ��
BYTE	CMountTemplates::GetRandomQuality()
{
	int q = rand()%100000;
	BYTE i=QualityMax-1;
	int r = 0;
	// ���������ݽű��д�,����Ʒ�ʻ�������ܺͲ�Ϊ100000.�п��ܻ��������Ʒ��,��ʱ����һ������.
	for( i; i>0; i-- )
	{
		if( q>=r && q<(r+m_Quality[i]) ) break;
		r+=m_Quality[i];
	}
	return i+1;
}
//�õ���ɫ�����ȫ����
bool	CMountTemplates::GetMountsAttrib( SMount& m, SPlayerMounts::Mounts& ptMounts )
{
// 	const STMount *pTm = GetTemplateMounts( ptMounts.ID );
// 	if( !pTm ) return false;
// 	m.ID = pTm->ID;						//���
// 	strcpy(m.Name,ptMounts.Name);		//����
// 	m.State = ptMounts.State;			//״̬
// 	m.Level = ptMounts.Level;			//�ȼ�
// 	m.Quality = ptMounts.Quality;		//Ʒ��	
// 	m.Hunger = ptMounts.Hunger;			//����
// 	m.Exp = ptMounts.Exp;				//����
// 	if( m.Level<=MaxLevel )
//         m.ExpNext = pTm->Level[m.Level].Exp;	//��������Ҫ�ľ���ֵ		
// 	else
// 		m.ExpNext = 0xffffffff;
// 	m.ResID = pTm->ResID;//ͼ��ID
// 	memcpy(m.mSkill,ptMounts.mSkill,sizeof(m.mSkill));	//����
// 	m.Introduction = pTm->Introduction;					//��� 
// 	m.Speed = pTm->Level[m.Level].Speed;				//�ٶ�
// 
// 	m.HP = pTm->Level[m.Level].HP;						//����	
// 	m.Amuck = pTm->Level[m.Level].Amuck;				//ɱ��	
// 	m.InnerAtt = pTm->Level[m.Level].InnerAtt;			//�ڹ�
// 	m.OutAtt = pTm->Level[m.Level].OutAtt;				//�⹦
// 	m.InnerDef = pTm->Level[m.Level].InnerDef;			//�ڷ�
// 	m.OutDef = pTm->Level[m.Level].OutDef;				//���
// 	
// 	int q = ptMounts.Quality-1;
// 
// 	float nmount =  m.HP * m.Level * m_QualityAddition[q].Modulus / 10;		//����
// 	if ( nmount > 0.0f && nmount < 1.0f)
// 		m.HP = 1;
// 	else
// 		m.HP = (WORD)nmount;
// 	nmount = m.Amuck * m.Level * m_QualityAddition[q].Modulus / 10 ;		//ɱ��
// 	if ( nmount > 0.0f && nmount < 1.0f)
// 		m.Amuck = 1;
// 	else
// 		m.Amuck = (WORD)nmount;
// 	nmount = m.InnerAtt * m.Level * m_QualityAddition[q].Modulus / 10;		//�ڹ�
// 	if ( nmount > 0.0f && nmount < 1.0f)
// 		m.InnerAtt = 1;
// 	else
// 		m.InnerAtt = (WORD)nmount;
// 	nmount = m.OutAtt * m.Level * m_QualityAddition[q].Modulus / 10;		//�⹦
// 	if ( nmount > 0.0f && nmount < 1.0f)
// 		m.OutAtt = 1;
// 	else
// 		m.OutAtt = (WORD)nmount;
// 	nmount =  m.InnerDef * m.Level * m_QualityAddition[q].Modulus / 10;		//�ڷ�
// 	if ( nmount > 0.0f && nmount < 1.0f)
// 		m.InnerDef = 1;
// 	else
// 		m.InnerDef = (WORD)nmount;
// 	nmount =  m.OutDef * m.Level * m_QualityAddition[q].Modulus / 10;		//���
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
// 	add.Modulus  = m_QualityAddition[q].Modulus;		//ϵ��
// 	add.InHunger = m_QualityAddition[q].InHunger;		//ÿ����ټ�����
// 	add.DeHunger = m_QualityAddition[q].DeHunger;		//ÿ����ټ�����
// 	add.UpdateTimer = m_QualityAddition[q].UpdateTimer;	//ʱ�������
	return true;
}
const char*		CMountTemplates::GetQualityName( BYTE q )
{
	static char* szName[QualityMax] = {
		"��Ʒ",
		"��ͨ",
		"��Ʒ",
		"��Ʒ",
		"��Ʒ",
		"��Ʒ"	
	};
	if( q<=QualityMax ) return szName[q-1];
	else	return " ";
}
DWORD	CMountTemplates::GetQualityColor( BYTE q )
{
	static DWORD color[QualityMax] = {	
			0x0000ad55,		//��ɫ
			0x0000ffff,		//��ɫ
			0x000007e0,		//��ɫ
			0x0000001f,		//��ɫ
			0x0000f81f,		//��ɫ
			0x0000fc08		//��ɫ
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
	{   //�ո�֮ǰ�Ķ�����
		if (((BYTE)szName[i]) <= 32)
			return FALSE;
		//����ע���а���#�ţ���Ϊ�����������#�����ֵ�
		if (((BYTE)szName[i]) == '#')
			return FALSE;
		// �����е����ţ����ݿ�������������
		if (((BYTE)szName[i]) == '\'')
			return FALSE;

		// �����е����ţ����ݿ�������������
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

		// ˫�ֽ��ַ����ؼ���2���ֽ�
		if (((BYTE)szName[i]) > 128) 
		{
			// �Ƿ�Ϊ���ɼ��ַ�
			if (!isVisibleCharacterW(*(wchar_t*)&szName[i]))
				return FALSE;

			i++;

			// ������Ҫ���ڶ����ַ��Ƿ񱻽ضϣ���Ȼ�����ģ�
			if (szName[i] == 0)
				return FALSE;
		}
		else
		{
			// �Ƿ�Ϊ���ɼ��ַ�
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
// 	nextExp = pmt->Level[ptMounts->Level].Exp;	//��������Ҫ�ľ���ֵ
// 	if(ptMounts->Level<maxleve && pmt->Level[ptMounts->Level+1].Exp==0)
// 		maxleve = ptMounts->Level;
// 	
// 	ptMounts->Exp += dwExp;
// 	if( ptMounts->Level>= (maxleve) )
// 	{//����Ѿ���������,��������
// 		ptMounts->Level = (BYTE)maxleve;
// 		nextExp = pmt->Level[ptMounts->Level].Exp;
// 		if( ptMounts->Exp >= nextExp )
// 			ptMounts->Exp = nextExp;
// 		if( ptMounts->Exp == oldExp )
// 			return -1;	//û�мӾ���
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
	sprintf(buf,"%s����/���＼�ܱ�.txt",m_root.c_str());
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

			m_skillTable[stTempSkill.dwSkillID] = stTempSkill;	//װ��map
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