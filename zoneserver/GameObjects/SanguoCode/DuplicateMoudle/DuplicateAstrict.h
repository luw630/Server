// ----- CDuplicateAstrict.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/28
//   --  Desc:   �ȶ������ĸ����������ж�
// --------------------------------------------------------------------
//   --  ����ĳһ�������Ƿ������ս���߼��жϣ���Ҫ����ĳһ��ʱ�����ս�����Ƿ�ﵽ������
//---------------------------------------------------------------------   
#pragma once

class CBaseDataManager;
struct ChapterConfig;
enum CheckResult;

class CDuplicateAstrict
{
public:
	CDuplicateAstrict();
	~CDuplicateAstrict();

	///@brief ��ʼ��ĳһ��������������Ϣ
	void Init(const ChapterConfig* config);
	///@brief ������ս��CDʱ�䣬��ʱ����
	void UpdateCD();
	///@brief ��ս�ɹ����һЩ�������
	void Resolves(int clearTime = 1);
	///@brief ��ʼ��սCD�ļ�ʱ����ʱ����
	void Action();
	///@brief ����Ƿ������ս
	CheckResult Check(const CBaseDataManager& dataManger, int clearTime = 1);

	///@brief ��ȡ��ǰ����ս�Ĵ���
	int GetCurChallengeNum();

	///@brief ���õ�ǰ��Ʊ��
	void SetCurChallengeNum(int num);
	///@brief ���õ�ǰ����ս����
	void ReSetCurTicketNum(int num);

private:
	int m_iOneDayChanllengeCount;   //ÿ�տ���ս�Ĵ��� <0��ʾ������//
	int m_iChanllengedTimes;   //�ѱ���ս�Ĵ���//
	float m_fCurCD;         //�´ο���ս��CD,��ʱ����//
};

