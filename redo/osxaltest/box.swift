// 7 august 2015
import Cocoa

struct BoxControl {
	var c: Control
	var stretchy: Bool
	var horzHuggingPri: NSLayoutPriority
	var vertHuggingPri: NSLayoutPriority
}

class Box : NSView, Control {
	private var controls: [BoxControl]
	private var parent: Control?
	private var vertical: Bool
	private var padded: Bool

	private var primaryDirPrefix: String
	private var secondaryDirPrefix: String
	private var primaryOrientation: NSLayoutConstraintOrientation
	private var secondaryOrientation: NSLayoutConstraintOrientation

	init(vertical: Bool, padded: Bool) {
		self.controls = []
		self.parent = nil
		self.vertical = vertical
		self.padded = padded

		self.primaryDirPrefix = "H:"
		self.secondaryDirPrefix = "V:"
		self.primaryOrientation = NSLayoutConstraintOrientation.Horizontal
		self.secondaryOrientation = NSLayoutConstraintOrientation.Vertical
		if self.vertical {
			self.primaryDirPrefix = "V:"
			self.secondaryDirPrefix = "H:"
			self.primaryOrientation = NSLayoutConstraintOrientation.Vertical
			self.secondaryOrientation = NSLayoutConstraintOrientation.Horizontal
		}

		super.init(frame: NSZeroRect)
		self.translatesAutoresizingMaskIntoConstraints = false
	}

	required init?(coder: NSCoder) {
		fatalError("can't use this constructor, sorry")
	}

	func Add(control: Control, _ stretchy: Bool) {
		var c: BoxControl

		var view = control.View()
		c = BoxControl(
			c:				control,
			stretchy:			stretchy,
			horzHuggingPri:	horzHuggingPri(view),
			vertHuggingPri:	vertHuggingPri(view))
		self.addSubview(view)
		self.controls.append(c)

		// if a control is stretchy, it should not hug in the primary direction
		// otherwise, it should *forcibly* hug
		if c.stretchy {
			setHuggingPri(view, myNSLayoutPriorityDefaultLow, self.primaryOrientation)
		} else {
			setHuggingPri(view, myNSLayoutPriorityRequired, self.primaryOrientation)
		}

		// make sure controls don't hug their secondary direction so they fill the width of the view
		setHuggingPri(view, myNSLayoutPriorityDefaultLow, self.secondaryOrientation)

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
		var constraint: String

		if self.controls.count == 0 {
			return
		}

		self.removeConstraints(self.constraints)

		// first collect the views
		var views = [String: NSView]()
		var n = 0
		for c in self.controls {
			views["view\(n)"] = c.c.View()
			n++
		}

		// next, assemble the views in the primary direction
		// they all go in a straight line
		constraint = "\(self.primaryDirPrefix)|"
		for i in 0..<n {
			if self.padded && i != 0 {
				constraint += "-"
			}
			constraint += "[view\(i)]"
		}
		constraint += "|"
		var constraints = mkconstraints(constraint, views)
		self.addConstraints(constraints)

		// next: assemble the views in the secondary direction
		// each of them will span the secondary direction
		for i in 0..<n {
			constraint = "\(self.secondaryDirPrefix)|[view\(i)]|"
			var constraints = mkconstraints(constraint, views)
			self.addConstraints(constraints)
		}
	}
}
