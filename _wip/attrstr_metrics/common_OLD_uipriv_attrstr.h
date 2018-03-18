

// TODO split these into a separate header file?

// drawtext.c
struct caretDrawParams {
	double r;
	double g;
	double b;
	double a;
	double xoff;
	double width;
};
extern void caretDrawParams(uiDrawContext *c, double height, struct caretDrawParams *p);
extern void drawTextBackground(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout, size_t start, size_t end, uiDrawBrush *brush, int isSelection);
