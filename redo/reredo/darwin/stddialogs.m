// 26 june 2015
#import "uipriv_darwin.h"

// TODO is whether extensions are shown a Finder setting?
static void setupSavePanel(NSSavePanel *s)
{
	[s setCanCreateDirectories:YES];
	[s setShowsHiddenFiles:YES];
	[s setExtensionHidden:NO];
	[s setCanSelectHiddenExtension:NO];
	[s setTreatsFilePackagesAsDirectories:YES];
}

static char *runSavePanel(NSSavePanel *s)
{
	char *filename;

	if ([s runModal] != NSFileHandlingPanelOKButton)
		return NULL;
	filename = uiDarwinNSStringToText([[s URL] path]);
	return filename;
}

char *uiOpenFile(void)
{
	NSOpenPanel *o;

	o = [NSOpenPanel openPanel];
	[o setCanChooseFiles:YES];
	[o setCanChooseDirectories:NO];
	[o setResolvesAliases:NO];
	[o setAllowsMultipleSelection:NO];
	setupSavePanel(o);
	// panel is autoreleased
	return runSavePanel(o);
}

char *uiSaveFile(void)
{
	NSSavePanel *s;

	s = [NSSavePanel savePanel];
	setupSavePanel(s);
	// panel is autoreleased
	return runSavePanel(s);
}

void uiMsgBox(const char *title, const char *description)
{
	// TODO
}

void uiMsgBoxError(const char *title, const char *description)
{
	// TODO
}
