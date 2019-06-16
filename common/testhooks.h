// 26 may 2019

#ifdef __cplusplus
extern "C" {
#endif

// errors.c
typedef void (*uiprivTestHookReportProgrammerErrorFunc)(const char *msg, void *data);
uiprivExtern void uiprivTestHookReportProgrammerError(uiprivTestHookReportProgrammerErrorFunc f, void *data);

// controls.c
uiprivExtern uiControl *uiprivTestHookControlWithInvalidControlMarker(void);
uiprivExtern uiControl *uiprivTestHookControlWithInvalidType(void);

#ifdef __cplusplus
}
#endif
