// 31 july 2015
import Cocoa

var nspinbox = 0

class Spinbox : NSView, Control {
	private var t: NSTextField
	private var s: NSStepper
	private var parent: Control?

	init() {
		var cell: NSTextFieldCell

		self.t = NSTextField(frame: NSZeroRect)
self.t.stringValue = "\(nspinbox)"
nspinbox++
		self.t.selectable = true
		self.t.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
		self.t.bordered = false
		self.t.bezelStyle = NSTextFieldBezelStyle.SquareBezel
		self.t.bezeled = true
		cell = self.t.cell() as! NSTextFieldCell
		cell.lineBreakMode = NSLineBreakMode.ByClipping
		cell.scrollable = true
		self.t.translatesAutoresizingMaskIntoConstraints = false

		// make the textbox grow horizontally and vertically
		setHorzHuggingPri(self.t, myNSLayoutPriorityDefaultLow)
		setVertHuggingPri(self.t, myNSLayoutPriorityDefaultLow)

		self.s = NSStepper(frame: NSZeroRect)
		self.s.increment = 1
		self.s.valueWraps = false
		self.s.autorepeat = true
		self.s.translatesAutoresizingMaskIntoConstraints = false

		// make the spinner grow vertically but not horizontally
		// TODO Required instead?
		setHorzHuggingPri(self.s, myNSLayoutPriorityDefaultHigh)
		setVertHuggingPri(self.s, myNSLayoutPriorityDefaultLow)

		self.parent = nil

		super.init(frame: NSZeroRect)
		self.translatesAutoresizingMaskIntoConstraints = false

		self.addSubview(self.t)
		self.addSubview(self.s)
		var views = [
			"t":	self.t,
			"s":	self.s,
		]
		var constraints = mkconstraints("H:|[t]-[s]|", views)
		self.addConstraints(constraints)
		constraints = mkconstraints("V:|[t]|", views)
		self.addConstraints(constraints)
		constraints = mkconstraints("V:|[s]|", views)
		self.addConstraints(constraints)
	}

	required init?(coder: NSCoder) {
		fatalError("can't use this constructor, sorry")
	}

	// TODO leave only the required amount of space around the alignment rect
	// TODO even with this the stepper sometimes gets cut off at the bottom *anyway*
	override var alignmentRectInsets: NSEdgeInsets {
		get {
			return NSEdgeInsetsMake(50, 50, 50, 50)
		}
	}

	func View() -> NSView {
		return self
	}

	func SetParent(p: Control) {
		self.parent = p
	}

	// TODO justify this
	// TODO no really, is this height the right way to go?
	// TODO restrict width to the text field only?
	override var intrinsicContentSize: NSSize {
		get {
			var s = super.intrinsicContentSize
			s.width = 96
			s.height = max(self.t.intrinsicContentSize.height, self.s.intrinsicContentSize.height)
			return s
		}
	}
}
