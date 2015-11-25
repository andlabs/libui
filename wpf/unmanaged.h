// 25 november 2015
#ifdef __cplusplus
extern "C" {
#else
#include "winapi.h"
#include "../ui.h"
#include "../common/uipriv.h"
#endif

// main.cpp
extern void initWPF(void);
extern void uninitWPF(void);

// alloc.c
extern int initAlloc(void);
extern void uninitAlloc(void);

// init.c
extern uiInitOptions options;

#ifdef __cplusplus
}
#endif
