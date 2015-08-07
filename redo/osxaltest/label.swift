// 31 july 2015
import Cocoa

class tLabel : tControl {
	private var t: NSTextField
	private var parent: tControl?
	private var horzpri, vertpri: NSLayoutPriority

	init() {
		var cell: NSTextFieldCell

		self.t = NSTextField(frame: NSZeroRect)
		self.t.stringValue = "Label"
		self.t.editable = false
		self.t.selectable = false
		self.t.drawsBackground = false
		self.t.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
		self.t.bordered = false
		self.t.bezelStyle = NSTextFieldBezelStyle.SquareBezel
		self.t.bezeled = false
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
	}

	func tRelayout() {
		if self.parent != nil {
			self.parent?.tRelayout()
		}
	}
}
