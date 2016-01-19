#ifdef __WAND__
target[name[channelstrip.o] type[object]]
#endif

#include "channelstrip.h"
#include "channeldata.h"
#include "channel.h"
#include "framework/boxvertical.h"
#include "framework/textbox.h"
#include "framework/knob.h"
#include "framework/slider.h"
#include "framework/floatconv.h"
#include <cmath>

ChannelStrip* ChannelStrip::create(GuiContainer& parent)
	{return new ChannelStrip(parent);}

void ChannelStrip::destroy()
	{
	delete this;
	}

const GuiHandle& ChannelStrip::handleNativeGet() const
	{return m_box->handleNativeGet();}

ChannelStrip::ChannelStrip(GuiContainer& parent):m_input_handler(*this)
	{
	m_box=BoxVertical::create(parent,&m_input_handler);
	m_box->slaveAssign(*this);

	m_label=Textbox::create(*m_box,0);
	m_label->widthMinSet(5);
	m_fadetime=Knob::create(*m_box,m_input_handler,0,"Fade\ntime/s");

	m_box->insertModeSet(BoxVertical::INSERTMODE_EXPAND
		|BoxVertical::INSERTMODE_FILL
		|BoxVertical::INSERTMODE_END);
	m_level=Slider::create(*m_box,m_input_handler,1,"Gain\n/dB",1);

	}

ChannelStrip::~ChannelStrip()
	{
	m_level->destroy();
	m_fadetime->destroy();
	m_label->destroy();
	m_box->slaveRelease();
	}

double ChannelStrip::ValueInputHandler::valueMap(ValueInput& source,double x) const noexcept
	{
	switch(source.idGet())
		{
		case 0:
			return 1e-3*exp2(12.0*x);
		case 1:
			return 120.0*(x - 1.0) + 6.0*x;
		}
	return x;
	}

double ChannelStrip::ValueInputHandler::valueMapInverse(ValueInput& source,double y) const noexcept
	{
	switch(source.idGet())
		{
		case 0:
			return log2(y*1e3)/12.0;
		case 1:
			return (y+120.0)/126;;
		}
	return y;
	}

double ChannelStrip::ValueInputHandler::valueGet(ValueInput& source,const char* text)
	{
	auto value=convert(text);
	switch(source.idGet())
		{
		case 0:
			r_strip.fadeTimeSet(value);
			break;
		case 1:
			r_strip.gainSet(value);
			break;
		}
	return value;
	}

void ChannelStrip::ValueInputHandler::textGet(ValueInput& source,double value,TextBuffer& buffer)
	{
	switch(source.idGet())
		{
		case 0:
			r_strip.fadeTimeSet(value);
			sprintf(buffer.begin(),"%.3f",value);
			return;
		case 1:
			r_strip.gainSet(value);
			sprintf(buffer.begin(),"%.3f",value);
			return;
		}
	sprintf(buffer.begin(),"%.3f",value);
	}

void ChannelStrip::ValueInputHandler::onCommand(BoxVertical& source,unsigned int id)
	{
	switch(id)
		{
		case 0:
		//	r_strip.labelSet(r_strip.m_label.textGet());
			break;
		}
	}


void ChannelStrip::update()
	{
	m_label->textSet(r_channel->labelGet().begin());
	auto& channel=r_channel->channelGet();
	m_fadetime->valueSet(channel.fadeTimeGet());
	m_level->valueSet(channel.gainGet());
	}

void ChannelStrip::gainSet(double gain)
	{
	auto& channel=r_channel->channelGet();
	channel.gainSet(gain);
	}

void ChannelStrip::fadeTimeSet(double time)
	{
	auto& channel=r_channel->channelGet();
	channel.fadeTimeSet(time);
	}
