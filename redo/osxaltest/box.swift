// 7 august 2015
import Cocoa

struct BoxControl {
	var c: Control
	var stretchy: Bool
}

class Box : NSView, Control {
	private var controls: [BoxControl]
	private var parent: Control?
	private var padded: Bool

	init(_ padded: Bool) {
		self.controls = []
		self.parent = nil
		self.padded = padded

		super.init(frame: NSZeroRect)
		self.translatesAutoresizingMaskIntoConstraints = false
	}

	func Add(control: Control, stretchy: Stretchy) {
		var c BoxControl

		c.c = control
		c.stretchy = stretchy
		self.addSubview(c.c.View())
		self.controls.append(c)
		// TODO relayout
	}

	func View() -> NSView {
		return self
	}

	func SetParent(p: Control) {
		self.parent = p
	}

	func Relayout() {
		if self.parent != nil {
			self.parent?.Relayout()
		}
	}
}
