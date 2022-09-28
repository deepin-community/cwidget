// center.cc

#include "center.h"

#include <sigc++/functors/mem_fun.h>

namespace cwidget
{
  namespace widgets
  {
    center::center(const widget_ref &w)
    {
      set_subwidget(w);
      set_opaque(false);

      do_layout.connect(sigc::mem_fun(*this, &center::layout_me));
    }

    int center::width_request()
    {
      widget_ref tmpref(this);

      widget_ref subwidget = get_subwidget();

      if(subwidget.valid() && subwidget->get_visible())
	return subwidget->width_request();
      else
	return 0;
    }

    int center::height_request(int width)
    {
      widget_ref tmpref(this);

      widget_ref subwidget = get_subwidget();

      if(subwidget.valid() && subwidget->get_visible())
	return subwidget->height_request(width);
      else
	return 0;
    }

    void center::layout_me()
    {
      widget_ref tmpref(this);

      widget_ref child=get_subwidget();

      if(child.valid())
	{
	  if(child->get_visible())
	    {
	      int child_w=child->width_request();
	      if(child_w>getmaxx())
		child_w=getmaxx();

	      int child_h=child->height_request(child_w);
	      if(child_h>getmaxy())
		child_h=getmaxy();
	      child->alloc_size((getmaxx()-child_w)/2, (getmaxy()-child_h)/2, child_w, child_h);
	    }
	  else
	    child->alloc_size(0, 0, 0, 0);
	}
    }
  }
}
