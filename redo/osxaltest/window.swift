// 1 august 2015
import Cocoa

// auto layout helpers
func tIsAmbiguous(view: NSView, indent: Int) {
	var s = string(indent, ' ')
	println("\(s) \(view.className) \(view.hasAmbiguousLayout)")
	if view.hasAmbiguousLayout {
		view.window.visualizeConstraints(view.superview.constraints)
	}
	for subview in view.subviews {
		tIsAmbiguous(subview, indent + 1)
	}
}

class tWindow : tControl {
	private var w: NSWindow;
	private var c: tControl;
	private var margined: Bool;

	init() {
		self.w = NSWindow(
			contentRect: NSMakeRect(0, 0, 320, 240),
			styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
			backing: NSBackingStoreBuffered,
			defer: true)
		self.w.title = "Auto Layout Test"
	}

	public func tSetControl(c: tControl) {
		self.c = c
		self.c.tSetParent(self, addToView: self.w.contentView)
		self.tRelayout()
	}

	public func tSetMargined(m: Bool) {
		self.margined = m
		self.tRelayout()
	}

	public func tShow() {
		self.w.cascadeTopLeftFromPoint(NSMakePoint(20, 20))
		self.w.makeKeyAndOrderFront(self)
		tIsAmbiguous(self.w.contentView, 0)
	}

	public func tRelayout() {
		if self.c == nil {
			return
		}

		var contentView = self.w.contentView
		contentView.removeConstraints(contentView.constraints)

		var p = tAutoLayoutParams()
		self.c.tFillAutoLayout(p)

		var views = [
			"view": p.view,
		]
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
		var constraints = NSLayoutConstraint(
			visualFormat:constraint,
			options:0,
			metrics:nil,
			views:views)
		contentView.addConstraints(constraints)

		constraint = "V:"
		if p.attachTop {
			constraint += "|" + margin
		}
		constraint += "[view]"
		if p.attachBottom {
			constraint += margin + "|"
		}
		constraints = NSLayoutConstraint(
			visualFormat:constraint,
			options:0,
			metrics:nil,
			views:views)
		contentView.addConstraints(constraints)
	}
}
