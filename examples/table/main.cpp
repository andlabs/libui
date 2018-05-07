/* vim: set noet ts=4 sw=4 sts=4: */

// example for giving table control a workout
//
// TODO:
// delete row(s)
// editing cells
// show selected rows
// column sorting
// other column types (numbers, colours etc)

#include <cstdio>
#include <cstring>
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>
#include "../../ui.h"

#include "planet.h"


static uiTableModel* model=0;
static uiWindow *mainwin;
static uiButton *deleteButton;
static uiTable *table;

// our set of dummy data
static std::vector<Planet> db;


//
// handler funcs to expose our data via a uiTableModel
//

static int handlerNumColumns(uiTableModelHandler *h, uiTableModel *m)
{
	//printf("handlerNumColumns()\n");
	return 7;
}

static uiTableModelColumnType handlerColumnType(uiTableModelHandler *h, uiTableModel *m, int col)
{
	switch( col) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		default:
			return uiTableModelColumnString;
	}
}


static int handlerNumRows(uiTableModelHandler *h, uiTableModel *m)
{
	return db.size();
}


static void* handlerCellValue(uiTableModelHandler *h, uiTableModel *m, int row, int col)
{
	assert(row>=0 && row<(int)db.size());
	assert(col>=0 && col<7);
	Planet &planet = db[row];
	char buf[32];
	switch (col) {
		case 0:
			return uiTableModelStrdup( planet.name.c_str() );
		case 1:
			return uiTableModelStrdup( Planet::econDesc[planet.economy] );
		case 2:
			return uiTableModelStrdup( Planet::govtDesc[planet.govt] );
		case 3:
			// TODO: investigate how to show numeric columns!
			sprintf(buf,"%d", planet.techLevel);
			return uiTableModelStrdup(buf);
		case 4:
			sprintf(buf,"%d", planet.turnover);
			return uiTableModelStrdup(buf);
		case 5:
			sprintf(buf,"%dKm", planet.radius);
			return uiTableModelStrdup(buf);
		case 6:
			sprintf(buf,"%dB", planet.population);
			return uiTableModelStrdup(buf);
		default: return NULL;
	}
}


static void handlerSetCellValue(uiTableModelHandler *h, uiTableModel *m, int row, int col, const void *val)
{
//	printf("handlerSetCell(row %d, col %d)\n", row,col);
}


static uiTableModelHandler handler = {
	handlerNumColumns,
	handlerColumnType,
	handlerNumRows,
	handlerCellValue,
	handlerSetCellValue
};



//
// event handlers
//

static int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

static int onShouldQuit(void *data)
{
	uiWindow *mainwin = uiWindow(data);

	uiControlDestroy(uiControl(mainwin));
	return 1;
}

static void onSelectionChanged(uiTable *t, void *data)
{
	int cnt = 0;
	printf("Selection changed!\n");
	uiTableIter *sel = uiTableGetSelection(t);
	while (uiTableIterAdvance(sel)) {
		printf("%d ", uiTableIterCurrent(sel));
		++cnt;
	}
	uiTableIterComplete(sel);
	printf("\n");

	if(cnt>0) {
		uiControlEnable(uiControl(deleteButton));
	} else {
		uiControlDisable(uiControl(deleteButton));
	}
//	uiTableDumpSelection(t);
}


// add a number of new rows
static void onAddEntries(uiButton *, void *data)
{
	uiSpinbox* entry = (uiSpinbox*)data;
	int cnt = uiSpinboxValue(entry);
	printf("add %d\n", cnt);
	Planet p;
	int i;
	for( i=0; i<cnt; ++i) {
		int idx = (int)db.size();
		p.Randomise();
		db.push_back( p );
		uiTableModelRowInserted(model,idx);
	}

}


// delete all selected rows
static void onDeleteEntries(uiButton *, void *data)
{
	std::vector<int> sel;
	uiTableIter *iter = uiTableGetSelection(table);
	while (uiTableIterAdvance(iter)) {
		sel.push_back(uiTableIterCurrent(iter));
	}
	uiTableIterComplete(iter);

	// sort in reverse order, so we don't screw invalidate
	// the indexes as we go
	std::sort(sel.rbegin(), sel.rend());
	for( int idx : sel ) {
		db.erase(db.begin()+idx);
		uiTableModelRowDeleted(model,idx);
	}
}

//
// build gui
//

static uiTable *makeTable(void)
{
	uiTable* t;
	model = uiNewTableModel(&handler);
	t = uiNewTable(model, uiTableStyleMultiSelect);
	uiTableAppendTextColumn(t, "Name", 0);
	uiTableAppendTextColumn(t, "Economy", 1);
	uiTableAppendTextColumn(t, "Government", 2);
	uiTableAppendTextColumn(t, "Tech Level", 3);
	uiTableAppendTextColumn(t, "Turnover", 4);
	uiTableAppendTextColumn(t, "Radius", 5);
	uiTableAppendTextColumn(t, "Population", 6);

	uiTableOnSelectionChanged(t, onSelectionChanged, NULL);
	return t;
}


static uiControl *makeLayout(void)
{
	uiBox *vbox;
	uiSpinbox *entry;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox,1);

	table = makeTable();
	uiBoxAppend(vbox,uiControl(table),1);
	{
		uiGroup* group = uiNewGroup("Add Entries");
		uiGroupSetMargined(group, 1);
		{
			uiBox* hbox = uiNewHorizontalBox();
			uiBoxSetPadded(hbox,1);

			uiBoxAppend(hbox, uiControl(uiNewLabel("number of entries")), 0);
			entry = uiNewSpinbox(1,100000);
			uiSpinboxSetValue(entry, 10);
			uiBoxAppend(hbox, uiControl(entry),0);

			uiButton* addButton = uiNewButton("Add");

			uiButtonOnClicked(addButton, onAddEntries, entry);



			uiBoxAppend(hbox, uiControl(addButton),0);



			uiGroupSetChild(group, uiControl(hbox));
		}
		uiBoxAppend(vbox, uiControl(group), 0);

		// delete button
		{
			deleteButton = uiNewButton("Delete selected");
			uiControlDisable(uiControl(deleteButton));
			uiButtonOnClicked(deleteButton, onDeleteEntries, 0);
			uiBoxAppend(vbox, uiControl(deleteButton), 0);
		}


	}


	return uiControl(vbox);
}






int main(void)
{
#ifdef _WIN32
	setbuf(stdout, NULL);
#endif

	uiInitOptions options;
	const char *err;

	memset(&options, 0, sizeof (uiInitOptions));
	err = uiInit(&options);
	if (err != NULL) {
		fprintf(stderr, "error initializing libui: %s", err);
		uiFreeInitError(err);
		return 1;
	}

	Planet p;
	int i;
	for( i=0; i<5; ++i) {
		p.Randomise();
		db.push_back( p );
	}



	mainwin = uiNewWindow("libui Table Example", 640, 480, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);
	uiOnShouldQuit(onShouldQuit, mainwin);

	uiControl* layout = makeLayout();

	uiWindowSetChild(mainwin, layout);
	uiWindowSetMargined(mainwin, 1);

	uiControlShow(uiControl(mainwin));
	uiMain();
	return 0;
}

