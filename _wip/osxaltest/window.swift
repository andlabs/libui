// 1 august 2015
import Cocoa

// auto layout helpers
func isAmbiguous(view: NSView, indent: Int) {
	var s = String(count: indent, repeatedValue: " " as Character)
	println("\(s) \(view.className) \(view.hasAmbiguousLayout)")
	if view.hasAmbiguousLayout {
		view.window?.visualizeConstraints(view.superview!.constraints)
	}
	for subview in view.subviews {
		isAmbiguous(subview as! NSView, indent + 1)
	}
}

class Window : NSWindow, Control {
	private var c: Control?
	private var margined: Bool

	init() {
		self.c = nil
		self.margined = false
		// we have to initialize our own instance variables first, unfortunately (thanks erica in irc.freenode.net/#swift-lang)
		super.init(
			contentRect: NSMakeRect(0, 0, 320, 240),
			styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
			backing: NSBackingStoreType.Buffered,
			defer: true)
		self.title = "Auto Layout Test"
	}

	required init?(coder: NSCoder) {
		fatalError("can't use this constructor, sorry")
	}

	func SetControl(c: Control) {
		self.c = c
		var contentView = self.contentView as! NSView
		contentView.addSubview(self.c!.View())
		self.relayout()
	}

	func SetMargined(m: Bool) {
		self.margined = m
		self.relayout()
	}

	func Show() {
		self.cascadeTopLeftFromPoint(NSMakePoint(20, 20))
		self.makeKeyAndOrderFront(self)
		isAmbiguous(self.contentView as! NSView, 0)
	}

	func View() -> NSView {
		fatalError("cannot call Window.View()")
	}

	func SetParent(p: Control) {
		fatalError("cannot call Window.SetParent()")
	}

	private func relayout() {
		if self.c == nil {
			return
		}

		var contentView = self.contentView as! NSView
		contentView.removeConstraints(contentView.constraints)

		var views = [
			"view":	self.c!.View(),
		]
		var margin = ""
		if self.margined {
			margin = "-"
		}

		var constraint = "H:|" + margin + "[view]" + margin + "|"
		var constraints = mkconstraints(constraint, nil, views)
		contentView.addConstraints(constraints)

		constraint = "V:|" + margin + "[view]" + margin + "|"
		constraints = mkconstraints(constraint, nil, views)
		contentView.addConstraints(constraints)
	}
}
