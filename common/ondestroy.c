// 9 may 2015
#include "../ui.h"
#include "uipriv.h"

static void defaultOnDestroy(const uiControl *ctl, void *data)
{
	(void)ctl; (void)data;
}

static void (*onDestroyFunc)(const uiControl *, void *) = defaultOnDestroy;
static void *onDestroyData = 0;

void uiOnDestroy(void (*f)(const uiControl *, void *), void *data)
{
	onDestroyFunc = f;
	onDestroyData = data;
}

void OnDestroy(const uiControl *ctl)
{
	(*onDestroyFunc)(ctl, onDestroyData);
}
