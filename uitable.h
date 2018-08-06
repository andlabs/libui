// 20 june 2016
// kept in a separate file for now

// uiImage stores an image for display on screen.
// 
// Images are built from one or more representations, each with the
// same aspect ratio but a different pixel size. libui automatically
// selects the most appropriate representation for drawing the image
// when it comes time to draw the image; what this means depends
// on the pixel density of the target context. Therefore, one can use
// uiImage to draw higher-detailed images on higher-density
// displays. The typical use cases are either:
// 
// 	- have just a single representation, at which point all screens
// 	  use the same image, and thus uiImage acts like a simple
// 	  bitmap image, or
// 	- have two images, one at normal resolution and one at 2x
// 	  resolution; this matches the current expectations of some
// 	  desktop systems at the time of writing (mid-2018)
// 
// uiImage is very simple: it only supports non-premultiplied 32-bit
// RGBA images, and libui does not provide any image file loading
// or image format conversion utilities on top of that.
typedef struct uiImage uiImage;

// @role uiImage constructor
// uiNewImage creates a new uiImage with the given width and
// height. This width and height should be the size in points of the
// image in the device-independent case; typically this is the 1x size.
// TODO for all uiImage functions: use const void * for const correctness
_UI_EXTERN uiImage *uiNewImage(double width, double height);

// @role uiImage destructor
// uiFreeImage frees the given image and all associated resources.
_UI_EXTERN void uiFreeImage(uiImage *i);

// uiImageAppend adds a representation to the uiImage.
// pixels should point to a byte array of non-premultiplied pixels
// stored in [R G B A] order (so ((uint8_t *) pixels)[0] is the R of the
// first pixel and [3] is the A of the first pixel). pixelWidth and
// pixelHeight is the size *in pixels* of the image, and pixelStride is
// the number *of bytes* per row of the pixels array. Therefore,
// pixels itself must be at least byteStride * pixelHeight bytes long.
_UI_EXTERN void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int byteStride);

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
