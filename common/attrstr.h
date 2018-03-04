// 19 february 2018

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
extern void uiprivAttrListRemoveAttribute(uiprivAttrList *alist, uiAttribute type, size_t start, size_t end);
extern void uiprivAttrListRemoveAttributes(uiprivAttrList *alist, size_t start, size_t end);
extern void uiprivAttrListRemoveCharacters(uiprivAttrList *alist, size_t start, size_t end);
extern void uiprivAttrListForEach(uiprivAttrList *alist, uiAttributedString *s, uiAttributedStringForEachAttributeFunc f, void *data);

// attrstr.c
extern const uint16_t *uiprivAttributedStringUTF16String(uiAttributedString *s);
extern size_t uiprivAttributedStringUTF16Len(uiAttributedString *s);
extern size_t uiprivAttributedStringUTF8ToUTF16(uiAttributedString *s, size_t n);
extern size_t *uiprivAttributedStringCopyUTF8ToUTF16Table(uiAttributedString *s, size_t *n);
extern size_t *uiprivAttributedStringCopyUTF16ToUTF8Table(uiAttributedString *s, size_t *n);
