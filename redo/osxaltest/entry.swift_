// 31 july 2015
import Cocoa

class tEntry : tControl {
	private var t: NSTextField
	private var parent: tControl?
	private var horzpri, vertpri: NSLayoutPriority

	init() {
		var cell: NSTextFieldCell

		self.t = NSTextField(frame: NSZeroRect)
		self.t.selectable = true
		self.t.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
		self.t.bordered = false
		self.t.bezelStyle = NSTextFieldBezelStyle.SquareBezel
		self.t.bezeled = true
		cell = self.t.cell() as! NSTextFieldCell
		cell.lineBreakMode = NSLineBreakMode.ByClipping
		cell.scrollable = true
		self.t.translatesAutoresizingMaskIntoConstraints = false

		self.parent = nil

		self.horzpri = self.t.contentHuggingPriorityForOrientation(NSLayoutConstraintOrientation.Horizontal)
		self.vertpri = self.t.contentHuggingPriorityForOrientation(NSLayoutConstraintOrientation.Vertical)
	}

	func tSetParent(p: tControl, addToView v: NSView) {
		self.parent = p
		v.addSubview(self.t)
	}

	func tFillAutoLayout(inout p: tAutoLayoutParams) {
		// reset the hugging priority
		self.t.setContentHuggingPriority(self.horzpri, forOrientation:NSLayoutConstraintOrientation.Horizontal)
		self.t.setContentHuggingPriority(self.vertpri, forOrientation:NSLayoutConstraintOrientation.Vertical)

		p.view = self.t
		p.attachLeft = true
		p.attachTop = true
		p.attachRight = true
		p.attachBottom = true
		p.nonStretchyWidthPredicate = "(==96)"		// TODO verify against Interface Builder
	}

	func tRelayout() {
		if self.parent != nil {
			self.parent?.tRelayout()
		}
	}
}
