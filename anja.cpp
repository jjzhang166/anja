//@	{"targets":[{"name":"anja","type":"application"}]}

#include "ui/uicontext.hpp"
#include "ui/window.hpp"
#include "ui/sessioneditor.hpp"
#include "sessiondata/session.hpp"
#include <cstdio>
#include <time.h>

namespace Anja
	{
	class SessionControl
		{
		public:
			SessionControl(UiContext& ctx):r_ctx(ctx)
				{}

			void closing(Window& win,int id)
				{r_ctx.exit();}

			UiContext::RunStatus idle(UiContext& ctx)
				{
				return UiContext::RunStatus::WAIT;
				}

			void keyDown(Anja::Window& win,int scancode,Anja::keymask_t keymask,int id)
				{
				printf("%d down\n",scancode);
				}

			void keyUp(Anja::Window& win,int scancode,Anja::keymask_t keymask,int id)
				{
				printf("%d up\n",scancode);
				}

		private:
			UiContext& r_ctx;
		};
	}

int main(int argc, char **argv)
	{
	try
		{
		Anja::UiContext ctx;
		ctx.dark(1);
		Anja::Session session;
		session.load("testbank/testbank.txt");
		Anja::Window mainwin(session.filenameGet().begin());
		Anja::SessionEditor editor(mainwin,session);
		Anja::SessionControl ctrl(ctx);
		mainwin.callback(ctrl,0);
		mainwin.show();
		ctx.run(ctrl);
		}
	catch(const Anja::Error& err)
		{
		fprintf(stderr,"Error: %s\n",err.message());
		return -1;
		}
	return 0;
	}
