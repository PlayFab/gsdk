//#include "pch.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <time.h>

struct FTimeHelper
{
public:

    FTimeHelper()
    {
        LARGE_INTEGER Frequency;
        QueryPerformanceFrequency(&Frequency);
        SecondsPerCycle = 1.0 / Frequency.QuadPart;
    }

    __inline double GetTimeSeconds()
    {
        LARGE_INTEGER Cycles;
        QueryPerformanceCounter(&Cycles);

        // add big number to make bugs apparent where return value is being passed to float
        return Cycles.QuadPart * SecondsPerCycle + 16777216.0;
    }

    void UtcTime(int& Year, int& Month, int& DayOfWeek, int& Day, int& Hour, int& Min, int& Sec, int& MSec)
    {
        SYSTEMTIME st;
        GetSystemTime(&st);

        Year = st.wYear;
        Month = st.wMonth;
        DayOfWeek = st.wDayOfWeek;
        Day = st.wDay;
        Hour = st.wHour;
        Min = st.wMinute;
        Sec = st.wSecond;
        MSec = st.wMilliseconds;
    }

    __inline unsigned long long TicksFromFileTime(const FILETIME& Filetime)
    {
        const unsigned long long NumTicks = (unsigned long long(Filetime.dwHighDateTime) << 32) + Filetime.dwLowDateTime;
        return NumTicks;
    }

    __inline double FiletimeToSeconds(const FILETIME& Filetime)
    {
        const unsigned long long TicksPerSecond = 10000000;
        return double(TicksFromFileTime(Filetime)) / double(TicksPerSecond);
    }

    void GetCPUTimePctRelative(double& InOutTotalProcessTime, double& InOutTotalUserAndKernelTime, float& InOutCPUTimePctRelative)
    {
        // Note we need to throttle the update frequency, since the numbers returned by GetProcessTimes are inaccurate when called over short timeframes.....  boo :(
        static double LastUpdateTime = GetTimeSeconds();
        const double CurrentTime = GetTimeSeconds();
        const double TimeSinceLastUpdate = CurrentTime - LastUpdateTime;
        const double UpdatePeriod = 0.25;
        if (TimeSinceLastUpdate < UpdatePeriod)
        {
            return;
        }
        LastUpdateTime = CurrentTime;

        FILETIME CreationTime = { 0 };
        FILETIME ExitTime = { 0 };
        FILETIME KernelTime = { 0 };
        FILETIME UserTime = { 0 };
        FILETIME CurrentProcessTime = { 0 };

        ::GetProcessTimes(::GetCurrentProcess(), &CreationTime, &ExitTime, &KernelTime, &UserTime);
        ::GetSystemTimeAsFileTime(&CurrentProcessTime);

        const double CurrentKernelTime = FiletimeToSeconds(KernelTime);
        const double CurrentUserTime = FiletimeToSeconds(UserTime);

        const double CurrentTotalUserAndKernelTime = CurrentKernelTime + CurrentUserTime;
        const double CurrentTotalProcessTime = FiletimeToSeconds(CurrentProcessTime) - FiletimeToSeconds(CreationTime);

        const double IntervalProcessTime = CurrentTotalProcessTime - InOutTotalProcessTime;
        const double IntervalUserAndKernelTime = CurrentTotalUserAndKernelTime - InOutTotalUserAndKernelTime;

        InOutCPUTimePctRelative = (float)(IntervalUserAndKernelTime / IntervalProcessTime) * 100.0f;
        InOutTotalProcessTime = CurrentTotalProcessTime;
        InOutTotalUserAndKernelTime = CurrentTotalUserAndKernelTime;
    }

private:
    double SecondsPerCycle = 0.0f;
};
FTimeHelper gTimeHelper = FTimeHelper();

#pragma optimize("", off)
volatile int BusyWaitInt = 1;
volatile float BusyWaitfloat = 1.0f;
void DoSomePretendWork()
{
    for (int i = 0; i < 5000; ++i)
    {
        BusyWaitInt *= BusyWaitInt;
        BusyWaitfloat *= BusyWaitfloat;
    }
}
#pragma optimize("", on)

int BusyWaitUntil(double EndOfBusyWaitTime)
{
    int UnitsOfWorkCompleted = 0;
    while (EndOfBusyWaitTime > gTimeHelper.GetTimeSeconds())
    {
        DoSomePretendWork();
        ++UnitsOfWorkCompleted;
    }
    return UnitsOfWorkCompleted;
}

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cout << "Usage: " << argv[0] << " <LogFileName> <FramesPerSecond> <WorkTimeMSPerFrame> <SleepZeroTimeMS>\n";
        return 0;
    }

    std::ofstream ofs(argv[1]);
    if (!ofs.good())
    {
        std::cout << "Failed to create log file " << argv[1] << "\n";
        return 0;
    }

    const double FramesPerSecond = atof(argv[2]);
    const double WorkTimeMSPerFrame = atof(argv[3]);
    const double SleepZeroTimeMS = atof(argv[4]);
    if (FramesPerSecond <= 0.0 || WorkTimeMSPerFrame <= 0.0)
    {
        std::cout << "Invalid values for FramesPerSecond and/or WorkTimeMSPerFrame!\n";
        return 0;
    }

    std::clog.rdbuf(ofs.rdbuf());

    TIMECAPS tc;
    if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
    {
        std::cout << "Error, call to \"timeGetDevCaps\" failed!\n";
        std::clog << "Error, call to \"timeGetDevCaps\" failed!\n";
        return 0;
    }

    // Set granularity of sleep and such as low as possible.
    if (timeBeginPeriod(tc.wPeriodMin) != TIMERR_NOERROR)
    {
        std::cout << "Error, call to \"timeBeginPeriod\" failed!\n";
        std::clog << "Error, call to \"timeBeginPeriod\" failed!\n";
        return 0;
    }

    std::cout << "Set timer granularity to " << tc.wPeriodMin << "ms.\n";

    std::cout << "Server Log Simulator started! Running at " << FramesPerSecond << " fps with " << WorkTimeMSPerFrame << "ms work per frame.  Capturing data to " << argv[1] << "\n";
    std::clog << "Frame Start Time,UnitsOfWorkCompleted,Expected Work Time,Expected SleepTime,Expected SleepZeroTime,Expected TotalSleepTime,Expected Frame Time,Actual Work Time,Actual Sleep Time,Actual SleepZero Time,Actual TotalSleep Time,Actual Frame Time,Work Delta,Sleep Delta,SleepZero Delta,TotalSleep Delta,Frame Delta,CPUTimePctRelative\n";

    double WorstWorkTimeDiff = 0.0f;
    double WorstSleepTimeDiff = 0.0f;
    double WorstSleepZeroTimeDiff = 0.0f;
    double WorstTotalSleepTimeDiff = 0.0f;
    double WorstFrameTimeDiff = 0.0f;

    const double ExpectedWorkTime = 0.001 * WorkTimeMSPerFrame;
    const double ExpectedFrameTime = 1.0 / FramesPerSecond;
    const double SleepZeroTime = 0.001 * SleepZeroTimeMS;
    const double ProcessStartTime = gTimeHelper.GetTimeSeconds();

    double LastTotalProcessTime = 0.0f;
    double LastTotalUserAndKernelTime = 0.0f;
    float LastCPUTimePctRelative = 0.0f;
    gTimeHelper.GetCPUTimePctRelative(LastTotalProcessTime, LastTotalUserAndKernelTime, LastCPUTimePctRelative);

    double FrameStartTime = ProcessStartTime;
    while (true)
    {
        // Pretend we are doing some frame work
        const double EndOfBusyWaitTime = FrameStartTime + ExpectedWorkTime;
        int UnitsOfWorkCompleted = BusyWaitUntil(EndOfBusyWaitTime);
        const double WorkTimeEnd = gTimeHelper.GetTimeSeconds();

        // Now sleep until the next frame
        const double SleepTimeStart = gTimeHelper.GetTimeSeconds();
        const double ExpectedFrameEndTime = FrameStartTime + ExpectedFrameTime;
        const double ExpectedTotalSleepTime = max(ExpectedFrameEndTime - SleepTimeStart, 0.0);
        double SleepTimeEnd = SleepTimeStart;

        const double ExpectedSleepTime = max(ExpectedTotalSleepTime - SleepZeroTime, 0.0);
        const double ExpectedSleepZeroTime = min(ExpectedTotalSleepTime, SleepZeroTime);
        if (ExpectedSleepTime > 0.0)
        {
            Sleep((DWORD)(ExpectedSleepTime * 1000.0));
            SleepTimeEnd = gTimeHelper.GetTimeSeconds();
        }

        const double SleepZeroTimeStart = gTimeHelper.GetTimeSeconds();
        double SleepZeroTimeEnd = SleepZeroTimeStart;
        if (ExpectedSleepZeroTime > 0.0)
        {
            while (gTimeHelper.GetTimeSeconds() < ExpectedFrameEndTime)
            {
                Sleep(0);
            }
            SleepZeroTimeEnd = gTimeHelper.GetTimeSeconds();
        }
        const double TotalSleepTimeEnd = gTimeHelper.GetTimeSeconds();

        // Gather all of our statistics
        const double FrameEndTime = gTimeHelper.GetTimeSeconds();
        const double ActualWorkTime = WorkTimeEnd - FrameStartTime;
        const double WorkTimeDiff = ActualWorkTime - ExpectedWorkTime;
        const double ActualSleepTime = SleepTimeEnd - SleepTimeStart;
        const double SleepTimeDiff = ActualSleepTime - ExpectedSleepTime;
        const double ActualSleepZeroTime = SleepZeroTimeEnd - SleepZeroTimeStart;
        const double SleepZeroTimeDiff = ActualSleepZeroTime - ExpectedSleepZeroTime;
        const double ActualTotalSleepTime = TotalSleepTimeEnd - SleepTimeStart;
        const double TotalSleepTimeDiff = ActualTotalSleepTime - ExpectedTotalSleepTime;
        const double ActualFrameTime = FrameEndTime - FrameStartTime;
        const double FrameTimeDiff = ActualFrameTime - ExpectedFrameTime;
        gTimeHelper.GetCPUTimePctRelative(LastTotalProcessTime, LastTotalUserAndKernelTime, LastCPUTimePctRelative);

        std::clog << FrameStartTime - ProcessStartTime << "," << UnitsOfWorkCompleted << "," << 1000.0 * ExpectedWorkTime << "," << 1000.0 * ExpectedSleepTime << "," << 1000.0 * ExpectedSleepZeroTime << "," << 1000.0 * ExpectedTotalSleepTime << "," << 1000.0 * ExpectedFrameTime << "," << 1000.0 * ActualWorkTime << "," << 1000.0 * ActualSleepTime << "," << 1000.0 * ActualSleepZeroTime << "," << 1000.0 * ActualTotalSleepTime << "," << 1000.0 * ActualFrameTime << "," << 1000.0 * WorkTimeDiff << "," << 1000.0 * SleepTimeDiff << "," << 1000.0 * SleepZeroTimeDiff << "," << 1000.0 * TotalSleepTimeDiff << "," << 1000.0 * FrameTimeDiff << "," << LastCPUTimePctRelative << "\n";

        if (WorkTimeDiff > WorstWorkTimeDiff || SleepTimeDiff > WorstSleepTimeDiff || SleepZeroTimeDiff > WorstSleepZeroTimeDiff || TotalSleepTimeDiff > WorstTotalSleepTimeDiff || FrameTimeDiff > WorstFrameTimeDiff)
        {
            WorstWorkTimeDiff = max(WorkTimeDiff, WorstWorkTimeDiff);
            WorstSleepTimeDiff = max(SleepTimeDiff, WorstSleepTimeDiff);
            WorstSleepZeroTimeDiff = max(SleepZeroTimeDiff, WorstSleepZeroTimeDiff);
            WorstTotalSleepTimeDiff = max(TotalSleepTimeDiff, WorstTotalSleepTimeDiff);
            WorstFrameTimeDiff = max(FrameTimeDiff, WorstFrameTimeDiff);

            int Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;
            gTimeHelper.UtcTime(Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec);

            std::cout << "New worst metric! Work Delta " << 1000.0 * WorstWorkTimeDiff << "ms, Sleep Delta: " << 1000.0 * WorstSleepTimeDiff << "ms, SleepZero Delta: " << 1000.0 * WorstSleepZeroTimeDiff << "ms, TotalSleep Delta: " << 1000.0 * WorstTotalSleepTimeDiff << "ms, Frame Delta: " << 1000.0 * WorstFrameTimeDiff << "ms,  at " << Year << "." << Month << "." << Day << "-" << Hour << "." << Min << "." << Sec << ":" << MSec << "\n";
        }

        // Update in preparation for the next loop
        FrameStartTime = FrameEndTime;
    }
}
