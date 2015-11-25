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
	s := "Hello from Go"
	w, _, _ := uiNewWindow.Call(
		uintptr(unsafe.Pointer(&s)),
		320, 240, 0)
	uiWindowOnClosing.Call(w, syscall.NewCallbackCDecl(onClosing), 0)
	uiControlShow.Call(w)
	uiMain.Call(w)
	fmt.Println("after main")
	uiUninit.Call(w)
	fmt.Println("out")
}
