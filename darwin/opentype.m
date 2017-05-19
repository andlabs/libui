// 11 may 2017
#import "uipriv_darwin.h"

struct uiOpenTypeFeatures {
	NSMutableDictionary *tags;
};

uiOpenTypeFeatures *uiNewOpenTypeFeatures(void)
{
	uiOpenTypeFeatures *otf;

	otf = uiNew(uiOpenTypeFeatures);
	otf->tags = [NSMutableDictionary new];
	return otf;
}

void uiFreeOpenTypeFeatures(uiOpenTypeFeatures *otf)
{
	[otf->tags release];
	uiFree(otf);
}

uiOpenTypeFeatures *uiOpenTypeFeaturesClone(uiOpenTypeFeatures *otf)
{
	uiOpenTypeFeatures *out;

	out = uiNew(uiOpenTypeFeatures);
	out->tags = [otf->tags mutableCopy];
	return out;
}

// TODO provide to aat.m too; remove x8tox32() when doing so
#define x8to32(x) ((uint32_t) (((uint8_t) (x)) & 0xFF))
#define mkTag(a, b, c, d)		\
	((x8tox32(a) << 24) |	\
	(x8tox32(b) << 16) |		\
	(x8tox32(c) << 8) |		\
	x8tox32(d))

// why are there no NSNumber methods for stdint.h or the equivalent core foundation types?...
#define mkMapObject(tag) [NSNumber numberWithUnsignedLongLong:((unsigned long long) tag)]
#define mapObjectValue(num) ((uint32_t) [num unsignedLongLongValue])

void uiOpenTypeFeaturesAdd(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value)
{
	NSNumber *tn, *vn;

	tn = mkMapObject(mkTag(a, b, c, d));
	vn = mkMapObject(value);
	[otf->tags setObject:vn forKey:tn];
}

void uiOpenTypeFeaturesRemove(uiOpenTypeFeatures *otf, char a, char b, char c, char d)
{
	NSNumber *tn;

	tn = mkMapObject(mkTag(a, b, c, d));
	[otf->tags removeObjectForKey:tn];
}

int uiOpenTypeFeaturesGet(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t *value)
{
	NSNumber *tn, *vn;

	tn = mkMapObject(mkTag(a, b, c, d));
	vn = (NSNumber *) [otf->tags objectForKey:tn];
	if (vn == nil)
		return 0;
	*value = mapObjectValue(vn);
	// TODO release vn?
	return 1;
}

void uiOpenTypeFeaturesForEach(uiOpenTypeFeatures *otf, uiOpenTypeFeaturesForEachFunc f, void *data)
{
	[otf->tags enumerateKeysAndObjectsUsingBlock:^(id key, id value, BOOL *stop) {
		NSNumber *tn = (NSNumber *) key;
		NSNumber *vn = (NSNumber *) value;
		uint32_t tag;
		uint8_t a, b, c, d;

		tag = mapObjectValue(tn);
		a = (uint8_t) ((tag >> 24) & 0xFF);
		b = (uint8_t) ((tag >> 16) & 0xFF);
		c = (uint8_t) ((tag >> 8) & 0xFF);
		d = (uint8_t) (tag & 0xFF);
		// TODO handle return value
		(*f)((char) a, (char) b, (char) c, (char) d,
			mapObjectValue(vn), data);
	}];
}

int uiOpenTypeFeaturesEqual(uiOpenTypeFeatures *a, uiOpenTypeFeatures *b)
{
	return [a->tags isEqualToDictionary:b->tags];
}

// actual conversion to a feature dictionary is handled in aat.m; see there for details
