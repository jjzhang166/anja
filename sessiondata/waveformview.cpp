//@	{
//@	 "targets":[{"name":"waveformview.o","type":"object"}]
//@	}

#include "waveformview.hpp"
#include "sessionfilerecord.hpp"
#include "../common/pathutils.hpp"
#include <time.h>

using namespace Anja;

namespace
	{
	class IdGenerator
		{
		public:
			IdGenerator() noexcept:m_value(time(NULL)){}

			uint32_t next() noexcept
				{
				++m_value;
				return m_value;
				}

		private:
			uint32_t m_value;
		};
	}

static IdGenerator s_id_gen;

static String filenameGet(const String& filename,const String& load_path)
	{
	if(absoluteIs(filename))
		{return filename;}
	auto fullpath=load_path;
	fullpath.append(filename);
	return fullpath;
	}

static String filenameGenerate(const String& label)
	{
	String ret(label);
	if(ret.length()!=0)
		{ret.append('-');}
	char buff[9];
	sprintf(buff,"%x",s_id_gen.next());
	ret.append(buff).append(".wav");
	return ret;
	}

void WaveformView::load(const SessionFileRecord& rec)
	{
	r_waveform_data.dataSet(rec);
	if(r_waveform_data.filenameGet().length()==0)
		{r_waveform.dataSet(rec);}
	else
		{
		auto f=::filenameGet(r_waveform_data.filenameGet(),r_dir_current);
		r_waveform.dataSet(rec,f.begin());
		}
	}

void WaveformView::store(SessionFileRecord& rec)
	{
	if(r_waveform.flagsGet()&Waveform::RECORDED)
		{
		auto filename=::filenameGet(filenameGenerate(r_waveform_data.keyLabelGet()),r_dir_current);
		r_waveform_data.filenameSet(makeRelativeTo(filename.begin(),r_dir_current.begin()));
		r_waveform.fileSave(filename.begin());
		}
	
	auto filename_out=r_waveform_data.filenameGet();
	if(*filename_out.begin()!='\0')
		{filename_out=makeRelativeTo(filename_out.begin(),r_dir_current.begin());}
	rec.propertySet(String("Filename"),filename_out);
	r_waveform.dataGet(rec);
	r_waveform_data.dataGet(rec);	
	}


void WaveformView::fileLoad(const char* filename)
	{
	if(absoluteIs(filename))
		{
		r_waveform.fileLoad(filename);
		r_waveform_data.filenameSet(makeRelativeTo(filename,r_dir_current.begin()));
		}
	else
		{
		auto fullpath=String(r_dir_current);
		fullpath.append('/').append(filename);
		r_waveform.fileLoad(fullpath.begin());
		r_waveform_data.filenameSet(String(filename));
		}
	}

void WaveformView::fileSave(const char* filename) const
	{
	if(absoluteIs(filename))
		{
		r_waveform.fileSave(filename);
		r_waveform_data.filenameSet(makeRelativeTo(filename,r_dir_current.begin()));
		}
	else
		{
		auto fullpath=String(r_dir_current);
		fullpath.append('/').append(filename);
		r_waveform.fileSave(fullpath.begin());
		r_waveform_data.filenameSet(String(filename));
		}
	}

bool WaveformView::loadPossible(const char* filename) const
	{
	if(absoluteIs(filename))
		{return r_waveform.loadPossible(filename);}

	return r_waveform.loadPossible(filename);
	}

bool WaveformView::fileLoaded(const char* filename) const
	{
	if(absoluteIs(filename))
		{
		auto cmp=makeRelativeTo(filename,r_dir_current.begin());
		return cmp==r_waveform_data.filenameGet();
		}
	return r_waveform_data.filenameGet()==filename;
	}