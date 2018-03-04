
// for attrstr.c
struct graphemes {
	size_t len;
	size_t *pointsToGraphemes;
	size_t *graphemesToPoints;
};
extern int graphemesTakesUTF16(void);
extern struct graphemes *graphemes(void *s, size_t len);

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
