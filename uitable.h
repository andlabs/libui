// 20 june 2016
// kept in a separate file for now

typedef struct uiTableModel uiTableModel;
typedef struct uiTableModelHandler uiTableModelHandler;

_UI_ENUM(uiTableModelColumnType) {
	uiTableModelColumnString,
};

struct uiTableModelHandler {
	int (*NumColumns)(uiTableModelHandler *, uiTableModel *);
	uiTableModelColumnType (*ColumnType)(uiTableModelHandler *, uiTableModel *, int);
	int (*NumRows)(uiTableModelHandler *, uiTableModel *);
	void *(*CellValue)(uiTableModelHandler *, uiTableModel *, int, int);
	void (*SetCellValue)(uiTableModelHandler *, uiTableModel *, int, int, void *);
};

_UI_EXTERN void *uiTableModelStrdup(const char *str);

_UI_EXTERN uiTableModel *uiNewTableModel(uiTableModelHandler *mh);
_UI_EXTERN void uiFreeTableModel(uiTableModel *m);
_UI_EXTERN void uiTableModelRowInserted(uiTableModel *m, int newIndex);
_UI_EXTERN void uiTableModelRowChanged(uiTableModel *m, int index);
_UI_EXTERN void uiTableModelRowDeleted(uiTableModel *m, int oldIndex);
// TODO reordering/moving

typedef struct uiTableColumn uiTableColumn;

_UI_EXTERN void uiTableColumnAppendTextPart(uiTableColumn *c, int modelColumn, int expand);

typedef struct uiTable uiTable;
#define uiTable(this) ((uiTable *) (this))
_UI_EXTERN uiTableColumn *uiTableAppendColumn(uiTable *t, const char *name);
_UI_EXTERN uiTableColumn *uiTableAppendTextColumn(uiTable *t, const char *name, int modelColumn);
_UI_EXTERN uiTable *uiNewTable(uiTableModel *model);
