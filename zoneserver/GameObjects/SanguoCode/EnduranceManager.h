// ------ EnduranceManager.h -----
//	--	 Author : LiuWeiWei	  --
//	--	 Date : 03/18/2015    --
//	--	 Desc : �����ظ������� --
// -------------------------------

#pragma once
#include "stdint.h"

struct SEnduranceData;

class CEnduranceManager
{
public:
	CEnduranceManager();
	~CEnduranceManager();
	/**@brief ��ʼ���������ݼ���������ߺ���Ҫ�ظ�������
	@param [in] ��������
	@param [in] bGMMark GM��ұ�־
	@param [in] newPlayer ����ұ�־
	@param [in] playerLevel ��ҵȼ�
	*/
	void Init(SEnduranceData* pData, bool bGMMark, bool newPlayer, const int32_t playerLevel);
	/**@brief �������ֵ
	@param [in] value ��ӵ�����ֵ ��Ϊ���� Ϊ��������ȡ��0�Ƚϵ����ֵ
	@param [in] playerLevel ��ҵȼ�
	@param [in] globalLimmitted �Ƿ���ȫ�����õ���������������������Ϊfalse���þ����ȼ�����Ӧ���������������ƾ���������
	*/
	void PlusEndurance(const int32_t value, const int32_t playerLevel, bool globalLimit = true);
	/**@brief �������ֵ
	@note �ȼ���Ӧ�ûظ�������ֵ
	@param [in] playerLevel ��ҵȼ�,�����GM�����������ֵΪȫ�����õ���������,����Ϊ��Ӧ�����ȼ�����������
	@param [in] compareValue �����Ƚ�ֵ
	@return �������������ֵ���ڱȽ�ֵ�򷵻�true ����false
	*/
	bool Check(const int32_t playerLevel, int32_t compareValue = 0);
	/**@brief ��ȡ����ֵ
	@note ���ô˽ӿڲ����Զ�����ظ�������ֵ
	*/
	DWORD GetEndurance() const;
	/**@brief ��ȡ��������
	@param [in] playerLevel ��ҵȼ�
	@param [in] globalLimit �Ƿ�Ϊȫ������
	*/
	int32_t GetEnduranceLimit(const int32_t playerLevel, bool globalLimit) const;
private:
	/**@brief �����ʱ�����ϴμ�������ظ�������ֵ
	@return �����ɻظ�������ֵ
	*/
	int32_t Recover();
private:
	SEnduranceData* m_pEnduranceData;	///<��������/
	__time64_t m_tLastCheckRecoverTime;		///<��һ�μ�������ظ���ʱ��/
	__time64_t m_tCurTime;				///<���ڻ�ȡ��ǰʱ��
	int32_t m_iRecoverSeconds;			///<�����ظ���������/
	int32_t m_iRecoverPoint;			///<��ǰ�ظ���������/
	int32_t m_iEnduranceLimit;			///<��������/
	bool m_bGMMark;						///<GM��ұ�־/
	bool m_bInitFinished;				///<��ʼ����ɱ�־/
};

