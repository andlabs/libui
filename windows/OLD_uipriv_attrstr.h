// dwrite.cpp
extern IDWriteFactory *dwfactory;
extern HRESULT initDrawText(void);
extern void uninitDrawText(void);
struct fontCollection {
	IDWriteFontCollection *fonts;
	WCHAR userLocale[LOCALE_NAME_MAX_LENGTH];
	int userLocaleSuccess;
};
extern fontCollection *loadFontCollection(void);
extern WCHAR *fontCollectionFamilyName(fontCollection *fc, IDWriteFontFamily *family);
extern void fontCollectionFree(fontCollection *fc);
extern WCHAR *fontCollectionCorrectString(fontCollection *fc, IDWriteLocalizedStrings *names);

