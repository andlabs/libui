// 6 april 2015
#include "ui.h"

typedef struct uiSize uiSize;
typedef struct uiSizing uiSizing;

struct uiSize {
	intmax_t width;
	intmax_t height;
};

// TODO handle destruction
struct uiControl {
	uintptr_t (*handle)(uiControl *);
//TODO	void (*setParent)(uiControl *, uintptr_t);
	uiSize (*preferredSize)(uiControl *, uiSizing *);
	void (*resize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
	void (*containerShow)(uiControl *);
	void (*containerHide)(uiControl *);
};
