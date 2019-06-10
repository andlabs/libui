// 10 june 2019

checkErrorCase(uiNewEvent(NULL),
	"uiNewEvent(): invalid null pointer for uiEventOptions")
checkErrorCase(uiNewEvent(p->eventOptionsBadSize),
	"uiNewEvent(): wrong size 1 for uiEventOptions")

checkErrorCase(uiEventFree(NULL),
	"uiEventFree(): invalid null pointer for uiEvent")
checkErrorCaseWhileFiring(uiEventFree(p->firingEvent),
	"uiEventFree(): can't change a uiEvent while it is firing")
checkErrorCase(uiEventFree(p->eventWithHandlers),
	"uiEventFree(): can't free event that still has handlers registered")

checkErrorCase(uiEventAddHandler(NULL, p->handlerFunc, p->senderPlaceholder, p->dataPlaceholder),
	"uiEventAddHandler(): invalid null pointer for uiEvent")
checkErrorCaseWhileFiring(uiEventAddHandler(p->firingEvent, p->handlerFunc, p->senderPlaceholder, p->dataPlaceholder),
	"uiEventAddHandler(): can't change a uiEvent while it is firing")
checkErrorCase(uiEventAddHandler(p->eventPlaceholder, NULL, p->senderPlaceholder, p->dataPlaceholder),
	"uiEventAddHandler(): invalid null pointer for uiEventHandler")
checkErrorCase(uiEventAddHandler(p->globalEvent, p->handlerFunc, p->nonNullSender, p->dataPlaceholder),
	"uiEventAddHandler(): can't use a non-NULL sender with a global event")
checkErrorCase(uiEventAddHandler(p->nonglobalEvent, p->handlerFunc, NULL, p->dataPlaceholder),
	"uiEventAddHandler(): can't use a NULL sender with a non-global event")

checkErrorCase(uiEventDeleteHandler(NULL, p->idPlaceholder),
	"uiEventDeleteHandler(): invalid null pointer for uiEvent")
checkErrorCaseWhileFiring(uiEventDeleteHandler(p->firingEvent, p->idPlaceholder),
	"uiEventDeleteHandler(): can't change a uiEvent while it is firing")
checkErrorCase(uiEventDeleteHandler(p->eventPlaceholder, 5),
	"uiEventDeleteHandler(): event handler 5 not found")

checkErrorCase(uiEventFire(NULL, p->senderPlaceholder, p->argsPlaceholder),
	"uiEventFire(): invalid null pointer for uiEvent")
checkErrorCaseWhileFiring(uiEventFire(p->firingEvent, p->senderPlaceholder, p->argsPlaceholder),
	"uiEventFire(): can't recursively fire a uiEvent")
checkErrorCase(uiEventFire(p->globalEvent, p->nonNullSender, p->argsPlaceholder),
	"uiEventFire(): can't use a non-NULL sender with a global event")
checkErrorCase(uiEventFire(p->nonglobalEvent, NULL, p->argsPlaceholder),
	"uiEventFire(): can't use a NULL sender with a non-global event")

checkErrorCase(uiEventHandlerBlocked(NULL, p->idPlaceholder),
	"uiEventHandlerBlocked(): invalid null pointer for uiEvent")
checkErrorCase(uiEventHandlerBlocked(p->eventPlaceholder, 5),
	"uiEventHandlerBlocked(): event handler 5 not found")

checkErrorCase(uiEventSetHandlerBlocked(NULL, p->idPlaceholder, p->blockedPlaceholder),
	"uiEventSetHandlerBlocked(): invalid null pointer for uiEvent")
checkErrorCaseWhileFiring(uiEventSetHandlerBlocked(p->firingEvent, p->idPlaceholder, p->blockedPlaceholder),
	"uiEventSetHandlerBlocked(): can't change a uiEvent while it is firing")
checkErrorCase(uiEventSetHandlerBlocked(p->eventPlaceholder, 5, p->blockedPlaceholder),
	"uiEventSetHandlerBlocked(): event handler 5 not found")

checkErrorCase(uiEventInvalidateSender(NULL, p->senderPlaceholder),
	"uiEventInvalidateSender(): invalid null pointer for uiEvent")
checkErrorCaseWhileFiring(uiEventInvalidateSender(p->firingEvent, p->senderPlaceholder),
	"uiEventInvalidateSender(): can't change a uiEvent while it is firing")
checkErrorCase(uiEventInvalidateSender(p->globalEvent, NULL),
	"uiEventInvalidateSender(): can't invalidate a global event")
checkErrorCase(uiEventInvalidateSender(p->nonglobalEvent, NULL),
	"uiEventInvalidateSender(): can't use a NULL sender with a non-global event")
