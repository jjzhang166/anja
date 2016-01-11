#ifdef __WAND__
target[name[waveformdataupdater.o] type[object]]
#endif

#include "waveformdataupdater.h"
#include "waveformdata.h"
#include "sessionview.h"

void WaveformDataUpdater::onSourceChange(WaveformDataView& source
	,const char* filename_new)
	{
	auto& wd=source.waveformDataGet();
	wd.fileLoad(filename_new);
	source.update();
	}

void WaveformDataUpdater::onDescriptionChange(WaveformDataView& source
	,const char* description_new)
	{
	auto& wd=source.waveformDataGet();
	wd.descriptionSet(description_new);
	source.update();
	r_view->keyboardViewUpdate();
	}

void WaveformDataUpdater::onColorChange(WaveformDataView& source
	,const ColorRGBA& color_new)
	{
	auto& wd=source.waveformDataGet();
	wd.keyColorSet(color_new);
	source.update();
	r_view->keyboardViewUpdate();
	}

void WaveformDataUpdater::onColorChange(WaveformDataView& source
	,const char* colorstr)
	{
	auto& wd=source.waveformDataGet();
	wd.keyColorSet(colorstr);
	source.update();
	r_view->keyboardViewUpdate();
	}

void WaveformDataUpdater::onOptionSet(WaveformDataView& source,uint32_t option)
	{
	auto& waveform=source.waveformDataGet().waveformGet();
	waveform.flagSet(option);
	source.update();
	}

void WaveformDataUpdater::onOptionUnset(WaveformDataView& source,uint32_t option)
	{
	auto& waveform=source.waveformDataGet().waveformGet();
	waveform.flagUnset(option);
	source.update();
	}
