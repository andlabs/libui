// 10 june 2018
#include "../common/table.h"

// table.cpp
#define uiprivNumLVN_GETDISPINFOSkip 3
struct uiTableModel {
	uiTableModelHandler *mh;
	std::vector<uiTable *> *tables;
};
typedef struct uiprivTableColumnParams uiprivTableColumnParams;
struct uiprivTableColumnParams {
	int textModelColumn;
	int textEditableModelColumn;
	uiTableTextColumnOptionalParams textParams;

	int imageModelColumn;

	int checkboxModelColumn;
	int checkboxEditableModelColumn;

	int progressBarModelColumn;

	int buttonModelColumn;
	int buttonClickableModelColumn;
};
struct uiTable {
	uiWindowsControl c;
	uiTableModel *model;
	HWND hwnd;
	std::vector<uiprivTableColumnParams *> *columns;
	WPARAM nColumns;
	int backgroundColumn;
	// TODO make sure replacing images while selected in the listview is even allowed
	HIMAGELIST imagelist;
	// TODO document all this
	std::map<std::pair<int, int>, LONG> *indeterminatePositions;
	BOOL inLButtonDown;
	// TODO is this even necessary? it seems NM_CLICK is not sent if NM_DBLCLICK or LVN_ITEMACTIVATE (one of the two) happens...
	BOOL inDoubleClickTimer;
	HWND edit;
	int editedItem;
	int editedSubitem;
};
extern int uiprivTableProgress(uiTable *t, int item, int subitem, int modelColumn, LONG *pos);

// tabledispinfo.cpp
extern HRESULT uiprivTableHandleLVN_GETDISPINFO(uiTable *t, NMLVDISPINFOW *nm, LRESULT *lResult);

// tabledraw.cpp
extern HRESULT uiprivTableHandleNM_CUSTOMDRAW(uiTable *t, NMLVCUSTOMDRAW *nm, LRESULT *lResult);
extern HRESULT uiprivUpdateImageListSize(uiTable *t);

// tableediting.cpp
extern HRESULT uiprivTableResizeWhileEditing(uiTable *t);
extern HRESULT uiprivTableHandleNM_CLICK(uiTable *t, NMITEMACTIVATE *nm, LRESULT *lResult);
extern HRESULT uiprivTableFinishEditingText(uiTable *t);
extern HRESULT uiprivTableAbortEditingText(uiTable *t);

// tablemetrics.cpp
typedef struct uiprivTableMetrics uiprivTableMetrics;
struct uiprivTableMetrics {
	BOOL hasText;
	BOOL hasImage;
	BOOL focused;
	BOOL selected;

	RECT itemBounds;
	RECT itemIcon;
	RECT itemLabel;
	RECT subitemBounds;
	RECT subitemIcon;
	RECT subitemLabel;

	LRESULT bitmapMargin;
	int cxIcon;
	int cyIcon;

	RECT realTextBackground;
	RECT realTextRect;
};
extern HRESULT uiprivTableGetMetrics(uiTable *t, int iItem, int iSubItem, uiprivTableMetrics **mout);
