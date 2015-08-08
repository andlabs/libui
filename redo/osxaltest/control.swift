// 31 july 2015
import Cocoa

protocol Control : class {
	func View() -> NSView
	func SetParent(p: Control)
}

func mkconstraints(constraint: String, views: [String: NSView]) -> [AnyObject] {
	return NSLayoutConstraint.constraintsWithVisualFormat(
		constraint,
		options: NSLayoutFormatOptions(0),
		metrics: nil,
		views: views)
}
