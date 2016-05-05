// 18 october 2015
#include "test.h"

// TODO manage the memory of the uiTableModel

static intmax_t nColumns = 4;
static uiTableColumnType coltypes[] = {
	uiTableColumnText,
	uiTableColumnText,
	uiTableColumnCheckbox,
	uiTableColumnCheckbox,
};

static intmax_t nRows = 6;

static intmax_t modelNumRows(uiTableModel *m, void *mData)
{
	return nRows;
}

void *modelCellValue(uiTableModel *m, void *mData, intmax_t row, intmax_t column)
{
	char line[20];

	line[0] = 'R';
	line[1] = 'o';
	line[2] = 'w';
	line[3] = ' ';
	line[4] = row + '0';
	line[5] = '\0';
	switch (column) {
	case 0:
	case 1:
		return uiTableModelFromString(line);
	case 2:
		return uiTableModelFromBool(row % 2 == 0);
	case 3:
		return uiTableModelFromBool(row % 3 == 0);
	}
	// TODO
	return NULL;
}

// TODO make this not need to be static
uiTableModelSpec spec;

void modelSetCellValue(uiTableModel *m, void *mData, intmax_t row, intmax_t column, void *value)
{
	// TODO
}

uiBox *makePage9(void)
{
	uiBox *page9;
	uiTable *table;
	uiTableModel *model;
	uiTableColumnParams p;
	intmax_t i;

	page9 = newVerticalBox();

	table = uiNewTable();
	uiBoxAppend(page9, uiControl(table), 1);

	spec.NumRows = modelNumRows;
	spec.CellValue = modelCellValue;
	spec.SetCellValue = modelSetCellValue;
	model = uiNewTableModel(nColumns, coltypes, &spec, NULL);
	uiTableSetModel(table, model);

	for (i = 0; i < nColumns; i++) {
		p.Name = "Column";
		p.Type = coltypes[i];
		p.Mutable = i % 2 == 1;
		p.ValueColumn = i;
		uiTableAppendColumn(table, &p);
	}

	return page9;
}
