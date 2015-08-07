// 31 july 2015
import Cocoa

// leave a whole lot of space around the alignment rect, just to be safe
// TODO fine tune this
// TODO de-duplicate this from spinbox.m
class tBoxContainer : NSView {
	override func alignmentRectInsets() -> NSEdgeInsets {
		return NSEdgeInsetsMake(50, 50, 50, 50)
	}
}

struct tBoxChild {
	var c: tControl
	var stretchy: Bool
}

class tBox : tControl {
	private var v: NSView
	private var children: [tBoxChild]
	private var vertical: Bool
	private var parent: tControl
	private var spaced: Bool

	// TODO rename to padded
	init(vertical: Bool, spaced: Bool) {
		self.v = tBoxContainer(frame: NSZeroRect)
		self.v.translatesAutoresizingMaskIntoConstraints = false
		self.children = []
		self.vertical = vertical
		self.parent = nil
		self.spaced = spaced
	}

	func tAddControl(c: tControl, stretchy: Bool) {
		c.tSetParent(self, addToView: self.v)
		self.children.append(tBoxChild(
			c:			c,
			stretchy:		stretchy))
		self.tRelayout()
	}

	func tSetParent(p: tControl, addToView v: NSView) {
		self.parent = p
		v.addSubview(self.v)
	}

	// TODO make the other dimension not hug (as an experiment)
	func tFillAutoLayout(p: tAutoLayoutParams) {
		var orientation: NSLayoutConstraintOrientation
		var i, n: UIntMax
		var pp: tAutoLayoutParams
		var nStretchy: UIntMax

		if self.children.count == 0 {
			goto selfOnly
		}

		self.v.removeConstraints(self.v.constraints)

		orientation = NSLayoutConstraintOrientation.Horizontal
		if self.vertical {
			orientation = NSLayoutConstraintOrientation.Vertical
		}

		var views = [String: NSView]()
		n = 0
		var predicates = [String]()
		for child in self.children {
			var priority: NSLayoutPriority

			pp.nonStretchyWidthPredicate = ""
			pp.nonStretchyHeightPredicate = ""
			// this also resets the hugging priority
			// TODO do this when adding and removing controls instead
			child.c.tFillAutoLayout(pp)
			priority = NSLayoutPriorityDefaultHigh			// forcibly hug; avoid stretching out
			if child.stretchy {
				priority = NSLayoutPriorityDefaultLow		// do not forcibly hug; freely stretch out
			}
			if self.vertical {
				predicates.append(pp.nonStretchyHeightPredicate)
			} else {
				predicates.append(pp.nonStretchyWidthPredicate)
			}
			pp.view.setContentHuggingPriority(priority, forOrientation:orientation)
			views[tAutoLayoutKey(n)] = pp.view
			n++
		}

		// first string the views together
		var constraint = "H:"
		if self.vertical {
			constraint = "V:"
		}
		var firstStretchy = true
		for i = 0; i < n; i++ {
			if self.spaced && i != 0 {
				constraint += "-"
			}
			constraint += "[" + tAutoLayoutKey(i)
			if self.children[i].stretchy {
				if firstStretchy {
					firstStretchy = false
					nStretchy = i
				} else {
					constraint += "(==" + tAutoLayoutKey(nStretchy) + ")"
				}
			} else {
				constraint += predicates[i]
			}
			constraint += "]"
		}
		constraint += "|"
		self.v.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat(
			visualFormat: constraint,
			options: 0,
			metrics: nil,
			views: views))
		// TODO do not release constraint; it's autoreleased?

		// next make the views span the full other dimension
		// TODO make all of these the same width/height
		for i = 0; i < n; i++ {
			constraint = "V:|["
			if self.vertical {
				constraint = "H:|["
			}
			constraint += tAutoLayoutKey(i) + "]|"
			self.v.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat(
				visualFormat: constraint,
				options: 0,
				metrics: nil,
				views: views))
			// TODO do not release constraint; it's autoreleased?
		}

		// and now populate for self
	selfOnly:
		p.view = self.v
		p.attachLeft = true
		p.attachTop = true
		// don't attach to the end if there weren't any stretchy controls
		// firstStretchy is false if there was at least one stretchy control
		if self.vertical {
			p.attachRight = true
			p.attachBottom = !firstStretchy
		} else {
			p.attachRight = !firstStretchy
			p.attachBottom = true
		}
	}

	func tRelayout() {
		if self.parent != nil {
			self.parent.tRelayout()
		}
	}
}
