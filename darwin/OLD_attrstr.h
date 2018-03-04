// 4 march 2018

// attrstr.m
extern void initUnderlineColors(void);
extern void uninitUnderlineColors(void);
typedef void (^backgroundBlock)(uiDrawContext *c, uiDrawTextLayout *layout, double x, double y);
extern CFAttributedStringRef attrstrToCoreFoundation(uiDrawTextLayoutParams *p, NSArray **backgroundBlocks);
