// Environment.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���ǳ��õ��������ĵ���Ŀ�ض��İ����ļ�
//

#pragma once
// ��־���ֵ����
float GetExpScale();
void SetExpScale(float expScale);
// �����Ǯ������
float GetDropMoneyScale();
void SetDropMoneyScale(float moneyScale);
// ������Ʒ���ʱ���
float GetDropItemScale();
void SetDropItemScale(float itemScale);

BYTE  GetMultiTimeCount();
void  SetMultiTimeCount(BYTE multiTime);

// �Ƿ�ʹ���µ����Կ���ϵͳ
BOOL UseAttribAnti();

// ��ȡ������������
void LoadEnvironment();
