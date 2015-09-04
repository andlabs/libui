# uiArea

uiArea is a uiControl that provides a canvas you can draw on. It receives keyboard and mouse events, supports scrolling, is DPI aware, and has several other useful features. A uiArea consists of the drawing area itself and horizontal and vertical scrollbars.

## The Area Handler

A uiArea is driven by an *area handler*. An area handler is an object with several methods that uiArea calls to do certain tasks. To create an area handler, simply have a structure whose first member is of type `uiAreaHandler`:

```c
struct uiAreaHandler {
	void (*Draw)(uiAreaHandler *h, uiArea *a, uiAreaDrawParams *p);
	void (*HScrollConfig)(uiAreaHandler *h, uiArea *a, uiAreaScrollConfig *c);
	void (*VScrollConfig)(uiAreaHandler *h, uiArea *a, uiAreaScrollConfig *c);
}
```

## Drawing

Unlike drawing canvas controls in other toolkits, uiArea does **not** have a fixed size. The coordinate (0, 0) is always the top-left corner of the drawing area, regardless of how big the uiArea is in the current window or where the scrollbars presently are. Instead, you simulate a size by setting the scrollbar bounds, and you are given the current scrolling positions to base your drawing with.

The visible drawing area is called the *content area* by the drawing machinery.

TODO have a diagram.

When a part of the uiArea needs to be redrawn, the area handler's `Draw()` method is called. It receives the area handler, the uiArea, and a structure of parameters necessary for drawing.

```c
struct uiAreaDrawParams {
	uiDrawContext *context;

	intmax_t contentWidth;
	intmax_t contentHeight;

	intmax_t hscrollpos;
	intmax_t vscrollpos;

	intmax_t clipX;
	intmax_t cilpY;
	intmax_t clipWidth;
	intmax_t clipHeight;

	TODO dpiX;
	TODO dpiY;
};
```

`context` is the drawing context; see drawing.md for details.

`contentWidth` and `contentHeight` is the current width and height of the content area. `hscrollpos` and `vscrollpos` are the current horizontal and vertical positions of the scrollbars, in units defined by the scrollbar configurations; see below.

`clipX`, `clipY`, `clipWidth`, and `clipHeight` define a rectangle, in content area coordinates, that the OS has requested to be redrawn. You can use this to optimize your drawing by only drawing where drawing is needed; the OS may choose to drop any drawing done outside the clip rectangle.

`dpiX` and `dpiY` are the uiArea's current DPI in the X and Y directions, respectively. Do not save these values; they are not guaranteed to stay the same once `Draw()` returns.
