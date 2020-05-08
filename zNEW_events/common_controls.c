...

void uiControlFree(uiControl *c)
{
	...
	}

	uiEventFire(uiControlOnFree(), c, NULL);
	uiEventInvalidateSender(uiControlOnFree(), c);

	callVtable(c->type->vtable.Free, c, c->implData);
	...
}

...
