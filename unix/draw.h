// 5 may 2016

// draw.c
struct uiDrawContext {
	cairo_t *cr;
};

// drawpath.c
extern void runPath(uiDrawPath *p, cairo_t *cr);
extern uiDrawFillMode pathFillMode(uiDrawPath *path);
