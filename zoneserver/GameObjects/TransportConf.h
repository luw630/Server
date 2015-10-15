
#ifndef _TRANSPORT_CONF_
#define _TRANSPORT_CONF_

#include <map>

/// one config infomation
struct TransportInfo
{
	struct TPosition3
	{
		float fPosx;
		float fPosy;
		float fPosz;
	};

	int			m_iSerialID;
	int			m_iSrcMapID;

	TPosition3	m_SrcCenterPos;
	int			m_iSrcRange;

	int			m_iDesMapID;
	TPosition3	m_DesPos;

	TransportInfo()
	{
		memset (this, 0, sizeof(TransportInfo));
	}
};

class TransportConf
{
private:
	TransportConf ()
	{}

public:
	~TransportConf ()
	{}

	/// typedef
	typedef std::map<int, TransportInfo> TP_MAP;
	typedef TP_MAP::const_iterator       TP_ITR;

public:
	/// singleton instance
	static TransportConf *instance()
	{
		if (! _instance)
		{
			_instance = new TransportConf;
		}

		return _instance;
	}

	/// destroy instance
	static void destroy_instance()
	{
		if (_instance)
		{
			delete _instance;
		}
	}

	/// load the Specific file
	bool load();

	/// return the Specific item, param is the SerialID
	const TransportInfo *get(int index) const;

public:
	static TransportConf *_instance;
	TP_MAP _data;
};

#define sTransportConf TransportConf::instance()
#endif
