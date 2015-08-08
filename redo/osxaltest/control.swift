// 31 july 2015
import Cocoa

protocol Control : class {
	func View() -> NSView
	func SetParent(p: Control)
}
