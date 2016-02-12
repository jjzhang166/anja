#ifdef __WAND__
target[name[waveformdata.o] type[object]]
#endif

#include "waveformdata.h"
#include "sessionfilerecord.h"
#include "colorstring.h"
#include "optionstring.h"
#include "framework/pathutils.h"
#include "framework/floatconv.h"
#include "framework/localeguard.h"

#include <cstdlib>
#include <cstring>

WaveformData::WaveformData(const SessionFileRecord& record
	,const ArrayDynamicShort<char>& load_path
	,Waveform& waveform
	,KeyboardLayout::KeyDescriptor* key):m_filename(""),m_description("")
	,m_key_label(""),m_color(0.25f,0.0f,.5f,1.0f)
	,r_key(key),r_waveform(&waveform),m_stateflags(0)
	{
	r_waveform->valuesInit();

	auto value=record.propertyGet("Filename");
	if(value!=nullptr)
		{fileLoad(*value,load_path);}

	value=record.propertyGet("Description");
	if(value!=nullptr)
		{descriptionSet(*value);}

	value=record.propertyGet("Color");
	if(value!=nullptr)
		{keyColorSet(colorFromString(value->begin()));}

	value=record.propertyGet("Playback gain/dB");
	if(value!=nullptr)
		{r_waveform->gainSet(convert(value->begin()));}

	value=record.propertyGet("Playback gain random level/dB");
	if(value!=nullptr)
		{r_waveform->gainRandomSet(convert(value->begin()));}

	value=record.propertyGet("Options");
	if(value!=nullptr)
		{
		r_waveform->flagsSet(optionsFromString(value->begin(),Waveform::FLAG_NAMES));
		}

	value=record.propertyGet("Playback channel");
	if(value!=nullptr)
		{
		long ch;
			{
			LocaleGuard locale("C");
			ch=atol(value->begin());
			}
		if(ch<1 || ch>16)
			{
			throw "A sound effect must be mapped to a channel number between 1 to 16 inclusive";
			}
		r_waveform->channelSet(ch-1);
		}

	value=record.propertyGet("Playback starting position/frames");
	if(value!=nullptr)
		{
		LocaleGuard locale("C");
		auto pos=atol(value->begin());
		r_waveform->offsetBeginSet(pos);
		}

	value=record.propertyGet("Playback end position/frames");
	if(value!=nullptr)
		{
		LocaleGuard locale("C");
		auto pos=atol(value->begin());
		r_waveform->offsetEndSet(pos);
		}
//	TODO Store other data not interpreted by Anja

	dirtyClear();
	}

void WaveformData::dataGet(SessionFileRecord& record) const
	{
	record.clear();
	record.propertySet("Filename",m_filename);
	record.propertySet("Description",m_description);
	record.propertySet("Color",ColorString(m_color).begin());
	char buffer[32];
	sprintf(buffer,"%u",r_waveform->channelGet() + 1);
	record.propertySet("Playback channel",buffer);
	sprintf(buffer,"%.7g",r_waveform->gainGet());
	record.propertySet("Playback gain/dB",buffer);
	sprintf(buffer,"%.7g",r_waveform->gainRandomGet());
	record.propertySet("Playback gain random level/dB",buffer);
	sprintf(buffer,"%u",r_waveform->offsetBeginGet());
	record.propertySet("Playback starting position/frames",buffer);
	sprintf(buffer,"%u",r_waveform->offsetEndGet());
	record.propertySet("Playback end position/frames",buffer);
	record.propertySet("Options",stringFromOptions(r_waveform->flagsGet()
		,Waveform::FLAG_NAMES));
//	TODO Save other data not interpreted by Anja
	}

WaveformData::WaveformData():m_filename(""),m_description("")
	,m_color{0.0f,0.0f,0.0f,1},r_key(nullptr),r_waveform(nullptr),m_stateflags(0)
	{}

void WaveformData::fileLoad(const char* filename)
	{
//	Do not try to load the file if it is the same file
	if(strcmp(m_filename.begin(),filename)==0 && *filename!='\0')
		{return;}

	r_waveform->fileLoad(filename);

	m_filename=filename;
	m_stateflags|=DIRTY;
	}

void WaveformData::fileLoad(const ArrayDynamicShort<char>& filename
	,const ArrayDynamicShort<char>& load_path)
	{
	if(absoluteIs(filename))
		{
		fileLoad(filename.begin());
		}
	else
	if(*(filename.begin())!='\0')
		{
		auto fullpath=load_path;
		fullpath.truncate().append(filename).append('\0');
		fileLoad(fullpath.begin());
		}
	}

void WaveformData::descriptionSet(const char* description)
	{
	if(strcmp(m_description.begin(),description)==0)
		{return;}
	m_description=description;
	m_key_label.clear();
	auto state=0;
	auto ptr=description;
	while(*ptr!='\0' && *ptr!=']')
		{
		switch(*ptr)
			{
			case '[':
				state=1;
				break;
			default:
				switch(state)
					{
					case 0:
						break;
					case 1:
						m_key_label.append(*ptr);
						break;
					}
			}
		++ptr;
		}

	if(m_key_label.length()==0)
		{
		auto ptr=description;
		while(*ptr!=' ' && *ptr!='\0')
			{
			m_key_label.append(*ptr);
			++ptr;
			}
		}
	m_key_label.append('\0');

	if(r_key!=nullptr)
		{r_key->labelSet(m_key_label.begin());}

	m_stateflags|=DIRTY;
	}

void WaveformData::descriptionSet(const ArrayDynamicShort<char>& description)
	{
	descriptionSet(description.begin());
	}


void WaveformData::keyColorSet(const ColorRGBA& color)
	{
	if(r_key!=nullptr)
		{r_key->colorBackgroundSet(color);}
	m_color=color;
	m_stateflags|=DIRTY;
	}

void WaveformData::init(Waveform& storage,KeyboardLayout::KeyDescriptor* key)
	{
	r_waveform=&storage;
	r_key=key;
	keyColorSet(COLORS[ColorID::BLACK]);
	descriptionSet("");
	fileLoad("");
	dirtyClear();
	}
