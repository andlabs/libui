// 31 july 2015
import Cocoa

// leave a whole lot of space around the alignment rect, just to be safe
class tSpinboxContainer : NSView {
	override var alignmentRectInsets: NSEdgeInsets {
		get {
print("in tSpinboxContainer.alignmentRectInsets")
			return NSEdgeInsetsMake(50, 50, 50, 50)
		}
	}
}

var nspinbox = 0

class tSpinbox : tControl {
	private var c: tSpinboxContainer
	private var t: NSTextField
	private var s: NSStepper
	private var parent: tControl?
	private var horzpri, vertpri: NSLayoutPriority

	init() {
		var cell: NSTextFieldCell

		self.c = tSpinboxContainer(frame: NSZeroRect)
		self.c.translatesAutoresizingMaskIntoConstraints = false

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
		self.c.addSubview(self.t)

		self.s = NSStepper(frame: NSZeroRect)
		self.s.increment = 1
		self.s.valueWraps = false
		self.s.autorepeat = true
		self.s.translatesAutoresizingMaskIntoConstraints = false
		self.c.addSubview(self.s)

		var views = [
			"t":	self.t,
			"s":	self.s,
		]
		var constraints = NSLayoutConstraint.constraintsWithVisualFormat(
			"H:|[t]-[s]|",
			options: NSLayoutFormatOptions(0),
			metrics: nil,
			views: views)
		self.c.addConstraints(constraints)
		constraints = NSLayoutConstraint.constraintsWithVisualFormat(
			"V:|[t]|",
			options: NSLayoutFormatOptions(0),
			metrics: nil,
			views: views)
		self.c.addConstraints(constraints)
		constraints = NSLayoutConstraint.constraintsWithVisualFormat(
			"V:|[s]|",
			options: NSLayoutFormatOptions(0),
			metrics: nil,
			views: views)
		self.c.addConstraints(constraints)

		self.parent = nil

		self.horzpri = self.c.contentHuggingPriorityForOrientation(NSLayoutConstraintOrientation.Horizontal)
		self.vertpri = self.c.contentHuggingPriorityForOrientation(NSLayoutConstraintOrientation.Vertical)
	}

	func tSetParent(p: tControl, addToView v: NSView) {
		self.parent = p
		v.addSubview(self.c)
	}

	func tFillAutoLayout(inout p: tAutoLayoutParams) {
		// reset the hugging priority
		self.c.setContentHuggingPriority(self.horzpri, forOrientation:NSLayoutConstraintOrientation.Horizontal)
		self.c.setContentHuggingPriority(self.vertpri, forOrientation:NSLayoutConstraintOrientation.Vertical)

		p.view = self.c
		p.attachLeft = true
		p.attachTop = true
		p.attachRight = true
		p.attachBottom = true
		p.nonStretchyWidthPredicate = "(==96)"		// TODO on the text field only
	}

	func tRelayout() {
		if self.parent != nil {
			self.parent?.tRelayout()
		}
	}
}
