// 21 june 2016
#include "test.h"

static uiTableModelHandler mh;

static int modelNumColumns(uiTableModelHandler *mh, uiTableModel *m)
{
	return 5;
}

static uiTableModelColumnType modelColumnType(uiTableModelHandler *mh, uiTableModel *m, int column)
{
	if (column == 3 || column == 4)
		return uiTableModelColumnColor;
	return uiTableModelColumnString;
}

static int modelNumRows(uiTableModelHandler *mh, uiTableModel *m)
{
	return 15;
}

static void *modelCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int col)
{
	char buf[256];

	if (col == 3) {
		if (row == 3)
			return uiTableModelGiveColor(1, 0, 0, 1);
		if (row == 11)
			return uiTableModelGiveColor(0, 0.5, 1, 0.5);
		return NULL;
	}
	if (col == 4) {
		if ((row % 2) == 1)
			return uiTableModelGiveColor(0.5, 0, 0.75, 1);
		return NULL;
	}
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
	uiTableColumnAppendTextPart(tc, 1, 0);
	uiTableColumnAppendTextPart(tc, 2, 1);
	uiTableColumnPartSetTextColor(tc, 1, 4);

	uiTableSetRowBackgroundColorModelColumn(t, 3);

	return page16;
}
