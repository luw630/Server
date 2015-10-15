#pragma once

//�Ṧ���ݱ�
class CFlyDataTable
{
public:
	enum {
		N_MAX_NAME_LEN = 128,
		N_MAX_INFOR_LEN = 1024,
	};

	enum{
		MaxLevel,					//���ȼ�
		DiffcultBase,				//�ѶȻ���
		DiffcultFactor,				//�Ѷ�ϵ��
		RemainTimeBase,				//����ʱ�����
		RemainTimeFactor,			//����ʱ��ϵ��
		JumpTileBase,				//��Ծ��������
		UpSpeedBase,				//�ƶ��ٶ���������
		UpSpeedFactor,				//�ƶ��ٶ�����ϵ��
		AGBase,						//����������
		AGFactor,					//������ϵ��
		ReduceCDBase,				//���͹�����ȴʱ�����
		ReduceCDFactor,				//���͹�����ȴʱ��ϵ��
		ReduceDelayBase,			//������ȴʱ�����
		ReduceDelayFactor,			//������ȴʱ��ϵ��
		AbsoluteParryBase,			//���Ӿ��Զ�������
		AbsoluteParryFactor,		//���Ӿ��Զ���ϵ��
		ConsumHPBase,				//��������ֵ����
		ConsumHPFactor,				//��������ֵϵ��
		ConsumMPBase,				//��������ֵ����
		ConsumMPFactor,				//��������ֵϵ��
		ConsumSPBase,				//��������ֵ����
		ConsumSPFactor,				//��������ֵϵ��
		ColdTime,					//��ȴʱ��


		E_FLY_MAX,
	};
	
	struct SFlyData
	{
		char name[N_MAX_NAME_LEN];
		int  school;				// ����
		DWORD  data[E_FLY_MAX];
		int	 type;
		char information[N_MAX_INFOR_LEN];		//����˵��
	};	
	typedef map<int, SFlyData>	TFLyMap;

	static bool Load( const char* filename);
	static void Destroy();

	static const SFlyData* GetData(int nId);

private:
	static TFLyMap	m_mFlyData;
};