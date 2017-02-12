// 12 february 2017
#import "uipriv_darwin.h"

// unlike the other systems, Core Text rolls family, size, weight, italic, width, AND opentype features into the "font" attribute
// TODO see if we could use NSAttributedString?
// TODO consider renaming this struct and the fep variable(s)
struct foreachParams {
	CFMutableAttributedStringRef mas;
	NSMutableDictionary *converted;
	uiDrawFontDescriptor *defaultFont;
};

static void ensureFontInRange(struct foreachParams *p, size_t start, size_t end)
{
	size_t i;
	NSNumber *n;
	uiDrawFontDescriptor *new;

	for (i = start; i < end; i++) {
		n = [NSNumber numberWithInteger:i];
		if ([p->converted objectForKey:n] != nil)
			continue;
		new = uiNew(uiDrawFontDescriptor);
		*new = *(p->defaultFont);
		[p->converted setObject:[NSValue valueWithPointer:new] forKey:n];
	}
}

static void adjustFontInRange(struct foreachParams *p, size_t start, size_t end, void (^adj)(uiDrawFontDescriptor *desc))
{
	size_t i;
	NSNumber *n;
	NSValue *v;

	for (i = start; i < end; i++) {
		n = [NSNumber numberWithInteger:i];
		v = (NSValue *) [p->converted objectForKey:n];
		adj((uiDrawFontDescriptor *) [v pointerValue]);
	}
}

static int processAttribute(uiAttributedString *s, uiAttribute type, uintptr_t value, size_t start, size_t end, void *data)
{
	struct foreachParams *p = (struct foreachParams *) data;

	start = attrstrUTF8ToUTF16(s, start);
	end = attrstrUTF8ToUTF16(s, end);
	switch (type) {
	case uiAttributeFamily:
		ensureFontInRange(p, start, end);
		adjustFontInRange(p, start, end, ^(uiDrawFontDescriptor *desc) {
			desc->Family = (char *) value;
		});
		break;
	// TODO
	}
	return 0;
}

static CTFontRef fontdescToCTFont(uiDrawFontDescriptor *fd)
{
	CTFontDescriptorRef desc;
	CTFontRef font;

	desc = fontdescToCTFontDescriptor(fd);
	font = CTFontCreateWithFontDescriptor(desc, fd->Size, NULL);
	CFRelease(desc);			// TODO correct?
	return font;
}

static void applyAndFreeFontAttributes(struct foreachParams *p)
{
	[p->converted enumerateKeysAndObjectsUsingBlock:^(NSNumber *key, NSValue *val, BOOL *stop) {
		uiDrawFontDescriptor *desc;
		CTFontRef font;
		CFRange range;

		desc = (uiDrawFontDescriptor *) [val pointerValue];
		font = fontdescToCTFont(desc);
		range.location = [key integerValue];
		range.length = 1;
		CFAttributedStringSetAttribute(p->mas, range, kCTFontAttributeName, font);
		CFRelease(font);
		uiFree(desc);
	}];
}

static const CTTextAlignment ctaligns[] = {
	[uiDrawTextLayoutAlignLeft] = kCTTextAlignmentLeft,
	[uiDrawTextLayoutAlignCenter] = kCTTextAlignmentCenter,
	[uiDrawTextLayoutAlignRight] = kCTTextAlignmentRight,
};

static CTParagraphStyleRef mkParagraphStyle(uiDrawTextLayoutParams *p)
{
	CTParagraphStyleRef ps;
	CTParagraphStyleSetting settings[16];
	size_t nSettings = 0;

	settings[nSettings].spec = kCTParagraphStyleSpecifierAlignment;
	settings[nSettings].valueSize = sizeof (CTTextAlignment);
	settings[nSettings].value = ctaligns + p->Align;
	nSettings++;

	ps = CTParagraphStyleCreate(settings, nSettings);
	if (ps == NULL) {
		// TODO
	}
	return ps;
}

CFAttributedStringRef attrstrToCoreFoundation(uiDrawTextLayoutParams *p)
{
	CFStringRef cfstr;
	CFMutableDictionaryRef defaultAttrs;
	CTFontRef defaultCTFont;
	CTParagraphStyleRef ps;
	CFAttributedStringRef base;
	CFMutableAttributedStringRef mas;
	struct foreachParams fep;

	cfstr = CFStringCreateWithCharacters(NULL, attrstrUTF16(p->String), attrstrUTF16Len(p->String));
	if (cfstr == NULL) {
		// TODO
	}
	defaultAttrs = CFDictionaryCreateMutable(NULL, 0,
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (defaultAttrs == NULL) {
		// TODO
	}
	defaultCTFont = fontdescToCTFont(p->DefaultFont);
	CFDictionaryAddValue(defaultAttrs, kCTFontAttributeName, defaultCTFont);
	CFRelease(defaultCTFont);
	ps = mkParagraphStyle(p);
	CFDictionaryAddValue(defaultAttrs, kCTParagraphStyleAttributeName, ps);
	CFRelease(ps);

	base = CFAttributedStringCreate(NULL, cfstr, defaultAttrs);
	if (base == NULL) {
		// TODO
	}
	CFRelease(cfstr);
	CFRelease(defaultAttrs);
	mas = CFAttributedStringCreateMutableCopy(NULL, 0, base);
	CFRelease(base);

	CFAttributedStringBeginEditing(mas);
	fep.mas = mas;
	fep.converted = [NSMutableDictionary new];
	fep.defaultFont = p->DefaultFont;
	uiAttributedStringForEachAttribute(p->String, processAttribute, &fep);
	applyAndFreeFontAttributes(&fep);
	[fep.converted release];
	CFAttributedStringEndEditing(mas);

	return mas;
}
