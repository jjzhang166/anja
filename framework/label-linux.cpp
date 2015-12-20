#ifdef __WAND__
target
	[
	name[label.o]
	type[object]
	platform[;GNU/Linux]
	dependency[gtk-3;external]
	dependency[gdk-3;external]
	dependency[pangocairo-1.0;external]
	dependency[pango-1.0;external]
	dependency[atk-1.0;external]
	dependency[cairo-gobject;external]
	dependency[cairo;external]
	dependency[gdk_pixbuf-2.0;external]
	dependency[gio-2.0;external]
	dependency[gobject-2.0;external]
	dependency[glib-2.0;external]
	]
#endif

#include "label.h"
#include "guicontainer.h"
#include "guihandle.h"

class LabelGtk:public Label
	{
	public:
		LabelGtk(GuiContainer& parent,const char* title);
		void destroy()
			{r_parent.componentRemove(*this);}

		const GuiHandle& handleNativeGet() const
			{return m_label;}

	private:
		static void onDestroy(GtkWidget* widget,void* labelgtk);
		GuiContainer& r_parent;
		GuiHandle m_label;
	};

void LabelGtk::onDestroy(GtkWidget* widget,void* labelgtk)
	{
	LabelGtk* _this=(LabelGtk*)labelgtk;
	delete _this;
	}

Label* Label::create(GuiContainer& parent,const char* title)
	{return new LabelGtk(parent,title);}

LabelGtk::LabelGtk(GuiContainer& parent,const char* title):r_parent(parent)
	{
	GtkWidget* widget=gtk_label_new(title);
	g_signal_connect(widget,"destroy",G_CALLBACK(onDestroy),this);
	m_label=widget;
	parent.componentAdd(*this);
	}
