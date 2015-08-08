// 31 july 2015
import Cocoa

class Label : NSTextField, Control {
	private var parent: Control?

	init() {
		var cell: NSTextFieldCell

		self.parent = nil

		super.init(frame: NSZeroRect)
		self.stringValue = "Label"
		self.editable = false
		self.selectable = false
		self.drawsBackground = false
		self.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
		self.bordered = false
		self.bezelStyle = NSTextFieldBezelStyle.SquareBezel
		self.bezeled = false
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
}
