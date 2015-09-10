// 9 september 2015
// from misctestprogs/scratcmac.swift 17 august 2015
import Cocoa

var keepAliveMainwin: NSWindow? = nil
var pprogram: AnyObject? = nil

class intrinsicSizeScrollbar : NSScroller {
	// notes: default intrinsic size is (-1,-1)
	override var intrinsicContentSize: NSSize {
		get {
			var s = super.intrinsicContentSize
			s.height = NSScroller.scrollerWidthForControlSize(
				self.controlSize,
				scrollerStyle: self.scrollerStyle)
			return s
		}
	}
}

class program : NSObject, NSTextFieldDelegate {
	private var nhspinb: NSTextField
	private var nhformatter: NSNumberFormatter

	private var scrollbar: NSScroller

	private var info: NSTextField

	init(_ contentView: NSView) {
		var views: [String: NSView]

		var nhlabel = newLabel("H Max")
		nhlabel.setContentHuggingPriority(1000, forOrientation: NSLayoutConstraintOrientation.Horizontal)
		self.nhspinb = newTextField()

		var container1 = newContainerView()
		container1.addSubview(nhlabel)
		container1.addSubview(self.nhspinb)
		views = [
			"nhlabel":		nhlabel,
			"nhspinb":	self.nhspinb,
		]
		addConstraint(container1, "H:|[nhlabel]-[nhspinb]|", views)
		addConstraint(container1, "V:|[nhlabel]|", views)
		addConstraint(container1, "V:|[nhspinb]|", views)

		var ss = NSScroller.preferredScrollerStyle()
		var ccs = NSScroller.scrollerWidthForControlSize(
			NSControlSize.RegularControlSize,
			scrollerStyle: ss)
		self.scrollbar = intrinsicSizeScrollbar(frame: NSMakeRect(0, 0, ccs * 5, ccs))
		self.scrollbar.scrollerStyle = ss
		self.scrollbar.knobStyle = NSScrollerKnobStyle.Default
		self.scrollbar.controlTint = NSControlTint.DefaultControlTint
		self.scrollbar.controlSize = NSControlSize.RegularControlSize
//TODO		self.scrollbar.arrowsPosition = NSScrollArrowPosition.ScrollerArrowsDefaultSetting
		self.scrollbar.translatesAutoresizingMaskIntoConstraints = false

		self.info = newLabel("")

		contentView.addSubview(container1)
		contentView.addSubview(self.scrollbar)
		contentView.addSubview(self.info)
		views = [
			"container1":	container1,
			"scrollbar":	self.scrollbar,
			"info":		self.info,
		]
		addConstraint(contentView, "V:|-[container1]-[scrollbar]-[info]-|", views)
		addConstraint(contentView, "H:|-[container1]-|", views)
		addConstraint(contentView, "H:|-[scrollbar]-|", views)
		addConstraint(contentView, "H:|-[info]-|", views)

		self.nhformatter = newNumberFormatter(0, 100000)

		super.init()

		self.nhspinb.formatter = self.nhformatter
		self.nhspinb.integerValue = 0
		self.nhspinb.delegate = self

		self.scrollbar.target = self
		self.scrollbar.action = "onScroll:"

		var nc = NSNotificationCenter.defaultCenter()
		nc.addObserver(self,
			selector: "scrollbarSizeChanged:",
			name: NSViewFrameDidChangeNotification,
			object: self.scrollbar)
		// this will post a notification, causing an update
		self.scrollbar.postsFrameChangedNotifications = true
	}

	override func controlTextDidChange(note: NSNotification) {
		update()
	}

	func scrollbarSizeChanged(note: NSNotification) {
		update()
	}

	func update() {
		var swidth = self.scrollbar.frame.width
		var max = CGFloat(self.nhspinb.integerValue)
		if max == 0 {
			self.scrollbar.knobProportion = 0
			// this hides the knob
			self.scrollbar.enabled = false
			self.updateLabels()
			return
		}
		self.scrollbar.knobProportion = swidth / max
		self.scrollbar.enabled = true
		self.updateLabels()
	}

	func lineincr() -> CGFloat {
		var swidth = self.scrollbar.frame.width
		var max = CGFloat(self.nhspinb.integerValue) - swidth
		if max <= 0 {
			return 0
		}
		return 1.0 / max
	}

	func pageincr() -> CGFloat {
		return self.lineincr() * self.scrollbar.frame.width
	}

	@IBAction func onScroll(sender: AnyObject) {
		switch self.scrollbar.hitPart {
		case NSScrollerPart.NoPart:
			// do nothing
			break
		case NSScrollerPart.DecrementPage:
			self.scrollbar.doubleValue -= Double(self.pageincr())
		case NSScrollerPart.Knob:
			// do nothing
			break
		case NSScrollerPart.IncrementPage:
			self.scrollbar.doubleValue += Double(self.pageincr())
		case NSScrollerPart.DecrementLine:
			self.scrollbar.doubleValue -= Double(self.lineincr())
		case NSScrollerPart.IncrementLine:
			self.scrollbar.doubleValue += Double(self.lineincr())
		case NSScrollerPart.KnobSlot:
			// do nothing
			break
		}
		self.updateLabels()
	}

	func updateLabels() {
		var swidth = self.scrollbar.frame.width
		var max = CGFloat(self.nhspinb.integerValue) - swidth
		if max < 0 {
			max = 0
		}
		var infostr = "Width: \(swidth)"
		infostr += "\nProportion: \(self.scrollbar.knobProportion)"
		infostr += "\nDouble Value: \(self.scrollbar.doubleValue)"
		var scaledValue = Int(self.scrollbar.doubleValue * Double(max))
		infostr += "\nScaled: \(scaledValue)"
		var lineincr: CGFloat = 0
		if max != 0 {
			lineincr = 1.0 / max
		}
		infostr += "\nLine Increment: \(lineincr)"
		infostr += "\nPage Increment: \(lineincr * swidth)"
		self.info.stringValue = infostr
	}
}

func newNumberFormatter(min: Int, max: Int) -> NSNumberFormatter {
	var nf = NSNumberFormatter()
	nf = NSNumberFormatter()
	nf.formatterBehavior = NSNumberFormatterBehavior.Behavior10_4
	nf.localizesFormat = false
	nf.usesGroupingSeparator = false
	nf.hasThousandSeparators = false
	nf.allowsFloats = false
	nf.minimum = min
	nf.maximum = max
	return nf
}

func newContainerView() -> NSView {
	var v = NSView(frame: NSZeroRect)
	v.translatesAutoresizingMaskIntoConstraints = false
	return v
}

class intrinsicWidthTextField : NSTextField {
	override var intrinsicContentSize: NSSize {
		get {
			var s = super.intrinsicContentSize
			s.width = 96
			return s
		}
	}
}

func newTextField() -> NSTextField {
	var tf: NSTextField
	var cell: NSTextFieldCell

	tf = intrinsicWidthTextField(frame: NSZeroRect)
	tf.selectable = true
	tf.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
	tf.bordered = false
	tf.bezelStyle = NSTextFieldBezelStyle.SquareBezel
	tf.bezeled = true
	cell = tf.cell() as! NSTextFieldCell
	cell.lineBreakMode = NSLineBreakMode.ByClipping
	cell.scrollable = true
	tf.translatesAutoresizingMaskIntoConstraints = false
	tf.setContentCompressionResistancePriority(1000, forOrientation: NSLayoutConstraintOrientation.Vertical)
	return tf
}

func newLabel(text: String) -> NSTextField {
	var tf: NSTextField
	var cell: NSTextFieldCell

	tf = NSTextField(frame: NSZeroRect)
	tf.stringValue = text
	tf.editable = false
	tf.selectable = false
	tf.drawsBackground = false
	tf.font = NSFont.systemFontOfSize(NSFont.systemFontSizeForControlSize(NSControlSize.RegularControlSize))
	tf.bordered = false
	tf.bezelStyle = NSTextFieldBezelStyle.SquareBezel
	tf.bezeled = false
	cell = tf.cell() as! NSTextFieldCell
	cell.lineBreakMode = NSLineBreakMode.ByClipping
	cell.scrollable = true
	tf.translatesAutoresizingMaskIntoConstraints = false
	tf.setContentHuggingPriority(250, forOrientation: NSLayoutConstraintOrientation.Horizontal)
	return tf
}

func appLaunched() {
	var mainwin = NSWindow(
		contentRect: NSMakeRect(0, 0, 320, 240),
		styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask),
		backing: NSBackingStoreType.Buffered,
		defer: true)
	var contentView = mainwin.contentView as! NSView

	pprogram = program(contentView)

	mainwin.cascadeTopLeftFromPoint(NSMakePoint(20, 20))
	mainwin.makeKeyAndOrderFront(mainwin)
	keepAliveMainwin = mainwin
}

func addConstraint(view: NSView, constraint: String, views: [String: NSView]) {
	var constraints = NSLayoutConstraint.constraintsWithVisualFormat(
		constraint,
		options: NSLayoutFormatOptions(0),
		metrics: nil,
		views: views)
	view.addConstraints(constraints)
}

class appDelegate : NSObject, NSApplicationDelegate {
	func applicationDidFinishLaunching(note: NSNotification) {
		appLaunched()
	}

	func applicationShouldTerminateAfterLastWindowClosed(app: NSApplication) -> Bool {
		return true
	}
}

func main() {
	var app = NSApplication.sharedApplication()
	app.setActivationPolicy(NSApplicationActivationPolicy.Regular)
	// NSApplication.delegate is weak; if we don't use the temporary variable, the delegate will die before it's used
	var delegate = appDelegate()
	app.delegate = delegate
	app.run()
}

main()
