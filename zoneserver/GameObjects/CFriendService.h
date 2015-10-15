#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <hash_map>

class CFriendService
{
	struct SFriendData
	{
		WORD		    nlevel;					//�ȼ�
		DWORD	    nrewarditemid;		//������ƷID			
	    WORD			nOnlineTime;		//����ʱ����
		WORD			ntotalCount;			//�ۼƴ���
		DWORD		nrewardExp;			//���ߺ��ѽ�������
	};
	typedef std::hash_map<WORD, SFriendData *> FriendBaseData;
private:
	CFriendService(){}
	bool LoadFriendBaseData();
public:
	static CFriendService& GetInstance()
	{
		static CFriendService instance;
		return instance;
	}

	bool ReLoad();
	bool Init();
	~CFriendService() { CleanUp(); }

 	const SFriendData *GetFriendBaseData(WORD levle) const; ///��ȡ���ѽ�������
	const DWORD	GetFriendRewarditem(WORD levle) const;//��ȡ�ȼ�������ƷID
	const DWORD	GetFriendRewardExp(WORD levle) const;//��ȡ�ȼ���������
	const DWORD	GetFriendRewardTime(WORD levle) const;//��ȡ�ȼ�����ʱ����
	const WORD		GetFriendRewardCount(WORD levle) const;//��ȡ�ȼ�����ʱ����
// 	const SFightPetRise *GetFightPetRise(BYTE Rare) const;//��ȡ�ɳ�����
// 	const QWORD GetFightPetLevelExp(WORD Level,BYTE index=1)	const;//��ȡ���������þ���
private:
	void CleanUp();

	// ��ֹ��������Ϳ�����ֵ
	CFriendService(CFriendService &);
	CFriendService& operator=(CFriendService &);

private:
	FriendBaseData m_baseData;			// ���ѽ�����������
};

#pragma warning(pop)