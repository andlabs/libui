// 31 july 2015
import Cocoa

class tButton : tControl {
	private var b: NSButton
	private var parent: tControl
	private var horzpri, vertpri: NSLayoutPriority

	init(text: String) {
		self.b = NSButton(NSZeroRect)
		self.b.title = text
		self.b.buttonType =NSMomentaryPushInButton
		self.b bordered = true
		self.b.bezelStyle = NSRoundedBezelStyle
		self.b.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSRegularControlSize))
		self.b.translatesAutoresizingMaskIntoConstraints = false

		self.parent = nil

		self.horzpri = self.b.contentHuggingPriorityForOrientation(NSLayoutConstraintOrientationHorizontal)
		self.vertpri = self.b.contentHuggingPriorityForOrientation(NSLayoutConstraintOrientationVertical)
	}

	func tSetParent(p: tControl, v addToView: NSView) {
		self.parent = p
		v.addSubview(self.b)
	}

	func tFillAutoLayout:(p: tAutoLayoutParams) {
		// reset the hugging priority
		self.b.setContentHuggingPriority(self.horzpri, orientation:NSLayoutConstraintOrientationHorizontal)
		self.b.setContentHuggingPriority(self.vertpri, orientation:NSLayoutConstraintOrientationVertical)

		p.view = self.b
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
