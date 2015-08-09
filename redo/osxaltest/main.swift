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

	var box = Box(vertical: firstvert, padded: spaced)
	mainwin.SetControl(box)

/*
	var hbox = {(entrys: Bool, buttons: Bool) -> Box in
		var hbox = Box(vertical: !firstvert, padded: spaced)
		hbox.Add(Entry(), entrys)
		hbox.Add(Button("Button"), buttons)
		return hbox
	}

	box.Add(hbox(true, true), false)
	box.Add(hbox(true, false), false)
	box.Add(hbox(false, true), false)
	box.Add(hbox(false, false), false)
*/

	box.Add(Entry(), false)

	var mkhbox = {() -> Box in
		Box(vertical: !firstvert, padded: spaced)
	}

	var hbox = mkhbox()
	hbox.Add(Button("Button"), true)
	hbox.Add(Button("Button"), true)
	box.Add(hbox, false)

	hbox = mkhbox()
	hbox.Add(Button("Button"), true)
	hbox.Add(Button("Button"), true)
	box.Add(hbox, false)

	hbox = mkhbox()
	hbox.Add(Button("Button"), true)
	hbox.Add(Button("A"), false)
	hbox.Add(Button("BB"), false)
	hbox.Add(Button("CCC"), false)
	box.Add(hbox, false)

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
	for arg in dropFirst(Process.arguments) {
		if arg == "spaced" {
			spaced = true
		} else if arg == "horizontal" {
			firstvert = false
		} else {
			fatalError("unrecognized option \(arg)")
		}
	}

	var app = NSApplication.sharedApplication()
	app.setActivationPolicy(NSApplicationActivationPolicy.Regular)
	// NSApplication.delegate is weak; if we don't use the temporary variable, the delegate will die before it's used
	var delegate = appDelegate()
	app.delegate = delegate
	app.run()
}

main()
