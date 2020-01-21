// 4 may 2019

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// I don't like this threading model, but let's use it for now so I can continue working

typedef struct threadThread threadThread;

typedef uint64_t threadSysError;
#ifdef _WIN32
#define threadSysErrorFmt "0x%08I32X"
#define threadSysErrorFmtArg(x) ((uint32_t) x)
#else
#include <string.h>
#define threadSysErrorFmt "%s (%d)"
#define threadSysErrorFmtArg(x) strerror((int) x), ((int) x)
#endif

extern threadSysError threadNewThread(void (*f)(void *data), void *data, threadThread **t);
extern threadSysError threadThreadWaitAndFree(threadThread *t);

typedef int64_t threadDuration;

#define threadDurationMin ((threadDuration) INT64_MIN)
#define threadDurationMax ((threadDuration) INT64_MAX)

#define threadNanosecond ((threadDuration) 1)
#define threadMicrosecond ((threadDuration) 1000)
#define threadMillisecond ((threadDuration) 1000000)
#define threadSecond ((threadDuration) 1000000000)

extern threadSysError threadSleep(threadDuration d);

#ifdef __cplusplus
}
#endif
