// 12 january 2020
#include <os/AppKit.h>
#include <os/InterfaceKit.h>
#include <os/KernelKit.h>
#include <os/SupportKit.h>
#define uiprivOSHeader "../ui_haiku.h"
#include "../common/uipriv.h"

// B_PRId32 is the correct format for status_t; see, for instance, https://review.haiku-os.org/c/haiku/+/2171/3/src/system/boot/platform/u-boot/start.cpp
#define uiprivStatustFmt B_PRId32

constexpr uint32 uiprivMsgQueueMain = 'uiQM';

// main.cpp
class uiprivApplication : public BApplication {
public:
	using BApplication::BApplication;
	virtual void MessageReceived(BMessage *msg) override;
};
extern uiprivApplication *uiprivApp;
