// 19 april 2019

#ifdef __cplusplus
extern "C" {
#endif

// init.c
extern const char **uiprivSysInitErrors(void);
extern int uiprivSysInit(void *options, uiInitError *err);
extern int uiprivInitReturnError(uiInitError *err, const char *msg);
extern int uiprivInitReturnErrorf(uiInitError *err, const char *msg, ...);

// errors.c
extern void uiprivInternalError(const char *fmt, ...);
enum {
	uiprivProgrammerErrorWrongStructSize,		// arguments: size_t badSize, const char *structName
	uiprivProgrammerErrorIndexOutOfRange,	// arguments: int badIndex, __func__
	uiprivProgrammerErrorNullPointer,			// arguments: const char *paramDesc, __func__
	uiprivProgrammerErrorIntIDNotFound,		// arguments: const char *idDesc, int badID, __func__
	// TODO type mismatch
	// TODO attempt to change event during uiEventFire()
	uiprivNumProgrammerErrors,
};
extern void uiprivProgrammerError(unsigned int which, ...);
extern void uiprivReportError(const char *prefix, const char *msg, const char *suffix, bool internal);

#ifdef __cplusplus
}
#endif
