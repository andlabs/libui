// 14 august 2015
#import "uipriv_darwin.h"

// A separator NSBox is horizontal if width >= height.
// Use Interface Builder's initial size as our initial size, to be safe.
#define separatorFrameWidth 96		/* alignment rect 96 */
#define separatorFrameHeight 5		/* alignment rect 1 */

struct uiSeparator {
	uiDarwinControl c;
	NSBox *box;
};

uiDarwinControlAllDefaults(uiSeparator, box)

uiSeparator *uiNewHorizontalSeparator(void)
{
	uiSeparator *s;

	uiDarwinNewControl(uiSeparator);

	s->box = [[NSBox alloc] initWithFrame:NSMakeRect(0, 0, separatorFrameWidth, separatorFrameHeight)];
	[s->box setBoxType:NSBoxSeparator];
	[s->box setBorderType:NSGrooveBorder];
	[s->box setTransparent:NO];
	[s->box setTitlePosition:NSNoTitle];

	return s;
}
