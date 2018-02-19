
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

// attrlist.c
struct attrlist;
extern void attrlistInsertAttribute(struct attrlist *alist, uiAttributeSpec *spec, size_t start, size_t end);
extern void attrlistInsertCharactersUnattributed(struct attrlist *alist, size_t start, size_t count);
extern void attrlistInsertCharactersExtendingAttributes(struct attrlist *alist, size_t start, size_t count);
extern void attrlistRemoveAttribute(struct attrlist *alist, uiAttribute type, size_t start, size_t end);
extern void attrlistRemoveAttributes(struct attrlist *alist, size_t start, size_t end);
extern void attrlistRemoveCharacters(struct attrlist *alist, size_t start, size_t end);
extern void attrlistForEach(struct attrlist *alist, uiAttributedString *s, uiAttributedStringForEachAttributeFunc f, void *data);
// TODO move these to the top like everythng else
extern struct attrlist *attrlistNew(void);
extern void attrlistFree(struct attrlist *alist);

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
