#pragma once

//��ҫ��������
enum GloryAddType
{
	GLORY_AYINLIANG,		// ��������
	GLORY_AJINBI,		// �������
	GLORY_AGLORYDOT,		// ������ҫ��

	GLORY_AMAX,
};

//��ҫ��������������
enum GloryConditionData
{
	GLORY_CLEVEL,	//��ҵȼ�
	GLORY_CCONTINUELOGIN,	//������½
	GLORY_CONLINE,		//����ʱ��
	GLORY_CCLEARANCETIMES,		//ͨ������
	GLORY_CKILLMONSTERNUM,		//ɱ����
	GLORY_CYINLIANGNUM,		//������
	GLORY_CYUANBAONUM,		//Ԫ����
	GLORY_CEQUITNUM,		//װ����
	GLORY_CEQUITINTENSIFYLEVEL,		//װ��ǿ���ȼ�
	GLORY_CEQUITGRADELEVEL,		//װ�����׵ȼ�
	GLORY_CEQUITSRARLEVEL,	//װ�����ǵȼ�
	GLORY_CEQUITKEYINGLEVEL,	//��ӡ�ȼ�
	GLORY_CGONGJI,		//������
	GLORY_CBIANSHENGNUM,		//�ɱ�����
	GLORY_CBELIEFSKILLNUM,	//ӵ�е�����������
	GLORY_CPACKAGENUM,		//������
	GLORY_CFINISHGLORYNUM,	//�����ҫ��


	GLORY_CMAX,
};

//�����������
enum GloryConditionID
{
	GLORY_CONLEVELID,	//��Ҵﵽx�ȼ�
	GLORY_CONONLINEID,	//�ۻ�����ʱ��
	GLORY_CONEQUITINTENSIFYLEVELID,	//ӵ��x��װ��ǿ����x��
	GLORY_CONEQUITGRADELEVELID,	//ӵ��x��װ�����׵�x��
	GLORY_CONEQUITSRARLEVELID,	//ӵ��x��װ�����ǵ�x��
	GLORY_CONEQUITKEYINGLEVELID,	//ӵ��x������x����ӡ��װ��
	GLORY_CONYINLIANGANDYUANBAOID,	//ӵ����Ϸ��x��ӵ��Ԫ��x
	GLORY_CONGONGJIID,	//ս�����ﵽx��
	GLORY_CONBIANSHENGNUMID,	//ӵ��x��������̬
	GLORY_CONBELIEFSKILLNUMID,	//�����������x��
	GLORY_CONPACKAGENUMID,	//����x������
	GLORY_CONFINISHGLORYNUMID,	//���x����ҫ

};


//��ҫ��������
class SGloryBaseData
{
public:
	SGloryBaseData() { memset(this, 0, sizeof(SGloryBaseData)); }

public:
	WORD m_GloryID;		//��ҫID
	char m_GloryName[128];	//��ҫ��
	BYTE m_GloryType;	//��ҫ����
	DWORD m_GloryIcon;		//��ҫͼ��
	BYTE m_GloryConditionIndex;	//������������
	char m_ConditionDetail[256];		//����˵��
	DWORD m_AwardFactor[GloryAddType::GLORY_AMAX];	//��������
	DWORD m_GetConditionFactor[GloryConditionData::GLORY_CMAX];	//��ȡ��ҫ����
};

//�ƺ���������˵��
enum TitleAddType
{
	TITLE_AHP,		//����Ѫ��
	TITLE_AGONGJI,		//���ӹ���
	TITLE_AFANGYU,		//���ӷ���

	TITLE_AMAX,
};

//�ƺ���������
enum TitleConsumeType
{
	TITLE_CGLORYDOT,		//������ҫ��
	TITLE_CGLORYITEMNUM,	//������ҫ������

	TITLE_CMAX
};

//�ƺŻ�������
class STitleBaseData
{
public:
	STitleBaseData() { memset(this, 0, sizeof(STitleBaseData)); }

public:
	
	WORD m_TitleID;		//�ƺ�ID
	char m_TitleName[128];	//�ƺ���
	DWORD m_ConsumeItemID;	//���ĵ��ߵ�ID
	DWORD m_TitleIcon;		//�ƺ�ͼ��
	DWORD m_AddFactor[TitleAddType::TITLE_AMAX];	//��������
	DWORD m_ConsumeFactor[TitleConsumeType::TITLE_CMAX];	//��������
	char m_TitleDescript[256];		//�ƺ�����
};
