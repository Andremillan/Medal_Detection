/**
* @file TimeMeasure.h
*
* @brief This is a cross-platform time measuring utility.
*		 You just add a macro in a block to measure time.
*
*        e.g.
*			void foo()
*			{
*				__COUNT_MICRO_SECONDS_IN_FUNCTION__
*
*				//. Time consuming operations ...
*			}
*
*		 [Output] foo(162): elapsed average time is 34.176 micro sec.
*
*        You can view the result using syslog in Linux (e.g. $ tail /var/log/syslog),
*        or Debug window of MSVS or Dbgview.exe in Windows.
*
* @author Cholgyun Ri
* Contact: cholgyun@163.com, zj.li@batatech.com
*
* @version 1.0   2012-09-01     Windows version
* @version 1.2   2018-03-01     Add the average time measuring
* @version 2.0   2021-03-04     Cross-platform version
* @version 2.1   2021-04-02     Update the average time logic to remove outliers
*/


#ifndef TimeMeasure_h__
#define TimeMeasure_h__


#include <stdio.h>
#include <chrono>

//#define DEPLOYMENT
#define RECPECTIVE_NO_AVERAGE		false	// "true" measures the respective time, "false" measures the average time

#ifdef DEPLOYMENT  //. Do nothing

#define __COUNT_MICRO_SECONDS_IN_FUNCTION__
#define __COUNT_MICRO_SECONDS_IN_BLOCK__

#define __COUNT_MILLI_SECONDS_IN_FUNCTION__
#define __COUNT_MILLI_SECONDS_IN_BLOCK__

#define __CLEAR_TIMER__

#else //DEPLOYMENT  //. Measure time

#define __COUNT_MICRO_SECONDS_IN_FUNCTION__		TimeMeasure _c(__FUNCTION__, __LINE__, true);
#define __COUNT_MICRO_SECONDS_IN_BLOCK__		TimeMeasure _c(__FILE__, __LINE__, true);

#define __COUNT_MILLI_SECONDS_IN_FUNCTION__		TimeMeasure _c(__FUNCTION__, __LINE__, false);
#define __COUNT_MILLI_SECONDS_IN_BLOCK__		TimeMeasure _c(__FILE__, __LINE__, false);

#define __CLEAR_TIMER__							TimeMeasure::clearTimer();

#endif//DEPLOYMENT


#define PATH_MAX		260

class TimeMeasure {
private:
	char m_txtPrefix[PATH_MAX];
	char m_txtSuffix[20];
	char m_txtUnit[10];
	std::chrono::time_point<std::chrono::high_resolution_clock> m_start, m_end;
	double * m_totalTime;
	int * m_totalFrameCount;
	bool m_bMicroNoMilli;

private:
	void getID(char *_text);

public:
	TimeMeasure(const char *_text, int number, bool bMicroNoMilli = true);

	~TimeMeasure();

	_declspec (dllexport) static void clearTimer();
};
#endif // TimeMeasure_h__
