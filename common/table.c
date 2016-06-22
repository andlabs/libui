// 21 june 2016
#include "../ui.h"
#include "uipriv.h"

uiTableColumn *uiTableAppendTextColumn(uiTable *t, const char *name, int modelColumn)
{
	uiTableColumn *tc;
	uiTableCellPart *part;

	part = uiNewTableTextPart(modelColumn);
	tc = uiTableAppendColumn(t, name);
	uiTableColumnAppend(tc, part, 1);
	return tc;
}
