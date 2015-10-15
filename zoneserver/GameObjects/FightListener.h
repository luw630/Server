#pragma once

#include "fightObject.h"
#include "player.h"

// ս���������Ա仯������
class AttribListener: public CFightListener
{
public:
	AttribListener(CPlayer* pObserver);

	virtual void CALLBACK OnExchange(CFightObject* pObj);
	virtual void CALLBACK SetSubject(CFightObject* pSubject);
	virtual void CALLBACK OnClose(CFightObject* pObj);

private:
	void sendAttribute( );

private:
	CPlayer	 *m_pObserver;
};