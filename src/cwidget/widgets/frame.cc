// frame.cc

#include "frame.h"

#include <cwidget/config/colors.h>

#include <sigc++/functors/mem_fun.h>

namespace cwidget
{
  namespace widgets
  {
    frame::frame(const widget_ref &w)
      :bin()
    {
      set_subwidget(w);

      do_layout.connect(sigc::mem_fun(*this, &frame::layout_me));
    }

    int frame::width_request()
    {
      widget_ref tmpref(this);

      widget_ref subwidget = get_subwidget();

      if(subwidget.valid() && subwidget->get_visible())
	return subwidget->width_request()+2;
      else
	return 2;
    }

    int frame::height_request(int width)
    {
      widget_ref tmpref(this);

      if(width<2)
	return 0;
      else
	{
	  widget_ref subwidget = get_subwidget();

	  if(subwidget.valid() && subwidget->get_visible())
	    return subwidget->height_request(width-2)+2;
	  else
	    return 2;
	}
    }

    void frame::layout_me()
    {
      widget_ref tmpref(this);

      widget_ref subwidget = get_subwidget();

      if(subwidget.valid())
	{
	  if(subwidget->get_visible())
	    subwidget->alloc_size(1, 1, getmaxx()-2, getmaxy()-2);
	  else
	    subwidget->alloc_size(0, 0, 0, 0);
	}
    }

    void frame::paint(const style &st)
    {
      widget_ref tmpref(this);

      border(0,0,0,0,0,0,0,0);

      widget_ref subwidget = get_subwidget();

      if(subwidget.valid() && subwidget->get_visible())
	subwidget->display(st);
    }
  }
}
