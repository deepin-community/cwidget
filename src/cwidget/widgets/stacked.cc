// stacked.cc

#include <cwidget/toplevel.h>
#include "stacked.h"

#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>

namespace cwidget
{
  namespace widgets
  {
    stacked::stacked(int w, int h)
      :req_w(w), req_h(h)
    {
      do_layout.connect(sigc::mem_fun(*this, &stacked::layout_me));
    }

    stacked::~stacked()
    {
      eassert(children.empty());
    }

    void stacked::destroy()
    {
      widget_ref tmpref(this);

      while(!children.empty())
	children.front().w->destroy();

      passthrough::destroy();
    }

    void stacked::add_widget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      sigc::connection shown_conn=w->shown_sig.connect(sigc::bind(sigc::mem_fun(*this, &stacked::raise_widget_bare), w.weak_ref()));
      sigc::connection hidden_conn=w->hidden_sig.connect(sigc::mem_fun(*this, &stacked::hide_widget));

      defocus();

      children.push_back(child_info(w, shown_conn, hidden_conn));

      w->set_owner(this);

      refocus();

      if(w->get_visible())
	toplevel::update();
    }

    void stacked::hide_widget()
    {
      toplevel::update();
    }

    void stacked::rem_widget(const widget_ref &wBare)
    {
      widget_ref tmpref(this);

      widget_ref w(wBare);

      for(childlist::iterator i=children.begin();
	  i!=children.end();
	  i++)
	{
	  if(i->w==w)
	    {
	      i->shown_conn.disconnect();
	      i->hidden_conn.disconnect();

	      children.erase(i);
	      w->set_owner(NULL);
	      if(w->get_visible())
		toplevel::update();

	      w->unfocussed();
	      refocus();

	      return;
	    }
	}
    }

    void stacked::raise_widget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      for(childlist::iterator i=children.begin();
	  i!=children.end();
	  i++)
	if(i->w==w)
	  {
	    defocus();

	    children.push_front(*i);
	    children.erase(i);

	    refocus();

	    toplevel::update();
	    return;
	  }
    }

    void stacked::lower_widget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      for(childlist::iterator i=children.begin();
	  i!=children.end();
	  i++)
	if(i->w==w)
	  {
	    defocus();

	    children.push_back(*i);
	    children.erase(i);

	    refocus();

	    toplevel::update();
	    return;
	  }
    }

    void stacked::paint(const style &st)
    {
      widget_ref tmpref(this);

      // Go through the children back-to-front (reverse order)
      for(childlist::reverse_iterator i=children.rbegin();
	  i!=children.rend();
	  i++)
	if(i->w->get_visible())
	  i->w->display(st);
    }

    void stacked::dispatch_mouse(short id, int x, int y, int z, mmask_t bstate)
    {
      widget_ref tmpref(this);

      for(childlist::iterator i=children.begin();
	  i!=children.end();
	  ++i)
	if(i->w->get_visible() && i->w->enclose(y, x))
	  {
	    i->w->dispatch_mouse(id, x-i->w->get_startx(), y-i->w->get_starty(),
				 z, bstate);
	    return;
	  }
    }

    widget_ref stacked::get_focus()
    {
      widget_ref tmpref(this);

      for(childlist::iterator i=children.begin();
	  i!=children.end();
	  i++)
	if(i->w->get_visible() && i->w->focus_me())
	  return i->w;
	else
	  return NULL;

      return NULL;
    }

    void stacked::show_all()
    {
      widget_ref tmpref(this);

      defocus();

      for(childlist::iterator i=children.begin();
	  i!=children.end();
	  i++)
	{
	  i->shown_conn.disconnect();

	  i->w->show_all();

	  i->shown_conn=i->w->shown_sig.connect(sigc::bind(sigc::mem_fun(*this, &stacked::raise_widget_bare), i->w.weak_ref()));
	}

      refocus();
    }

    int stacked::width_request()
    {
      return req_w;
    }

    int stacked::height_request(int w)
    {
      return req_h;
    }

    void stacked::layout_me()
    {
      widget_ref tmpref(this);

      for(childlist::iterator i=children.begin(); i!=children.end(); i++)
	i->w->alloc_size(0, 0, getmaxx(), getmaxy());
    }
  }
}
