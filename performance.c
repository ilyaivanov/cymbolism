#include "types.h"

#define PRINT OutputDebugStringA

typedef enum PerfMetric
{
    StartUp,
    FontInitialization,
    FontKerningTablesInitialization,
    FontTexturesInitialization,

    FrameTotal,
    FramePrintText,
    FramePrintTextFindKerning,
    FramePrintTextDrawTexture,

    NumberOfPerfMetrics
} PerfMetric;

typedef struct PerformanceMetrics
{
    u64 perf[NumberOfPerfMetrics];
    u64 runningMetrics[NumberOfPerfMetrics];
    u64 count[NumberOfPerfMetrics];
} PerformanceMetrics;

PerformanceMetrics metrics = {0};

inline void Start(PerfMetric metric)
{
    metrics.runningMetrics[metric] = __rdtsc();
}

inline void Stop(PerfMetric metric)
{
    metrics.perf[metric] += __rdtsc() - metrics.runningMetrics[metric];
    metrics.count[metric] += 1;
}

#define PROC_FREQUENCY 3000000000.0f

void PrintRegularMetric(char* buff, char *message, PerfMetric metric)
{
    float ms = (float)metrics.perf[metric] / PROC_FREQUENCY * 1000;
    sprintf(buff, "%s %-12lld  %05.2fms\n", message, metrics.perf[metric], ms);
    PRINT(buff);
}

void PrintRelativeMetric(char* buff, char *message, PerfMetric metric, PerfMetric parentMetric)
{
    float percents = (float)metrics.perf[metric] / (float)metrics.perf[parentMetric] * 100;
    float ms = (float)metrics.perf[metric] / PROC_FREQUENCY * 1000;
    i32 count = metrics.count[metric];
    sprintf(buff, "%s %-12lld  %05.2fms  %-6d %.2f%%\n", message, metrics.perf[metric], ms, count, percents);
    PRINT(buff);
}

void PrintStartupResults()
{
    char buff[512] = {0};

    PrintRegularMetric (buff, "Startup       :", StartUp);
    PrintRelativeMetric(buff, " - Font Init  :", FontInitialization, StartUp);
    PrintRelativeMetric(buff, "  -- Kerning  :", FontKerningTablesInitialization, FontInitialization);
    PrintRelativeMetric(buff, "  -- Textures :", FontTexturesInitialization, FontInitialization);
    
}

void ResetMetrics()
{
    memset(&metrics, 0, sizeof(metrics));
}

void PrintFrameStats()
{
    char buff[512] = {0};

    PRINT("***********************************************\n");
    PrintRegularMetric (buff, "Frame               :", FrameTotal);
    PrintRelativeMetric(buff, " - Text             :", FramePrintText, FrameTotal);
    PrintRelativeMetric(buff, "   -- Find Kerning  :", FramePrintTextFindKerning, FramePrintText);
    PrintRelativeMetric(buff, "   -- Draw Texture  :", FramePrintTextDrawTexture, FramePrintText);
}