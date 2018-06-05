// 21 june 2016
#include "test.h"

static uiTableModelHandler mh;

static int modelNumColumns(uiTableModelHandler *mh, uiTableModel *m)
{
	return 9;
}

static uiTableDataType modelColumnType(uiTableModelHandler *mh, uiTableModel *m, int column)
{
	if (column == 3 || column == 4)
		return uiTableDataTypeColor;
	if (column == 5)
		return uiTableDataTypeImage;
	if (column == 7 || column == 8)
		return uiTableDataTypeInt;
	return uiTableDataTypeString;
}

static int modelNumRows(uiTableModelHandler *mh, uiTableModel *m)
{
	return 15;
}

static uiImage *img[2];
static char row9text[1024];
static int yellowRow = -1;
static int checkStates[15];

static uiTableData *modelCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int col)
{
	char buf[256];

	if (col == 3) {
		if (row == yellowRow)
			return uiNewTableDataColor(1, 1, 0, 1);
		if (row == 3)
			return uiNewTableDataColor(1, 0, 0, 1);
		if (row == 11)
			return uiNewTableDataColor(0, 0.5, 1, 0.5);
		return NULL;
	}
	if (col == 4) {
		if ((row % 2) == 1)
			return uiNewTableDataColor(0.5, 0, 0.75, 1);
		return NULL;
	}
	if (col == 5) {
		if (row < 8)
			return uiNewTableDataImage(img[0]);
		return uiNewTableDataImage(img[1]);
	}
	if (col == 7)
		return uiNewTableDataInt(checkStates[row]);
	if (col == 8) {
		if (row == 0)
			return uiNewTableDataInt(0);
		if (row == 13)
			return uiNewTableDataInt(100);
		if (row == 14)
			return uiNewTableDataInt(-1);
		return uiNewTableDataInt(50);
	}
	switch (col) {
	case 0:
		sprintf(buf, "Row %d", row);
		break;
	case 2:
		if (row == 9)
			return uiNewTableDataString(row9text);
		// fall through
	case 1:
		strcpy(buf, "Part");
		break;
	case 6:
		strcpy(buf, "Make Yellow");
		break;
	}
	return uiNewTableDataString(buf);
}

static void modelSetCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int col, const uiTableData *val)
{
	if (row == 9 && col == 2)
		strcpy(row9text, uiTableDataString(val));
	if (col == 6) {
		int prevYellowRow;

		prevYellowRow = yellowRow;
		yellowRow = row;
		if (prevYellowRow != -1)
			uiTableModelRowChanged(m, prevYellowRow);
		uiTableModelRowChanged(m, yellowRow);
	}
	if (col == 7)
		checkStates[row] = uiTableDataInt(val);
}

uiBox *makePage16(void)
{
	uiBox *page16;
	uiTableModel *m;
	uiTable *t;
	uiTableTextColumnOptionalParams p;

	img[0] = uiNewImage(16, 16);
	appendImageNamed(img[0], "andlabs_16x16test_24june2016.png");
	appendImageNamed(img[0], "andlabs_32x32test_24june2016.png");
	img[1] = uiNewImage(16, 16);
	appendImageNamed(img[1], "tango-icon-theme-0.8.90_16x16_x-office-spreadsheet.png");
	appendImageNamed(img[1], "tango-icon-theme-0.8.90_32x32_x-office-spreadsheet.png");

	strcpy(row9text, "Part");

	memset(checkStates, 0, 15 * sizeof (int));

	page16 = newVerticalBox();

	mh.NumColumns = modelNumColumns;
	mh.ColumnType = modelColumnType;
	mh.NumRows = modelNumRows;
	mh.CellValue = modelCellValue;
	mh.SetCellValue = modelSetCellValue;
	m = uiNewTableModel(&mh);

	t = uiNewTable(m);
	uiBoxAppend(page16, uiControl(t), 1);

	uiTableAppendTextColumn(t, "Column 1",
		0, uiTableModelColumnNeverEditable, NULL);

	memset(&p, 0, sizeof (uiTableTextColumnOptionalParams));
	p.ColorModelColumn = 4;
	uiTableAppendImageTextColumn(t, "Column 2",
		5,
		1, uiTableModelColumnNeverEditable, &p);
	uiTableAppendTextColumn(t, "Editable",
		2, uiTableModelColumnAlwaysEditable, NULL);

	uiTableSetRowBackgroundColorModelColumn(t, 3);

	uiTableAppendCheckboxColumn(t, "Checkboxes",
		7, uiTableModelColumnAlwaysEditable);
	uiTableAppendButtonColumn(t, "Buttons",
		6, uiTableModelColumnAlwaysEditable);

	uiTableAppendProgressBarColumn(t, "Progress Bar",
		8);

	return page16;
}
