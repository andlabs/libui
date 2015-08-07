// 1 august 2015
import Cocoa

// auto layout helpers
func tIsAmbiguous(view: NSView, indent: Int) {
	var s = String(count: indent, repeatedValue: " " as Character)
	debugPrint("\(s) \(view.className) \(view.hasAmbiguousLayout)")
	if view.hasAmbiguousLayout {
		view.window?.visualizeConstraints(view.superview!.constraints)
	}
	for subview in view.subviews {
		tIsAmbiguous(subview as! NSView, indent + 1)
	}
}

class tWindow : tControl {
	private var w: NSWindow
	private var c: tControl?
	private var margined: Bool

	init() {
		self.w = NSWindow(
			contentRect: NSMakeRect(0, 0, 320, 240),
			styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
			backing: NSBackingStoreType.Buffered,
			defer: true)
		self.w.title = "Auto Layout Test"
		self.c = nil
		self.margined = false
	}

	func tSetControl(c: tControl) {
		self.c = c
		// TODO use self.c here
		c.tSetParent(self, addToView: self.w.contentView as! NSView)
		self.tRelayout()
	}

	func tSetMargined(m: Bool) {
		self.margined = m
		self.tRelayout()
	}

	func tShow() {
		self.w.cascadeTopLeftFromPoint(NSMakePoint(20, 20))
		self.w.makeKeyAndOrderFront(self)
		tIsAmbiguous(self.w.contentView as! NSView, 0)
	}

	func tSetParent(p: tControl, addToView: NSView) {
		fatalError("cannot call tWindow.tSetParent()")
	}

	func tFillAutoLayout(inout p: tAutoLayoutParams) {
		fatalError("cannot call tWindow.tFillAutoLayout()")
	}

	func tRelayout() {
		if self.c == nil {
			return
		}

		var contentView = self.w.contentView as! NSView
		contentView.removeConstraints(contentView.constraints)

		var p = tAutoLayoutParams()
		c?.tFillAutoLayout(&p)

		// TODO why can't I just say var views = [ "view": p.view ]?
		// I think the parser is getting confused
		var views = [String: NSView]()
		views["view"] = p.view
		var margin = ""
		if self.margined {
			margin = "-"
		}

		// TODO always append margins even if not attached?
		// or if not attached, append ->=0- as well?
		var constraint = "H:"
		if p.attachLeft {
			constraint += "|" + margin
		}
		constraint += "[view]"
		if p.attachRight {
			constraint += margin + "|"
		}
		var constraints = NSLayoutConstraint.constraintsWithVisualFormat(
			constraint,
			options: NSLayoutFormatOptions(0),
			metrics: nil,
			views: views)
		contentView.addConstraints(constraints)

		constraint = "V:"
		if p.attachTop {
			constraint += "|" + margin
		}
		constraint += "[view]"
		if p.attachBottom {
			constraint += margin + "|"
		}
		constraints = NSLayoutConstraint.constraintsWithVisualFormat(
			constraint,
			options: NSLayoutFormatOptions(0),
			metrics: nil,
			views: views)
		contentView.addConstraints(constraints)
	}
}
