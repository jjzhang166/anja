#ifdef __WAND__
target[name[channelview.h] type[include]]
dependency[channelview.o]
#endif

#ifndef CHANNELVIEW_H
#define CHANNELVIEW_H

#include "framework/widget.h"
#include "channelstrip.h"
#include "framework/array_simple.h"

class GuiContainer;
class BoxHorizontal;
class ChannelData;

class ChannelView:public Widget
	{
	public:
		static ChannelView* create(GuiContainer& parent
			,ChannelStrip::EventHandler& handler,const ChannelData* channels
			,unsigned int n_channels);

		void destroy();

		void channelDataSet(const ChannelData& data,unsigned int channel);

		void channelDataSet(const ChannelData* channels,unsigned int n_channels);

		const GuiHandle& handleNativeGet() const;

	private:
		ChannelView(GuiContainer& parent,ChannelStrip::EventHandler& handler
			,const ChannelData* channels,unsigned int n_channels);
		~ChannelView();
		BoxHorizontal* m_box;
		ArraySimple<ChannelStrip*> m_strips;
	};

#endif
