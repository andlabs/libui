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
// TODO see if we either need the stride or can provide a way to get the OS-preferred stride (in cairo we do)
_UI_EXTERN void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int byteStride);

// uiTableValue stores a value to be passed along uiTable and
// uiTableModel.
//
// You do not create uiTableValues directly; instead, you create a
// uiTableValue of a given type using the specialized constructor
// functions.
//
// uiTableValues are immutable and the uiTableModel and uiTable
// take ownership of the uiTableValue object once returned, copying
// its contents as necessary.
typedef struct uiTableValue uiTableValue;

// @role uiTableValue destructor
// uiFreeTableValue() frees a uiTableValue. You generally do not
// need to call this yourself, as uiTable and uiTableModel do this
// for you. In fact, it is an error to call this function on a uiTableValue
// that has been given to a uiTable or uiTableModel. You can call this,
// however, if you created a uiTableValue that you aren't going to
// use later, or if you called a uiTableModelHandler method directly
// and thus never transferred ownership of the uiTableValue.
_UI_EXTERN void uiFreeTableValue(uiTableValue *v);

// uiTableValueType holds the possible uiTableValue types that may
// be returned by uiTableValueGetType(). Refer to the documentation
// for each type's constructor function for details on each type.
// TODO actually validate these
_UI_ENUM(uiTableValueType) {
	uiTableValueTypeString,
	uiTableValueTypeImage,
	uiTableValueTypeInt,
	uiTableValueTypeColor,
};

// uiTableValueGetType() returns the type of v.
// TODO I don't like this name
_UI_EXTERN uiTableValueType uiTableValueGetType(const uiTableValue *v);

// uiNewTableValueString() returns a new uiTableValue that contains
// str. str is copied; you do not need to keep it alive after
// uiNewTableValueString() returns.
_UI_EXTERN uiTableValue *uiNewTableValueString(const char *str);

// uiTableValueString() returns the string stored in v. The returned
// string is owned by v. It is an error to call this on a uiTableValue
// that does not hold a string.
_UI_EXTERN const char *uiTableValueString(const uiTableValue *v);

// uiNewTableValueImage() returns a new uiTableValue that contains
// the given uiImage.
// 
// Unlike other similar constructors, uiNewTableValueImage() does
// NOT copy the image. This is because images are comparatively
// larger than the other objects in question. Therefore, you MUST
// keep the image alive as long as the returned uiTableValue is alive.
// As a general rule, if libui calls a uiTableModelHandler method, the
// uiImage is safe to free once any of your code is once again
// executed.
_UI_EXTERN uiTableValue *uiNewTableValueImage(uiImage *img);

// uiTableValueImage() returns the uiImage stored in v. As these
// images are not owned by v, you should not assume anything
// about the lifetime of the image (unless you created the image,
// and thus control its lifetime). It is an error to call this on a
// uiTableValue that does not hold an image.
_UI_EXTERN uiImage *uiTableValueImage(const uiTableValue *v);

// uiNewTableValueInt() returns a uiTableValue that stores the given
// int. This can be used both for boolean values (nonzero is true, as
// in C) or progresses (in which case the valid range is -1..100
// inclusive).
_UI_EXTERN uiTableValue *uiNewTableValueInt(int i);

// uiTableValueInt() returns the int stored in v. It is an error to call
// this on a uiTableValue that does not store an int.
_UI_EXTERN int uiTableValueInt(const uiTableValue *v);

// uiNewTableValueColor() returns a uiTableValue that stores the
// given color.
_UI_EXTERN uiTableValue *uiNewTableValueColor(double r, double g, double b, double a);

// uiTableValueColor() returns the color stored in v. It is an error to
// call this on a uiTableValue that does not store a color.
// TODO define whether all this, for both uiTableValue and uiAttribute, is undefined behavior or a caught error
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
