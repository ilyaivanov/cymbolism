#include "types.h"

#define PRINT OutputDebugStringA

typedef enum PerfMetric
{
    StartUp,
    FontInitialization,
    FontKerningTablesInitialization,
    FontTexturesInitialization,

    NumberOfPerfMetrics
} PerfMetric;

typedef struct PerformanceMetrics
{
    u64 perf[NumberOfPerfMetrics];
    u64 runningMetrics[NumberOfPerfMetrics];
    u64 count[NumberOfPerfMetrics];
} PerformanceMetrics;

PerformanceMetrics metrics = {0};

void Start(PerfMetric metric)
{
    metrics.runningMetrics[metric] = __rdtsc();
}

void Stop(PerfMetric metric)
{
    metrics.perf[metric] += __rdtsc() - metrics.runningMetrics[metric];
    metrics.count[metric] += 1;
}

#define PROC_FREQUENCY 3000000000.0f

void PrintRegularMetric(char* buff, char *message, PerfMetric metric)
{
    float ms = (float)metrics.perf[metric] / PROC_FREQUENCY * 1000;
    sprintf(buff, "%s %-12lld  %.2fms\n", message, metrics.perf[metric], ms);
    PRINT(buff);
}

void PrintRelativeMetric(char* buff, char *message, PerfMetric metric, PerfMetric parentMetric)
{
    float percents = (float)metrics.perf[metric] / (float)metrics.perf[parentMetric] * 100;
    float ms = (float)metrics.perf[metric] / PROC_FREQUENCY * 1000;
    sprintf(buff, "%s %-12lld  %.2fms   %.2f%%\n", message, metrics.perf[metric], ms, percents);
    PRINT(buff);
}

void PrintStartupResults()
{
    char buff[512] = {0};

    PrintRegularMetric(buff,  "Startup       :", StartUp);
    PrintRelativeMetric(buff, " - Font Init  :", FontInitialization, StartUp);
    PrintRelativeMetric(buff, "  -- Kerning  :", FontKerningTablesInitialization, FontInitialization);
    PrintRelativeMetric(buff, "  -- Textures :", FontTexturesInitialization, FontInitialization);
}