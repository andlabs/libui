// 1 november 2016
#import "uipriv_darwin.h"

// see http://stackoverflow.com/a/40352996/3408572
static void minMaxAutoLayoutSizes(NSWindow *w, NSSize *min, NSSize *max)
{
	NSLayoutConstraint *cw, *ch;
	NSView *contentView;
	NSRect prevFrame;

	// if adding these constraints causes the window to change size somehow, don't show it to the user and change it back afterwards
	NSDisableScreenUpdates();
	prevFrame = [w frame];

	// minimum: encourage the window to be as small as possible
	contentView = [w contentView];
	cw = mkConstraint(contentView, NSLayoutAttributeWidth,
		NSLayoutRelationEqual,
		nil, NSLayoutAttributeNotAnAttribute,
		0, 0,
		@"window minimum width finding constraint");
	[cw setPriority:NSLayoutPriorityDragThatCanResizeWindow];
	[contentView addConstraint:cw];
	ch = mkConstraint(contentView, NSLayoutAttributeHeight,
		NSLayoutRelationEqual,
		nil, NSLayoutAttributeNotAnAttribute,
		0, 0,
		@"window minimum height finding constraint");
	[ch setPriority:NSLayoutPriorityDragThatCanResizeWindow];
	[contentView addConstraint:ch];
	*min = [contentView fittingSize];
	[contentView removeConstraint:cw];
	[contentView removeConstraint:ch];

	// maximum: encourage the window to be as large as possible
	contentView = [w contentView];
	cw = mkConstraint(contentView, NSLayoutAttributeWidth,
		NSLayoutRelationEqual,
		nil, NSLayoutAttributeNotAnAttribute,
		0, DBL_MAX,
		@"window maximum width finding constraint");
	[cw setPriority:NSLayoutPriorityDragThatCanResizeWindow];
	[contentView addConstraint:cw];
	ch = mkConstraint(contentView, NSLayoutAttributeHeight,
		NSLayoutRelationEqual,
		nil, NSLayoutAttributeNotAnAttribute,
		0, DBL_MAX,
		@"window maximum height finding constraint");
	[ch setPriority:NSLayoutPriorityDragThatCanResizeWindow];
	[contentView addConstraint:ch];
	*max = [contentView fittingSize];
	[contentView removeConstraint:cw];
	[contentView removeConstraint:ch];

	[w setFrame:prevFrame display:YES];		// TODO really YES?
	NSEnableScreenUpdates();
}

static void handleResizeLeft(NSRect *frame, NSPoint old, NSPoint new)
{
	frame->origin.x += new.x - old.x;
}

// TODO properly handle crossing the menubar; this just stops at the menubar
static void handleResizeTop(NSRect *frame, NSPoint old, NSPoint new)
{
	CGFloat offset;
	CGFloat newHeight;
	CGFloat oldTop, newTop;
	NSRect mainWorkArea;
	CGFloat menubarBottom;

	offset = new.y - old.y;
	newHeight = frame->size.height + offset;

	// we have gone too high if we started under the menubar AND we are about to cross it
	oldTop = frame->origin.y + frame->size.height;
	newTop = frame->origin.y + newHeight;
	mainWorkArea = [[NSScreen mainScreen] visibleFrame];
	menubarBottom = mainWorkArea.origin.y + mainWorkArea.size.height;
	if (oldTop < menubarBottom)
		if (newTop >= menubarBottom)
			newTop = menubarBottom;		// TODO

	frame->size.height = newHeight;
}

static void handleResizeRight(NSRect *frame, NSPoint old, NSPoint new)
{
	frame->size.width += new.x - old.x;
}


// TODO properly handle crossing the menubar; this just stops at the menubar
static void handleResizeBottom(NSRect *frame, NSPoint old, NSPoint new)
{
	CGFloat offset;
	CGFloat newY, newHeight;
	NSRect mainFrame;
	CGFloat menubarTop;

	offset = new.y - old.y;
	newY = frame->origin.y + offset;
	newHeight = frame->size.height - offset;

	// we have gone too low if we started above the menubar AND we are about to cross it
	mainFrame = [[NSScreen mainScreen] frame];
	menubarTop = mainFrame.origin.y + mainFrame.size.height;
	if (frame->origin.y >= menubarTop)
		if (newY < menubarTop)
			newY = menubarTop;
			// TODO change newHeight too?

	frame->origin.y = newY;
	frame->size.height = newHeight;
}

struct onResizeDragParams {
	NSWindow *w;
	NSRect initialFrame;
	NSPoint initialPoint;
	uiWindowResizeEdge edge;
	NSSize min;
	NSSize max;
};

static void onResizeDrag(struct onResizeDragParams *p, NSEvent *e)
{
	NSPoint new;
	NSRect frame;

	new = [e locationInWindow];
	frame = p->initialFrame;

NSLog(@"old %@ new %@", NSStringFromPoint(p->initialPoint), NSStringFromPoint(new));
NSLog(@"frame %@", NSStringFromRect(frame));

	// horizontal
	switch (p->edge) {
	case uiWindowResizeEdgeLeft:
	case uiWindowResizeEdgeTopLeft:
	case uiWindowResizeEdgeBottomLeft:
		handleResizeLeft(&frame, p->initialPoint, new);
		break;
	case uiWindowResizeEdgeRight:
	case uiWindowResizeEdgeTopRight:
	case uiWindowResizeEdgeBottomRight:
		handleResizeRight(&frame, p->initialPoint, new);
		break;
	}
	// vertical
	switch (p->edge) {
	case uiWindowResizeEdgeTop:
	case uiWindowResizeEdgeTopLeft:
	case uiWindowResizeEdgeTopRight:
		handleResizeTop(&frame, p->initialPoint, new);
		break;
	case uiWindowResizeEdgeBottom:
	case uiWindowResizeEdgeBottomLeft:
	case uiWindowResizeEdgeBottomRight:
		handleResizeBottom(&frame, p->initialPoint, new);
		break;
	}

	// constrain
	// TODO should we constrain against anything else as well? minMaxAutoLayoutSizes() already gives us nonnegative sizes, but...
	if (frame.size.width < p->min.width)
		frame.size.width = p->min.width;
	if (frame.size.height < p->min.height)
		frame.size.height = p->min.height;
	// TODO > or >= ?
	if (frame.size.width > p->max.width)
		frame.size.width = p->max.width;
	if (frame.size.height > p->max.height)
		frame.size.height = p->max.height;

NSLog(@"becomes %@", NSStringFromRect(frame));

	[p->w setFrame:frame display:YES];			// and do reflect the new frame immediately
}

void doManualResize(NSWindow *w, NSEvent *initialEvent, uiWindowResizeEdge edge)
{
	__block struct onResizeDragParams rdp;
	struct nextEventArgs nea;
	BOOL (^handleEvent)(NSEvent *e);
	__block BOOL done;

	rdp.w = w;
	rdp.initialFrame = [rdp.w frame];
	rdp.initialPoint = [initialEvent locationInWindow];
	rdp.edge = edge;
	// TODO what happens if these change during the loop?
	minMaxAutoLayoutSizes(rdp.w, &(rdp.min), &(rdp.max));
NSLog(@"min %@", NSStringFromSize(rdp.min));
NSLog(@"max %@", NSStringFromSize(rdp.max));

	nea.mask = NSLeftMouseDraggedMask | NSLeftMouseUpMask;
	nea.duration = [NSDate distantFuture];
	nea.mode = NSEventTrackingRunLoopMode;		// nextEventMatchingMask: docs suggest using this for manual mouse tracking
	nea.dequeue = YES;
	handleEvent = ^(NSEvent *e) {
		if ([e type] == NSLeftMouseUp) {
			done = YES;
			return YES;	// do not send
		}
		onResizeDrag(&rdp, e);
		return YES;		// do not send
	};
	done = NO;
	while (mainStep(&nea, handleEvent))
		if (done)
			break;
}
