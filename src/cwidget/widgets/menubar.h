// menubar.h   -*-c++-*-
//
//   Copyright (C) 2000-2005 Daniel Burrows
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
//  Provides a horizontal menubar and a space for submenus.  This widget and
// its menus are superimposed on top of another widget.

#ifndef MENUBAR_H
#define MENUBAR_H

#include "widget.h"
#include "container.h"
#include <cwidget/config/keybindings.h>

#include <list>
#include <string>
#include <vector>

namespace cwidget
{
  namespace widgets
  {
    class menu;

    typedef util::ref_ptr<menu> menu_ref;

    class menubar:public container
    {
      struct item
      {
	std::wstring title;
	util::ref_ptr<menu> child_menu;

	item(std::wstring _title, util::ref_ptr<menu> _child_menu)
	  :title(_title), child_menu(_child_menu)
	{
	}
      };

      typedef std::vector<item> itemlist;
      typedef std::list<widget_ref> activemenulist;

      // A list of the items in the menubar itself
      itemlist items;
      // A list of active menus
      activemenulist active_menus;

      /** The index of the leftmost visible menu item. */
      itemlist::size_type startloc;

      // True if the menu-bar is visible and/or being used
      bool active;

      // True if the menu-bar should always be visible
      bool always_visible;

      /** The index of the currently selected item. */
      itemlist::size_type curloc;

      // the widget underneath this one.
      widget_ref subwidget;

      // Returns the starting X location of the given item in the menu
      int get_menustart(itemlist::size_type idx) const;

      /** Re-calculates the starting X location, moving it left or right
       *  if needed.
       */
      void update_x_start();

      // Show/hide menus
      void show_menu(const menu_ref &w);
      void show_menu_bare(menu &w);

      void hide_menu(const menu_ref &w);
      void hide_menu_bare(menu &w);

      void appear();
      void disappear();

      // Similar to the passthrough widget's routine (there's not enough
      // similarity, though, to justify making this a passthrough widget)
      widget_ref get_focus();

      void got_focus();
      void lost_focus();
    protected:
      virtual bool handle_key(const config::key &k);

      menubar(bool _always_visible);
    public:
      static util::ref_ptr<menubar> create(bool always_visible = true)
      {
	util::ref_ptr<menubar> rval(new menubar(always_visible));
	rval->decref();
	return rval;
      }

      ~menubar();

      /** The 'active' widget of a menubar is always its subwidget. */
      widget_ref get_active_widget();

      void destroy();

      int width_request();
      int height_request(int w);
      void layout_me();

      void set_subwidget(const widget_ref &w);

      void append_item(const std::wstring &title, const menu_ref &menu);
      void append_item(const std::wstring &title, menu &menu)
      {
	append_item(title, menu_ref(&menu));
      }

      void show_all();

      /** Add a widget as the new subwidget, like a bin. */
      void add_widget(const widget_ref &w);
      /** Remove the subwidget OR a menu. */
      void rem_widget(const widget_ref &w);

      virtual void paint(const style &st);
      virtual bool focus_me();
      virtual void dispatch_mouse(short id, int x, int y, int z,
				  mmask_t bmask);

      bool get_cursorvisible();
      point get_cursorloc();

      bool get_always_visible() {return always_visible;}
      void set_always_visible(bool _always_visible);

      static config::keybindings *bindings;
      static void init_bindings();
    };

    typedef util::ref_ptr<menubar> menubar_ref;
  }
}

#endif
