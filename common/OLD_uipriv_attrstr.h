
// for attrstr.c
struct graphemes {
	size_t len;
	size_t *pointsToGraphemes;
	size_t *graphemesToPoints;
};
extern int graphemesTakesUTF16(void);
extern struct graphemes *graphemes(void *s, size_t len);

// TODO split these into a separate header file?

// attrstr.c
extern const uint16_t *attrstrUTF16(uiAttributedString *s);
extern size_t attrstrUTF16Len(uiAttributedString *s);
extern size_t attrstrUTF8ToUTF16(uiAttributedString *s, size_t n);
extern size_t *attrstrCopyUTF8ToUTF16(uiAttributedString *s, size_t *n);
extern size_t *attrstrCopyUTF16ToUTF8(uiAttributedString *s, size_t *n);

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
