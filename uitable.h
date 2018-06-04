// 20 june 2016
// kept in a separate file for now

typedef struct uiImage uiImage;

// TODO use const void * for const correctness
_UI_EXTERN uiImage *uiNewImage(double width, double height);
_UI_EXTERN void uiFreeImage(uiImage *i);
_UI_EXTERN void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int pixelStride);

typedef struct uiTableData uiTableData;

_UI_EXTERN void uiFreeTableData(uiTableData *d);

// TODO actually validate these
_UI_ENUM(uiTableDataType) {
	uiTableDataTypeString,
	uiTableDataTypeImage,
	uiTableDataTypeInt,
	uiTableDataTypeColor,
};

// TODO I don't like this name
_UI_EXTERN uiTableDataType uiTableDataGetType(const uiTableData *d);

_UI_EXTERN uiTableData *uiNewTableDataString(const char *str);
_UI_EXTERN const char *uiTableDataString(const uiTableData *d);

_UI_EXTERN uiTableData *uiNewTableDataImage(uiImage *img);
_UI_EXTERN uiImage *uiTableDataImage(const uiTableData *d);

_UI_EXTERN uiTableData *uiNewTableDataInt(int i);
_UI_EXTERN int uiTableDataInt(const uiTableData *d);

_UI_EXTERN uiTableData *uiNewTableDataColor(double r, double g, double b, double a);
_UI_EXTERN void uiTableDataColor(const uiTableData *d, double *r, double *g, double *b, double *a);

typedef struct uiTableModel uiTableModel;
typedef struct uiTableModelHandler uiTableModelHandler;

// TODO validate ranges; validate types on each getter/setter call (? table columns only?)
struct uiTableModelHandler {
	int (*NumColumns)(uiTableModelHandler *, uiTableModel *);
	uiTableDataType (*ColumnType)(uiTableModelHandler *, uiTableModel *, int);
	int (*NumRows)(uiTableModelHandler *, uiTableModel *);
	uiTableData *(*CellValue)(uiTableModelHandler *, uiTableModel *, int, int);
	void (*SetCellValue)(uiTableModelHandler *, uiTableModel *, int, int, const uiTableData *);
};

_UI_EXTERN uiTableModel *uiNewTableModel(uiTableModelHandler *mh);
_UI_EXTERN void uiFreeTableModel(uiTableModel *m);
_UI_EXTERN void uiTableModelRowInserted(uiTableModel *m, int newIndex);
_UI_EXTERN void uiTableModelRowChanged(uiTableModel *m, int index);
_UI_EXTERN void uiTableModelRowDeleted(uiTableModel *m, int oldIndex);
// TODO reordering/moving

#define uiTableModelColumnNeverEditable (-1)
#define uiTableModelColumnAlwaysEditable (-2)

typedef struct uiTableTextColumnOptionalParams uiTableTextColumnOptionalParams;

struct uiTableTextColumnOptionalParams {
	int ColorModelColumn;
};

typedef struct uiTable uiTable;
#define uiTable(this) ((uiTable *) (this))
_UI_EXTERN void uiTableAppendTextColumn(uiTable *t,
	const char *name,
	int textModelColumn,
	int textEditableModelColumn,
	uiTableTextColumnOptionalParams *params);
_UI_EXTERN void uiTableAppendImageColumn(uiTable *t,
	const char *name,
	int imageModelColumn);
_UI_EXTERN void uiTableAppendImageTextColumn(uiTable *t,
	const char *name,
	int imageModelColumn,
	int textModelColumn,
	int textEditableModelColumn,
	uiTableTextColumnOptionalParams *textParams);
_UI_EXTERN void uiTableAppendCheckboxColumn(uiTable *t,
	const char *name,
	int checkboxModelColumn,
	int checkboxEditableModelColumn);
_UI_EXTERN void uiTableAppendCheckboxTextColumn(uiTable *t,
	const char *name,
	int checkboxModelColumn,
	int checkboxEditableModelColumn,
	int textModelColumn,
	int textEditableModelColumn,
	uiTableTextColumnOptionalParams *textParams);
_UI_EXTERN void uiTableAppendProgressBarColumn(uiTable *t,
	const char *name,
	int progressModelColumn);
_UI_EXTERN void uiTableAppendButtonColumn(uiTable *t,
	const char *name,
	int buttonTextModelColumn,
	int buttonClickableModelColumn);
// TODO getter?
_UI_EXTERN void uiTableSetRowBackgroundColorModelColumn(uiTable *t, int modelColumn);
_UI_EXTERN uiTable *uiNewTable(uiTableModel *model);
