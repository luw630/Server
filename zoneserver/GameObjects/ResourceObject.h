#pragma    once

#include "objectservice\include\iobject.h"
class CResourceObject : public IObject 
{
public:
	int AddObject(LPIObject pChild) { return 1; }
	int DelObject(LPIObject pChild) { return 1; }
    void OnClose() {}

protected:
    int OnCreate(_W64 long pParameter) { return 1; }
    void OnRun(void);

public:
    //  virtual void DoAI();			// ִ����ͨAI
    //  virtual void DoMoveAI();		// ִ���ƶ�AI
    //  virtual BOOL ReplaceTempAction();
    //  virtual BOOL SetTempAction(EActionState act, LPVOID Param);

    virtual BOOL EndPrevAction();   // ��һ����������ʱ�Ļص�
    virtual BOOL SetCurAction();    // ���ݱ����������õ�ǰ�Ķ���
    virtual BOOL DoCurAction();     // ִ�е�ǰ�Ķ���


public:
    CResourceObject(void);
    ~CResourceObject(void);

};