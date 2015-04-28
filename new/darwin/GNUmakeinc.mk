# 28 april 2015

osMFILES = \
	darwin/alloc.m \
	darwin/bin.m \
	darwin/container.m \
	darwin/init.m \
	darwin/main.m \
	darwin/menu.m \
	darwin/text.m \
	darwin/util.m

osHFILES = \
	darwin/uipriv_darwin.h

osCFLAGS = -mmacosx-version-min=10.7 -DMACOSX_DEPLOYMENT_TARGET=10.7
osLDFLAGS = -mmacosx-version-min=10.7 -lobjc -framework Foundation -framework AppKit

# the gcc flags don't work with Apple's linker
# fortunately, we don't need any; Apple's linker warns about undefined symbols in -shared builds!
osLDWarnUndefinedFlags = 


osLIBSUFFIX = .dylib
osEXESUFFIX =
