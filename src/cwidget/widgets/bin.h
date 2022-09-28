// bin.h        -*-c++-*-
//
//  Generic stuff for a container that can only handle one child.

#ifndef BIN_H
#define BIN_H

#include "passthrough.h"

#include <sigc++/connection.h>

namespace cwidget
{
  namespace widgets
  {
    class bin : public passthrough
    {
      widget_ref subwidget;

      // These are unfortunate necessities; when a widget is /removed/
      // (but not destroyed), it is necessary to delete the connections to
      // it.  :-(
      sigc::connection show_conn, hide_conn;

      // right now these just show or hide the bin itself
      void show_widget(const widget_ref &w);
      void hide_widget(const widget_ref &w);

      void show_widget_bare(widget &w);
      void hide_widget_bare(widget &w);

    protected:
      bin();

    public:
      virtual ~bin();

      void set_subwidget(const util::ref_ptr<widget> &w);
      void set_subwidget(widget &w)
      {
	set_subwidget(util::ref_ptr<widget>(&w));
      }

      widget_ref get_subwidget() {return subwidget;}

      void destroy();

      virtual void show_all();

      virtual void add_widget(const widget_ref &w);
      virtual void rem_widget(const widget_ref &w);

      widget_ref get_focus();

      void paint(const style &st);
    };
  }
}

#endif
