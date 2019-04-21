// 19 april 2019

#ifdef __cplusplus
extern "C" {
#endif

// init.c
extern int uiprivInitCheckParams(void *options, uiInitError *err, const char *initErrors[]);
extern int uiprivInitReturnError(uiInitError *err, const char *msg);
extern int uiprivInitReturnErrorf(uiInitError *err, const char *msg, ...);
extern void uiprivMarkInitialized(void);

#ifdef __cplusplus
}
#endif
