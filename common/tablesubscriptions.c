// 18 october 2015
#include "../ui.h"
#include "uipriv.h"

struct uiTableSubscriptions {
	struct ptrArray *tables;
};

uiTableSubscriptions *uiTableNewSubscriptions(void)
{
	uiTableSubscriptions *s;

	s = uiNew(uiTableSubscriptions);
	s->tables = newPtrArray();
	return s;
}

void uiTableFreeSubscriptions(uiTableSubscriptions *s)
{
	ptrArrayDestroy(s->tables);
	uiFree(s);
}

// TODO make more robust: complain if table already subscribed
void uiTableSubscriptionsSubscribe(uiTableSubscriptions *s, uiTable *t)
{
	ptrArrayAppend(s->tables, t);
}

void uiTableSubscriptionsUnsubscribe(uiTableSubscriptions *s, uiTable *t)
{
	uintmax_t i;
	uiTable *table;

	for (i = 0; i < s->tables->len; i++) {
		table = ptrArrayIndex(s->tables, uiTable *, i);
		if (table == t) {
			ptrArrayDelete(s->tables, i);
			return;
		}
	}
	complain("attempt to unsubscribe table that isn't subscribed in uiTableSubscriptionsUnsubscribe()");
}

void uiTableSubscriptionsNotify(uiTableSubscriptions *s, uiTableNotification n, intmax_t row, intmax_t column)
{
	uintmax_t i;
	uiTable *table;

	for (i = 0; i < s->tables->len; i++) {
		table = ptrArrayIndex(s->tables, uiTable *, i);
		tableNotify(table, n, row, column);
	}
}
