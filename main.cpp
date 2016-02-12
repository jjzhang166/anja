#ifdef __WAND__
target[name[anja] type[application]]
#endif

#include "session.h"
#include "sessionview.h"
#include "keyboardcontroller.h"
#include "channelcontroller.h"
#include "waveformrangetrimmer.h"
#include "waveformdataupdater.h"
#include "sessiondataupdater.h"
#include "sessioncontrol.h"
#include "mainwinhandler.h"
#include "framework/window.h"

int main(int argc,char* argv[])
	{
	try
		{
		Session session;
		if(argc>1)
			{
			try
				{session.load(argv[1]);}
			catch(...)
				{}
			}
		WaveformRangeTrimmer trimmer;
		KeyboardController keyboardevents(session);
		WaveformDataUpdater waveform_updater;
		ChannelController channelctrl(session);
		SessionDataUpdater session_updater;
		MainwinHandler mainwin_handler;

		auto event_loop=EventLoop::create(0);
		auto mainwin=Window::create(*event_loop,mainwin_handler);
		SessionControl session_control(*mainwin);

		auto view=SessionView::create(*mainwin,session,session_control
			,keyboardevents,waveform_updater,trimmer,channelctrl
			,session_updater);

		mainwin_handler.sessionActionsSet(view->actionsGet());
		keyboardevents.sessionViewSet(view);
		waveform_updater.sessionViewSet(view);
		channelctrl.sessionViewSet(view);
		session_updater.sessionViewSet(view);
		session_control.sessionViewSet(view);
		}
	catch(const char* msg)
		{printf("Error: %s\n",msg);}
	return 0;
	}
