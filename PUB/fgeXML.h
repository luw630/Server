/**
** Author:	�˳�
** QQ:		23427470
** Mail:	aron_d@yeah.net
** Time:	
*
** Fantasy Game Engine 1.0 [ Sparrow ]
** Copyright (C) 2005-, Fantasy Games
** Kernel functions
*
*  �汾	 :	1.1
������ ����XML�ļ�,���ԶԴ󲿷��﷨������Ч�ķ����� CDATAĿǰֻ֧��ȫ��Ψһ��
����ͨ�� AddTag �ӿڷ������չ�µı�ǩ�� Ŀǰ֧�ֵı�ǩ��
</ 
<!--
<?
<
<![CDATA[
**/

#pragma once
#include <list>
#include <queue>
#include <string>
#include <Windows.h>
namespace xml{
#define xml_vertion 0x01

	inline int StrHash(const std::string &str)
	{
		register unsigned int h;
		register const std::string::traits_type::_Elem *p;

		for(h=0, p = str.c_str( ); *p ; p++)
			//h = 31 * h + *p;
			h = (h<<5) - h + *p;

		return h&0x7FFFFFFF;
	}
	class ByteMatch
	{
	public:
		ByteMatch( const BYTE* pmatchData, size_t size );
		const BYTE*		Find( const BYTE* pData,int size ) const;
		size_t			GetSize( )	{ return m_size; }
	private:
		enum { OFFSET_SIZE = 257 };
		const BYTE			*m_pMatchData;
		size_t				m_size;
		BYTE				m_len;
		BYTE				m_off[ OFFSET_SIZE ];
	};


	class CXML;
	enum{ TAG_LEN = 80 };
	enum{ TYPE_ELEMENT=1,TYPE_CDATA_ELEMENT, TYPE_COMMENT };

	typedef const char* (CXML::* PARSE)(const char* xml);     //��������
	class	XMLException :public std::exception
	{
	public:
		XMLException(const char* msg) :std::exception(msg) {}
	};

	//��ǩ�ṹ
	struct	TAG
	{
		char name[TAG_LEN];//��ǩ����
		PARSE parseFun;    //��������   

		TAG()
		{
			name[0] = '\0';
			parseFun = NULL;           
		};
		TAG(const char* szTagName,PARSE func)
		{
			strcpy(name,szTagName);
			parseFun = func;
		};
		TAG(const TAG& t)
		{
			memcpy(name,t.name,TAG_LEN);
			parseFun = t.parseFun;
		}
	};
	struct	TagSearch{
		TAG *tag;
		char *pos;
		char *xml;

		TagSearch()
		{
			pos = 0;
			tag = 0;
			xml = 0;
		};

		TagSearch(TAG &t)
		{
			pos = t.name;
			tag = &t;
			xml = 0;
		};
		void Reset()
		{
			pos = 0;
			tag = 0;
			xml = 0;
		}
	};



	struct	Attribute
	{
	private:
		std::string	name;
		std::string	value;
		int hash;
	public:
		Attribute() { hash=0; }
		Attribute( const char* _name, const char* _value )
		{
			name = _name;
			value = _value;
			hash = StrHash( _name );
		}
		Attribute( const  std::string _name, const  std::string _value )
		{
			name = _name;
			value = _value;
			hash = StrHash( _name.c_str() );
		}
		void	SetName( const char* _name )
		{
			name = _name;
			hash = StrHash( _name );
		}
		void	SetValue( const char* _value )
		{
			value = _value;
		}

		const std::string& GetName( ){ return name; }
		std::string&	  GetValue( ){ return value; }
		int				  GetHash( ){ return hash; }	
	};

	struct	Element
	{
		std::string			 name;			//�ڵ���	
		std::string          value;         //ֵ
		std::list<Attribute> attrib;		//���Խڵ�
		std::list<Element*>	 subElement;	//�ӽڵ�ڵ�
		Element*			 parentElement;	//���ڵ�
		int					 bDataValue;	//ֵΪ����[[data ??? data]]	

		typedef std::list<Attribute>::iterator attrib_iterator;
		typedef std::list<Element*>::iterator element_iterator;

		Element( ){
			parentElement = 0;
			bDataValue = 0;
		}
		~Element( );

		void			Clear();
		const char*		GetAttribute(const char* name)
		{
			int hash = StrHash( name );
			std::list<Attribute>::iterator i=attrib.begin();
			while(i!=attrib.end())
			{
				if(i->GetHash() == hash && i->GetName().compare(name) == 0)
					return i->GetValue().c_str();
				i++;
			}
			return 0;
		}
		bool			GetAttribute(const char* name,int& buf);
		bool			GetAttribute(const char* name,unsigned int& buf);
		bool			GetAttribute(const char* name,unsigned long& buf);
		bool			GetAttribute(const char* name,long& buf);
		bool			GetAttribute(const char* name,bool& buf);
		bool			GetAttribute(const char* name,float& buf);
		bool			GetAttribute(const char* name,std::string& str);

		const char*		GetValue( ){return value.c_str();}
		bool			GetValue(std::string& str);
		bool			GetValue(int& buf);
		bool			GetValue(unsigned int& buf);
		bool			GetValue(unsigned long& buf);
		bool			GetValue(long& buf);
		bool			GetValue(bool& buf);
		bool			GetValue(float& buf);

		Element*		GetSubElement(const char* name);
		template<class T>
		bool			GetSubElementValue( char* name, T& buf, Element* pe=0)
		{
			Element* e = GetSubElement(name);
			if(e==0) return false;
			return e->GetValue(buf);
		}

		bool			SetAttribute(const char* name, const char* value);
		bool			SetValue(const char* value);

		virtual	void	Save( std::ostream& stream,int level );
		virtual void	SaveBinary( std::ostream& stream );
		virtual void	ParseBinary( std::istream& stream );

		virtual Element* Clone( );

	private:
		Element& operator= ( const Element& e )	{ return *this; }
		Element( const Element& )				{ }

	protected:
		virtual void	SaveTypeBinary( std::ostream& stream );

	};
	//���ݽ��,���ý�������
	class	DataElement :public Element
	{
	public:
		virtual	void	Save( std::ostream& stream ,int level );
		virtual Element* Clone( );
	protected:
		virtual void	SaveTypeBinary( std::ostream& stream );
	};
	//ע�ͽ��
	class	CommentElement :public Element
	{
	public:
		virtual	void	Save( std::ostream& stream ,int level );
		virtual Element* Clone( );
	protected:
		virtual void	SaveTypeBinary( std::ostream& stream );
	};

	//--------------------------------------
	class	CXML
	{
	public:
		CXML(void);
		virtual ~CXML(void);
	public:

		bool			Open(const char* filename);
		bool			OpenBinary(const char* filename);
		virtual bool    Parse(const char* xmlText);
		const char*		ParseEx(const char* xmlText);
		bool			IsOpen( );
		Element*        GetRootElement() { if(m_root.subElement.size()==0) return 0; return *m_root.subElement.begin();}
		//bool			SetRootElement(Element e);
		//ִ�й����������
		Element*		FindElement(const char* name,Element* pe=0);
		Element*		FindNext(const char* name);

		//BYTE*         GetCData()       { return m_cdata; }
		void            Create(const char* filename,const char* encoding);
		const char*		GetEncoding( );
		void			Save( );
		void			Save(const char* encoding);
		void            Save(const char* filename,const char* encoding);
		void            Save(const char* filename,Element *root,const char* encoding);
		void            Save(std::ostream& stream,Element* e,const char* encoding, int level=0);
		void            SaveBinary(std::ostream& stream,Element* e,const char* encoding, int level=0);
		void			SaveBinary( const char* filename );
		void			SaveBinary( std::ostream& stream );
		void			ParseBinary( std::istream& stream );
		void            Clear();

		template<class T>
		bool			GetElementValue( char* name, T& buf, Element* pe=0)
		{
			Element* e = FindElement(name,pe);
			if(e==0)
				return false;
			return e->GetValue(buf);
		}

		const char*		GetElementValue( char* name,Element* pe=0)
		{
			Element* e = FindElement(name,pe);
			if(e==0)
				return false;
			return e->GetValue( );
		}

		template<class T>
		bool			GetElementValueNext( char* name, T& buf)
		{
			Element* e = FindNext(name);
			if(e==0)
				return false;
			return e->GetValue(buf);
		}

	protected:
		void	        AddTag(const char* tag, PARSE func);
		TAG*	        GetTagParseFunc(const char *tag);
		const char*		Find(const char* xml, const char* tag);
		const char*		SkipTo(const char* xml, const char* tag);
		const char*		SkipBlank(const char* xml);
		bool            StrCmp(const char* xml, const char* key);
		const char*		ReadWord(const char* xml,char* word,int bufsize,TAG **ppTag=0);   //pTag:���ڷ��عؼ���
		const char*		ReadText(const char* xml,std::string &buf, char* end);
		const char*		ReadTag(const char* xml,TAG **ppTag);                             //��ȡ��ǩ

		const char*		parseLf(const char* xml);//����������         <                
		const char*		parseComment(const char* xml);//����ע��      !--
		const char*		parseDeclaration(const char* xml);//          ?
		const char*		parseRf(const char* xml);//����������         >
		const char*		parseEndEmptyElment(const char* xml);//Ԫ�ؽ���    /> 
		const char*		parseEndElment(const char* xml);//Ԫ�ؽ���    </    
		const char*		parseCDataBegin(const char* xml);//Ԫ�ؽ���   <![CDATA[ 
		const char*		parseCDataEnd(const char* xml);//Ԫ�ؽ���     ]]> 

		const char*		readAttrib(Element &e, const char* xml);

		int             IsAlpha( unsigned char anyByte);
		int             IsAlphaNum( unsigned char anyByte);

		Element*		ParseBinary( std::istream& stream, Element* parent );

	protected:		
		std::string		file;
		Element	        m_root; //��·��
		Element	        m_head; //�ļ�ͷ
		Element*	    m_pcr;  //����·��
		std::list<TAG>  m_tag;
		//BYTE*           m_cdata;
		std::list<TagSearch> openList;  //������
		std::queue<Element*> findEslements;
	private:
		const char*		m_xmlEndTemp;
	};
}