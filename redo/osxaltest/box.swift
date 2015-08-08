// 7 august 2015
import Cocoa

struct BoxControl {
	var c: Control
	var stretchy: Bool
}

class Box : NSView, Control {
	private var controls: [BoxControl]
	private var parent: Control?
	private var vertical: Bool
	private var padded: Bool

	private var primaryDirPrefix: String
	private var secondaryDirPrefix: String

	init(vertical: Bool, padded: Bool) {
		self.controls = []
		self.parent = nil
		self.vertical = vertical
		self.padded = padded

		self.primaryDirPrefix = "H:"
		self.secondaryDirPrefix = "V:"
		if self.vertical {
			self.primaryDirPrefix = "V:"
			self.secondaryDirPrefix = "H:"
		}

		super.init(frame: NSZeroRect)
		self.translatesAutoresizingMaskIntoConstraints = false
	}

	required init?(coder: NSCoder) {
		fatalError("can't use this constructor, sorry")
	}

	func Add(control: Control, stretchy: Stretchy) {
		var c BoxControl

		c.c = control
		c.stretchy = stretchy
		self.addSubview(c.c.View())
		self.controls.append(c)
		self.relayout()
	}

	func View() -> NSView {
		return self
	}

	func SetParent(p: Control) {
		self.parent = p
	}

	// TODO stretchiness
	// - we will need a trailing view if there are no stretchy controls
	private func relayout() {
		if self.children.count == 0 {
			return
		}

		self.removeConstraints(self.constraints)

		// first collect the views
		var views = [String: NSView]()
		var n = 0
		for c in self.controls {
			views["view\(n)"] = c.c.View()
		}

		// next, assemble the views in the primary direction
		// they all go in a straight line

		// next: assemble the views in the secondary direction
		// each of them will span the secondary direction
	}
}
