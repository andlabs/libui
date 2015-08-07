// 31 july 2015
import Cocoa

var spaced = false
var firstvert = true

// keep alive
// apparently I'm not allowed to declare a variable and then assign to it first thing in a function
// it'd be great if people weren't so afraid of nil pointers
var keepAliveMainwin: Window? = nil

func appLaunched() {
	var mainwin = Window()
	mainwin.SetMargined(spaced)

	mainwin.Show()

	keepAliveMainwin = mainwin
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
	spaced = Process.arguments.count > 1

	var app = NSApplication.sharedApplication()
	app.setActivationPolicy(NSApplicationActivationPolicy.Regular)
	// NSApplication.delegate is weak; if we don't use the temporary variable, the delegate will die before it's used
	var delegate = appDelegate()
	app.delegate = delegate
	app.run()
}

main()
