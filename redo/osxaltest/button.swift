// 31 july 2015
import Cocoa

class Button : NSButton, Control {
	private var parent: Control?

	init(_ text: String) {
		self.parent = nil

		super.init(frame: NSZeroRect)
		self.title = text
		self.setButtonType(NSButtonType.MomentaryPushInButton)
		self.bordered = true
		self.bezelStyle = NSBezelStyle.RoundedBezelStyle
		self.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
		self.translatesAutoresizingMaskIntoConstraints = false
	}

	required init?(coder: NSCoder) {
		fatalError("can't use this constructor, sorry")
	}

	func View() -> NSView {
		return self
	}

	func SetParent(p: Control) {
		self.parent = p
	}
}
