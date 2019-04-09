// 3 november 2017
#import "uipriv_darwin.h"

// functions and constants FROM THE DEPTHS BELOW!
// note: for constants, dlsym() returns the address of the constant itself, as if we had done &constantName
// we also provide default values just in case

// these values come from 10.12.6
CFStringRef uiprivUNDOC_kCTFontPreferredSubFamilyNameKey = CFSTR("CTFontPreferredSubFamilyName");
CFStringRef uiprivUNDOC_kCTFontPreferredFamilyNameKey = CFSTR("CTFontPreferredFamilyName");

// note that we treat any error as "the symbols aren't there" (and don't care if dlclose() failed)
void uiprivLoadUndocumented(void)
{
	void *handle;
	CFStringRef *str;

	// dlsym() walks the dependency chain, so opening the current process should be sufficient
	handle = dlopen(NULL, RTLD_LAZY);
	if (handle == NULL)
		return;
#define GET(var, fn) *((void **) (&var)) = dlsym(handle, #fn)
	GET(str, kCTFontPreferredSubFamilyNameKey);
NSLog(@"get %p", str);
	if (str != NULL)
		uiprivUNDOC_kCTFontPreferredSubFamilyNameKey = *str;
	GET(str, kCTFontPreferredFamilyNameKey);
	if (str != NULL)
		uiprivUNDOC_kCTFontPreferredFamilyNameKey = *str;
	dlclose(handle);
}
