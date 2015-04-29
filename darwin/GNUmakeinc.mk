# 28 april 2015

osMFILES = \
	darwin/alloc.m \
	darwin/bin.m \
	darwin/button.m \
	darwin/checkbox.m \
	darwin/container.m \
	darwin/entry.m \
	darwin/init.m \
	darwin/label.m \
	darwin/main.m \
	darwin/menu.m \
	darwin/newcontrol.m \
	darwin/tab.m \
	darwin/text.m \
	darwin/util.m \
	darwin/window.m

osHFILES = \
	darwin/uipriv_darwin.h

osCFLAGS = -mmacosx-version-min=10.7 -DMACOSX_DEPLOYMENT_TARGET=10.7
osLDFLAGS = -mmacosx-version-min=10.7 -lobjc -framework Foundation -framework AppKit

# the gcc flags don't work with Apple's linker
# fortunately, we don't need any; Apple's linker warns about undefined symbols in -shared builds!
osLDWarnUndefinedFlags = 


osLIBSUFFIX = .dylib
osEXESUFFIX =
