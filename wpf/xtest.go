// 25 november 2015
package main
import "fmt"
import "runtime"
import "syscall"
import "unsafe"
var libui = syscall.NewLazyDLL("libui.dll")
var uiInit = libui.NewProc("uiInit")
var uiNewWindow = libui.NewProc("uiNewWindow")
var uiWindowOnClosing = libui.NewProc("uiWindowOnClosing")
var uiNewButton = libui.NewProc("uiNewButton")
var uiWindowSetChild = libui.NewProc("uiWindowSetChild")
var uiWindowSetMargined = libui.NewProc("uiWindowSetMargined")
var uiNewVerticalBox = libui.NewProc("uiNewVerticalBox")
var uiBoxAppend = libui.NewProc("uiBoxAppend")
var uiBoxSetPadded = libui.NewProc("uiBoxSetPadded")
var uiControlShow = libui.NewProc("uiControlShow")
var uiMain = libui.NewProc("uiMain")
var uiQuit = libui.NewProc("uiQuit")
var uiUninit = libui.NewProc("uiUninit")
func onClosing(w uintptr, data uintptr) int32 {
	fmt.Println("in closing")
	uiQuit.Call()
	return 1
}
func main() {
	runtime.LockOSThread()
	b := make([]byte, 256)		// to compensate for uiInitOptions
	e, _, _ := uiInit.Call(uintptr(unsafe.Pointer(&b[0])))
	if e != 0 { panic(e) }
	ss := "Hello from Go"
	s := []byte(ss)
	w, _, _ := uiNewWindow.Call(
		uintptr(unsafe.Pointer(&s[0])),
		320, 240, 0)
	uiWindowOnClosing.Call(w, syscall.NewCallbackCDecl(onClosing), 0)
	box, _, _ := uiNewVerticalBox.Call()
	btn, _, _ := uiNewButton.Call(
		uintptr(unsafe.Pointer(&s[0])))
	uiBoxAppend.Call(box, btn, 0)
	btn, _, _ = uiNewButton.Call(
		uintptr(unsafe.Pointer(&s[0])))
	uiBoxAppend.Call(box, btn, 1)
	btn, _, _ = uiNewButton.Call(
		uintptr(unsafe.Pointer(&s[0])))
	uiBoxAppend.Call(box, btn, 0)
	btn, _, _ = uiNewButton.Call(
		uintptr(unsafe.Pointer(&s[0])))
	uiBoxAppend.Call(box, btn, 1)
	btn, _, _ = uiNewButton.Call(
		uintptr(unsafe.Pointer(&s[0])))
	uiBoxAppend.Call(box, btn, 0)
	uiWindowSetChild.Call(w, box)
	uiBoxSetPadded.Call(box, 1)
	uiWindowSetMargined.Call(w, 1)
	uiControlShow.Call(w)
	uiMain.Call(w)
	fmt.Println("after main")
	uiUninit.Call(w)
	fmt.Println("out")
}
