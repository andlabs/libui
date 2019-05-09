// 19 april 2019

#ifdef __cplusplus
extern "C" {
#endif

// init.c
extern const char **uiprivSysInitErrors(void);
extern int uiprivSysInit(void *options, uiInitError *err);
extern int uiprivInitReturnError(uiInitError *err, const char *msg);
extern int uiprivInitReturnErrorf(uiInitError *err, const char *msg, ...);

#ifdef __cplusplus
}
#endif
