#pragma once

class CProtectedDataTable
{
public:
	enum{ E_N_MAX_NAME_LEN=128,
		E_N_MAX_INFOR_LEN = 1024,
		};

	enum {
		MaxLevel,					//���ȼ�
		DiffcultBase,				//�ѶȻ���
		DiffcultFactor,				//�Ѷ�ϵ��
		RemainTimeBase,				//����ʱ�����
		RemainTimeFactor,			//����ʱ��ϵ��
		OutDefenceBase,				//�������
		OutDefenceFactor,			//���ϵ��
		InnerDefenceBase,			//�ڷ�����
		InnerDefenceFactor,			//�ڷ�ϵ��
		HPLimitedBase,				//�������޻���
		HPLimitedFactor,			//��������ϵ��
		ReduceHurtRateBase,			//�����˺���������
		ReduceHurtRateFactor,		//�����˺�����ϵ��
		ReduceAbsoluteHurtBase,		//��������˺�����
		ReduceAbsoluteHurtFactor,	//��������˺�ϵ��
        CountAttackRateBase,		//�����˺���������
		CountAttackRateFactor,		//�����˺�����ϵ��
		ConsumHPBase,				//��������ֵ����
		ConsumHPFactor,				//��������ֵϵ��
		ConsumMPBase,				//��������ֵ����
		ConsumMPFactor,				//��������ֵϵ��
		ConsumSPBase,				//��������ֵ����
		ConsumSPFactor,				//��������ֵϵ��
		ColdTime,					//��ȴʱ��

		PDT_MAX,
	};

	struct ProtectedData
	{
		char name[E_N_MAX_NAME_LEN];
		int  school;					//����
		DWORD  data[PDT_MAX];
		int  type;
		char information[E_N_MAX_INFOR_LEN];		//����˵��
	};
	typedef map<int, ProtectedData> TPDMap;

	static bool Load(const char* filename);
	static void Destroy();

	static const ProtectedData* GetData( int id );
protected:
	static TPDMap m_mProtectedData;
};