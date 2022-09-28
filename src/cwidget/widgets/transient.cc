// transient.cc
//
// Copyright 2005 Daniel Burrows

#include "transient.h"

#include <sigc++/functors/mem_fun.h>

namespace cwidget
{
  namespace widgets
  {
    transient::transient(const widget_ref &w)
    {
      set_subwidget(w);

      do_layout.connect(sigc::mem_fun(*this, &transient::layout_me));
    }

    void transient::layout_me()
    {
      widget_ref w=get_subwidget();

      if(w.valid())
	{
	  if(w->get_visible())
	    w->alloc_size(0, 0, getmaxx(), getmaxy());
	  else
	    w->alloc_size(0, 0, 0, 0);
	}
    }

    int transient::width_request()
    {
      widget_ref w=get_subwidget();

      if(w.valid())
	return w->width_request();
      else
	return 0;
    }

    int transient::height_request(int width)
    {
      widget_ref w=get_subwidget();

      if(w.valid())
	return w->height_request(width);
      else
	return 0;
    }

    bool transient::focus_me()
    {
      return true;
    }

    bool transient::handle_char(chtype ch)
    {
      destroy();
      return true;
    }
  }
}
