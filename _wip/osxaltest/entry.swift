// 31 july 2015
import Cocoa

class Entry : NSTextField, Control {
	private var parent: Control?

	init() {
		var cell: NSTextFieldCell

		self.parent = nil

		super.init(frame: NSZeroRect)
		self.selectable = true
		self.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
		self.bordered = false
		self.bezelStyle = NSTextFieldBezelStyle.SquareBezel
		self.bezeled = true
		cell = self.cell() as! NSTextFieldCell
		cell.lineBreakMode = NSLineBreakMode.ByClipping
		cell.scrollable = true
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

	// by default a text entry has no intrinsic content width
	// in order for our layout containers to work, we need to give it one
	// give it what Interface Builder uses as a default
	// TODO verify against Interface Builder
	override var intrinsicContentSize: NSSize {
		get {
			var s = super.intrinsicContentSize
			s.width = 96
			return s
		}
	}
}
