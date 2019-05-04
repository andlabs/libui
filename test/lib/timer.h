// 2 may 2019

#include <stdint.h>

typedef int64_t timerDuration;
typedef int64_t timerTime;

#define timerNanosecond ((timerDuration) 1)
#define timerMicrosecond ((timerDuration) 1000)
#define timerMillisecond ((timerDuration) 1000000)
#define timerSecond ((timerDuration) 1000000000)

extern timerTime timerMonotonicNow(void);
extern timerDuration timerTimeSub(timerTime end, timerTime start);

// The Go algorithm says 32 should be enough.
// We use 33 to count the terminating NUL.
#define timerDurationStringLen 33
extern void timerDurationString(timerDuration d, char buf[timerDurationStringLen]);

typedef uint64_t timerSysError;
#ifdef _WIN32
#define timerSysErrorFmt "0x%08I32X"
#define timerSysErrorFmtArg(x) ((uint32_t) x)
#else
#include <string.h>
#define timerSysErrorFmt "%s (%d)"
#define timerSysErrorFmtArg(x) strerror((int) x), ((int) x)
#endif

extern timerSysError timerRunWithTimeout(timerDuration d, void (*f)(void *data), void *data, int *timedOut);

extern timerSysError timerSleep(timerDuration d);
