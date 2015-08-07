// 31 july 2015
import Cocoa

class tEntry : tControl {
	private var b: NSButton
	private var parent: tControl
	private var horzpri, vertpri: NSLayoutPriority

	init() {
		self.t = NSTextField(NSZeroRect)
		self.t.stringValue = "Label"
		self.t.setEditable = false
		self.t.selectable = false
		self.t.drawsBackground = false
		self.t.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSRegularControlSize))
		self.t.bordered = false
		self.t.bezelStyle = NSTextFieldSquareBezel
		self.t.bezeled = false
		self.t.cell.lineBreakMode = NSLineBreakByClipping
		self.t.cell.scrollable = true
		self.t.translatesAutoresizingMaskIntoConstraints = false

		self.parent = nil

		self.horzpri = self.t.contentHuggingPriorityForOrientation(NSLayoutConstraintOrientationHorizontal)
		self.vertpri = self.t contentHuggingPriorityForOrientation(NSLayoutConstraintOrientationVertical)
	}

	func tSetParent(p: tControl, v addToView: NSView) {
		self.parent = p
		v.addSubview(self.t)
	}

	func tFillAutoLayout:(p: tAutoLayoutParams) {
		// reset the hugging priority
		self.t.setContentHuggingPriority(self.horzpri, orientation:NSLayoutConstraintOrientationHorizontal)
		self.t.setContentHuggingPriority(self.vertpri, orientation:NSLayoutConstraintOrientationVertical)

		p.view = self.t
		p.attachLeft = true
		p.attachTop = true
		p.attachRight = true
		p.attachBottom = true
	}

	func tRelayout() {
		if self->parent != nil {
			self.parent.tRelayout()
		}
	}
}
