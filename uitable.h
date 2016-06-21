// 20 june 2016
// kept in a separate file for now

typedef struct uiTableModel uiTableModel;
typedef struct uiTableModelHandler uiTableModelHandler;

_UI_ENUM(uiTableModelColumnType) {
	uiTableModelColumnString,
};

struct uiTableModelHandler {
	int (*NumColumns)(uiTableModel *);
	uiTableModelColumnType (*ColumnType)(uiTableModel *, int);
	int (*NumRows)(uiTableModel *);
	void *(*CellValue)(uiTableModel *, int, int);
	void (*SetCellValue)(uiTableModel *, int, int, void *);
};

_UI_EXTERN void *uiTableModelStrdup(const char *str);

_UI_EXTERN uiTableModel *uiNewTableModel(uiTableModelHandler *mh);
_UI_EXTERN void uiFreeTableModel(uiTableModel *m);
_UI_EXTERN void uiTableModelRowInserted(uiTableModel *m, int newIndex);
_UI_EXTERN void uiTableModelRowChanged(uiTableModel *m, int index);
_UI_EXTERN void uiTableModelRowDeleted(uiTableModel *m, int oldIndex);

typedef struct uiTableCellLayout uiTableCellLayout;
typedef struct uiTableCellPart uiTableCellPart;

_UI_EXTERN uiTableCellLayout *uiNewTableCellLayout(void);
_UI_EXTERN void uiFreeTableCellLayout(uiTableCellLayout *c);
_UI_EXTERN void uiTableCellLayoutAppend(uiTableCellLayout *c, uiTableCellPart *part, int expand);

_UI_EXTERN uiTableCellPart *uiNewTableTextPart(int modelColumn);
_UI_EXTERN void uiFreeTableCellPart(uiTableCellPart *p);

typedef struct uiTable uiTable;
#define uiTable(this) ((uiTable *) (this))
_UI_EXTERN void uiTableAppendColumn(uiTable *t, const char *name, uiTableCellLayout *layout);
_UI_EXTERN void uiTableAppendTextColumn(uiTable *t, const char *name, int modelColumn);
_UI_EXTERN uiTable *uiNewTable(uiTableModel *model);
