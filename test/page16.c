// 21 june 2016
#include "test.h"

static uiTableModelHandler mh;

static nt modelNumColumns(uiTableModelHandler *mh, uiTableModel *m)
{
	return 3;
}

static uiTableModelColumnType modelColumnType(uiTableModelHandler *mh, uiTableModel *m, int column)
{
	return uiTableModelColumnString;
}

static int modelNumRows(uiTableModelHandler *mh, uiTableModel *m)
{
	return 15;
}

static void *modelCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int col)
{
	char buf[256];

	switch (col) {
	case 0:
		sprintf(buf, "Row %d", row);
		break;
	case 1:
	case 2:
		strcpy(buf, "Part");
		break;
	}
	return uiTableModelStrdup(buf);
}

static void modelSetCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int col, void *val)
{
	// not implemented yet
}

uiBox *makePage16(void)
{
	uiBox *page16;
	uiTableModel *m;
	uiTable *t;
	uiTableColumn *tc;

	page16 = newVerticalBox();

	mh.NumColumns = modelNumColumns;
	mh.ColumnType = modelColumnType;
	mh.NumRows = modelNumRows;
	mh.CellValue = modelCellValue;
	mh.SetCellValue = modelSetCellValue;
	m = uiNewTableModel(&mh);

	t = uiNewTable(m);
	uiBoxAppend(page16, uiControl(t), 1);

	uiTableAppendTextColumn(t, "Column 1", 0);

	tc = uiTableAppendColumn(t, "Column 2");
	uiTableColumnAppend(tc, uiNewTableTextPart(1), 0);
	uiTableColumnAppend(tc, uiNewTableTextPart(2), 1);

	return page16;
}
