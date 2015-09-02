// 26 june 2015
#import "uipriv_darwin.h"

// note: whether extensions are actually shown depends on a user setting in Finder; we can't control it here
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

static void msgbox(const char *title, const char *description, NSAlertStyle style)
{
	NSAlert *a;

	a = [NSAlert new];
	[a setAlertStyle:style];
	[a setShowsHelp:NO];
	[a setShowsSuppressionButton:NO];
	[a setMessageText:toNSString(title)];
	[a setInformativeText:toNSString(description)];
	[a addButtonWithTitle:@"OK"];
	[a runModal];
	[a release];
}

void uiMsgBox(const char *title, const char *description)
{
	msgbox(title, description, NSInformationalAlertStyle);
}

void uiMsgBoxError(const char *title, const char *description)
{
	msgbox(title, description, NSCriticalAlertStyle);
}
