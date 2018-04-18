// 19 february 2018

#ifdef __cplusplus
extern "C" {
#endif

// attribute.c
extern uiAttribute *uiprivAttributeRetain(uiAttribute *a);
extern void uiprivAttributeRelease(uiAttribute *a);
extern int uiprivAttributeEqual(const uiAttribute *a, const uiAttribute *b);

// opentype.c
extern int uiprivOpenTypeFeaturesEqual(const uiOpenTypeFeatures *a, const uiOpenTypeFeatures *b);

// attrlist.c
typedef struct uiprivAttrList uiprivAttrList;
extern uiprivAttrList *uiprivNewAttrList(void);
extern void uiprivFreeAttrList(uiprivAttrList *alist);
extern void uiprivAttrListInsertAttribute(uiprivAttrList *alist, uiAttribute *val, size_t start, size_t end);
extern void uiprivAttrListInsertCharactersUnattributed(uiprivAttrList *alist, size_t start, size_t count);
extern void uiprivAttrListInsertCharactersExtendingAttributes(uiprivAttrList *alist, size_t start, size_t count);
extern void uiprivAttrListRemoveAttribute(uiprivAttrList *alist, uiAttributeType type, size_t start, size_t end);
extern void uiprivAttrListRemoveAttributes(uiprivAttrList *alist, size_t start, size_t end);
extern void uiprivAttrListRemoveCharacters(uiprivAttrList *alist, size_t start, size_t end);
extern void uiprivAttrListForEach(const uiprivAttrList *alist, const uiAttributedString *s, uiAttributedStringForEachAttributeFunc f, void *data);

// attrstr.c
extern const uint16_t *uiprivAttributedStringUTF16String(const uiAttributedString *s);
extern size_t uiprivAttributedStringUTF16Len(const uiAttributedString *s);
extern size_t uiprivAttributedStringUTF8ToUTF16(const uiAttributedString *s, size_t n);
extern size_t *uiprivAttributedStringCopyUTF8ToUTF16Table(const uiAttributedString *s, size_t *n);
extern size_t *uiprivAttributedStringCopyUTF16ToUTF8Table(const uiAttributedString *s, size_t *n);

// per-OS graphemes.c/graphemes.cpp/graphemes.m/etc.
typedef struct uiprivGraphemes uiprivGraphemes;
struct uiprivGraphemes {
	size_t len;
	size_t *pointsToGraphemes;
	size_t *graphemesToPoints;
};
extern int uiprivGraphemesTakesUTF16(void);
extern uiprivGraphemes *uiprivNewGraphemes(void *s, size_t len);

#ifdef __cplusplus
}
#endif
