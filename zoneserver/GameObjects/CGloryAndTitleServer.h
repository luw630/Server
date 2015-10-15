#pragma once
#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>
/*******************add by ly 2014/3/24*********************/
//��ҫ�ͳƺŷ������
class SGloryBaseData;
class STitleBaseData;
class CGloryAndTitleServer
{
	typedef std::hash_map<WORD, SGloryBaseData*>  GloryBaseData;
	typedef std::hash_map<WORD, STitleBaseData*>  TitleBaseData;
private:
	CGloryAndTitleServer(); 
public:
	static CGloryAndTitleServer& GetInstance()
	{
		static CGloryAndTitleServer instance;
		return instance;
	}

	bool Init();
	bool ReLoad();
	void Clear();

	//ͨ����ҫID��ȡ��ҫ������Ϣ
	const SGloryBaseData* GetGloryBaseDataByID(WORD GloryID) const;
	//��ȡ��ҫ���е�ID
	WORD* ReturnAllGloryID(int *ArrSize);

	//ͨ���ƺ�ID��ȡ�ƺŻ�����Ϣ
	const STitleBaseData* GetTitleBaseDataByID(WORD TitleID) const;
	const STitleBaseData* GetTitleBaseDatabyName(const char* Name) const;


	~CGloryAndTitleServer();

private:
	// ��ֹ��������Ϳ�����ֵ
	CGloryAndTitleServer(CGloryAndTitleServer &);
	CGloryAndTitleServer& operator=(CGloryAndTitleServer &);

private:
	GloryBaseData m_GloryBaseData;
	TitleBaseData m_TitleBaseData;
};
#pragma warning(pop)
