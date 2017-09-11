/* vim :set ts=4 sw=4 sts=4 noet : */
#include "uipriv_windows.hpp"


static void uiTableDestroy(uiControl *c);
static void uiTableMinimumSize(uiWindowsControl *c, int *width, int *height);
static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nm, LRESULT *lResult);



typedef struct growable growable;

struct growable
{
	void** data;
	int cap;
	int len;
};

static void growable_init( growable *g, int cap )
{
	g->data = (void**)uiAlloc(cap * sizeof(void*), "void*[]");
	g->cap = cap;
	g->len = 0;
}


static void growable_destroy( growable *g )
{
	uiFree(g->data);
}

static void growable_append( growable *g, void* element )
{
	if (g->len==g->cap) {
		// grow the array
		g->cap *= 2;
		g->data = (void**)uiRealloc((void*)g->data, g->cap, "void*[]");
	}
	g->data[g->len] = element;
	++g->len;
}


struct uiTableModel {
	uiTableModelHandler *mh;
	// the uiTable controls using this model
	growable tables;
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
	growable columns;
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

	m = uiNew(uiTableModel);
	m->mh = mh;
	growable_init(&m->tables, 1);
	return m;
}

void uiFreeTableModel(uiTableModel *m)
{
	// TODO: should assert(m->tables.len==0) ?
	growable_destroy(&m->tables);
	uiFree(m);
}

void uiTableModelRowInserted(uiTableModel *m, int newIndex)
{
	int i;
	uiTable* t;
	LVITEM item;

	item.mask = 0;
	item.iItem = newIndex;
	item.iSubItem = 0; //?
	for (i=0; i<m->tables.len; ++i) {
		t = (uiTable*)m->tables.data[i];
		ListView_InsertItem( t->hwnd, &item );
	}
}

void uiTableModelRowChanged(uiTableModel *m, int index)
{
	int i;
	uiTable* t;
	for (i=0; i<m->tables.len; ++i) {
		t = (uiTable*)m->tables.data[i];
		ListView_Update( t->hwnd, index );
	}
}

void uiTableModelRowDeleted(uiTableModel *m, int oldIndex)
{
	int i;
	uiTable* t;
	for (i=0; i<m->tables.len; ++i) {
		t = (uiTable*)m->tables.data[i];
		ListView_DeleteItem( t->hwnd, oldIndex );
	}
}


// helper - find desired listview index of this column
static int findColIndex(uiTableColumn *c)
{
	uiTable *t = c->t;
	int out = 0;
	int i;

	for( i=0; i<t->columns.len; ++i ) {
		uiTableColumn *candidate = (uiTableColumn*)t->columns.data[i];
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

	growable_init(&t->columns,4);

	t->model = model;
	t->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		WC_LISTVIEW, L"",
		WS_CHILD | LVS_AUTOARRANGE | LVS_REPORT | LVS_OWNERDATA,
		hInstance, NULL,
		TRUE);

	growable_append(&model->tables, t);

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

	growable_append(&t->columns, c);
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

	// TODO: detach from model...
	// TODO: clean up column in turn
	growable_destroy(&t->columns);

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

	// TODO: FIX. windows expects string to hang around until next WM_NOTIFY...
	// but this will leak on exit. And is shared between controls...
	static WCHAR* wstr=0;
	if( wstr != 0 ) {
		uiFree(wstr);
	}

	switch ( nm->code) {
		case LVN_GETDISPINFO:
		{
			NMLVDISPINFO* plvdi = (NMLVDISPINFO*)nm;	
			int col = plvdi->item.iSubItem;
			int row = plvdi->item.iItem;
		
			if (col<0 || col>=t->columns.len) {
				break;
			}

			uiTableColumn *tc = (uiTableColumn*)t->columns.data[col];

			int mcol = tc->modelColumn;
			uiTableModelColumnType typ = (*mh->ColumnType)(mh,t->model,mcol);
			if (typ==uiTableModelColumnString) {
				void* data = (*(mh->CellValue))(mh, t->model, row, mcol);
				wstr = toUTF16((const char*)data);
				uiFree(data);
				plvdi->item.pszText = wstr;
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






