#include "StdAfx.h"
#include "TransportConf.h"
#include "tinyxml/tinyxml.h"

TransportConf *TransportConf::_instance = 0;

bool TransportConf::load()
{
	// clear the container, for reload
	this->_data.clear();

	// filename
	const char *fn = "Data\\Transport.xml";
    
	// load and parse
	TiXmlDocument *pDoc = 0;	
	pDoc = new TiXmlDocument(fn);
	if (! pDoc)
	{
		rfalse ("Notice, load file %s failed!", fn);
		return false;
	}

	if(! pDoc->LoadFile())
	{
		rfalse ("Notice, load file %s failed!", fn);
		return false;
	}

	TiXmlElement *rootElement=pDoc->FirstChildElement("MAPLIST");
	if (! rootElement) return false;

	TiXmlElement *pchangeElement = rootElement->FirstChildElement("MAPCHANGELIST");
	if(pchangeElement)
	{
		TiXmlElement *firstMap=pchangeElement->FirstChildElement("MAPCHANGE");
		if(!firstMap)
		{
			rfalse ("Notice, load file %s failed!", fn);
			return false ;
		}

		while (firstMap)
		{
			// one config
			TransportInfo one_conf;

			TiXmlAttribute* pAttrib=firstMap->FirstAttribute();			

			//�б���
			one_conf.m_iSerialID = atol(pAttrib->Value());
			pAttrib=pAttrib->Next();

			//������ͼID
			one_conf.m_iSrcMapID = atol(pAttrib->Value());
			pAttrib=pAttrib->Next();

			//����λ��
			one_conf.m_SrcCenterPos.fPosx = atol(pAttrib->Value());
			pAttrib=pAttrib->Next();
			one_conf.m_SrcCenterPos.fPosy = atol(pAttrib->Value());
			pAttrib=pAttrib->Next();
			one_conf.m_SrcCenterPos.fPosz = atol(pAttrib->Value());
			pAttrib=pAttrib->Next();

			//�����뾶
			one_conf.m_iSrcRange = atol(pAttrib->Value());
			pAttrib=pAttrib->Next();

			//Ŀ���ͼID
			one_conf.m_iDesMapID = atol(pAttrib->Value());	
			pAttrib=pAttrib->Next(); 
			
			//Ŀ���ͼλ��
			one_conf.m_DesPos.fPosx = atol(pAttrib->Value());
			pAttrib=pAttrib->Next();
			one_conf.m_DesPos.fPosy = atol(pAttrib->Value());
			pAttrib=pAttrib->Next();
			one_conf.m_DesPos.fPosz = atol(pAttrib->Value());
			pAttrib=pAttrib->Next();
			
			this->_data[one_conf.m_iSerialID] = one_conf;

			firstMap = firstMap->NextSiblingElement("MAPCHANGE");
		}	
	}

	if (pDoc) delete pDoc;

	return true;
}

const TransportInfo *TransportConf::get(int index) const
{
	TP_ITR itr = this->_data.find(index);
	if (itr != _data.end())
		return &((*itr).second);

	return 0;
}
