// size_box.cc
//
// Copyright 2004 Daniel Burrows

#include "size_box.h"

#include <sigc++/functors/mem_fun.h>

#include <utility>

using namespace std;

namespace cwidget
{
  namespace widgets
  {
    size_box::size_box(size s, const widget_ref &w) : min_size(s)
    {
      set_subwidget(w);
      set_opaque(false);

      do_layout.connect(sigc::mem_fun(*this, &size_box::layout_me));
    }

    int size_box::width_request()
    {
      widget_ref tmpref(this);

      widget_ref child = get_subwidget();

      if(child.valid())
	return max(child->width_request(), min_size.w);
      else
	return min_size.w;
    }

    int size_box::height_request(int w)
    {
      widget_ref tmpref(this);

      widget_ref child = get_subwidget();

      if(child.valid())
	return max(child->height_request(w), min_size.h);
      else
	return min_size.h;
    }

    void size_box::layout_me()
    {
      widget_ref tmpref(this);

      widget_ref child = get_subwidget();

      if(child.valid())
	{
	  if(child->get_visible())
	    child->alloc_size(0, 0, getmaxx(), getmaxy());
	  else
	    child->alloc_size(0, 0, 0, 0);
	}
    }
  }
}
