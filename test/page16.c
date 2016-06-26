// 21 june 2016
#include "test.h"

static uiTableModelHandler mh;

static int modelNumColumns(uiTableModelHandler *mh, uiTableModel *m)
{
	return 6;
}

static uiTableModelColumnType modelColumnType(uiTableModelHandler *mh, uiTableModel *m, int column)
{
	if (column == 3 || column == 4)
		return uiTableModelColumnColor;
	if (column == 5)
		return uiTableModelColumnImage;
	return uiTableModelColumnString;
}

static int modelNumRows(uiTableModelHandler *mh, uiTableModel *m)
{
	return 15;
}

static uiImage *img[2];
static char row9text[1024];

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
	if (col == 5) {
		if (row < 8)
			return img[0];
		return img[1];
	}
	switch (col) {
	case 0:
		sprintf(buf, "Row %d", row);
		break;
	case 2:
		if (row == 9)
			return uiTableModelStrdup(row9text);
		// fall through
	case 1:
		strcpy(buf, "Part");
		break;
	}
	return uiTableModelStrdup(buf);
}

static void modelSetCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int col, const void *val)
{
	if (row == 9 && col == 2)
		strcpy(row9text, (const char *) val);
}

uiBox *makePage16(void)
{
	uiBox *page16;
	uiTableModel *m;
	uiTable *t;
	uiTableColumn *tc;

	img[0] = uiNewImage(16, 16);
	appendImageNamed(img[0], "andlabs_16x16test_24june2016.png");
	appendImageNamed(img[0], "andlabs_32x32test_24june2016.png");
	img[1] = uiNewImage(16, 16);
	appendImageNamed(img[1], "tango-icon-theme-0.8.90_16x16_x-office-spreadsheet.png");
	appendImageNamed(img[1], "tango-icon-theme-0.8.90_32x32_x-office-spreadsheet.png");

	strcpy(row9text, "Part");

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
	uiTableColumnAppendImagePart(tc, 5, 0);
	uiTableColumnAppendTextPart(tc, 1, 0);
	uiTableColumnAppendTextPart(tc, 2, 1);
	uiTableColumnPartSetTextColor(tc, 1, 4);
	uiTableColumnPartSetEditable(tc, 2, 1);

	uiTableSetRowBackgroundColorModelColumn(t, 3);

	return page16;
}
