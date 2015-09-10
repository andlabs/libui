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

class program : NSObject {
	private var nhspinb: NSTextField
	private var nvspinb: NSTextField
	private var scrollbar: NSScroller

	init(_ contentView: NSView) {
		self.nhspinb = newTextField()
		self.nvspinb = newTextField()

		var ss = NSScroller.preferredScrollerStyle()
		var ccs = NSScroller.scrollerWidthForControlSize(
			NSControlSize.RegularControlSize,
			scrollerStyle: ss)
		self.scrollbar = intrinsicSizeScrollbar(frame: NSMakeRect(0, 0, ccs * 5, ccs))
		self.scrollbar.scrollerStyle = ss
		self.scrollbar.knobStyle = NSScrollerKnobStyle.Default
		self.scrollbar.controlTint = NSControlTint.DefaultControlTint
		self.scrollbar.controlSize = NSControlSize.RegularControlSize
//TODO		self.scrollbar.arrowPosition = NSScrollArrowPosition.ScrollerArrowsDefaultSetting
		self.scrollbar.translatesAutoresizingMaskIntoConstraints = false

		contentView.addSubview(self.nhspinb)
		contentView.addSubview(self.nvspinb)
		contentView.addSubview(self.scrollbar)

		var views: [String: NSView] = [
			"nhspinb":	nhspinb,
			"nvspinb":		nvspinb,
			"scrollbar":	scrollbar,
		]
		addConstraint(contentView, "V:|-[nhspinb]-[scrollbar]-|", views)
		addConstraint(contentView, "V:|-[nvspinb]-[scrollbar]-|", views)
		addConstraint(contentView, "H:|-[nhspinb]-[nvspinb]-|", views)
		addConstraint(contentView, "H:|-[scrollbar]-|", views)
	}
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
