#ifdef __WAND__
target[name[audioengineanja.o] type[object]]
#endif

#include "audioengineanja.h"
#include "wavetable.h"
#include "clock.h"
#include "units.h"
#include "playbackrange.h"
#include "filterstep.h"
#include "midiconstants/controlcodes.h"
#include "midiconstants/statuscodes.h"
#include "midiconstants/controlcodes.h"
#include "framework/minifloat.h"

#include <cstring>

AudioEngineAnja::AudioEngineAnja(Wavetable& waveforms):
	r_waveforms(&waveforms),m_sample_rate(0),m_event_queue(32)
	,m_voice_current(0)
	,m_source_buffers(32),r_source_buffers(waveforms.length())
	,m_fader_filter_factor(0)
	,m_buffer_temp(1),m_buffers_out(16)
	,m_master_gain_out(1.0),m_master_gain_in(1.0),m_multioutput(0)
	{
	reset();
	}

AudioEngineAnja::~AudioEngineAnja()
	{}

void AudioEngineAnja::onActivate(AudioConnection& source)
	{
	if(m_multioutput)
		{
		char port_name_buffer[8];
		for(int k=0;k<16;++k)
			{
			sprintf(port_name_buffer,"Ch %u",k);
			source.audioPortOutputAdd(port_name_buffer);
			}
		}

	source.audioPortOutputAdd("Master out")
		.midiPortInputAdd("MIDI in")
		.midiPortOutputAdd("MIDI out");

	m_sample_rate=source.sampleRateGet();
	m_fader_filter_factor=timeConstantToDecayFactor(1e-3,m_sample_rate);
	m_master_gain_out=m_master_gain_in;
	}

void AudioEngineAnja::onDeactivate(AudioConnection& source)
	{
	m_sample_rate=0.0;
	reset();
	}

void AudioEngineAnja::reset()
	{
	m_event_next={0,{MIDIConstants::StatusCodes::INVALID,0,0,0},0.0f};
	//	Drain event queue
	while(!m_event_queue.empty())
		{
		m_event_queue.pop_front();
		}

	//	Reset channel states
		{
		auto ptr_channel=m_channels.begin();
		auto ptr_channel_end=m_channels.end();
		while(ptr_channel!=ptr_channel_end)
			{
			*ptr_channel={1.0,1.0,1.0,1.0};
			++ptr_channel;
			}
		}

	//	Reset playback states
		{
		auto ptr=m_source_buffers.begin();
		auto ptr_end=m_source_buffers.end();
		while(ptr!=ptr_end)
			{
			if(ptr->valid())
				{ptr->release();}
			++ptr;
			}
		}
	}

void AudioEngineAnja::buffersAllocate(AudioConnection& source,unsigned int n_frames)
	{
	m_buffer_temp=ArraySimple<float>(n_frames);
	m_buffers_out=ArraySimple<float>(n_frames*m_channels.length());
	}


void AudioEngineAnja::eventPost(uint8_t status,uint8_t value_0,uint8_t value_1) noexcept
	{
	if(m_sample_rate > 0.0)
		{
		QueueElement tmp;
		tmp.event=
			{
			secondsToFrames(clockGet(),m_sample_rate)-m_time_start
			,{status,value_0,value_1,0}
			,0.0f
			};
		m_event_queue.push_back(tmp.vector);
		}
	}

void AudioEngineAnja::eventPost(uint8_t status,uint8_t value_0,float value_1) noexcept
	{
	if(m_sample_rate > 0.0)
		{
		QueueElement tmp;
		tmp.event=
			{
			secondsToFrames(clockGet(),m_sample_rate)-m_time_start
			,{status,value_0,0,Event::VALUE_1_FLOAT}
			,value_1
			};
		m_event_queue.push_back(tmp.vector);
		}
	}

void AudioEngineAnja::eventControlProcess(const AudioEngineAnja::Event& event) noexcept
	{
	switch(event.status_word[1])
		{
		case MIDIConstants::ControlCodes::CHANNEL_VOLUME:
			{
			auto value=event.status_word[3]==Event::VALUE_1_FLOAT?
				event.value : float(event.status_word[2])/127.0f;
			auto channel=event.status_word[0]&0xf;
			m_channels[channel].gain_in_current=value;
			}
			break;

		case MIDIConstants::ControlCodes::SOUND_1: //Fade out
			{
			auto value=event.status_word[3]==Event::VALUE_1_FLOAT?
				event.value : 4.096*Minifloat::fromBits(event.status_word[2])/15.5;
			auto channel=event.status_word[0]&0xf;
			m_channels[channel].fade_factor=timeConstantToDecayFactor(value,m_sample_rate);
			}
			break;

		case MIDIConstants::ControlCodes::SOUND_2: //Fade in
			{
			auto value=event.status_word[3]==Event::VALUE_1_FLOAT?
				event.value :  4.096*Minifloat::fromBits(event.status_word[2])/15.5;
			auto channel=event.status_word[0]&0xf;
			m_channels[channel].fade_factor=timeConstantToDecayFactor(-value,m_sample_rate);
			}
			break;
		}
	}

void AudioEngineAnja::eventProcess(const AudioEngineAnja::Event& event
	,unsigned int time_offset) noexcept
	{
	if(event.status_word[0]==MIDIConstants::StatusCodes::RESET)
		{
		auto buffer=m_source_buffers.begin();
		auto buffers_end=m_source_buffers.end();
		auto decay_factor=timeConstantToDecayFactor(1e-2,m_sample_rate);
		while(buffer!=buffers_end)
			{
			buffer->fadeOut(decay_factor);
			++buffer;
			}
		return;
		}

	switch(event.status_word[0]&0xf0)
		{
		case MIDIConstants::StatusCodes::NOTE_ON:
			{
			auto voice=m_voice_current;
			auto slot=event.status_word[1];

			auto& waveform=(*r_waveforms)[slot];

			m_source_buffers[voice].waveformSet(m_randgen,waveform,time_offset);
			r_source_buffers[slot]=voice;
			m_voice_current=(voice+1)%m_source_buffers.length();
			}
			break;

		case MIDIConstants::StatusCodes::NOTE_OFF:
			{
		//	NOTE: The replace lock cannot be released here, due to the possible
		//	sustain flag. Instead, it has to be done in audioProcess, when
		//	the rendering loop detects that the clip should not continue.
			auto slot=event.status_word[1];
			auto& playback_buffer=m_source_buffers[ r_source_buffers[slot] ];

		//	NOTE: When note is stopped in sustain mode, ensure that the LOOP
		//	flag is is disabled to prevent infinite sound
			if(playback_buffer.flagsGet()&Waveform::SUSTAIN)
				{playback_buffer.flagsUnset(Waveform::LOOP);}
			else
				{playback_buffer.stop(time_offset);}
			}
			break;

		case MIDIConstants::StatusCodes::CONTROLLER:
			eventControlProcess(event);
			break;
		}
	}

void AudioEngineAnja::eventsUIFetch(unsigned int n_frames) noexcept
	{
	Event event_next=m_event_next;
	auto& queue=m_event_queue;
	const auto now_in=secondsToFrames(clockGet(),m_sample_rate);
	auto now=now_in;
	while(n_frames!=0)
		{
		if(now>=event_next.delay) // If next event has passed
			{
			eventProcess(event_next,now-now_in);
		//	Set the status code to invalid so it does not do anything more
			event_next.status_word[0]=MIDIConstants::StatusCodes::INVALID;

		//	Check if there are any more events
			while(!queue.empty())
				{
				QueueElement tmp;
				tmp.vector=queue.pop_front();
				event_next=tmp.event;
				if(now>=event_next.delay) //Process next event now
					{
					eventProcess(event_next,now-now_in);
					event_next.status_word[0]=MIDIConstants::StatusCodes::INVALID;
					}
				else //	It is not time for current event yet. Exit the loop
					{break;}
				}
			}
		++now;
		--n_frames;
		}
	m_event_next=event_next;
	}

void AudioEngineAnja::voicesRender(unsigned int n_frames) noexcept
	{
	memset(m_buffers_out.begin(),0,m_buffers_out.length()*sizeof(float));
	auto src_begin=m_source_buffers.begin();
	auto src_current=src_begin;
	auto src_end=m_source_buffers.end();
	while(src_current!=src_end)
		{
		if(src_current->valid())
			{
			auto ptr_voice=m_buffer_temp.begin();
			auto N=src_current->outputBufferGenerate(ptr_voice,n_frames);
			auto channel_index=src_current->channelGet();
			auto ptr_buffer_out=m_buffers_out.begin()+n_frames*channel_index;
			auto ptr_buffer_in=ptr_voice;
			while(N!=0)
				{
			//	TODO: We can vectorize here
				*ptr_buffer_out+=*ptr_buffer_in;
				--N;
				++ptr_buffer_in;
				++ptr_buffer_out;
				}
			if(src_current->done())
				{
				src_current->release();
				m_voice_current=src_current-src_begin;
				}
			}
		++src_current;
		}
	}

void AudioEngineAnja::channelsMix(AudioConnection& source,unsigned int n_frames) noexcept
	{
	auto N_ch=m_channels.length();
	auto multioutput=m_multioutput;
	auto buffer_out=source.audioBufferOutputGet(N_ch*multioutput,n_frames);
	memset(buffer_out,0,n_frames*sizeof(float));
	auto N_ch_in=N_ch;
	auto ptr_in=m_buffers_out.begin();
	auto channel=m_channels.begin();
	auto filterfactor=m_fader_filter_factor;
	float dummy;
	while(N_ch!=0)
		{
	//	TODO: This will only capture the most recent channel event within
	//	the current block. Also the event offset is not considered. It works
	//	as long as the buffer is small, but fails for larger buffers.

		auto N=n_frames;

		auto ptr_out=buffer_out;
	//	HACK Use the address of a dummy variable so we can write to
	//	*ptr_out_raw in any case
		float* ptr_out_raw=&dummy;
		if(multioutput)
			{ptr_out_raw=source.audioBufferOutputGet(N_ch_in-N_ch,N);}

		auto gain_in_current=channel->gain_in_current;
		auto gain_in_old=channel->gain_in_old;
		auto gain_out=channel->gain_out;
		auto fade_factor=channel->fade_factor;
		while(N!=0)
			{
			auto x=(*ptr_in) * gain_in_old * gain_out;
			*ptr_out_raw=x;
			*ptr_out+=x;
			++ptr_in;
			++ptr_out;
			ptr_out_raw=(ptr_out_raw==&dummy)?ptr_out_raw:ptr_out_raw + 1;
			--N;
			gain_out=std::max(1e-4,std::min(1.0,fade_factor*gain_out));
			gain_in_old=filterstep(gain_in_current,gain_in_old,filterfactor);
			}
		channel->gain_out=gain_out;
		channel->gain_in_old=gain_in_old;
		++channel;
		--N_ch;
		}
	}


void AudioEngineAnja::masterGainAdjust(AudioConnection& source
	,unsigned int n_frames) noexcept
	{
	auto filterfactor=m_fader_filter_factor;
	auto buffer_out=source.audioBufferOutputGet(m_channels.length()*m_multioutput
		,n_frames);
	auto gain_in=m_master_gain_in;
	auto gain_out=m_master_gain_out;
	while(n_frames!=0)
		{
		*buffer_out *= gain_out;
		gain_out=filterstep(gain_in,gain_out,filterfactor);
		++buffer_out;
		--n_frames;
		}
	m_master_gain_out=gain_out;
	}

void AudioEngineAnja::voicesStop() noexcept
	{
	auto src_begin=m_source_buffers.begin();
	auto src_current=src_begin;
	auto src_end=m_source_buffers.end();
	while(src_current!=src_end)
		{
		if(src_current->valid())
			{
			auto ch=src_current->channelGet();
			if(m_channels[ch].gain_out<1e-3
				&& m_channels[ch].fade_factor<1.0)
				{src_current->stop();}
			}
		++src_current;
		}
	}

void AudioEngineAnja::audioProcess(AudioConnection& source,unsigned int n_frames) noexcept
	{
	eventsUIFetch(n_frames);
	voicesRender(n_frames);
	channelsMix(source,n_frames);
	masterGainAdjust(source,n_frames);
	voicesStop();
	}
