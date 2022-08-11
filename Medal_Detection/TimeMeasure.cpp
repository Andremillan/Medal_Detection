#include "stdafx.h"
#include "TimeMeasure.h"

#include <vector>
#include <algorithm>
#include <string>
#include <cstring>

#define FIRST_AVG_COUNT			(10)
#define OUTLIER_THR				(2)


double g_totalTimeBuf[PATH_MAX] = { 0 };
int g_totalFrameCountBuf[PATH_MAX] = { 0 };

std::vector<std::string> g_idList;
intptr_t g_id;


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
void _log(const char * logstr)
{
	OutputDebugStringA(logstr);
	//printf(logstr);
}
#else
#include <syslog.h>
void _log(const char * logstr)
{
	syslog(LOG_INFO, "%s", logstr);
}
#endif


TimeMeasure::TimeMeasure(const char *_text, int number, bool bMicroNoMilli)
	: m_bMicroNoMilli(bMicroNoMilli)
{
	char w_txt[PATH_MAX];
	snprintf(w_txt, PATH_MAX, "%s(%d)", _text, number);

	snprintf(m_txtPrefix, PATH_MAX, "%s", w_txt);
	if (bMicroNoMilli)
	{
		snprintf(m_txtUnit, 10, "micro");
	}
	else
	{
		snprintf(m_txtUnit, 10, "milli");
	}
	snprintf(m_txtSuffix, 20, "elapsed");

#if RECPECTIVE_NO_AVERAGE
#else
	getID(w_txt);
#endif

	m_start = std::chrono::high_resolution_clock::now();
}

TimeMeasure::~TimeMeasure()
{
	double w_time;

	m_end = std::chrono::high_resolution_clock::now();
	char logstr[PATH_MAX * 4];
	if (m_bMicroNoMilli)
	{
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start);
		w_time = (double)duration.count();
	}
	else
	{
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(m_end - m_start);
		w_time = (double)duration.count();
	}

#if RECPECTIVE_NO_AVERAGE // Respective
	sprintf_s(logstr, 1024, "%s: %s time is %3.3f %s sec.\n", m_txtPrefix, m_txtSuffix, w_time, m_txtUnit);
#else // Average
	if (*m_totalFrameCount > FIRST_AVG_COUNT &&
		*m_totalTime * OUTLIER_THR < w_time * *m_totalFrameCount)
	{//... Outliers
		snprintf(logstr, 1024, "%s: %s (abnormal) time is %3.3f %s sec.\n", m_txtPrefix, m_txtSuffix, w_time, m_txtUnit);
	}
	else
	{//... Inliers
		(*m_totalFrameCount)++;
		*m_totalTime += w_time;

		w_time = *m_totalTime / (*m_totalFrameCount);
		snprintf(logstr, 1024, "%s: %s average time is %3.3f %s sec.\n", m_txtPrefix, m_txtSuffix, w_time, m_txtUnit);
	}
#endif

	_log(logstr);
}

void TimeMeasure::getID(char *_text)
{
	typedef std::vector<std::string>::iterator IdItr;
	IdItr itr;

	std::string strId(_text);
	itr = std::find(g_idList.begin(), g_idList.end(), strId);
	intptr_t index = std::distance(g_idList.begin(), itr);
	if (itr != g_idList.end())
	{
		g_id = index;
	}
	else
	{
		g_id = index;
		g_idList.push_back(strId);
	}

	m_totalTime = g_totalTimeBuf + g_id;
	m_totalFrameCount = g_totalFrameCountBuf + g_id;
}

void TimeMeasure::clearTimer()
{
	memset(g_totalTimeBuf, 0, PATH_MAX * sizeof(*g_totalTimeBuf));
	memset(g_totalFrameCountBuf, 0, PATH_MAX * sizeof(*g_totalFrameCountBuf));
	g_idList.clear();
}
