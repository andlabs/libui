// 8 august 2015
import Cocoa

func mkconstraints(constraint: String, metrics: [String: Double]?, views: [String: NSView]) -> [AnyObject] {
	return NSLayoutConstraint.constraintsWithVisualFormat(
		constraint,
		options: NSLayoutFormatOptions(0),
		metrics: metrics,
		views: views)
}

// the swift bridge isn't perfect; it won't recognize these properly
// thanks to Eridius in freenode/#swift-lang
let myNSLayoutPriorityRequired: NSLayoutPriority = 1000
let myNSLayoutPriorityDefaultHigh: NSLayoutPriority = 750
let myNSLayoutPriorityDragThatCanResizeWindow: NSLayoutPriority = 510
let myNSLayoutPriorityWindowSizeStayPut: NSLayoutPriority = 500
let myNSLayoutPriorityDragThatCannotResizeWindow: NSLayoutPriority = 490
let myNSLayoutPriorityDefaultLow: NSLayoutPriority = 250
let myNSLayoutPriorityFittingSizeCompression: NSLayoutPriority = 50

func horzHuggingPri(view: NSView) -> NSLayoutPriority {
	return view.contentHuggingPriorityForOrientation(NSLayoutConstraintOrientation.Horizontal)
}

func vertHuggingPri(view: NSView) -> NSLayoutPriority {
	return view.contentHuggingPriorityForOrientation(NSLayoutConstraintOrientation.Vertical)
}

func setHuggingPri(view: NSView, priority: NSLayoutPriority, orientation: NSLayoutConstraintOrientation) {
	view.setContentHuggingPriority(priority, forOrientation: orientation)
}

func setHorzHuggingPri(view: NSView, priority: NSLayoutPriority) {
	setHuggingPri(view, priority, NSLayoutConstraintOrientation.Horizontal)
}

func setVertHuggingPri(view: NSView, priority: NSLayoutPriority) {
	setHuggingPri(view, priority, NSLayoutConstraintOrientation.Vertical)
}

func fittingAlignmentSize(view: NSView) -> NSSize {
	var s = view.fittingSize
	// the fitting size is for a frame rect; we need an alignment rect
	var r = NSMakeRect(0, 0, s.width, s.height)
	r = view.alignmentRectForFrame(r)
	return r.size
}
