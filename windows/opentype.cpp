// 11 may 2017
#include "uipriv_windows.hpp"
#include "attrstr.hpp"

// TODO pull out my decision for empty uiOpenTypeFeatures, assuming that it isn't in another file or that I even made one

static uiForEach addToTypography(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value, void *data)
{
	IDWriteTypography *dt = (IDWriteTypography *) data;
	DWRITE_FONT_FEATURE dff;
	HRESULT hr;

	ZeroMemory(&dff, sizeof (DWRITE_FONT_FEATURE));
	// yes, the cast here is necessary (the compiler will complain otherwise)...
	dff.nameTag = (DWRITE_FONT_FEATURE_TAG) DWRITE_MAKE_OPENTYPE_TAG(a, b, c, d);
	dff.parameter = (UINT32) value;
	hr = dt->AddFontFeature(dff);
	if (hr != S_OK)
		logHRESULT(L"error adding OpenType feature to IDWriteTypography", hr);
	return uiForEachContinue;
}

IDWriteTypography *uiprivOpenTypeFeaturesToIDWriteTypography(const uiOpenTypeFeatures *otf)
{
	IDWriteTypography *dt;
	HRESULT hr;

	hr = dwfactory->CreateTypography(&dt);
	if (hr != S_OK)
		logHRESULT(L"error creating IDWriteTypography", hr);
	uiOpenTypeFeaturesForEach(otf, addToTypography, dt);
	return dt;
}
