// 12 january 2020
#include <os/AppKit.h>
#include <os/InterfaceKit.h>
#include <os/KernelKit.h>
#include <os/SupportKit.h>
#define uiprivOSHeader "../ui_haiku.h"
#include "../common/uipriv.h"

// main.cpp
class uiprivApplication : public BApplication {
};
extern uiprivApplication *uiprivApp;
