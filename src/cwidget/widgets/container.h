// container.h              -*-c++-*-
//
//
//   Copyright (C) 2000, 2005 Daniel Burrows
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of
//   the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; see the file COPYING.  If not, write to
//   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//   Boston, MA 02111-1307, USA.
//
//  A generic interface for a widget that can hold other widgets.

#ifndef CONTAINER_H
#define CONTAINER_H

#include "widget.h"

namespace cwidget
{
  namespace widgets
  {
    class container : public widget
    {
    public:
      container() : widget() {}
      ~container();

      virtual void add_widget(const widget_ref &)=0;
      void add_visible_widget(const widget_ref &, bool visible);
      virtual void rem_widget(const widget_ref &)=0;

      // Variants of the above that take a bare reference; used for weak
      // slot connections.
      void add_widget_bare(widget &w)
      {
	add_widget(widget_ref(&w));
      }

      void add_visible_widget_bare(widget &w, bool visible)
      {
	add_visible_widget(widget_ref(&w), visible);
      }

      void rem_widget_bare(widget &w)
      {
	rem_widget(widget_ref(&w));
      }

      /** Return the currently "active" child of this container, or \b NULL. */
      virtual widget_ref get_active_widget() = 0;

      /** Display this widget and all its subwidgets. */
      virtual void show_all()=0;
    };
  }
}

#endif
