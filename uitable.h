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



// uiTableStyleFlags are bitflags to control table styles.
_UI_ENUM(uiTableStyleFlags) {
	// uiTableStyleMultiSelect allows the user to select more than
	// one row at a time (usually by holding down shift or ctrl keys)
	uiTableStyleMultiSelect = 1<<0,
	// TODO support more flags:
	// - hide column headers?
	// - allow column reordering?
	// - allow column resizing?
	// - allow click on columns (+indicators for ascending/descending)?
};

// uiTableSetStyle sets the style flags governing the table
_UI_EXTERN void uiTableSetStyle(uiTable *t, uiTableStyleFlags style);

// uiTableStyleFlags returns the tables currently-active style flags
_UI_EXTERN uiTableStyleFlags uiTableStyle(uiTable *t);

// uiTableOnSelectionChanged sets a handler function which is invoked
// (with `data` as a parameter whenever the set of selected item(s)
// is changed. This will replace any previously-set handler function.
_UI_EXTERN void uiTableOnSelectionChanged(uiTable *t, void (*f)(uiTable *, void *), void *data);


// uiTableIter is an object for iterating over the selected rows in a uiTable
// The effect of modifying the uiTable or underlying data model during
// iteration is undefined, and likely varies by platform.
typedef struct uiTableIter uiTableIter;

// uiTableGetSelection returns a uiTableIter which can be used to iterate
// over the selected rows in the table.
// After use, it should be freed with uiTableIterComplete().
//
//  example usage:
//
//		uiTableIter *sel = uiTableGetSelection(t);
//  	while (uiTableIterAdvance(sel)) {
//			int rowIndex = uiTableIterCurrent(sel);
//			...
//		}
//		uiTableIterComplete(sel);
//
_UI_EXTERN uiTableIter* uiTableGetSelection(uiTable *t);

// uiTableIterAdvance advances the iterator to the next item.
// if no more items are available, 0 is returned, else 1
_UI_EXTERN int uiTableIterAdvance(uiTableIter *it);

// uiTableIterCurrent returns the row index at the current
// position (undefined after uiTableIterAdvance returns 0)
_UI_EXTERN int uiTableIterCurrent(uiTableIter *it);

// uiTableIterComplete frees the iteration object.
// There is no requirement that the iteration must fetch
// every item - you can call this at any point.
// (TODO should be called uiTableIterDestroy?
_UI_EXTERN void uiTableIterComplete(uiTableIter *it);


