typedef struct uiTable uiTable;
typedef struct uiTableModel uiTableModel;
typedef struct uiTableModelSpec uiTableModelSpec;
typedef struct uiTableColumnParams uiTableColumnParams;
typedef enum uiTableColumnType uiTableColumnType;
typedef enum uiTableNotification uiTableNotification;

_UI_EXTERN uintmax_t uiTableType(void);
#define uiTable(this) ((uiTable *) uiIsA((this), uiTableType(), 1))
_UI_EXTERN void uiTableSetModel(uiTable *t, uiTableModel *m);
_UI_EXTERN void uiTableAppendColumn(uiTable *t, uiTableColumnParams *p);
_UI_EXTERN uiTable *uiNewTable(void);

enum uiTableColumnType {
	uiTableColumnText,
//TODO	uiTableColumnImage,
	uiTableColumnCheckbox,
};

struct uiTableModelSpec {
	intmax_t (*NumRows)(uiTableModel *m, void *mData);
	void *(*CellValue)(uiTableModel *m, void *mData, intmax_t row, intmax_t column);
	void (*SetCellValue)(uiTableModel *m, void *mData, intmax_t row, intmax_t column, void *value);
};

enum uiTableNotification {
	uiTableRowInserted,
	uiTableRowDeleted,
	uiTableCellChanged,
};

_UI_EXTERN uiTableModel *uiNewTableModel(uintmax_t nCols, uiTableColumnType *types, uiTableModelSpec *spec, void *mData);
_UI_EXTERN void uiFreeTableModel(uiTableModel *m);
_UI_EXTERN void uiTableModelNotify(uiTableModel *m, uiTableNotification notification, intmax_t row, intmax_t column);

#define uiTableModelFromBool(b) ((void *) ((intptr_t) (b)))
_UI_EXTERN void *uiTableModelFromString(const char *str);

struct uiTableColumnParams {
	const char *Name;
	// TODO make this unnecessary
	uiTableColumnType Type;
	int Mutable;			// TODO move to the model?
	intmax_t ValueColumn;
	// TODO background color
};
