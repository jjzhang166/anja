//@	{
//@  "targets":[{"name":"mixerconsole.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"mixerconsole.o","rel":"implementation"}]
//@	}

#ifndef ANJA_MIXERCONSOLE_HPP
#define ANJA_MIXERCONSOLE_HPP

#include "box.hpp"
#include "../common/arraysimple.hpp"
#include "channelstrip.hpp"
#include "../common/arraydynamicshort.hpp"

namespace Anja
	{
	class Session;

	class MixerConsole
		{
		public:
			MixerConsole(MixerConsole&&)=delete;
			MixerConsole& operator=(MixerConsole&&)=delete;

			explicit MixerConsole(Container& cnt,Session& session);


		private:
			Box m_strip_box;
				ArrayDynamicShort<Separator> m_separators;
				ArraySimple<ChannelStrip> m_strips;
				Box m_master;
					Label m_master_label;
					Slider m_master_slider;
					TextEntry m_master_entry;
		};
	}

#endif
