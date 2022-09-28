// stacked.h        -*-c++-*-
//
//  Manages a set of overlapping widgets, displaying them in a consistent
// order (it is possible to change the stacking order)
//
//  The size of the widget is unrelated to the sizes of its components.
//  (why? why not size it in a more flexible way?)

#ifndef STACKED_H
#define STACKED_H

#include "passthrough.h"

#include <sigc++/connection.h>

namespace cwidget
{
  namespace widgets
  {
    class stacked : public passthrough
    {
      // bleach, but we need somewhere to store the info on what the signals to
      // disconnect are :(
      struct child_info
      {
	widget_ref w;

	sigc::connection shown_conn, hidden_conn;

	child_info(const widget_ref &_w,
		   sigc::connection &_shown_conn,
		   sigc::connection &_hidden_conn)
	  :w(_w), shown_conn(_shown_conn),
	   hidden_conn(_hidden_conn)
	{
	}
      };

      typedef std::list<child_info> childlist;

      childlist children;

      int req_w, req_h;

      void layout_me();

      void hide_widget();
    protected:
      void paint(const style &st);

      // The size passed in is used as a preferred size.  (what we get might be
      // larger or smaller)
      stacked(int w, int h);
    public:
      ~stacked();

      void destroy();

      static util::ref_ptr<stacked> create(int w=0, int h=0)
      {
	util::ref_ptr<stacked> rval(new stacked(w, h));
	rval->decref();
	return rval;
      }

      void add_widget(const widget_ref &w);
      void rem_widget(const widget_ref &w);
      void raise_widget(const widget_ref &w);
      void lower_widget(const widget_ref &w);

      void raise_widget_bare(widget &w)
      {
	raise_widget(widget_ref(&w));
      }
      void lower_widget_bare(widget &w)
      {
	lower_widget(widget_ref(&w));
      }

      void dispatch_mouse(short id, int x, int y, int z, mmask_t bstate);

      widget_ref get_focus();

      void show_all();

      int width_request();
      int height_request(int w);
    };

    typedef util::ref_ptr<stacked> stacked_ref;
  }
}

#endif
