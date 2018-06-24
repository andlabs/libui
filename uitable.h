// 20 june 2016
// kept in a separate file for now

typedef struct uiImage uiImage;

// TODO use const void * for const correctness
_UI_EXTERN uiImage *uiNewImage(double width, double height);
_UI_EXTERN void uiFreeImage(uiImage *i);
_UI_EXTERN void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int pixelStride);

typedef struct uiTableValue uiTableValue;

_UI_EXTERN void uiFreeTableValue(uiTableValue *v);

// TODO actually validate these
_UI_ENUM(uiTableValueType) {
	uiTableValueTypeString,
	uiTableValueTypeImage,
	uiTableValueTypeInt,
	uiTableValueTypeColor,
};

// TODO I don't like this name
_UI_EXTERN uiTableValueType uiTableValueGetType(const uiTableValue *v);

_UI_EXTERN uiTableValue *uiNewTableValueString(const char *str);
_UI_EXTERN const char *uiTableValueString(const uiTableValue *v);

_UI_EXTERN uiTableValue *uiNewTableValueImage(uiImage *img);
_UI_EXTERN uiImage *uiTableValueImage(const uiTableValue *v);

_UI_EXTERN uiTableValue *uiNewTableValueInt(int i);
_UI_EXTERN int uiTableValueInt(const uiTableValue *v);

_UI_EXTERN uiTableValue *uiNewTableValueColor(double r, double g, double b, double a);
_UI_EXTERN void uiTableValueColor(const uiTableValue *v, double *r, double *g, double *b, double *a);

typedef struct uiTableModel uiTableModel;
typedef struct uiTableModelHandler uiTableModelHandler;

// TODO validate ranges; validate types on each getter/setter call (? table columns only?)
struct uiTableModelHandler {
	int (*NumColumns)(uiTableModelHandler *, uiTableModel *);
	uiTableValueType (*ColumnType)(uiTableModelHandler *, uiTableModel *, int);
	int (*NumRows)(uiTableModelHandler *, uiTableModel *);
	uiTableValue *(*CellValue)(uiTableModelHandler *, uiTableModel *, int, int);
	void (*SetCellValue)(uiTableModelHandler *, uiTableModel *, int, int, const uiTableValue *);
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
typedef struct uiTableParams uiTableParams;

struct uiTableTextColumnOptionalParams {
	int ColorModelColumn;
};

struct uiTableParams {
	uiTableModel *Model;
	int RowBackgroundColorModelColumn;
};

typedef struct uiTable uiTable;
#define uiTable(this) ((uiTable *) (this))
_UI_EXTERN void uiTableAppendTextColumn(uiTable *t,
	const char *name,
	int textModelColumn,
	int textEditableModelColumn,
	uiTableTextColumnOptionalParams *textParams);
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
	int buttonModelColumn,
	int buttonClickableModelColumn);
_UI_EXTERN uiTable *uiNewTable(uiTableParams *params);
