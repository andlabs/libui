// 11 may 2017
#include "uipriv_windows.hpp"

typedef std::map<uint32_t, uint32_t> tagmap;

struct uiOpenTypeFeatures {
	tagmap *tags;
};

uiOpenTypeFeatures *uiNewOpenTypeFeatures(void)
{
	uiOpenTypeFeatures *otf;

	otf = uiNew(uiOpenTypeFeatures);
	otf->tags = new tagmap;
	return otf;
}

void uiFreeOpenTypeFeatures(uiOpenTypeFeatures *otf)
{
	delete otf->tags;
	uiFree(otf);
}

uiOpenTypeFeatures *uiOpenTypeFeaturesClone(const uiOpenTypeFeatures *otf)
{
	uiOpenTypeFeatures *out;

	out = uiNew(uiOpenTypeFeatures);
	out->tags = new tagmap;
	*(out->tags) = *(otf->tags);
	return out;
}

#define mktag(a, b, c, d) ((uint32_t) DWRITE_MAKE_OPENTYPE_TAG(a, b, c, d))

void uiOpenTypeFeaturesAdd(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value)
{
	(*(otf->tags))[mktag(a, b, c, d)] = value;
}

void uiOpenTypeFeaturesRemove(uiOpenTypeFeatures *otf, char a, char b, char c, char d)
{
	// this will just return 0 if nothing was removed (if I'm reading the help pages I've found correctly)
	otf->tags->erase(mktag(a, b, c, d));
}

// TODO will the const wreck stuff up?
int uiOpenTypeFeaturesGet(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t *value)
{
	tagmap::const_iterator iter;

	iter = otf->tags->find(mktag(a, b, c, d));
	if (iter == otf->tags->end())
		return 0;
	*value = iter->second;
	return 1;
}

void uiOpenTypeFeaturesForEach(const uiOpenTypeFeatures *otf, uiOpenTypeFeaturesForEachFunc f, void *data)
{
	tagmap::const_iterator iter, end;

	end = otf->tags->end();
	for (iter = otf->tags->begin(); iter != end; iter++) {
		uint8_t a, b, c, d;
		uiForEach ret;

		a = (uint8_t) (iter->first & 0xFF);
		b = (uint8_t) ((iter->first >> 8) & 0xFF);
		c = (uint8_t) ((iter->first >> 16) & 0xFF);
		d = (uint8_t) ((iter->first >> 24) & 0xFF);
		ret = (*f)(otf, (char) a, (char) b, (char) c, (char) d,
			iter->second, data);
		if (ret == uiForEachStop)
			return;
	}
}

int uiOpenTypeFeaturesEqual(const uiOpenTypeFeatures *a, const uiOpenTypeFeatures *b)
{
	if (a == NULL && b == NULL)
		return 1;
	if (a == NULL || b == NULL)
		return 0;
	// TODO make sure this is correct
	return *(a->tags) == *(b->tags);
}

IDWriteTypography *otfToDirectWrite(const uiOpenTypeFeatures *otf)
{
	IDWriteTypography *dt;
	tagmap::const_iterator iter, end;
	DWRITE_FONT_FEATURE dff;
	HRESULT hr;

	hr = dwfactory->CreateTypography(&dt);
	if (hr != S_OK)
		logHRESULT(L"error creating IDWriteTypography", hr);
	end = otf->tags->end();
	for (iter = otf->tags->begin(); iter != end; iter++) {
		ZeroMemory(&dff, sizeof (DWRITE_FONT_FEATURE));
		dff.nameTag = (DWRITE_FONT_FEATURE_TAG) (iter->first);
		dff.parameter = (UINT32) (iter->second);
		hr = dt->AddFontFeature(dff);
		if (hr != S_OK)
			logHRESULT(L"error adding OpenType feature to IDWriteTypography", hr);
	}
	return dt;
}
