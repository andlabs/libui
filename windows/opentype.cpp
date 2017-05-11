// 11 may 2017
#include "uipriv_windows.hpp"

struct uiOpenTypeFeatures {
	xxxx;
};

uiOpenTypeFeatures *uiNewOpenTypeFeatures(void)
{
	uiOpenTypeFeatures *otf;

	otf = uiNew(uiOpenTypeFeatures);
	xxxx;
	return otf;
}

void uiFreeOpenTypeFeatures(uiOpenTypeFeatures *otf)
{
	xxxxxx;
	uiFree(otf);
}

uiOpenTypeFeatures *uiOpenTypeFeaturesClone(uiOpenTypeFeatures *otf)
{
	uiOpenTypeFeatures *out;

	out = uiNew(uiOpenTypeFeatures);
	xxxxxx;
	return out;
}

void uiOpenTypeFeaturesAdd(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value)
{
}

void uiOpenTypeFeaturesRemove(uiOpenTypeFeatures *otf, char a, char b, char c, char d)
{
}

int uiOpenTypeFeaturesGet(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t *value)
{
}

void uiOpenTypeFeaturesForEach(uiOpenTypeFeatures *otf, uiOpenTypeFeaturesForEachFunc f, void *data)
{
}

int uiOpenTypeFeaturesEqual(uiOpenTypeFeatures *a, uiOpenTypeFeatures *b)
{
}

// TODO put the internal function to produce a backend object from one here
