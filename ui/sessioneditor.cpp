//@	 {"targets":[{"name":"sessioneditor.o","type":"object"}]}

#include "sessioneditor.hpp"
#include "../sessiondata/session.hpp"
#include "../sessiondata/keymap.hpp"

using namespace Anja;

void SessionEditor::nameChanged(ChannelStrip& strip,int id)
	{
	m_waveform.channelName(id,strip.name().begin());
	if(id>=0 && id<12)
		{m_keyboard.keyLabel(channelToScancode(id),strip.name().begin()).redraw();}
	if(r_cb_obj!=nullptr)
		{m_channel_name_callback(r_cb_obj,strip,id);}
	}

void SessionEditor::colorChanged(ChannelStrip& strip,int id)
	{
	if(id>=0 && id<12)
		{
		m_keyboard.keyColor(channelToScancode(id),strip.color()).redraw();
		}
	}


void SessionEditor::descriptionChanged(WaveformEditor& wf,WaveformEditId id)
	{
	auto slot=r_session.slotActiveGet();
	auto scancode=slotToScancode(slot);
	if(scancode!=0xff);
		{
		m_keyboard.keyLabel(scancode,r_session.waveformViewGet(slot).keyLabelGet().begin())
			.redraw();
		}
	}

void SessionEditor::colorChanged(WaveformEditor& wf,WaveformEditId id)
	{
	auto slot=r_session.slotActiveGet();
	auto scancode=slotToScancode(slot);
	if(scancode!=0xff)
		{
		m_keyboard.keyColor(scancode,r_session.waveformViewGet(slot).keyColorGet())
			.redraw();
		}
	}

void SessionEditor::colorPresetsChanged(ColorPicker& picker)
	{
	r_session.colorPresetsSet(picker.presetsBegin(),picker.presetsEnd());
	m_waveform.colorPresets(r_session.colorPresetsGet());
	m_mixer.colorPresets(r_session.colorPresetsGet());
	}

void SessionEditor::masterGainChanged(MixerConsole& mixer,MixerId id)
	{
	r_session.gainSet(mixer.masterGain());
	}

void SessionEditor::indexSelected(KeyboardView& keyboard,KeyboardViewId id)
	{
	auto scancode=keyboard.selection();
	auto index=scancodeToChannel(scancode);
	if(index!=-1)
		{
		m_tabs.activate(1);
		m_mixer.focus(index);
		return;
		}

	index=scancodeToSlot(scancode);
	if(index!=0xff)
		{
		m_tabs.activate(0);
		m_waveform.waveform(r_session.waveformViewGet(index));
		r_session.slotActiveSet(index);
		}

	auto slot=r_session.slotActiveGet();
	scancode=slotToScancode(slot);
	if(scancode!=0xff);
		{
		keyboard.selection(scancode).keyLabel(AUDITION_KEY
			,Anja::String("Audition: ").append(r_session.waveformViewGet(slot).keyLabelGet()).begin());
		}
	}

SessionEditor& SessionEditor::sessionUpdated()
	{
	m_keyboard.reset().keyLabel(AUDITION_KEY,"Audition");
		{
		auto N=std::min(r_session.channelsCountGet(),12);
		for(decltype(N) k=0;k<N;++k)
			{
			auto key=channelToScancode(k);
			auto ch=r_session.channelViewGet(k);
			m_keyboard.keyColor(key,ch.colorGet()).keyLabel(key,ch.labelGet().begin());
			}
		}

		{
		auto N=r_session.slotsCountGet();
		for(decltype(N) k=0;k<N;++k)
			{
			auto scancode=slotToScancode(k);
			if(scancode==0xff)
				{break;}
			auto wf=r_session.waveformViewGet(k);
			m_keyboard.keyColor(scancode,wf.keyColorGet());
			if(wf.keyLabelGet().length()!=0)
				{m_keyboard.keyLabel(scancode,wf.keyLabelGet().begin());}
			}
		}

		{
		auto slot=r_session.slotActiveGet();
		auto scancode=slotToScancode(slot);
		if(scancode!=0xff)
			{
			m_keyboard.selection(scancode).keyLabel(AUDITION_KEY
				,Anja::String("Audition: ").append(r_session.waveformViewGet(slot).keyLabelGet()).begin());
			}
		}
	m_keyboard.redraw();

	auto& color_presets=r_session.colorPresetsGet();
	auto channel_names=r_session.channelLabelsGet();

	m_waveform.colorPresets(color_presets)
		.channelNames(channel_names)
		.waveform(r_session.waveformViewGet(r_session.slotActiveGet()))
		.waveformUpdate();

	m_mixer.colorPresets(color_presets).channels(r_session);

	m_settings.sessionUpdated();
	return *this;
	}


SessionEditor::SessionEditor(Container& cnt,Session& session)
	:r_cb_obj(nullptr)
	,r_session(session)
	,m_hsplit(cnt,true)
		,m_keyboard(m_hsplit.insertMode({Anja::Paned::SHRINK_ALLOWED|Anja::Paned::RESIZE}))
		,m_tabs(m_hsplit)
			,m_waveform(m_tabs.tabTitle("Waveform")
				,session.waveformViewGet(session.slotActiveGet())
				,session.channelLabelsGet())
			,m_mixer(m_tabs.tabTitle("Channel Mixer"),session)
			,m_settings(m_tabs.tabTitle("Session"),session)
	{

	sessionUpdated();

	m_keyboard.callback(*this,KeyboardViewId::KEYBOARD_MAIN);
	m_waveform.colorPresets(session.colorPresetsGet())
		.callback(*this,WaveformEditId::WAVEFORM_CURRENT);
	m_mixer.colorPresets(session.colorPresetsGet());
	m_mixer.channelsCallback(*this)
		.callback(*this,MixerId::CHANNEL_MIXER);
	}
