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
		tIsAmbiguous(subview as! NSView, indent + 1)
	}
}

class Window : NSWindow, Control {
	private var c: tControl?
	private var margined: Bool

	init() {
		super.init(
			contentRect: NSMakeRect(0, 0, 320, 240),
			styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
			backing: NSBackingStoreType.Buffered,
			defer: true)
		self.title = "Auto Layout Test"
		self.c = nil
		self.margined = false
	}

	func SetControl(c: Control) {
		self.c = c
		var contentView = self.contentView as! NSView
		contentView.addSubview(self.c?.View())
		self.Relayout()
	}

	func SetMargined(m: Bool) {
		self.margined = m
		self.Relayout()
	}

	func Show() {
		self.cascadeTopLeftFromPoint(NSMakePoint(20, 20))
		self.makeKeyAndOrderFront(self)
		tIsAmbiguous(self.contentView as! NSView, 0)
	}

	func View() -> NSView {
		fatalError("cannot call Window.View()")
	}

	func SetParent(p: Control) {
		fatalError("cannot call Window.SetParent()")
	}

	func Relayout() {
		if self.c == nil {
			return
		}

		var contentView = self.w.contentView as! NSView
		contentView.removeConstraints(contentView.constraints)

		// TODO why can't I just say var views = [ "view": p.view ]?
		// I think the parser is getting confused
		var views = [
			"view":	self.c?.View(),
		]
		var margin = ""
		if self.margined {
			margin = "-"
		}

		var constraint = "H:|" + margin + "[view]" + margin + "|"
		var constraints = mkconstraints(constraint, views)
		contentView.addConstraints(constraints)

		constraint = "V:|" + margin + "[view]" + margin + "|"
		constraints = mkconstraints(constraint, views)
		contentView.addConstraints(constraints)
	}
}
