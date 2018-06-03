// 20 june 2016
// kept in a separate file for now

typedef struct uiImage uiImage;

// TODO use const void * for const correctness
_UI_EXTERN uiImage *uiNewImage(double width, double height);
_UI_EXTERN void uiFreeImage(uiImage *i);
_UI_EXTERN void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int pixelStride);

typedef struct uiTableModel uiTableModel;
typedef struct uiTableModelHandler uiTableModelHandler;

// TODO actually validate these
_UI_ENUM(uiTableModelColumnType) {
	uiTableModelColumnString,
	uiTableModelColumnImage,
	uiTableModelColumnInt,
	uiTableModelColumnColor,
};

// TODO validate ranges; validate types on each getter/setter call (? table columns only?)
struct uiTableModelHandler {
	int (*NumColumns)(uiTableModelHandler *, uiTableModel *);
	uiTableModelColumnType (*ColumnType)(uiTableModelHandler *, uiTableModel *, int);
	int (*NumRows)(uiTableModelHandler *, uiTableModel *);
	void *(*CellValue)(uiTableModelHandler *, uiTableModel *, int, int);
	void (*SetCellValue)(uiTableModelHandler *, uiTableModel *, int, int, const void *);
};

_UI_EXTERN void *uiTableModelStrdup(const char *str);
// TODO rename the strdup one to this too
_UI_EXTERN void *uiTableModelGiveColor(double r, double g, double b, double a);
_UI_EXTERN void *uiTableModelGiveInt(int i);
// TODO TakeString
// TODO add const
_UI_EXTERN int uiTableModelTakeInt(void *v);

_UI_EXTERN uiTableModel *uiNewTableModel(uiTableModelHandler *mh);
_UI_EXTERN void uiFreeTableModel(uiTableModel *m);
_UI_EXTERN void uiTableModelRowInserted(uiTableModel *m, int newIndex);
_UI_EXTERN void uiTableModelRowChanged(uiTableModel *m, int index);
_UI_EXTERN void uiTableModelRowDeleted(uiTableModel *m, int oldIndex);
// TODO reordering/moving

typedef struct uiTableColumn uiTableColumn;

_UI_EXTERN void uiTableColumnAppendTextPart(uiTableColumn *c, int modelColumn, int expand);
// TODO images shouldn't expand...
_UI_EXTERN void uiTableColumnAppendImagePart(uiTableColumn *c, int modelColumn, int expand);
_UI_EXTERN void uiTableColumnAppendButtonPart(uiTableColumn *c, int modelColumn, int expand);
// TODO should these have labels?
_UI_EXTERN void uiTableColumnAppendCheckboxPart(uiTableColumn *c, int modelColumn, int expand);
_UI_EXTERN void uiTableColumnAppendProgressBarPart(uiTableColumn *c, int modelColumn, int expand);
// TODO Editable?
_UI_EXTERN void uiTableColumnPartSetEditable(uiTableColumn *c, int part, int editable);
_UI_EXTERN void uiTableColumnPartSetTextColor(uiTableColumn *c, int part, int modelColumn);

typedef struct uiTable uiTable;
#define uiTable(this) ((uiTable *) (this))
_UI_EXTERN uiTableColumn *uiTableAppendColumn(uiTable *t, const char *name);
_UI_EXTERN uiTableColumn *uiTableAppendTextColumn(uiTable *t, const char *name, int modelColumn);
// TODO getter?
_UI_EXTERN void uiTableSetRowBackgroundColorModelColumn(uiTable *t, int modelColumn);
_UI_EXTERN uiTable *uiNewTable(uiTableModel *model);
