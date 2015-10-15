#pragma once
class ThreadWrapper
{
public:
	ThreadWrapper(void);
	~ThreadWrapper(void);
private:
	HANDLE hndThread;
	DWORD  nThreadid;

};
