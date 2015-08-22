// 14 august 2015
#import "uipriv_darwin.h"

struct uiRadioButtons {
	uiDarwinControl c;
	NSMatrix *matrix;
};

uiDarwinDefineControl(
	uiRadioButtons,						// type name
	uiRadioButtonsType,						// type function
	matrix								// handle
)

static NSButtonCell *cellAt(uiRadioButtons *r, uintmax_t n)
{
	return (NSButtonCell *) [r->matrix cellAtRow:n column:0];
}

void uiRadioButtonsAppend(uiRadioButtons *r, const char *text)
{
	[r->matrix addRow];
	[cellAt(r, [r->matrix numberOfRows] - 1) setTitle:toNSString(text)];
	// this will definitely cause a resize in at least the vertical direction, even if not in the horizontal
	uiDarwinControlTriggerRelayout(uiDarwinControl(r));
}

uiRadioButtons *uiNewRadioButtons(void)
{
	uiRadioButtons *r;

	r = (uiRadioButtons *) uiNewControl(uiRadioButtonsType());

	r->matrix = [[NSMatrix alloc] initWithFrame:NSZeroRect];
	[r->matrix setMode:NSRadioModeMatrix];
	// TODO should we allow an initial state of no selection, but not allow the user to select nothing?
	[r->matrix setAllowsEmptySelection:NO];
	[r->matrix setIntercellSpacing:NSMakeSize(4, 2)];
	[r->matrix setAutorecalculatesCellSize:YES];
	[r->matrix setDrawsBackground:NO];
	[r->matrix setDrawsCellBackground:NO];
	[r->matrix setAutosizesCells:YES];
	[[r->matrix prototype] setButtonType:NSRadioButton];
	// works on NSCells too (same selector)
	uiDarwinSetControlFont((NSControl *) [r->matrix prototype], NSRegularControlSize);

	uiDarwinFinishNewControl(r, uiRadioButtons);

	return r;
}
