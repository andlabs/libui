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

#ifdef __cplusplus
}
#endif
