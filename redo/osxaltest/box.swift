// 31 july 2015
import Cocoa

// leave a whole lot of space around the alignment rect, just to be safe
// TODO fine tune this
// TODO de-duplicate this from spinbox.m
class tBoxContainer : NSView {
	override var alignmentRectInsets: NSEdgeInsets {
		get {
debugPrint("in tBoxContainer.alignmentRectInsets")
			return NSEdgeInsetsMake(50, 50, 50, 50)
		}
	}
}

struct tBoxChild {
	var c: tControl
	var stretchy: Bool
}

// the swift bridge isn't perfect; it won't recognize these properly
// thanks to Eridius in freenode/#swift-lang
let myNSLayoutPriorityRequired: NSLayoutPriority = 1000
let myNSLayoutPriorityDefaultHigh: NSLayoutPriority = 750
let myNSLayoutPriorityDragThatCanResizeWindow: NSLayoutPriority = 510
let myNSLayoutPriorityWindowSizeStayPut: NSLayoutPriority = 500
let myNSLayoutPriorityDragThatCannotResizeWindow: NSLayoutPriority = 490
let myNSLayoutPriorityDefaultLow: NSLayoutPriority = 250
let myNSLayoutPriorityFittingSizeCompression: NSLayoutPriority = 50

class tBox : tControl {
	private var v: NSView
	private var children: [tBoxChild]
	private var vertical: Bool
	private var parent: tControl?
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
	func tFillAutoLayout(inout p: tAutoLayoutParams) {
		var hasStretchy = false
		if self.children.count == 0 {
			hasStretchy = self.actualLayoutWork()
		}
		p.view = self.v
		p.attachLeft = true
		p.attachTop = true
		// don't attach to the end if there weren't any stretchy controls
		if self.vertical {
			p.attachRight = true
			p.attachBottom = hasStretchy
		} else {
			p.attachRight = hasStretchy
			p.attachBottom = true
		}
	}

	func actualLayoutWork() -> Bool {
		var orientation: NSLayoutConstraintOrientation
		// TODO don't use UIntMax
		var i, n: UIntMax
		var nStretchy: UIntMax

		self.v.removeConstraints(self.v.constraints)

		orientation = NSLayoutConstraintOrientation.Horizontal
		if self.vertical {
			orientation = NSLayoutConstraintOrientation.Vertical
		}

		var views = [String: NSView]()
		n = 0
		var predicates = [String]()
		var pp = tAutoLayoutParams()
		for child in self.children {
			var priority: NSLayoutPriority

			pp.nonStretchyWidthPredicate = ""
			pp.nonStretchyHeightPredicate = ""
			// this also resets the hugging priority
			// TODO do this when adding and removing controls instead
			child.c.tFillAutoLayout(&pp)
			priority = myNSLayoutPriorityDefaultHigh			// forcibly hug; avoid stretching out
			if child.stretchy {
				priority = myNSLayoutPriorityDefaultLow		// do not forcibly hug; freely stretch out
			}
			if self.vertical {
				predicates.append(pp.nonStretchyHeightPredicate)
			} else {
				predicates.append(pp.nonStretchyWidthPredicate)
			}
			pp.view?.setContentHuggingPriority(priority, forOrientation:orientation)
			views[tAutoLayoutKey(n)] = pp.view
			n++
		}

		// first string the views together
		var constraint = "H:"
		if self.vertical {
			constraint = "V:"
		}
		var firstStretchy = true
		// swift can't tell that nStretchy isn't used until firstStretchy becomes false
		nStretchy = 0
		for i in 0..<n {
			if self.spaced && i != 0 {
				constraint += "-"
			}
			constraint += "[" + tAutoLayoutKey(i)
			// swift currently can't do self.children[i].stretchy
			var child = self.children[Int(i)]
			if child.stretchy {
				if firstStretchy {
					firstStretchy = false
					nStretchy = i
				} else {
					constraint += "(==" + tAutoLayoutKey(nStretchy) + ")"
				}
			} else {
				constraint += predicates[Int(i)]
			}
			constraint += "]"
		}
		constraint += "|"
		self.v.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat(
			constraint,
			options: NSLayoutFormatOptions(0),
			metrics: nil,
			views: views))
		// TODO do not release constraint; it's autoreleased?

		// next make the views span the full other dimension
		// TODO make all of these the same width/height
		for i in 0..<n {
			constraint = "V:|["
			if self.vertical {
				constraint = "H:|["
			}
			constraint += tAutoLayoutKey(i) + "]|"
			self.v.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat(
				constraint,
				options: NSLayoutFormatOptions(0),
				metrics: nil,
				views: views))
			// TODO do not release constraint; it's autoreleased?
		}

		// the caller needs to know if a control was stretchy
		// firstStretchy is false if there was one
		return !firstStretchy
	}

	func tRelayout() {
		if self.parent != nil {
			self.parent?.tRelayout()
		}
	}
}
