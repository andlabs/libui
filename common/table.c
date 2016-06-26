// 21 june 2016
#include "../ui.h"
#include "uipriv.h"

void *uiTableModelGiveInt(int i)
{
	return (void *) ((intptr_t) i);
}

int uiTableModelTakeInt(void *v)
{
	return (int) ((intptr_t) v);
}

uiTableColumn *uiTableAppendTextColumn(uiTable *t, const char *name, int modelColumn)
{
	uiTableColumn *tc;

	tc = uiTableAppendColumn(t, name);
	uiTableColumnAppendTextPart(tc, modelColumn, 1);
	return tc;
}
