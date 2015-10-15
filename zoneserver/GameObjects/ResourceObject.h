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
    //  virtual void DoAI();			// 执行普通AI
    //  virtual void DoMoveAI();		// 执行移动AI
    //  virtual BOOL ReplaceTempAction();
    //  virtual BOOL SetTempAction(EActionState act, LPVOID Param);

    virtual BOOL EndPrevAction();   // 上一个动作结束时的回调
    virtual BOOL SetCurAction();    // 根据备份数据设置当前的动作
    virtual BOOL DoCurAction();     // 执行当前的动作


public:
    CResourceObject(void);
    ~CResourceObject(void);

};