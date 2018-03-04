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
