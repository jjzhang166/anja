#ifdef __WAND__
target[name[inputentry_test] type[application]]
#endif

#include "window.h"
#include "inputentry.h"
#include "boxvertical.h"

int main()
	{
	auto event_loop=EventLoop::create();
	auto mainwin=Window::create(*event_loop);
	auto box=BoxVertical::create(*mainwin);
	auto inputentry=InputEntry::create(*box,"Begin:","Auto");
	auto inputentry2=InputEntry::create(*box,"End:","Auto");

	return 0;
	}