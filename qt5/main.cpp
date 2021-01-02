
#include "uipriv_qt5.hpp"

#include <QApplication>
#include <QTimer>
#include <QWidget>
#include <QThread>

#include <functional>

const char *styleSheet = R"qcss(
QGroupBox {
	font: bold;
	color: #333;
	/*border: none;*/
}
)qcss";

const char *uiInit(uiInitOptions *o)
{
	Q_UNUSED(o); // don't care about something that isn't used

	if (QCoreApplication::instance() != nullptr) {
		return "another QApplication instance already exists";
	}

	static int argc = 0;
	static const char *argv[] = {"libui"};
	auto app = new QApplication(argc,(char **)argv);

	if (app == nullptr) {
		return "failed to allocate new QApplication";
	}

	qSetMessagePattern("%{file}:%{line} %{function}(): %{message}");

	// few consistency things
	app->setQuitOnLastWindowClosed(false);
	app->setStyleSheet(styleSheet);

	initAlloc();

	return NULL;
}

void uiUninit(void)
{
	auto app = static_cast<QApplication*>(QCoreApplication::instance());

	// not strictly necessary, just so we don't get needless nag from uninitAlloc()
	// becasue of cascading it's not possible to iterate over the list (copy)
	while (app->topLevelWidgets().count()) {
		delete app->topLevelWidgets().first();
	}

	uninitAlloc();

	delete app;
}

void uiFreeInitError(const char *err)
{
	Q_UNUSED(err); // we only return static constants for errors
}

void uiMain(void)
{
	if (QApplication::instance()) {
		QApplication::instance()->exec();
	}
}

int uiMainStep(int wait)
{
	if (QApplication::instance()) {
		if (wait) {
			// processEvents will always return immediately if there are no
			// immediate events to process

			// we would have to install a event listener and run in while loop
			// that seems very questionable.. don't understand the use case
			qWarning("TODO: wait - no equivalent?");
		}
		QApplication::instance()->processEvents();
	}
	return 0;
}

void uiQuit(void)
{
	QTimer::singleShot(0,QApplication::instance(), &QApplication::quit);
}

void uiQueueMain(void (*f)(void *data), void *data)
{
	if (!QCoreApplication::instance()) {
		// no instance...?
		return;
	}

	if (QThread::currentThread()->eventDispatcher()) {
		// the simple way to queue something to run on main thread
		QTimer::singleShot(0,QCoreApplication::instance(), [f,data]{
			f(data);
		});
	} else {
		// very dirty workaround, spawn a new thread.. with event dispatcher to deliver this

		// this could be done with custom signal, but since I decided to do
		// this without using moc this is the simplest solution I could think of.

		// sort of works, but not very light weight and will block to ensure resource cleanup
		class Thread : public QThread
		{
			std::function<void ()> taskForMain_;

			void run()
			{
				QTimer::singleShot(0,QCoreApplication::instance(), taskForMain_);
				exec(); // start event loop
			}
		public:
			Thread(std::function<void ()> taskForMain)
				: taskForMain_(taskForMain)
			{
				start();
			}
			~Thread()
			{
				quit();
				wait();
			}
		};
		Thread([f,data]{
			f(data);
		});
	}
}
