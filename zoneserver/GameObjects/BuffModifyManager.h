#ifndef BUFFMODIFYMANAGER_H
#define BUFFMODIFYMANAGER_H

class BuffModify;

// AI�������ϵ����Ըı�Buff������
class ModifyBuffManager
{
public:
	typedef std::map<WORD, BuffModify*> BuffList;

public:
	BuffManager() : m_curObj(0){};
	~BuffManager();

	void Init(CFightObject *object);
	bool BuildStub(BuffModify *e);
	void ClearStub(BuffModify *e);
	bool AddBuff(WORD id);
	bool IsOwnerBuff(WORD id);
	void Free();

private:
	BuffList		m_buffList;
	CFightObject	*m_curObj;
};

#endif //BUFFMODIFYMANAGER_H