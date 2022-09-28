// passthrough.cc

#include "passthrough.h"

#include <sigc++/functors/mem_fun.h>

namespace cwidget
{
  namespace widgets
  {
    passthrough::passthrough()
    {
      focussed.connect(sigc::mem_fun(*this, &passthrough::gained_focus));
      unfocussed.connect(sigc::mem_fun(*this, &passthrough::lost_focus));
    }

    widget_ref passthrough::get_active_widget()
    {
      return get_focus();
    }

    void passthrough::defocus()
    {
      lost_focus();
    }

    void passthrough::refocus()
    {
      gained_focus();
    }

    void passthrough::gained_focus()
    {
      widget_ref tmpref(this);

      widget_ref w = get_focus();

      if(w.valid())
	w->focussed();
    }

    void passthrough::lost_focus()
    {
      widget_ref tmpref(this);

      widget_ref w = get_focus();

      if(w.valid())
	w->unfocussed();
    }

    bool passthrough::focus_me()
    {
      widget_ref tmpref(this);

      widget_ref w = get_focus();

      if(w.valid() && w->focus_me())
	return true;
      else
	return container::focus_me();
    }

    bool passthrough::get_cursorvisible()
    {
      widget_ref tmpref(this);

      widget_ref w = get_focus();

      return w.valid() && w->get_cursorvisible();
    }

    point passthrough::get_cursorloc()
    {
      widget_ref tmpref(this);

      widget_ref w = get_focus();

      if(w.valid())
	{
	  point p=w->get_cursorloc();
	  p.x+=w->get_startx();
	  p.y+=w->get_starty();

	  return p;
	}
      else
	return point(0, 0);
    }

    bool passthrough::handle_key(const config::key &k)
    {
      widget_ref tmpref(this);

      widget_ref w = get_focus();

      if(w.valid() && w->get_visible() && w->focus_me())
	return w->dispatch_key(k) || container::handle_key(k);
      else
	return container::handle_key(k);
    }

    void passthrough::dispatch_mouse(short id, int x, int y, int z,
					mmask_t bstate)
    {
      widget_ref tmpref(this);

      widget_ref w = get_focus();

      if(w.valid() && w->get_visible())
	w->dispatch_mouse(id, x-w->get_startx(), y-w->get_starty(), z, bstate);
    }
  }
}
