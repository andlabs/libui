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
};
typedef struct uiprivSubitemDrawParams uiprivSubitemDrawParams;
struct uiprivSubitemDrawParams {
	bool selected;
	LRESULT bitmapMargin;
	RECT bounds;
	RECT icon;
	RECT label;
};

// tabletext.cpp
extern HRESULT uiprivLVN_GETDISPINFOText(uiTable *t, NMLVDISPINFOW *nm, uiprivTableColumnParams *p);

// tableimages.cpp
extern HRESULT uiprivLVN_GETDISPINFOImagesCheckboxes(uiTable *t, NMLVDISPINFOW *nm, uiprivTableColumnParams *p);
extern HRESULT uiprivNM_CUSTOMDRAWImagesCheckboxes(uiTable *t, NMLVCUSTOMDRAW *nm, uiprivSubitemDrawParams *dp);

// tabledraw.cpp
extern HRESULT uiprivTableHandleNM_CUSTOMDRAW(uiTable *t, NMLVCUSTOMDRAW *nm, LRESULT *lResult);
extern HRESULT uiprivUpdateImageListSize(uiTable *t);
