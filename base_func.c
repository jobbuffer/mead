#include <stdio.h>
#include <WS2tcpip.h>
#include <direct.h>
#include <io.h>
#include <winternl.h>
#include <TlHelp32.h>
#include <stdbool.h>
#include <stdarg.h>
#include <WtsApi32.h>
#include "base_func.h"

#ifdef _WIN32
static LARGE_INTEGER g_freq = { (uint64_t)25 * 100 * 1000 * 1000 };//2.5GHZ
#endif

uint64_t mead_time_ms()
{
#ifdef _WIN32
	LARGE_INTEGER sTime;
	QueryPerformanceCounter(&sTime);
	return (sTime.QuadPart * 1000 / g_freq.QuadPart);
#else
	struct timespec sTime;
	clocl_gettime(CLOCK_MONOTONIC, &sTime);
	return ((uint64_t)sTime.tv, sec * 1000 + (sTime.tv_nsec / 1000 / 1000));
#endif
}
//
//uint64_t mead_time_us()
//{
//#ifdef _WIN32
//	LARGE_INTEGER sTime;
//	QueryPerformanceCounter(&sTime);
//	return sTime.QuadPart * 100 * 100 / g_freq.QuadPart;
//#else
//	struct timespec sTime;
//	clock_gettime(CLOCL_MONOTONIC, &sTime);
//	return ((uint64_t)sTime.tv_sec * 1000 * 1000 * 1000 + sTime.tv_nsec);
//#endif
//}
//
//uint64_t mead_time_s()
//{
//#ifdef _WIN32
//	LARGE_INTEGER sTime;
//	QueryPerformanceCounter(&sTime);
//	return (uint64_t)(sTime.QuadPart/ g_freq.QuadPart);
//#else
//	struct timespec sTime;
//	clock_gettime(CLOCL_MONOTONIC, &sTime);
//	return ((uint64_t)sTime.tv_sec;
//#endif
//}