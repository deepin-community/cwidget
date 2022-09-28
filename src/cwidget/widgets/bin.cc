// bin.cc

#include "bin.h"

#include <cwidget/toplevel.h>

#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>

namespace cwidget
{
  namespace widgets
  {
    bin::bin()
      :passthrough(), subwidget(NULL)
    {
    }

    bin::~bin()
    {
      if(subwidget.valid())
	set_subwidget(NULL);
    }

    widget_ref bin::get_focus()
    {
      widget_ref tmpref(this);

      widget_ref w = subwidget;

      if(w.valid() && w->get_visible())
	return w;
      else
	return NULL;
    }

    void bin::set_subwidget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      if(subwidget.valid())
	{
	  subwidget->set_owner(NULL);
	  subwidget->unfocussed();
	  subwidget=NULL;
	  show_conn.disconnect();
	  hide_conn.disconnect();
	}

      subwidget = w;

      if(w.valid())
	{
	  show_conn = w->shown_sig.connect(sigc::bind(sigc::mem_fun(*this, &bin::show_widget_bare), w.weak_ref()));
	  hide_conn = w->hidden_sig.connect(sigc::bind(sigc::mem_fun(*this, &bin::hide_widget_bare), w.weak_ref()));
	  w->set_owner(this);
	  if(get_isfocussed())
	    w->focussed();
	}

      toplevel::queuelayout();
    }

    void bin::destroy()
    {
      widget_ref tmpref(this);

      if(subwidget.valid())
	subwidget->destroy();
      eassert(!subwidget.valid());

      container::destroy();
    }

    void bin::add_widget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      eassert(!subwidget.valid());
      eassert(w.valid());

      set_subwidget(w);

      // I assume that we're hidden right now.
      if(w->get_visible())
	show();

      if(get_isfocussed())
	w->focussed();
    }

    void bin::rem_widget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      eassert(w == subwidget);
      set_subwidget(NULL);

      if(get_visible())
	hide();

      if(get_isfocussed())
	w->unfocussed();
    }

    void bin::show_all()
    {
      widget_ref tmpref(this);

      if(subwidget.valid())
	subwidget->show_all();

      show();
    }

    void bin::show_widget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      eassert(w==subwidget);

      show();
    }

    void bin::show_widget_bare(widget &w)
    {
      widget_ref tmpref(this);

      show_widget(widget_ref(&w));
    }

    void bin::hide_widget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      eassert(w==subwidget);
      hide();
    }

    void bin::hide_widget_bare(widget &w)
    {
      widget_ref tmpref(this);

      hide_widget(widget_ref(&w));
    }

    void bin::paint(const style &st)
    {
      widget_ref tmpref(this);

      if(subwidget.valid() && subwidget->get_visible())
	subwidget->display(st);
    }
  }
}
