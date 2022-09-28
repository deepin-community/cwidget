// multiplex.h                       (This is -*-c++-*-)
// Copyright 1999-2006, 2009 Daniel Burrows
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.

#ifndef VSMULTIPLEX_H
#define VSMULTIPLEX_H

#include <cwidget/curses++.h>
#include "passthrough.h"

#include <cwidget/generic/util/eassert.h>

#include <list>
#include <string>

namespace cwidget
{
  namespace widgets
  {
    /** This widget displays exactly one of its children at once.
     *
     *  Hiding a child will prevent it from appearing; showing a child
     *  will add it to the list of visible children, and make it the
     *  currently visible child if it isn't already.
     *
     *  An optional "tab bar" listing the children of the multiplexer
     *  can be activated using set_show_tabs.
     *
     *  This widget requests enough space for its largest visible
     *  child.
     */
    class multiplex : public passthrough
    {
      struct child_info
      {
	widget_ref w;
	std::wstring title;

	child_info(const widget_ref &_w, const std::wstring &_title)
	  :w(_w), title(_title)
	{
	}
      };

      std::list<child_info> children;

      std::list<child_info>::iterator visible_child;

      /** If \b true and more than one child is visible, "tabs" are
       *  displayed at the top of the widget, corresponding to the visible
       *  children.
       */
      bool show_tabs;

      /** \return \b true if show_tabs is \b true and more than one
       *  child is visible.
       */
      bool tabs_visible() const;

      void show_widget(const widget_ref &widget);
      // Used to bring a widget to the front
      void hide_widget(const widget_ref &widget);
      // Used to hide a widget

      void show_widget_bare(widget &widget);
      void hide_widget_bare(widget &widget);

      void got_focus();
      void lost_focus();
    protected:
      bool winavail() {return get_win();}

      multiplex(bool _show_tabs);
    public:
      static util::ref_ptr<multiplex> create(bool show_tabs = false)
      {
	util::ref_ptr<multiplex> rval(new multiplex(show_tabs));
	rval->decref();
	return rval;
      }

      virtual ~multiplex();

      /** Returns the maximum width requested by any child. */
      int width_request();

      /** Returns the maximum height requested by any child. */
      int height_request(int width);

      void destroy();

      void layout_me();

      virtual widget_ref get_focus();
      widget_ref visible_widget();
      unsigned int num_children();
      // Returns the number of widgets in the multiplexer.
      unsigned int num_visible();

      virtual void paint(const style &st);
      void dispatch_mouse(short id, int x, int y, int z, mmask_t bstate);

      void show_all();

      void set_show_tabs(bool shown);

      /** Add a title-less widget.  Provided to implement a required
       *  function and for backwards compatibility; use of this routine is
       *  deprecated.
       */
      void add_widget(const widget_ref &widget);
      void add_widget(const widget_ref &widget, const std::wstring &title);
      void add_widget_bare(widget &widget, const std::wstring &title)
      {
	add_widget(widget_ref(&widget), title);
      }

      void add_widget_after(const widget_ref &widget,
			    const widget_ref &after);

      void add_widget_after_bare(cwidget::widgets::widget &widget,
				 cwidget::widgets::widget &after)
      {
	add_widget_after(widget_ref(&widget), widget_ref(&after));
      }


      void add_widget_after(const widget_ref &widget,
			    const widget_ref &after,
			    const std::wstring &title);


      void add_widget_after_bare(cwidget::widgets::widget &widget,
				 cwidget::widgets::widget &after,
				 const std::wstring &title)
      {
	add_widget_after(widget_ref(&widget), widget_ref(&after), title);
      }


      void rem_widget(const widget_ref &widget);

      // These cycle forward and backwards through the list of visible items.
      void cycle_forward();
      void cycle_backward();

      /** Emitted when the currently visible widget changes. */
      sigc::signal0<void> cycled;
    };

    typedef util::ref_ptr<multiplex> multiplex_ref;
  }
}

#endif
