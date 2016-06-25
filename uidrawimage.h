// 25 june 2016

typedef struct uiDrawSingleResImage uiDrawSingleResImage;
typedef struct uiDrawMultiResImage uiDrawMultiResImage;

_UI_EXTERN uiDrawSingleResImage *uiDrawNewSingleResImage(void *data, int width, int height, int stride);
_UI_EXTERN void uiDrawFreeImage(uiDrawSingleResImage *i);
_UI_EXTERN void uiDrawSingleResImageNativeSize(uiDrawSingleResImage *i, uiDrawContext *c, double *width, double *height);

_UI_EXTERN uiDrawMultiResImage *uiDrawNewMultiResImage(double width, double height);
_UI_EXTERN void uiDrawFreeMultiResImage(uiDrawMultiResImage *i);
_UI_EXTERN void uiDrawMultiResImageAppend(void *data, int pixelWidth, int pixelHeight, int pixelStride);

_UI_EXTERN void uiDrawSingleRes(uiDrawContext *c, double x, double y, double width, double height uiDrawSingleResImage *image, int interpolate);
_UI_EXTERN void uiDrawMultiRes(uiDrawContext *c, double x, double y, double width, double height, uiDrawMultiResImage *image, int interpolate);
