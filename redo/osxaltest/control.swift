// 31 july 2015
import Cocoa

// TODO stretchy across both dimensions
// for a vertical box, the horizontal width should be variable
struct tAutoLayoutParams {
	var view: NSView? = nil
	var attachLeft: Bool = false
	var attachTop: Bool = false
	var attachRight: Bool = false
	var attachBottom: Bool = false
	var nonStretchyWidthPredicate: String = ""
	var nonStretchyHeightPredicate: String = ""
}

protocol tControl : class {
	func tSetParent(p: tControl, addToView: NSView)
	func tFillAutoLayout(inout p: tAutoLayoutParams)
	func tRelayout()
}

func tAutoLayoutKey(n: UIntMax) -> String {
	return NSString(format: "view%d", n) as String
}

func mkconstraints(constraint: String, views: [String: NSView]) -> [AnyObject] {
println("making constraint \(constraint)")
	return NSLayoutConstraint.constraintsWithVisualFormat(
		constraint,
		options: NSLayoutFormatOptions(0),
		metrics: nil,
		views: views)
}
