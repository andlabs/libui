# 16 october 2015

EXESUFFIX = .exe
LIBSUFFIX = .dll
OSHSUFFIX = .h
STATICLIBSUFFIX = .lib
TOOLCHAIN = msvc

USESSONAME = 0

# notice that usp10.lib comes before gdi32.lib
# TODO prune this list
NATIVE_UI_LDFLAGS = \
	user32.lib kernel32.lib usp10.lib gdi32.lib comctl32.lib uxtheme.lib msimg32.lib comdlg32.lib d2d1.lib dwrite.lib ole32.lib oleaut32.lib oleacc.lib uuid.lib
