//----------------------------------------------------------------------------
/** @file SgTime.cpp
    @see SgTime.h.
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTime.h"

#include <cstring>
#include <ctime>
#include <iomanip>
#include <limits>
#include <iostream>
#include <sstream>
#if UNIX
#include <errno.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#endif
#include "SgException.h"

using namespace std;

//----------------------------------------------------------------------------

namespace {

SgTimeMode g_defaultMode = SG_TIME_CPU;

bool g_isInitialized = false;

clock_t g_ticksPerSecond;

clock_t g_ticksPerMinute;

void Init()
{
#if UNIX
    int ticksPerSecond = sysconf(_SC_CLK_TCK);
    if (ticksPerSecond < 0) // Shouldn't happen
    {
        throw SgException("Could not get _SC_CLK_TCK.");
    }
    g_ticksPerSecond = static_cast<clock_t>(ticksPerSecond);
#else
#error "Time functions not implemented"
#endif
    g_ticksPerMinute = 60 * g_ticksPerSecond;
    g_isInitialized = true;
}

} // namespace

//----------------------------------------------------------------------------

string SgTime::Format(double time, bool minsAndSecs)
{
    ostringstream out;
    if (minsAndSecs)
    {
        int mins = static_cast<int>(time / 60);
        int secs = static_cast<int>(time - mins * 60);
        out << setw(2) << mins << ':' << setw(2) << setfill('0') 
            << secs;
    }
    else
        out << setprecision(2) << fixed << time;
    return out.str();
}

double SgTime::Get()
{
    return Get(g_defaultMode);
}

double SgTime::Get(SgTimeMode mode)
{
    if (! g_isInitialized)
        Init();
#if UNIX
    switch (mode)
    {
    case SG_TIME_CPU:
        {
            struct tms buf;
            if (times(&buf) == static_cast<clock_t>(-1))
            {
                std::cerr << "Time measurement overflow.\n";
                return 0;
            }
            clock_t clockTicks =
                buf.tms_utime + buf.tms_stime
                + buf.tms_cutime + buf.tms_cstime;
            return static_cast<double>(clockTicks) / g_ticksPerSecond;
        }
    case SG_TIME_REAL:
        {
            struct timeval timeVal;
            if (gettimeofday(&timeVal, 0) != 0)
                throw SgException(string("gettimeofday: ") + strerror(errno));
            return (timeVal.tv_sec
                    + static_cast<double>(timeVal.tv_usec) / 1e6);
        }
    default:
        SG_ASSERT(false);
        return 0;
    }
#else
#error "Time functions not implemented"
#endif
}

SgTimeMode SgTime::DefaultMode()
{
    return g_defaultMode;
}

void SgTime::SetDefaultMode(SgTimeMode mode)
{
    g_defaultMode = mode;
}

string SgTime::TodaysDate()
{
    time_t systime = time(0);
    struct tm* currtime = localtime(&systime);
    const int bufSize = 14;
    char buf[bufSize];
    strftime(buf, bufSize - 1, "%Y-%m-%d", currtime);
    return string(buf);
}

//----------------------------------------------------------------------------

