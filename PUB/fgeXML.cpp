#include "StdAfx.h"
#include "fgexml.h"
#include <assert.h>
#include <fstream>

namespace xml{
	//----------------------------------
	ByteMatch::ByteMatch( const BYTE* pmatchData, size_t size )
	{
		m_pMatchData = pmatchData;
		m_size = size;
		m_len = (BYTE)min(size,UCHAR_MAX);
		memset( m_off, m_len, sizeof(m_off) );
		--m_len;

		BYTE last = 0;
		for( BYTE offset = m_len; offset>0; --offset, ++last )
		{
			m_off[ *(pmatchData+last) ] = offset;
		}
	}
	const BYTE*	ByteMatch::Find( const BYTE* pData,int size ) const
	{
		if( size<=0 )
			return 0;
		size_t offset = m_size-1;
		const BYTE* _pData = pData;
		for( size_t idx = offset; idx < (size_t)size; idx += offset )
		{
			size_t  idx_pat = m_size-1;
			size_t  idx_tmp = idx;

			for( ; _pData[idx_tmp] == m_pMatchData[idx_pat]; --idx_pat, --idx_tmp )
			{
				if( !idx_pat )
				{
					return pData+idx_tmp;
				}
			}

			offset = m_off[ _pData[idx] ];
		}
		return 0;
	}

	//----------------------------------
	CXML::CXML(void)
	{
		m_head.name = "head";
		m_head.attrib.push_back( Attribute("encoding","gb2312") );
		m_root.parentElement =NULL;
		m_root.name = "Root";
		m_pcr = &m_root;

		//AddTag("/>",&CXML::parseEndEmptyElment);
		AddTag("<![CDATA[",&CXML::parseCDataBegin);
		AddTag("<!--",&CXML::parseComment);
		AddTag("</",&CXML::parseEndElment);        
		AddTag("<?",&CXML::parseDeclaration);
		AddTag("<",&CXML::parseLf);
		//AddTag(">",&CXML::parseRf);

		//AddTag("]]>",&CXML::parseCDataEnd);

		for(std::list<TAG>::iterator i = m_tag.begin(); i!=m_tag.end(); i++)
		{
			openList.push_back(TagSearch(*i));
		} 

		m_xmlEndTemp = 0;
	}

	CXML::~CXML(void)
	{ 
		Clear();
		m_tag.clear();
		openList.clear();
	}
	void CXML::Clear()
	{
		//        SAFE_DELETE_ARRAY(m_cdata);
		m_root.Clear( );
		m_pcr = &m_root;
	}

	void CXML::AddTag(const char* tag, PARSE func)
	{
		m_tag.push_back(TAG(tag,func));
	}

	//bool CXML::SetRootElement(Element e)
	//{
	//	if(m_root.subElement.size()==0)
	//		m_root.subElement.push_back(e);
	//	else
	//		return false;
	//	return true;
	//}
	void	CXML::Create(const char* filename,const char* encoding)
	{
		Clear( );
		file = filename;
		Element *pRoot = new Element();
		pRoot->name = "Root";
		m_root.subElement.push_back( pRoot );
		m_head.attrib.clear( );
		m_head.attrib.push_back( Attribute("encoding",encoding) );
	}

	bool CXML::IsOpen( )
	{
		return file.size()>0;
	}
	bool CXML::Open(const char* filename)
	{
		Clear( );
		file = filename;

		std::ifstream file;
		file.open(filename,std::ios_base::in|std::ios_base::binary);
		if(!file.good())
		{
			return false;
		}

		//File file;
		//if(!file.OpenFile(filename,READ,OPEN_EXISTING))
		//{
		//	char szmsg[256];
		//	sprintf(szmsg,"Can not open file: %s",filename);
		//	throw xml::XMLException(szmsg);
		//}
		file.seekg(0,std::ios_base::end ); 
		DWORD size = file.tellg( );
		bool  result = false;
		char* buf = new char[size+1];
		//ZeroMemory(buf,size+1);
		try{
			file.seekg(0);
			file.read(buf,size);
			//file.Read(buf,size);
			m_xmlEndTemp = buf+size;
			buf[size] = '\0';
			result = Parse(buf);
		}
		catch(XMLException& e)
		{
			if(buf){ delete[] buf; buf = 0; }
			file.close();
			throw e;
		}

		if(buf){ delete[] buf; buf = 0; }
		file.close();
		return result;
	}
	bool CXML::OpenBinary(const char* filename)
	{
		std::ifstream file;
		file.open(filename,std::ios_base::in|std::ios_base::binary);
		if(!file.good())
		{
			return false;
		}
		this->ParseBinary( file );
		file.close();
		return true;
	}
	const char* CXML::parseLf(const char* xml)//解析左括号;
	{
		char szBuf[80];
		TAG *tag = 0;

		xml+=1;//xml = SkipTo(xml,"<");
		xml = ReadWord(xml,szBuf,80,&tag);
		//TAG *tag = GetTagParseFunc(szBuf);
		if(szBuf[0])
		{
			//读取一个元素,元素可以为空元素，但是必须关闭
			Element *e = new Element;
			e->name = szBuf; 
			e->parentElement = m_pcr;

			m_pcr->subElement.push_back(e);
			m_pcr = e;//&(*(--m_pcr->subElement.end()));
			//读取属性
			xml = readAttrib(*m_pcr,xml);
			//xml == ">" || "/>"
			if(StrCmp(xml,"/>"))
				return parseEndEmptyElment(xml);
			else if(StrCmp(xml,">"))
				xml++;
			else
				throw XMLException("标签未正确关闭");

			const char* xmlNext = 0;
			const char* xmlRet = 0;
			if(StrCmp(xml,"[[data"))
			{
				xml+=strlen("[[data");
				m_pcr->bDataValue = *xml;
				sprintf(szBuf,"%cdata]]",*xml++);
				//xmlNext = Find(xml," data]]");
				xmlNext = Find(xml,szBuf);
				xmlRet = xmlNext + strlen("[[data ");
			}
			else
			{
				xmlRet = xmlNext = Find(xml,"<");
			}

			//读值
			if(!xmlNext)
				throw XMLException("标签未正确关闭");
			if(xmlNext > xml)
			{
				int n = (int)(xmlNext-xml);

				while(n>0 && ( *xml=='\r' || *xml=='\n' || *xml=='\t') )
				{
					xml++;
					n--;
				}
				//m_pcr->value.resize(n);
				for(int i=0; i<n; i++)
				{
					if(xml[i]!='\r')
						m_pcr->value += xml[i];
				}
				//m_pcr->value[n] = '\0';
			}
			return xmlRet;
		}
		else
			throw XMLException("无元素名或关键字错误.");
		return SkipTo(xml,">");
	}
	//?
	const char* CXML::parseDeclaration(const char* xml)
	{
		//char szBuf[80];
		//ReadWord(xml,szBuf,80);
		xml+=2;//xml = SkipTo(xml,"<?");
		char szBuf[80];
		TAG *tag = 0;
		xml = ReadWord(xml,szBuf,80,&tag);
		if(!strcmp(szBuf,"xml"))
		{
			xml = readAttrib(m_head,xml);
			if(!StrCmp(xml,"?>"))
				throw XMLException("?>标签未正确关闭");
		}
		return SkipTo(xml,"?>");
	}
	//!--
	const char* CXML::parseComment(const char* xml)
	{       
		////读值
		//xml = SkipTo(xml,"<!--");
		//const char* xmlNext = Find(xml,"-->");
		//if(!xmlNext)
		//	throw XMLException("注释未正确关闭");
		//Element* pElement = new CommentElement;
		//pElement->parentElement = m_pcr;
		//if(xmlNext > xml)
		//{
		//	size_t n = xmlNext-xml;
		//	pElement->value.append(xml,n);
		//}
		//m_pcr->subElement.push_back(pElement);
		//return SkipBlank(xmlNext+3);
		xml+=4;//SkipTo(xml,"<!--");
		return SkipTo(xml,"-->");
	}
	//解析右括号 >
	const char* CXML::parseRf(const char* xml)
	{
		//if(m_pcr)
		//    m_pcr = m_pcr->parentElement;
		//else
		//    throw XMLException(" > 不匹配.");

		return SkipTo(xml,">");
	}
	//元素结束 </...> 要求有<element> 的配对   
	const char* CXML::parseEndElment(const char* xml)
	{
		char szBuf[80];
		TAG  *tag = 0;

		xml+=2;//xml = SkipTo(xml,"</");
		xml = ReadWord(xml,szBuf,80,&tag);

		if(*xml!='>')
			throw XMLException("缺少>.");
		if(m_pcr && m_pcr->name.compare(szBuf)==0 )
			m_pcr = m_pcr->parentElement;
		else
			throw XMLException("元素不匹配.");
		return xml+1;
	}
	//元素结束 />    
	const char* CXML::parseEndEmptyElment(const char* xml)
	{
		if(m_pcr)
			m_pcr = m_pcr->parentElement;
		else
			throw XMLException("元素不匹配.");
		return SkipTo(xml,"/>");
	}

	//元素结束  <![CDATA[ 
	const char* CXML::parseCDataBegin(const char* xml)
	{
		xml = SkipTo(xml,"<![CDATA[");
		//读值
		const char* xmlNext = Find(xml,"]]>");
		if(!xmlNext)
			throw XMLException("CDATA未正确关闭");
		Element* pElement = new DataElement;
		pElement->parentElement = m_pcr;
		if(xmlNext > xml)
		{
			size_t n = xmlNext-xml;
			pElement->value.append(xml,n);
		}
		m_pcr->subElement.push_back(pElement);
		return SkipBlank(xmlNext+3);
	}
	//元素结束  ]]>
	const char* CXML::parseCDataEnd(const char* xml)
	{
		return SkipTo(xml,"]]>");
	}

	//读取元素的属性 '>'结束
	const char* CXML::readAttrib(Element &e,const char* xml)
	{
		char szBuf[80];
		Attribute attrib;
		TAG     *tag = 0;
		do{
			xml = ReadWord(xml,szBuf,80,&tag);
			//if(tag)
			//    return (this->*(tag->parseFun))(xml);
			if(!szBuf[0])
				break;


			attrib.SetName( szBuf );

			xml = SkipBlank(xml);
			if(*xml != '=')
				throw XMLException("属性赋值不正确");
			xml++;
			xml = SkipBlank(xml);
			if(*xml != '"')
				throw XMLException("属性赋值不正确");
			xml++;

			//read value
			xml = ReadText(xml,attrib.GetValue(),"\"");//(xml,szBuf,80);
			//if(!szBuf[0])
			//    throw XMLException("属性赋值不正确");
			//attrib.value=szBuf;

			//if(*xml != '"')
			//    throw XMLException("属性赋值不正确");
			xml++;
			e.attrib.push_back(attrib);
			xml = SkipBlank(xml);
		}while(1);
		return xml;
	}

	TAG* CXML::GetTagParseFunc(const char *tag)
	{
		std::list<TAG>::iterator i = m_tag.begin();
		while(i!=m_tag.end())
		{
			if(strcmp(i->name,tag) == 0)
				return &(*i);
			i++;
		}	
		return NULL;

	}
	//解析xml文本
	bool CXML::Parse(const char* xmlText)
	{
		Clear( );
		if(xmlText == 0)
			return false;
		if(m_xmlEndTemp==NULL)
			m_xmlEndTemp = xmlText+strlen(xmlText);

		xmlText = Find(xmlText,"<");
		if(xmlText == 0)
			throw XMLException("无标签");

		const char* msg = xmlText;
		TAG *tag = 0;   
		try{     
			while(*msg)
			{
				tag = 0;  
				msg = ReadTag(msg,&tag);
				if(tag == 0)
					break;
				msg = (this->*(tag->parseFun))(msg);
			}
			m_xmlEndTemp = 0;
		}
		catch( const XMLException& e )
		{
			std::string strmsg = e.what();
			if( tag!=0 )
			{
				strmsg += ". 在:";	strmsg.append(msg,80);		strmsg += "....附近!";
			}
			throw XMLException(strmsg.c_str());
		}
		if(m_pcr != &m_root)
			throw XMLException("标签未正确关闭");
		if(m_pcr->subElement.size()!=1)
			throw XMLException("有且只能有一个根元素");

		return true;

	}
	//解析xml,返回文档结尾
	const char* CXML::ParseEx(const char* xmlText)
	{
		Clear( );
		if(xmlText == 0)
			return xmlText;

		if(m_xmlEndTemp==NULL)
			m_xmlEndTemp = xmlText+strlen(xmlText);

		xmlText = Find(xmlText,"<");
		if(xmlText == 0)
			return xmlText;
		const char* msg = xmlText;

		TAG *tag = 0;        
		while(*msg)
		{
			tag = 0;  
			msg = ReadTag(msg,&tag);
			if(tag == 0)
				break;
			msg = (this->*(tag->parseFun))(msg);
			if(m_pcr == &m_root)
				return msg;
		}

		if(m_pcr != &m_root)
			throw XMLException("标签未正确关闭");
		if(m_pcr->subElement.size()!=1)
			throw XMLException("有且只能有一个根元素");
		return msg;
	}
	//-----------
	//辅助函数
	//查找
	const char* CXML::Find(const char* xml, const char* tag)
	{
		if( !xml ) return 0;
		////子串搜索
		//const char* tempXml = xml;
		//const char* tempTag = tag;
		//const char* at = tempXml;
		//while(*tempXml/* && (*tempXml!='<'|| '<'==*tag)*/)
		//{
		//	if( !(*tempTag) )
		//		break;
		//	if(*tempTag==*tempXml){               
		//		tempTag++;
		//		tempXml++;
		//	}
		//	else
		//	{
		//		tempTag = tag;
		//		at = ++tempXml;
		//	}            
		//}
		//if( !(*tempTag) )
		//	return at;
		//else
		//	return 0;
		// dc 2009-1-9 优化
		//子串搜索
		const char* tempTag = 0;
		size_t	tag_size = strlen(tag);
		if( tag_size==1 )
		{
			while(*xml)
			{
				if(*tag==*xml)
				{
					return xml;
				}
				xml++;
			}
			return 0;
		}
		else
		{
			ByteMatch m((const BYTE*)tag,strlen(tag));
			if(m_xmlEndTemp!=0)
				return (const char*)m.Find((const BYTE*)xml,(int)(m_xmlEndTemp-xml));
			else
				return (const char*)m.Find((const BYTE*)xml,(int)strlen(xml));
		}
	}
	//跳过
	const char* CXML::SkipTo(const char* xml, const char* tag)
	{
		if( !xml ) return 0;
		//子串搜索
		const char* tempTag = 0;
		size_t	tag_size = strlen(tag);
		if( tag_size==1 )
		{
			while(*xml)
			{
				if(*tag==*xml)
				{
					tempTag = tag;
					break;
				}
				xml++;
			}
		}
		else
		{
			ByteMatch m((const BYTE*)tag,tag_size);
			if(m_xmlEndTemp!=0)
				tempTag = (const char*)m.Find((const BYTE*)xml,(int)(m_xmlEndTemp-xml));
			else
				tempTag = (const char*)m.Find((const BYTE*)xml,(int)strlen(xml));			
		}
		if(!tempTag)
			throw XMLException("标签未正确关闭");
		else
			return tempTag+tag_size;
	}
	//跳过空白
	const char* CXML::SkipBlank(const char* xml)
	{
		while(*xml)
		{
			switch(*xml)
			{
			case ' ':
			case '\n':
			case '\r':
			case '\t':
				xml++;
				break;
			default:
				return xml;
			}
		}
		return xml;
	}
	//字符串比较
	bool CXML::StrCmp(const char* xml, const char* key)
	{
		while(*xml)
		{
			if(*key == *xml){               
				key++;
				xml++;
			}
			else
				break;
		}
		return !(*key);
	}
	//读取一个单词   
	const char* CXML::ReadWord(const char* xml,char* word,int bufsize,TAG **ppTag)
	{
		assert(bufsize>0 && word);
		char* tempWord = word;
		*tempWord = '\0';

		xml = SkipBlank(xml);
		if( xml && *xml 
			&& ( IsAlphaNum((unsigned char) *xml) || IsAlpha((unsigned char) *xml) || *xml == '_' ) )
		{
			while( xml && *xml
				&& ( IsAlphaNum( (unsigned char ) *xml ) 
				|| *xml == '_'
				|| *xml == '-'
				|| *xml == '.'
				|| *xml == ':' ) )
			{
				*tempWord++ = *xml++;
			}
			*tempWord = '\0';
		}      
		return SkipBlank(xml);
	}
	//读取文本内容
	const char* CXML::ReadText(const char* xml,std::string &buf, char* end)
	{
		buf.clear();
		//读值
		const char* xmlNext = Find(xml,end);
		if(!xmlNext)
			throw XMLException("文本中有非法字符:<");
		if(xmlNext > xml)
		{
			buf.assign(xml, xmlNext-xml);
			//size_t n = xmlNext-xml;
			//buf.resize(n+1);
			//for(size_t i=0; i<n; i++)
			//	buf[i] = xml[i];
			//buf[n] = '\0';
		}
		return xmlNext;
	}
	//读取标签
	const char* CXML::ReadTag(const char* xml,TAG **ppTag)
	{
		assert(ppTag);
		xml = Find(xml,"<");
		if(!xml)
			return 0;

		//2.0
		std::list<TAG>::iterator i = m_tag.begin();
		while(i!=m_tag.end())
		{
			if( this->StrCmp(xml, i->name) )
				break;
			i++;
		}
		if(i!=m_tag.end())
		{
			*ppTag = &(*i);   
			//xml += strlen(i->name);
			//return xml;
		}

		//1.0
		//std::list<TagSearch> closeList;
		//TagSearch result;
		//while(*xml)
		//{
		//    std::list<TagSearch>::iterator i;           
		//    //先找closelist
		//    for( i = closeList.begin(); i!=closeList.end();i )
		//    {
		//        if(*i->pos != *xml)
		//        {
		//            std::list<TagSearch>::iterator old = i++;                    
		//            closeList.erase(old);
		//        }
		//        else{ 
		//            i->pos++;
		//            if(!(*i->pos)) { 
		//                result = *i;
		//                result.xml = i->xml;
		//                std::list<TagSearch>::iterator old = i++;                    
		//                closeList.erase(old);   
		//            }
		//            else
		//                i++;  
		//        }
		//    }

		//    if(closeList.size() == 0 && result.tag)
		//    {
		//        *ppTag = result.tag;                
		//        return result.xml;
		//    }

		//    if(closeList.size() == 0)
		//    {
		//        //再找openList
		//        for( i = openList.begin(); i!=openList.end(); i++)
		//        { 
		//            if(*i->tag->name == *xml)
		//            {             
		//                if(!(*(i->pos+1))) { 
		//                    result = *i;
		//                    result.xml = xml;                           
		//                }
		//                else{
		//                    TagSearch t = *i;
		//                    t.pos = t.tag->name+1;
		//                    t.xml= xml;
		//                    closeList.push_back(t);     
		//                }
		//            }
		//        }
		//    }

		//    //记录到单词缓冲区中
		//    xml++;
		//}
		return xml;
	}
	int CXML::IsAlpha( unsigned char anyByte)
	{
		if ( anyByte < 127 )
			return isalpha( anyByte );
		else
			return 1;

	}

	int CXML::IsAlphaNum( unsigned char anyByte)
	{
		if ( anyByte < 127 )
			return isalnum( anyByte );
		else
			return 1;
	}

	const char*	CXML::GetEncoding( )
	{
		return m_head.GetAttribute("encoding");
	}
	void CXML::Save( const char* encoding )
	{
		if(file.size()>0)
			Save(file.c_str(),encoding);
	}
	//保存
	void CXML::Save( )
	{
		if(file.size()>0)
			Save(file.c_str(),m_head.GetAttribute("encoding"));
	}
	void CXML::Save(const char* filename,const char* encoding)
	{ 
		Save(filename,GetRootElement(),encoding);
	}
	void CXML::Save(const char* filename,Element *root,const char* encoding)
	{
		if(!root)
			return;

		file = filename;
		std::ofstream file;
		file.open(filename);
		if(file.good() == false)
			throw XMLException("不能创建文件");
		file<<"<?xml version=\"1.0\" encoding=\""<<encoding<<"\"?>"<<std::endl;;
		root->Save( file , 0 );
		file.close();
	}
	void CXML::Save(std::ostream& stream,Element* e,const char* encoding, int level)
	{
		stream<<"<?xml version=\"1.0\" encoding=\""<<encoding<<"\"?>"<<std::endl;
		e->Save(stream,0);
	}

	void CXML::SaveBinary( const char* filename )
	{
		std::ofstream file;
		file.open(filename, std::ios_base::out|std::ios_base::binary);
		SaveBinary( file );
		file.close();
	}
	void CXML::SaveBinary( std::ostream& stream )
	{
		SaveBinary(stream,GetRootElement(),m_head.GetAttribute("encoding"),0);
	}
	void CXML::SaveBinary(std::ostream& stream,Element* e,const char* encoding, int level)
	{
		std::streamsize len = (std::streamsize)strlen( encoding );
		stream.write((const char*)&len,sizeof(std::streamsize));
		stream.write(encoding,len);
		e->SaveBinary(stream);
	}
	void CXML::ParseBinary( std::istream& stream )
	{
		this->Clear( );

		stream.seekg(0);
		std::streamsize len = 0;
		std::string value;

		stream.read((char*)&len,sizeof(std::streamsize));
		value.resize(len);
		stream.read(&value[0],len);

		m_head.attrib.push_back( Attribute("encoding",value) );

		if(stream.good())
			ParseBinary( stream, &m_root );
	}
	Element* CXML::ParseBinary( std::istream& stream, Element* parent )
	{
		Element* pE = 0;
		int type = 0;
		stream.read( (char*)&type, sizeof(type) );

		switch( type )
		{
		case TYPE_ELEMENT:
			pE = new Element;
			break;
		case TYPE_CDATA_ELEMENT:
			pE = new DataElement;
			break;
		case TYPE_COMMENT:
			pE = new CommentElement;
			break;
		default:
			return 0;
		}

		pE->parentElement = parent;
		pE->ParseBinary( stream );
		parent->subElement.push_back( pE );

		int num = 0;
		//读子节点
		stream.read((char*)&num,sizeof(num));
		for( int i=0; i<num; i++ )
		{
			ParseBinary(stream, pE);
		}

		return pE;
	}
	//执行广度优先搜索
	Element* CXML::FindElement(const char* name,Element* pe)
	{
		if(!pe)
			pe = this->GetRootElement();
		while(!findEslements.empty())
			findEslements.pop();

		while(pe && pe->name.compare(name)!=0)
		{
			std::list<Element*>::iterator i=pe->subElement.begin();
			while(i!=pe->subElement.end())
				findEslements.push(*i++);

			if(findEslements.size() == 0)
				return 0;	
			else
				pe = findEslements.front();
			findEslements.pop();
		}
		//findEslements.pop();
		return pe;
	}
	Element* CXML::FindNext(const char* name)
	{
		if(findEslements.size() == 0)
			return 0;	

		Element* pe = findEslements.front();
		findEslements.pop();
		while(pe && pe->name.compare(name)!=0)
		{
			std::list<Element*>::iterator i=pe->subElement.begin();
			while(i!=pe->subElement.end())
				findEslements.push(*i++);

			if(findEslements.size() == 0)
				return 0;	
			else
				pe = findEslements.front();
			findEslements.pop();
		}
		//findEslements.pop();
		return pe;
	}

	//---------------------------------------------------------------
	Element::~Element( )
	{
		Clear( );
	}
	Element* Element::Clone( )
	{
		Element *pe = new Element();

		pe->name = name;
		pe->value = value;
		pe->bDataValue = bDataValue;
		pe->attrib = attrib;
		pe->parentElement = parentElement;

		for(std::list<Element*>::iterator i=subElement.begin(); i!=subElement.end(); i++)
		{
			pe->subElement.push_back( (*i)->Clone( ) );
		}

		return pe;
	}
	void Element::Clear()
	{
		bDataValue = false;
		name.clear();
		value.clear();
		attrib.clear();
		parentElement = 0;

		std::list<Element*>::iterator i = subElement.begin( );
		while( i!=subElement.end( ) )
		{
			delete *i;
			i++;
		}
		subElement.clear( );
	}
	//-得到属性值
	bool Element::GetAttribute(const char* name,int& buf)
	{
		const char* pvalue = GetAttribute(name);
		if(pvalue)
		{
			if(strncmp(pvalue,"0x",2)==0)
				buf = strtol(pvalue,(char**)NULL,16);
			else
				buf = atoi(pvalue);
			return true;
		}				
		return false;
	}
	bool Element::GetAttribute(const char* name,unsigned int& buf)
	{
		const char* pvalue = GetAttribute(name);
		if(pvalue)
		{
			if(strncmp(pvalue,"0x",2)==0)
				buf = strtoul(pvalue,(char**)NULL,16);
			else
				buf = strtoul(pvalue,(char**)NULL,10);
			return true;
		}				
		return false;
	}
	bool Element::GetAttribute(const char* name,unsigned long& buf)
	{
		const char* pvalue = GetAttribute(name);
		if(pvalue)
		{
			if(strncmp(pvalue,"0x",2)==0)
				buf = strtoul(pvalue,(char**)NULL,16);
			else
				buf = strtoul(pvalue,(char**)NULL,10);
			return true;
		}				
		return false;
	}
	bool Element::GetAttribute(const char* name,long& buf)
	{
		const char* pvalue = GetAttribute(name);
		if(pvalue)
		{
			if(strncmp(pvalue,"0x",2)==0)
				buf = strtol(pvalue,(char**)NULL,16);
			else
				buf = strtol(pvalue,(char**)NULL,10);
			return true;
		}				
		return false;
	}
	bool Element::GetAttribute(const char* name,bool& buf)
	{
		const char* pvalue = GetAttribute(name);
		if(pvalue)
		{
			buf = atoi(pvalue)!=0;
			return true;
		}				
		return false;
	}
	bool Element::GetAttribute(const char* name,float& buf)
	{
		const char* pvalue = GetAttribute(name);
		if(pvalue)
		{
			buf = (float)atof(pvalue);
			return true;
		}				
		return false;
	}
	bool	Element::GetAttribute(const char* name,std::string& str)
	{
		const char* pvalue = GetAttribute(name);
		if(pvalue)
		{
			str = pvalue;
			return true;
		}				
		return false;
	}

	//-得到值
	bool Element::GetValue(std::string& str)
	{
		if(value.empty())
			return false;
		str = value;
		return true;
	}
	bool Element::GetValue(int& buf)
	{
		const char* pvalue = value.c_str();
		if(pvalue)
		{
			if(strncmp(pvalue,"0x",2)==0)
				buf = strtol(pvalue,(char**)NULL,16);
			else
				buf = atoi(pvalue);
			return true;
		}				
		return false;
	}
	bool Element::GetValue(unsigned int& buf)
	{
		const char* pvalue = value.c_str();
		if(pvalue)
		{
			if(strncmp(pvalue,"0x",2)==0)
				buf = strtoul(pvalue,(char**)NULL,16);
			else
				buf = strtoul(pvalue,(char**)NULL,10);
			return true;
		}				
		return false;
	}
	bool Element::GetValue(unsigned long& buf)
	{
		const char* pvalue = value.c_str();
		if(pvalue)
		{
			if(strncmp(pvalue,"0x",2)==0)
				buf = strtoul(pvalue,(char**)NULL,16);
			else
				buf = strtoul(pvalue,(char**)NULL,10);
			return true;
		}				
		return false;
	}
	bool Element::GetValue(long& buf)
	{
		const char* pvalue = value.c_str();
		if(pvalue)
		{
			if(strncmp(pvalue,"0x",2)==0)
				buf = strtol(pvalue,(char**)NULL,16);
			else
				buf = strtol(pvalue,(char**)NULL,10);
			return true;
		}				
		return false;
	}
	bool Element::GetValue(bool& buf)
	{
		const char* pvalue = value.c_str();
		if(pvalue)
		{
			buf = atoi(pvalue)!=0;
			return true;
		}				
		return false;
	}
	bool Element::GetValue(float& buf)
	{
		const char* pvalue = value.c_str();
		if(pvalue)
		{
			buf = (float)atof(pvalue);
			return true;
		}				
		return false;
	}
	Element* Element::GetSubElement(const char* name)
	{
		std::list<Element*>::iterator i = subElement.begin();	//子节点节点
		while( i!= subElement.end() )
		{
			if( (*i)->name.compare(name)==0 )
				return *i;
			i++;
		}
		return 0;
	}
	//
	bool Element::SetAttribute(const char* name, const char* value)
	{
		int nameid = StrHash( name );
		std::list<Attribute>::iterator i=attrib.begin();
		while(i!=attrib.end())
		{
			if( nameid = i->GetHash( ) &&
				i->GetName().compare(name) == 0)
			{
				i->SetValue(value);
				return true;
			}
			i++;
		}
		return false;
	}
	bool Element::SetValue(const char* value)
	{
		this->value = value;
		return true;
	}
	//保存
	void Element::Save( std::ostream& stream ,int level )
	{
		//for(int i=0; i<level; i++)	stream<<'\t';
		//写元素名
		stream<<'<'<<name;
		//写属性
		for(std::list<Attribute>::iterator i = attrib.begin(); i!=attrib.end(); i++)
		{
			stream<<' '<<i->GetName().c_str()<<"=\""<<i->GetValue().c_str()<<'"';
		}
		stream<<'>';

		//写值
		if(bDataValue)
		{
			stream<<"[[data"<<(char)bDataValue;
		}
		stream<<value;
		if(bDataValue)
		{
			stream<<(char)bDataValue<<"data]]";
		}

		//写子元素
		if(subElement.size())
		{
			//换行
			if(value.length()==0)
			{
				stream<<std::endl;
			}
			for(std::list<Element*>::iterator i = subElement.begin(); i!=subElement.end(); i++)
			{
				if(value.length()==0)
					for(int i=0; i<=level; i++)	stream<<'\t';
				(*i)->Save(stream,level+1);
				stream<<std::endl;
			}			
			for(int i=0; i<level; i++)	stream<<'\t';
		}
		//写结束标记
		stream<<"</"<<name<<'>';
	}
	void Element::SaveBinary( std::ostream& stream )
	{
		SaveTypeBinary(stream);
		stream.tellp( );
		//name,value;
		std::streamsize size = (std::streamsize)name.size( );
		stream.write((const char*)&size,sizeof(size));
		stream.write(name.c_str(),(std::streamsize)name.length());

		stream.write((const char*)&bDataValue,sizeof(bDataValue));

		size = (std::streamsize)value.size( );
		stream.write((const char*)&size,sizeof(size));
		stream.write(value.c_str(),(std::streamsize)value.length());

		//属性
		size = (std::streamsize)attrib.size( );
		stream.write((const char*)&size,sizeof(size));
		for( std::list<Attribute>::iterator i=attrib.begin(); i!=attrib.end(); i++ )
		{
			size = (std::streamsize)i->GetName().size();
			stream.write((const char*)&size,sizeof(size));
			stream.write(i->GetName().c_str(),(std::streamsize)i->GetName().length());

			size = (std::streamsize)i->GetValue().size();
			stream.write((const char*)&size,sizeof(size));
			//stream<<(int)i->value.size();
			stream.write(i->GetValue().c_str(),(std::streamsize)i->GetValue().length());
		}
		//子节点
		size = (std::streamsize)subElement.size( );
		stream.write((const char*)&size,sizeof(size));
		for( std::list<Element*>::iterator iSub=subElement.begin(); iSub!=subElement.end(); iSub++ )
		{
			(*iSub)->SaveBinary( stream );
		}
	}
	void Element::SaveTypeBinary( std::ostream& stream )
	{
		int type = TYPE_ELEMENT;
		stream.write( (const char*)&type, sizeof(type) );
	}
	void Element::ParseBinary( std::istream& stream )
	{
		std::streamsize size,num = 0;
		stream.read((char*)&size,sizeof(size));
		name.resize(size);
		stream.read(&name[0],size);
		//name[size]='\0';

		stream.read((char*)&bDataValue,sizeof(bDataValue));
		stream.read((char*)&size,sizeof(size));
		value.resize(size);
		stream.read(&value[0],size);
		//value[size]='\0';

		//属性
		stream.read((char*)&num,sizeof(num));

		std::string	name;
		std::string	value;
		for( std::streamsize i=0; i<num; i++ )
		{
			stream.read((char*)&size,sizeof(size));
			name.resize(size);
			stream.read(&name[0],size);
			//a.name[size]='\0';

			stream.read((char*)&size,sizeof(size));
			value.resize(size);
			stream.read(&value[0],size);
			//a.value[size]='\0';
			this->attrib.push_back( Attribute(name,value) );
		}
		////子节点
		//size = subElement.size( );
		//stream.write((const char*)&size,sizeof(size));
		//for( std::list<Element*>::iterator iSub=subElement.begin(); iSub!=subElement.end(); iSub++ )
		//{
		//	(*iSub)->SaveBinary( stream );
		//}
	}


	//--------------------------------------------------------
	Element* DataElement::Clone( )
	{
		Element *pe = new DataElement();

		pe->name = name;
		pe->value = value;
		pe->bDataValue = bDataValue;
		pe->attrib = attrib;
		pe->parentElement = parentElement;

		for(std::list<Element*>::iterator i=subElement.begin(); i!=subElement.end(); i++)
		{
			pe->subElement.push_back( (*i)->Clone( ) );
		}

		return pe;
	}
	void DataElement::Save( std::ostream& stream ,int level )
	{
		for(int i=0; i<level; i++)	stream<<'\t';
		stream<<"<![CDATA[";
		//写值
		if(value.c_str())
		{
			stream<<value.c_str();
		}
		//写结束标记
		stream<<"]]>"<<std::endl;
	}

	void DataElement::SaveTypeBinary( std::ostream& stream )
	{
		int type = TYPE_CDATA_ELEMENT;
		stream.write( (const char*)&type, sizeof(type) );
	}
	//--------------------------------------------------------
	Element* CommentElement::Clone( )
	{
		Element *pe = new CommentElement();

		pe->name = name;
		pe->value = value;
		pe->bDataValue = bDataValue;
		pe->attrib = attrib;
		pe->parentElement = parentElement;

		for(std::list<Element*>::iterator i=subElement.begin(); i!=subElement.end(); i++)
		{
			pe->subElement.push_back( (*i)->Clone( ) );
		}

		return pe;
	}
	void CommentElement::Save( std::ostream& stream ,int level )
	{
		for(int i=0; i<level; i++)	stream<<'\t';
		stream<<"<!--";
		//写值
		stream<<value.c_str();
		//写结束标记
		stream<<"-->"<<std::endl;
	}

	void CommentElement::SaveTypeBinary( std::ostream& stream )
	{
		int type = TYPE_COMMENT;
		stream.write( (const char*)&type, sizeof(type) );
	}
}