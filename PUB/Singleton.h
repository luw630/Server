/*
** Author:	邓超
** QQ:		23427470
** Time:	
** version:	1.1

** Fantasy Game Engine 1.0
** Copyright (C) 2005-2010, Fantasy Games
** Kernel functions

*/

#pragma once
#pragma optimize("", off)
#include <assert.h>
#include <memory.h>
//单件模板,通过对单件的模板和函数式宏封装,统一了单件的调用方式,减少了代码和编码时间.
template <typename T>
class CSingleton
{
public:
	CSingleton( ){ }
	virtual ~CSingleton(){ }
public:
	static T* Instance( )
	{
		static std::auto_ptr<T> ptrInstance;
		if(ptrInstance.get() == 0)
			ptrInstance.reset( new T() ); 	
		return ptrInstance.get();
	}
	static T* getSingleton( )
	{
		return Instance();
	}
private:
};
//
template <typename T, int ID>
class CSinglecall
{
public:
	CSinglecall( ){	m_instanceID = ID;	}
	virtual ~CSinglecall(){	}
	int	GetInstanceID( ){ return m_instanceID; }
public:
	static T* Instance( )
	{
		static std::auto_ptr<T> ptrInstance;
		if(ptrInstance.get() == 0)
			ptrInstance.reset( new T() ); 	
		return ptrInstance.get();
	}
	static T* getInstance( )
	{
		return Instance();
	}
private:

	int m_instanceID;
};

#define INSTANCE(CLASS) CSingleton< CLASS >::Instance( /*#CLASS*/ )
#define SINGLECALL(CLASS,ID) CSinglecall< CLASS,ID >::Instance( /*#CLASS*/ )
#define SINGLETON(CLASS) friend CSingleton< CLASS >

#pragma optimize("", on)