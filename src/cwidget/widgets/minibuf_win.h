// minibuf_win.h       -*-c++-*-
//
//  Copyright 2000-2001, 2004-2005 Daniel Burrows
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
//
//  Apologies for the lame name, but I couldn't think of anything else.
//
//  This class provides basic support for a common UI theme: a widget with a
// header and status line, where the status line can contain various widgets
// for getting input, displaying messages, etc. (the header line will probably
// vanish in the future)

#ifndef MINIBUF_WIN_H
#define MINIBUF_WIN_H

#include <cwidget/style.h>

#include "passthrough.h"
#include "widget.h"

#include <sigc++/connection.h>

namespace cwidget
{
  namespace widgets
  {
    class minibuf;
    class label;
    class multiplex;

    class minibuf_win:public passthrough
    {
      util::ref_ptr<label> status_lbl, header;

      widget_ref main_widget;
      // This is displayed in the center of the screen.

      util::ref_ptr<multiplex> status;

      sigc::connection main_destroy_conn;

    protected:
      minibuf_win();
    public:
      static
      util::ref_ptr<minibuf_win> create()
      {
	util::ref_ptr<minibuf_win> rval = new minibuf_win;
	rval->decref();
	return rval;
      }

      ~minibuf_win();

      void destroy();

      void set_main_widget(const widget_ref &w);

      int width_request();
      int height_request(int w);
      void layout_me();

      virtual void paint(const style &st);

      void show_header();
      void show_status();
      // Should mainly be used if it's necessary to force an update of the status
      // line

      void set_status(std::string new_status);
      void set_header(std::string new_header);
      // Set the status and header lines of the window, respectively.
      // These routines do NOT call refresh()!

      widget_ref get_focus();

      static const style &retr_header_style() {return get_style("Header");}
      static const style &retr_status_style() {return get_style("Status");}
      void display_error(std::string err);

      void add_widget(const widget_ref &widget);
      // Adds a widget.  Widgets are automatically shown if they're the first
      // one to be added, otherwise show() should be called.
      void rem_widget(const widget_ref &widget);
      // Removes a widget

      void show_all();
    };

    typedef util::ref_ptr<minibuf_win> minibuf_win_ref;
  }
}

#endif
