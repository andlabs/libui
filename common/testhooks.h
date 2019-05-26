// 26 may 2019

#ifdef __cplusplus
extern "C" {
#endif

// errors.c
typedef void (*uiprivTestHookReportProgrammerErrorFunc)(const char *prefix, const char *msg, const char *suffix, bool internal);
uiprivExtern void uiprivTestHookReportProgrammerError(uiprivTestHookReportProgrammerErrorFunc f);

#ifdef __cplusplus
}
#endif
