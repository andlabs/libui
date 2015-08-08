// 31 july 2015
import Cocoa

protocol Control : class {
	func View() -> NSView
	func SetParent(p: Control)
}

// TODO move to layout.swift

func mkconstraints(constraint: String, views: [String: NSView]) -> [AnyObject] {
	return NSLayoutConstraint.constraintsWithVisualFormat(
		constraint,
		options: NSLayoutFormatOptions(0),
		metrics: nil,
		views: views)
}

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
