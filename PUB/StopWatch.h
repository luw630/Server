// StopWatch.h: interface for the CStopWatch class.
//
//////////////////////////////////////////////////////////////////////
#if !defined(StopWatch_h)
#define StopWatch_h
#include <windows.h>
class CStopWatch  
{
public:
	CStopWatch()
	{
		m_nStartTime.QuadPart =0 ;
		QueryPerformanceFrequency(&m_nFrequency);

	};
	~CStopWatch()
	{

	};	
	void tStart()
	{
     	QueryPerformanceCounter(&m_nStartTime);
	};
	double tEnd()
	{
		LARGE_INTEGER nStopTime; 
		QueryPerformanceCounter( &nStopTime);
		double dTime((float)(nStopTime.QuadPart - m_nStartTime.QuadPart)/m_nFrequency.QuadPart);
		return dTime;
	}
	bool isValid(){return true;}
		;
private:
    LARGE_INTEGER m_nFrequency;
	LARGE_INTEGER m_nStartTime;

};

#endif // StopWatch_h
