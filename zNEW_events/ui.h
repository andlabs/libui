...
uiprivExtern void uiQueueMain(void (*f)(void *data), void *data);

typedef struct uiEvent uiEvent;
typedef struct uiEventOptions uiEventOptions;

typedef void (*uiEventHandler)(void *sender, void *args, void *data);

struct uiEventOptions {
	size_t Size;
	bool Global;
};

uiprivExtern uiEvent *uiNewEvent(const uiEventOptions *options);
uiprivExtern void uiEventFree(uiEvent *e);
uiprivExtern int uiEventAddHandler(uiEvent *e, uiEventHandler handler, void *sender, void *data);
uiprivExtern void uiEventDeleteHandler(uiEvent *e, int id);
uiprivExtern void uiEventFire(uiEvent *e, void *sender, void *args);
uiprivExtern bool uiEventHandlerBlocked(const uiEvent *e, int id);
uiprivExtern void uiEventSetHandlerBlocked(uiEvent *e, int id, bool blocked);
uiprivExtern void uiEventInvalidateSender(uiEvent *e, void *sender);

typedef struct uiControl uiControl;
...
uiprivExtern void *uiControlImplData(uiControl *c);
uiprivExtern uiEvent *uiControlOnFree(void);

#ifdef __cplusplus
...
