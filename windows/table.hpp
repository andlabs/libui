// 10 june 2018

// table.cpp
#define uiprivNumLVN_GETDISPINFOSkip 3
struct uiTableModel {
	uiTableModelHandler *mh;
	std::vector<uiTable *> *tables;
};
typedef struct uiprivTableColumnParams uiprivTableColumnParams;
struct uiprivTableColumnParams {
	int textModelColumn;
	int textEditableColumn;
	uiTableTextColumnOptionalParams textParams;

	int imageModelColumn;

	int checkboxModelColumn;
	int checkboxEditableColumn;

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
};
extern int uiprivTableProgress(uiTable *t, int item, int subitem, int modelColumn, LONG *pos);

// tabledispinfo.cpp
extern HRESULT uiprivTableHandleLVN_GETDISPINFO(uiTable *t, NMLVDISPINFOW *nm, LRESULT *lResult);

// tabledraw.cpp
extern HRESULT uiprivTableHandleNM_CUSTOMDRAW(uiTable *t, NMLVCUSTOMDRAW *nm, LRESULT *lResult);
extern HRESULT uiprivUpdateImageListSize(uiTable *t);
