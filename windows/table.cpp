/* vim :set ts=4 sw=4 sts=4 noet : */
#include "uipriv_windows.hpp"

#include <vector>

static void uiTableDestroy(uiControl *c);
static void uiTableMinimumSize(uiWindowsControl *c, int *width, int *height);
static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nm, LRESULT *lResult);



struct uiTable;

struct uiTableModel {
	uiTableModelHandler *mh;
	// the uiTable controls using this model
    std::vector<uiTable*> tables;
};


struct uiTableColumn {
	uiTable *t;
	WCHAR *name;
	// don't really support parts (but this would be a list of parts, if we did ;-)
	int modelColumn;	// -1 = none
};


struct uiTable {
	uiWindowsControl c;
	uiTableModel *model;
	HWND hwnd;
	WCHAR* tmpText; // to hold strings which need to persist between WM_NOTIFYs
    std::vector<uiTableColumn*> columns;
};


void *uiTableModelStrdup(const char *str)
{
	return strdup(str);
}

void *uiTableModelGiveColor(double r, double g, double b, double a)
{
	return 0;	// TODO
}

uiTableModel *uiNewTableModel(uiTableModelHandler *mh)
{
	uiTableModel *m;

	m = new uiTableModel();
	m->mh = mh;
	return m;
}

void uiFreeTableModel(uiTableModel *m)
{
	// TODO: should assert(m->tables.empty()) ?
	delete m;
}

void uiTableModelRowInserted(uiTableModel *m, int newIndex)
{
	LVITEM item;

	item.mask = 0;
	item.iItem = newIndex;
	item.iSubItem = 0; //?
    for (auto t : m->tables) {
		ListView_InsertItem( t->hwnd, &item );
	}
}

void uiTableModelRowChanged(uiTableModel *m, int index)
{
    for (auto t : m->tables) {
		ListView_Update( t->hwnd, index );
	}
}

void uiTableModelRowDeleted(uiTableModel *m, int oldIndex)
{
    for (auto t : m->tables) {
		ListView_DeleteItem( t->hwnd, oldIndex );
	}
}


// helper - find desired listview index of this column
static int findColIndex(uiTableColumn *c)
{
	uiTable *t = c->t;
	int out = 0;

	for (auto candidate : t->columns) {
		if( candidate==c) {
			break;
		}
		// only count assigned columns
		// (columns not assigned until uiTableColumnAppendTextPart() is called)
		if( candidate->modelColumn >=0 ) {
			++out;
		}
	}
	return out;
}

void uiTableColumnAppendTextPart(uiTableColumn *c, int modelColumn, int expand)
{
	int lvIndex;
	if (c->modelColumn != -1) {
		return; // already set
	}
	c->modelColumn = modelColumn;

   	lvIndex = findColIndex(c);

	// tell the listview ctrl
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 120;
	lvc.pszText = c->name;
	ListView_InsertColumn(c->t->hwnd, lvIndex, &lvc);
}

void uiTableColumnAppendImagePart(uiTableColumn *c, int modelColumn, int expand)
{
	// not implemented
}


void uiTableColumnAppendButtonPart(uiTableColumn *c, int modelColumn, int expand)
{
	// not implemented
}

void uiTableColumnAppendCheckboxPart(uiTableColumn *c, int modelColumn, int expand)
{
	// not implemented
}

void uiTableColumnAppendProgressBarPart(uiTableColumn *c, int modelColumn, int expand)
{
	// not implemented
}

void uiTableColumnPartSetEditable(uiTableColumn *c, int part, int editable)
{
	// TODO
}

void uiTableColumnPartSetTextColor(uiTableColumn *c, int part, int modelColumn)
{
	// TODO
}


/* uiTable stuff */

uiWindowsControlAllDefaultsExceptDestroy(uiTable)

uiTable *uiNewTable(uiTableModel *model)
{
	uiTable *t;
	uiWindowsNewControl(uiTable, t);
    /*std::vector<uiTableColumn*>* tmp =*/ new(&t->columns) std::vector<uiTableColumn*>();  // placement new

	t->model = model;
	t->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		WC_LISTVIEW, L"",
		WS_CHILD | LVS_AUTOARRANGE | LVS_REPORT | LVS_OWNERDATA,
		hInstance, NULL,
		TRUE);
    t->tmpText = NULL;

	model->tables.push_back(t);

	uiWindowsRegisterWM_NOTIFYHandler(t->hwnd, onWM_NOTIFY, uiControl(t));
	//uiWindowsRegisterWM_COMMANDHandler(t->hwnd, onWM_COMMAND, uiControl(t));
	
	int n = (*(model->mh->NumRows))(model->mh, model);

	ListView_SetItemCountEx(t->hwnd,n, 0); //LVSICF_NOINVALIDATEALL
	return t;
}




uiTableColumn *uiTableAppendColumn(uiTable *t, const char *name)
{
	uiTableColumn* c = uiNew(uiTableColumn);
	c->name = toUTF16(name);
	c->t = t;
	c->modelColumn = -1;	// unassigned
	// we defer the actual column creation until a part is added...

	t->columns.push_back(c);
	return c;
}


void uiTableSetRowBackgroundColorModelColumn(uiTable *t, int modelColumn)
{
	// TODO
}


#if 0
static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	//uiTable *e = uiTable(c);
	/*
	if (code != EN_CHANGE)
	return FALSE;
	if (e->inhibitChanged)
	return FALSE;
	(*(e->onChanged))(e, e->onChangedData);
	*/
	*lResult = 0;
	return TRUE;
}

void ListView_SetItemCountEx(
   HWND  hwndLV,
   int	 cItems,
   DWORD dwFlags
);
LVSICF_NOINVALIDATEALL

#endif

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);


	uiWindowsUnregisterWM_NOTIFYHandler(t->hwnd);
//	uiWindowsUnregisterWM_COMMANDHandler(t->hwnd);
	uiWindowsEnsureDestroyWindow(t->hwnd);

    if( t->tmpText ) {
        uiFree(t->tmpText);
        t->tmpText = NULL;
    }

	// TODO: detach from model...
	// TODO: clean up column in turn
    t->columns.~vector<uiTableColumn*>();    // (created with placement new)

	uiFreeControl(uiControl(t));

}

static void uiTableMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	*width = 100;
	*height = 100;
	// TODO
}


static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nm, LRESULT *lResult)
{
	uiTable *t = uiTable(c);
	uiTableModelHandler *mh = t->model->mh;
	BOOL ret = FALSE;

    // can now free any string left over from previous WM_NOTIFY
	if (t->tmpText) {
		uiFree(t->tmpText);
        t->tmpText = NULL;
	}

	switch ( nm->code) {
		case LVN_GETDISPINFO:
		{
			NMLVDISPINFO* plvdi = (NMLVDISPINFO*)nm;	
			int col = plvdi->item.iSubItem;
			int row = plvdi->item.iItem;
		
			if (col<0 || col>=(int)t->columns.size()) {
				break;
			}

			uiTableColumn *tc = (uiTableColumn*)t->columns[col];

			int mcol = tc->modelColumn;
			uiTableModelColumnType typ = (*mh->ColumnType)(mh,t->model,mcol);
			if (typ==uiTableModelColumnString) {
				void* data = (*(mh->CellValue))(mh, t->model, row, mcol);
	            // windows expects string to hang around until next WM_NOTIFY...
				t->tmpText = toUTF16((const char*)data);
				uiFree(data);
				plvdi->item.pszText = t->tmpText;
			} else {
				// TODO!
				plvdi->item.pszText = TEXT("???");
			}
			break;
		}
		default:
			break;
	}



	*lResult = 0;
	return ret;
}






