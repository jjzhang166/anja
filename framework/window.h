#ifdef __WAND__
target[name[window.h] type[include]]
dependency[window.o]
#endif

#ifndef WINDOW_H
#define WINDOW_H

#include "keymask.h"
#include "eventloop.h"
#include "guicontainer.h"
#include <cstdint>

class GuiHandle;

class Window:public GuiContainer
	{
	public:
		class EventHandler
			{
			public:
				virtual void onMouseMove(int x,int y,keymask_t key_mask)
					{}

				virtual void onMouseDown(int x,int y,keymask_t key_mask)
					{}

				virtual void onMouseUp(int x,int y,keymask_t key_mask)
					{}

				virtual void onKeyDown(uint8_t scancode)
					{}

				virtual void onKeyUp(uint8_t scancode)
					{}

				virtual bool onClose()
					{return true;}
			};

		static Window* create(EventLoop& event_loop)
			{return create(event_loop,s_null_handler,nullptr);}

		static Window* create(EventLoop& event_loop,EventHandler& handler)
			{return create(event_loop,handler,nullptr);}

		static Window* create(Window& owner)
			{return create(owner.r_event_loop,s_null_handler,&owner);}

		static Window* create(Window& owner,EventHandler& handler)
			{return create(owner.r_event_loop,handler,&owner);}

		virtual void eventHandlerSet(EventHandler& handler)=0;
		virtual void titleSet(const char* title_new)=0;


	protected:
		Window(EventLoop& event_loop):r_event_loop(event_loop)
			{r_event_loop.windowRegister();}

		~Window()
			{r_event_loop.windowUnregister();}

	private:
		static Window* create(EventLoop& event_loop,EventHandler& handler,Window* owner);

		EventLoop& r_event_loop;
		static EventHandler s_null_handler;
	};

#endif
