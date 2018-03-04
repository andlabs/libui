// 4 march 2018

// fontmatch.m
extern CTFontDescriptorRef fontdescToCTFontDescriptor(uiDrawFontDescriptor *fd);
extern CTFontDescriptorRef fontdescAppendFeatures(CTFontDescriptorRef desc, const uiOpenTypeFeatures *otf);
extern void fontdescFromCTFontDescriptor(CTFontDescriptorRef ctdesc, uiDrawFontDescriptor *uidesc);

// attrstr.m
extern void initUnderlineColors(void);
extern void uninitUnderlineColors(void);
typedef void (^backgroundBlock)(uiDrawContext *c, uiDrawTextLayout *layout, double x, double y);
extern CFAttributedStringRef attrstrToCoreFoundation(uiDrawTextLayoutParams *p, NSArray **backgroundBlocks);

// aat.m
typedef void (^aatBlock)(uint16_t type, uint16_t selector);
extern void openTypeToAAT(char a, char b, char c, char d, uint32_t value, aatBlock f);

// opentype.m
// TODO this is only used by opentype.m and aat.m; figure out some better way to handle this
// TODO remove x8tox32()
#define x8tox32(x) ((uint32_t) (((uint8_t) (x)) & 0xFF))
#define mkTag(a, b, c, d)		\
	((x8tox32(a) << 24) |	\
	(x8tox32(b) << 16) |		\
	(x8tox32(c) << 8) |		\
	x8tox32(d))
