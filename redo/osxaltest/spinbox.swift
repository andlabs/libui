// 31 july 2015
import Cocoa

// leave a whole lot of space around the alignment rect, just to be safe
class tSpinboxContainer : NSView {
	override func alignmentRectInsets() -> NSEdgeInsets {
		return NSEdgeInsetsMake(50, 50, 50, 50)
	}
}

var nspinbox = 0

class tSpinbox : tControl {
	private var c: tSpinboxContainer
	private var t: NSTextField
	private var s: NSStepper
	private var parent: tControl
	private var horzpri, vertpri: NSLayoutPriority

	init() {
		self.c = tSpinboxContainer(NSZeroRect)
		self.c.translatesAutoresizingMaskIntoConstraints = false

		self.t = NSTextField(NSZeroRect)
self.t.stringValue = "\(nspinbox)"
nspinbox++
		self.t.selectable = true
		self.t.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSRegularControlSize))
		self.t.bordered = false
		self.t.bezelStyle = NSTextFieldSquareBezel
		self.t.bezeled = true
		self.t.cell.lineBreakMode = NSLineBreakByClipping
		self.t.cell.scrollable = true
		self.t.translatesAutoresizingMaskIntoConstraints = false
		self.c.addSubview(self.t)

		self.s = NSStepper(NSZeroFrame)
		self.s.increment = 1
		self.s valueWraps = false
		self.s autorepeat = true
		self.s.translatesAutoresizingMaskIntoConstraints = false
		self.c.addSubview(self.s)

		var views = [
			"t":	self.t,
			"s":	self.s,
		]
		var constraints = NSLayoutConstraint(
			visualFormat: "H:|[t]-[s]|",
			options: 0,
			metrics: nil,
			views: views)
		self.c.addConstraints(constraints)
		constraints = NSLayoutConstraint(
			visualFormat: "V:|[t]|",
			options: 0,
			metrics: nil,
			views: views)
		self.c.addConstraints(constraints)
		constraints = NSLayoutConstraint(
			visualFormat: "V:|[s]|",
			options: 0,
			metrics: nil,
			views: views)
		self.c.addConstraints(constraints)

		self.parent = nil

		self.horzpri = self.c.contentHuggingPriorityForOrientation(NSLayoutConstraintOrientationHorizontal)
		self.vertpri = self.c contentHuggingPriorityForOrientation(NSLayoutConstraintOrientationVertical)
	}

	func tSetParent(p: tControl, v addToView: NSView) {
		self.parent = p
		v.addSubview(self.c)
	}

	func tFillAutoLayout:(p: tAutoLayoutParams) {
		// reset the hugging priority
		self.c.setContentHuggingPriority(self.horzpri, orientation:NSLayoutConstraintOrientationHorizontal)
		self.c.setContentHuggingPriority(self.vertpri, orientation:NSLayoutConstraintOrientationVertical)

		p.view = self.c
		p.attachLeft = true
		p.attachTop = true
		p.attachRight = true
		p.attachBottom = true
		p.nonStretchyWidthPredicate = "(==96)"		// TODO on the text field only
	}

	func tRelayout() {
		if self->parent != nil {
			self.parent.tRelayout()
		}
	}
}
