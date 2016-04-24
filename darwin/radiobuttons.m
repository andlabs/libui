// 14 august 2015
#import "uipriv_darwin.h"

// TODO the selection should NOT be lost when starting a new drag

struct uiRadioButtons {
	uiDarwinControl c;
	NSMatrix *matrix;
};

uiDarwinDefineControl(
	uiRadioButtons,						// type name
	matrix								// handle
)

static NSButtonCell *cellAt(uiRadioButtons *r, uintmax_t n)
{
	return (NSButtonCell *) [r->matrix cellAtRow:n column:0];
}

void uiRadioButtonsAppend(uiRadioButtons *r, const char *text)
{
	intmax_t prevSelection;

	// renewRows:columns: will reset the selection
	prevSelection = [r->matrix selectedRow];

	[r->matrix renewRows:([r->matrix numberOfRows] + 1) columns:1];
	[cellAt(r, [r->matrix numberOfRows] - 1) setTitle:toNSString(text)];

	// this will definitely cause a resize in at least the vertical direction, even if not in the horizontal
	// DO NOT CALL sizeToCells! this will glitch out; see http://stackoverflow.com/questions/32162562/dynamically-adding-cells-to-a-nsmatrix-laid-out-with-auto-layout-has-weird-effec

	// and renew the previous selection
	// we need to turn on allowing empty selection for this to work properly on the initial state
	// TODO this doesn't actually work
	[r->matrix setAllowsEmptySelection:YES];
	[r->matrix selectCellAtRow:prevSelection column:0];
	[r->matrix setAllowsEmptySelection:NO];

	uiDarwinControlTriggerRelayout(uiDarwinControl(r));
}

uiRadioButtons *uiNewRadioButtons(void)
{
	uiRadioButtons *r;
	NSButtonCell *cell;

	r = (uiRadioButtons *) uiNewControl(uiRadioButtons);

	// we have to set up the NSMatrix this way (prototype first)
	// otherwise we won't be able to change its properties (such as the button type)
	cell = [NSButtonCell new];
	[cell setButtonType:NSRadioButton];
	// works on NSCells too (same selector)
	uiDarwinSetControlFont((NSControl *) cell, NSRegularControlSize);

	r->matrix = [[NSMatrix alloc] initWithFrame:NSZeroRect
		mode:NSRadioModeMatrix
		prototype:cell
		numberOfRows:0
		numberOfColumns:0];
	// we manually twiddle this property to allow programmatic non-selection state
	[r->matrix setAllowsEmptySelection:NO];
	[r->matrix setSelectionByRect:YES];
	[r->matrix setIntercellSpacing:NSMakeSize(4, 2)];
	[r->matrix setAutorecalculatesCellSize:YES];
	[r->matrix setDrawsBackground:NO];
	[r->matrix setDrawsCellBackground:NO];
	[r->matrix setAutosizesCells:YES];

	uiDarwinFinishNewControl(r, uiRadioButtons);

	return r;
}
