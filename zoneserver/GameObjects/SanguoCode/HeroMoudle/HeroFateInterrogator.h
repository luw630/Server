// ----------------------------------
// ------- author : LiuWeiWei -------
// ------- desc : Ӣ��Ե�ֲ�ѯ�� -----
// ------- date : 06/27/2015 --------
// ----------------------------------
#pragma once
#include "../PUB/Singleton.h"
#include "stdint.h"
#include <unordered_map>
#include <vector>
using namespace std;
class CHeroDataManager;
struct FateConfig;

class CHeroFateInterrogator : public CSingleton<CHeroFateInterrogator>
{
public:
	CHeroFateInterrogator();
	~CHeroFateInterrogator();
	/**@brief ��ȡ�佫���Ե���佫�б�
	@param [in] heroID �������佫ID
	*/
	const vector<int32_t>* GetRelatedHeroes(int32_t heroID);
	/**@brief ��ȡ�佫�Ѽ����Ե������
	@param [in] heroID �����ҵ��佫ID
	@param [in] pHeroDataManager �佫���ݹ�����
	@param [out] vecFateConfigs ���Ѽ����Ե������ָ��ѹ�뵽���б�
	*/
	void GetActivatedFateConfigs(IN int32_t heroID, IN const CHeroDataManager* pHeroDataMgr, OUT vector<FateConfig*>& vecFateConfigs);
private:
	unordered_map<int32_t, vector<FateConfig>>& m_FateConfigMap;	///< ����IDδKeyֵ��Ӣ�۵�����Ե������
	unordered_map<int32_t, vector<int32_t>> m_RelatedHeroMap;	///< ������IDΪkeyֵ��Ӣ����Ե�����佫�б�
};

