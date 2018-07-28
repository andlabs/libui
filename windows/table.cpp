#include "uipriv_windows.hpp"
#include "table.hpp"

// general TODOs:
// - tooltips don't work properly on columns with icons (the listview always thinks there's enough room for a short label because it's not taking the icon into account); is this a bug in our LVN_GETDISPINFO handler or something else?
// - should clicking on some other column of the same row, even one that doesn't edit, cancel editing?
// - implement keyboard accessibility
// - implement accessibility in general (Dynamic Annotations maybe?)
// - if I didn't handle these already: "drawing focus rects here, subitem navigation and activation with the keyboard"

uiTableModel *uiNewTableModel(uiTableModelHandler *mh)
{
	uiTableModel *m;

	m = uiprivNew(uiTableModel);
	m->mh = mh;
	m->tables = new std::vector<uiTable *>;
	return m;
}

void uiFreeTableModel(uiTableModel *m)
{
	delete m->tables;
	uiprivFree(m);
}

// TODO document that when this is called, the model must return the new row count when asked
void uiTableModelRowInserted(uiTableModel *m, int newIndex)
{
	LVITEMW item;
	int newCount;

	newCount = uiprivTableModelNumRows(m);
	ZeroMemory(&item, sizeof (LVITEMW));
	item.mask = 0;
	item.iItem = newIndex;
	item.iSubItem = 0;
	for (auto t : *(m->tables)) {
		// actually insert the rows
		if (SendMessageW(t->hwnd, LVM_SETITEMCOUNT, (WPARAM) newCount, LVSICF_NOINVALIDATEALL) == 0)
			logLastError(L"error calling LVM_SETITEMCOUNT in uiTableModelRowInserted()");
		// and redraw every row from the new row down to simulate adding it
		if (SendMessageW(t->hwnd, LVM_REDRAWITEMS, (WPARAM) newIndex, (LPARAM) (newCount - 1)) == FALSE)
			logLastError(L"error calling LVM_REDRAWITEMS in uiTableModelRowInserted()");

		// update selection state
		if (SendMessageW(t->hwnd, LVM_INSERTITEM, 0, (LPARAM) (&item)) == (LRESULT) (-1))
			logLastError(L"error calling LVM_INSERTITEM in uiTableModelRowInserted() to update selection state");
	}
}

// TODO compare LVM_UPDATE and LVM_REDRAWITEMS
void uiTableModelRowChanged(uiTableModel *m, int index)
{
	for (auto t : *(m->tables))
		if (SendMessageW(t->hwnd, LVM_UPDATE, (WPARAM) index, 0) == (LRESULT) (-1))
			logLastError(L"error calling LVM_UPDATE in uiTableModelRowChanged()");
}

// TODO document that when this is called, the model must return the OLD row count when asked
// TODO for this and the above, see what GTK+ requires and adjust accordingly
void uiTableModelRowDeleted(uiTableModel *m, int oldIndex)
{
	int newCount;

	newCount = uiprivTableModelNumRows(m);
	newCount--;
	for (auto t : *(m->tables)) {
		// update selection state
		if (SendMessageW(t->hwnd, LVM_DELETEITEM, (WPARAM) oldIndex, 0) == (LRESULT) (-1))
			logLastError(L"error calling LVM_DELETEITEM in uiTableModelRowDeleted() to update selection state");

		// actually delete the rows
		if (SendMessageW(t->hwnd, LVM_SETITEMCOUNT, (WPARAM) newCount, LVSICF_NOINVALIDATEALL) == 0)
			logLastError(L"error calling LVM_SETITEMCOUNT in uiTableModelRowDeleted()");
		// and redraw every row from the new nth row down to simulate removing the old nth row
		if (SendMessageW(t->hwnd, LVM_REDRAWITEMS, (WPARAM) oldIndex, (LPARAM) (newCount - 1)) == FALSE)
			logLastError(L"error calling LVM_REDRAWITEMS in uiTableModelRowDeleted()");
	}
}

uiTableModelHandler *uiprivTableModelHandler(uiTableModel *m)
{
	return m->mh;
}

// TODO explain all this
static LRESULT CALLBACK tableSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIDSubclass, DWORD_PTR dwRefData)
{
	uiTable *t = (uiTable *) dwRefData;
	NMHDR *nmhdr = (NMHDR *) lParam;
	bool finishEdit, abortEdit;
	HWND header;
	LRESULT lResult;
	HRESULT hr;

	finishEdit = false;
	abortEdit = false;
	switch (uMsg) {
	case WM_TIMER:
		if (wParam == (WPARAM) (&(t->inDoubleClickTimer))) {
			t->inDoubleClickTimer = FALSE;
			// TODO check errors
			KillTimer(hwnd, wParam);
			return 0;
		}
		if (wParam != (WPARAM) t)
			break;
		// TODO only increment and update if visible?
		for (auto &i : *(t->indeterminatePositions)) {
			i.second++;
			// TODO check errors
			SendMessageW(hwnd, LVM_UPDATE, (WPARAM) (i.first.first), 0);
		}
		return 0;
	case WM_LBUTTONDOWN:
		t->inLButtonDown = TRUE;
		lResult = DefSubclassProc(hwnd, uMsg, wParam, lParam);
		t->inLButtonDown = FALSE;
		return lResult;
	case WM_COMMAND:
		if (HIWORD(wParam) == EN_UPDATE) {
			// the real list view resizes the edit control on this notification specifically
			hr = uiprivTableResizeWhileEditing(t);
			if (hr != S_OK) {
				// TODO
			}
			break;
		}
		// the real list view accepts changes in this case
		if (HIWORD(wParam) == EN_KILLFOCUS)
			finishEdit = true;
		break;		// don't override default handling
	case WM_NOTIFY:
		// list view accepts changes on column resize, but does not provide such notifications :/
		header = (HWND) SendMessageW(t->hwnd, LVM_GETHEADER, 0, 0);
		if (nmhdr->hwndFrom == header) {
			NMHEADERW *nm = (NMHEADERW *) nmhdr;

			switch (nmhdr->code) {
			case HDN_ITEMCHANGED:
				if ((nm->pitem->mask & HDI_WIDTH) == 0)
					break;
				// fall through
			case HDN_DIVIDERDBLCLICK:
			case HDN_TRACK:
			case HDN_ENDTRACK:
				finishEdit = true;
			}
		}
		// I think this mirrors the WM_COMMAND one above... TODO
		if (nmhdr->code == NM_KILLFOCUS)
			finishEdit = true;
		break;		// don't override default handling
	case LVM_CANCELEDITLABEL:
		finishEdit = true;
		// TODO properly imitate notifiactions
		break;		// don't override default handling
	// TODO finish edit on WM_WINDOWPOSCHANGING and WM_SIZE?
	// for the next three: this item is about to go away; don't bother keeping changes
	case LVM_SETITEMCOUNT:
		if (wParam <= t->editedItem)
			abortEdit = true;
		break;		// don't override default handling
	case LVM_DELETEITEM:
		if (wParam == t->editedItem)
			abortEdit = true;
		break;		// don't override default handling
	case LVM_DELETEALLITEMS:
		abortEdit = true;
		break;		// don't override default handling
	case WM_NCDESTROY:
		if (RemoveWindowSubclass(hwnd, tableSubProc, uIDSubclass) == FALSE)
			logLastError(L"RemoveWindowSubclass()");
		// fall through
	}
	if (finishEdit) {
		hr = uiprivTableFinishEditingText(t);
		if (hr != S_OK) {
			// TODO
		}
	} else if (abortEdit) {
		hr = uiprivTableAbortEditingText(t);
		if (hr != S_OK) {
			// TODO
		}
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

int uiprivTableProgress(uiTable *t, int item, int subitem, int modelColumn, LONG *pos)
{
	uiTableValue *value;
	int progress;
	std::pair<int, int> p;
	std::map<std::pair<int, int>, LONG>::iterator iter;
	bool startTimer = false;
	bool stopTimer = false;

	value = uiprivTableModelCellValue(t->model, item, modelColumn);
	progress = uiTableValueInt(value);
	uiFreeTableValue(value);

	p.first = item;
	p.second = subitem;
	iter = t->indeterminatePositions->find(p);
	if (iter == t->indeterminatePositions->end()) {
		if (progress == -1) {
			startTimer = t->indeterminatePositions->size() == 0;
			(*(t->indeterminatePositions))[p] = 0;
			if (pos != NULL)
				*pos = 0;
		}
	} else
		if (progress != -1) {
			t->indeterminatePositions->erase(p);
			stopTimer = t->indeterminatePositions->size() == 0;
		} else if (pos != NULL)
			*pos = iter->second;

	if (startTimer)
		// the interval shown here is PBM_SETMARQUEE's default
		// TODO should we pass a function here instead? it seems to be called by DispatchMessage(), not DefWindowProc(), but I'm still unsure
		if (SetTimer(t->hwnd, (UINT_PTR) t, 30, NULL) == 0)
			logLastError(L"SetTimer()");
	if (stopTimer)
		if (KillTimer(t->hwnd, (UINT_PTR) (&t)) == 0)
			logLastError(L"KillTimer()");

	return progress;
}

// TODO properly integrate compound statements
static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nmhdr, LRESULT *lResult)
{
	uiTable *t = uiTable(c);
	HRESULT hr;

	switch (nmhdr->code) {
	case LVN_GETDISPINFO:
		hr = uiprivTableHandleLVN_GETDISPINFO(t, (NMLVDISPINFOW *) nmhdr, lResult);
		if (hr != S_OK) {
			// TODO
			return FALSE;
		}
		return TRUE;
	case NM_CUSTOMDRAW:
		hr = uiprivTableHandleNM_CUSTOMDRAW(t, (NMLVCUSTOMDRAW *) nmhdr, lResult);
		if (hr != S_OK) {
			// TODO
			return FALSE;
		}
		return TRUE;
	case NM_CLICK:
#if 0
		{
			NMITEMACTIVATE *nm = (NMITEMACTIVATE *) nmhdr;
			LVHITTESTINFO ht;
			WCHAR buf[256];

			ZeroMemory(&ht, sizeof (LVHITTESTINFO));
			ht.pt = nm->ptAction;
			if (SendMessageW(t->hwnd, LVM_SUBITEMHITTEST, 0, (LPARAM) (&ht)) == (LRESULT) (-1))
				MessageBoxW(GetAncestor(t->hwnd, GA_ROOT), L"No hit", L"No hit", MB_OK);
			else {
				wsprintf(buf, L"item %d subitem %d htflags 0x%I32X",
					ht.iItem, ht.iSubItem, ht.flags);
				MessageBoxW(GetAncestor(t->hwnd, GA_ROOT), buf, buf, MB_OK);
			}
		}
		*lResult = 0;
		return TRUE;
#else
		hr = uiprivTableHandleNM_CLICK(t, (NMITEMACTIVATE *) nmhdr, lResult);
		if (hr != S_OK) {
			// TODO
			return FALSE;
		}
		return TRUE;
#endif
	case LVN_ITEMCHANGED:
		{
			NMLISTVIEW *nm = (NMLISTVIEW *) nmhdr;
			UINT oldSelected, newSelected;
			HRESULT hr;

			// TODO clean up these if cases
			if (!t->inLButtonDown && t->edit == NULL)
				return FALSE;
			oldSelected = nm->uOldState & LVIS_SELECTED;
			newSelected = nm->uNewState & LVIS_SELECTED;
			if (t->inLButtonDown && oldSelected == 0 && newSelected != 0) {
				t->inDoubleClickTimer = TRUE;
				// TODO check error
				SetTimer(t->hwnd, (UINT_PTR) (&(t->inDoubleClickTimer)),
					GetDoubleClickTime(), NULL);
				*lResult = 0;
				return TRUE;
			}
			// the nm->iItem == -1 case is because that is used if "the change has been applied to all items in the list view"
			if (t->edit != NULL && oldSelected != 0 && newSelected == 0 && (t->editedItem == nm->iItem || nm->iItem == -1)) {
				// TODO see if the real list view accepts or rejects changes here; Windows Explorer accepts
				hr = uiprivTableFinishEditingText(t);
				if (hr != S_OK) {
					// TODO
					return FALSE;
				}
				*lResult = 0;
				return TRUE;
			}
			return FALSE;
		}
	// the real list view accepts changes when scrolling or clicking column headers
	case LVN_BEGINSCROLL:
	case LVN_COLUMNCLICK:
		hr = uiprivTableFinishEditingText(t);
		if (hr != S_OK) {
			// TODO
			return FALSE;
		}
		*lResult = 0;
		return TRUE;
	}
	return FALSE;
}

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);
	uiTableModel *model = t->model;
	std::vector<uiTable *>::iterator it;
	HRESULT hr;

	hr = uiprivTableAbortEditingText(t);
	if (hr != S_OK) {
		// TODO
	}
	uiWindowsUnregisterWM_NOTIFYHandler(t->hwnd);
	uiWindowsEnsureDestroyWindow(t->hwnd);
	// detach table from model
	for (it = model->tables->begin(); it != model->tables->end(); it++) {
		if (*it == t) {
			model->tables->erase(it);
			break;
		}
	}
	// free the columns
	for (auto col : *(t->columns))
		uiprivFree(col);
	delete t->columns;
	// t->imagelist will be automatically destroyed
	delete t->indeterminatePositions;
	uiFreeControl(uiControl(t));
}

uiWindowsControlAllDefaultsExceptDestroy(uiTable)

// suggested listview sizing from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing:
// "columns widths that avoid truncated data x an integral number of items"
// Don't think that'll cut it when some cells have overlong data (eg
// stupidly long URLs). So for now, just hardcode a minimum.
// TODO Investigate using LVM_GETHEADER/HDM_LAYOUT here
// TODO investigate using LVM_APPROXIMATEVIEWRECT here
#define tableMinWidth 107		/* in line with other controls */
#define tableMinHeight (14 * 3)	/* header + 2 lines (roughly) */

static void uiTableMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiTable *t = uiTable(c);
	uiWindowsSizing sizing;
	int x, y;

	x = tableMinWidth;
	y = tableMinHeight;
	uiWindowsGetSizing(t->hwnd, &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, &x, &y);
	*width = x;
	*height = y;
}

static uiprivTableColumnParams *appendColumn(uiTable *t, const char *name, int colfmt)
{
	WCHAR *wstr;
	LVCOLUMNW lvc;
	uiprivTableColumnParams *p;

	ZeroMemory(&lvc, sizeof (LVCOLUMNW));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = colfmt;
	lvc.cx = 120;			// TODO
	wstr = toUTF16(name);
	lvc.pszText = wstr;
	if (SendMessageW(t->hwnd, LVM_INSERTCOLUMNW, t->nColumns, (LPARAM) (&lvc)) == (LRESULT) (-1))
		logLastError(L"error calling LVM_INSERTCOLUMNW in appendColumn()");
	uiprivFree(wstr);
	t->nColumns++;

	p = uiprivNew(uiprivTableColumnParams);
	p->textModelColumn = -1;
	p->textEditableModelColumn = -1;
	p->textParams = uiprivDefaultTextColumnOptionalParams;
	p->imageModelColumn = -1;
	p->checkboxModelColumn = -1;
	p->checkboxEditableModelColumn = -1;
	p->progressBarModelColumn = -1;
	p->buttonModelColumn = -1;
	t->columns->push_back(p);
	return p;
}

void uiTableAppendTextColumn(uiTable *t, const char *name, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->textModelColumn = textModelColumn;
	p->textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->textParams = *textParams;
}

void uiTableAppendImageColumn(uiTable *t, const char *name, int imageModelColumn)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->imageModelColumn = imageModelColumn;
}

void uiTableAppendImageTextColumn(uiTable *t, const char *name, int imageModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->textModelColumn = textModelColumn;
	p->textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->textParams = *textParams;
	p->imageModelColumn = imageModelColumn;
}

void uiTableAppendCheckboxColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->checkboxModelColumn = checkboxModelColumn;
	p->checkboxEditableModelColumn = checkboxEditableModelColumn;
}

void uiTableAppendCheckboxTextColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->textModelColumn = textModelColumn;
	p->textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->textParams = *textParams;
	p->checkboxModelColumn = checkboxModelColumn;
	p->checkboxEditableModelColumn = checkboxEditableModelColumn;
}

void uiTableAppendProgressBarColumn(uiTable *t, const char *name, int progressModelColumn)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->progressBarModelColumn = progressModelColumn;
}

void uiTableAppendButtonColumn(uiTable *t, const char *name, int buttonModelColumn, int buttonClickableModelColumn)
{
	uiprivTableColumnParams *p;

	// TODO see if we can get rid of this parameter
	p = appendColumn(t, name, LVCFMT_LEFT);
	p->buttonModelColumn = buttonModelColumn;
	p->buttonClickableModelColumn = buttonClickableModelColumn;
}

uiTable *uiNewTable(uiTableParams *p)
{
	uiTable *t;
	int n;
	HRESULT hr;

	uiWindowsNewControl(uiTable, t);

	t->columns = new std::vector<uiprivTableColumnParams *>;
	t->model = p->Model;
	t->backgroundColumn = p->RowBackgroundColorModelColumn;

	// WS_CLIPCHILDREN is here to prevent drawing over the edit box used for editing text
	t->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		WC_LISTVIEW, L"",
		LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL | WS_CLIPCHILDREN | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL,
		hInstance, NULL,
		TRUE);
	t->model->tables->push_back(t);
	uiWindowsRegisterWM_NOTIFYHandler(t->hwnd, onWM_NOTIFY, uiControl(t));

	// TODO: try LVS_EX_AUTOSIZECOLUMNS
	// TODO check error
	SendMessageW(t->hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE,
		(WPARAM) (LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_SUBITEMIMAGES),
		(LPARAM) (LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_SUBITEMIMAGES));
	n = uiprivTableModelNumRows(t->model);
	if (SendMessageW(t->hwnd, LVM_SETITEMCOUNT, (WPARAM) n, 0) == 0)
		logLastError(L"error calling LVM_SETITEMCOUNT in uiNewTable()");

	hr = uiprivUpdateImageListSize(t);
	if (hr != S_OK) {
		// TODO
	}

	t->indeterminatePositions = new std::map<std::pair<int, int>, LONG>;
	if (SetWindowSubclass(t->hwnd, tableSubProc, 0, (DWORD_PTR) t) == FALSE)
		logLastError(L"SetWindowSubclass()");

	return t;
}
