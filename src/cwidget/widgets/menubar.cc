// menubar.cc
//
//   Copyright (C) 2000-2006 Daniel Burrows
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

#include "container.h"
#include "menubar.h"
#include "menu.h"

#include <cwidget/toplevel.h>

#include <cwidget/config/colors.h>

#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>

#include <stdlib.h>

using namespace std;

namespace cwidget
{
  namespace widgets
  {
    config::keybindings *menubar::bindings=NULL;

    menubar::menubar(bool _always_visible)
      :container(), startloc(0), active(false),
       always_visible(_always_visible), curloc(0), subwidget(NULL)
    {
      do_layout.connect(sigc::mem_fun(*this, &menubar::layout_me));

      focussed.connect(sigc::mem_fun(*this, &menubar::got_focus));
      unfocussed.connect(sigc::mem_fun(*this, &menubar::lost_focus));
    }

    menubar::~menubar()
    {
      eassert(!subwidget.valid());
      eassert(items.empty());
      eassert(active_menus.empty());
    }

    widget_ref menubar::get_active_widget()
    {
      return subwidget;
    }

    void menubar::destroy()
    {
      widget_ref tmpref(this);

      if(subwidget.valid())
	subwidget->destroy();
      eassert(!subwidget.valid());

      // ew.  You see, we need to individually destroy the subwidgets, but
      // doing so will cause them to be removed, so we can't iterate over
      // the main list...
      vector<item> curr_items(items);

      for(vector<item>::const_iterator i = curr_items.begin();
	  i != curr_items.end(); ++i)
	i->child_menu->destroy();

      eassert(items.empty());
      eassert(active_menus.empty());

      container::destroy();
    }

    void menubar::got_focus()
    {
      widget_ref tmpref(this);

      widget_ref w=get_focus();
      if(w.valid())
	w->focussed();
    }

    void menubar::lost_focus()
    {
      widget_ref tmpref(this);

      widget_ref w=get_focus();
      if(w.valid())
	w->unfocussed();
    }

    bool menubar::focus_me()
    {
      if(active)
	return true;
      else if(subwidget.valid() && subwidget->focus_me())
	return true;
      else
	return widget::focus_me();
    }

    widget_ref menubar::get_focus()
    {
      if(active)
	{
	  if(!active_menus.empty())
	    return active_menus.front();
	  else
	    return NULL;
	}
      else if(subwidget.valid())
	return subwidget;
      else
	return NULL;
      // NB: could just end with 'return subwidget' but this makes the
      // order more explicit.
    }

    bool menubar::get_cursorvisible()
    {
      widget_ref w = get_focus();
      return (w.valid() && w->get_cursorvisible()) ||
	(!w.valid() && active);
    }

    point menubar::get_cursorloc()
    {
      widget_ref w = get_focus();

      if(w.valid())
	{
	  point p=w->get_cursorloc();
	  p.x+=w->get_startx();
	  p.y+=w->get_starty();
	  return p;
	}
      else if(active)
	return point(get_menustart(curloc), 0);
      else
	return point(0, 0);
    }

    int menubar::get_menustart(itemlist::size_type idx) const
    {
      int rval = 0;

      if(idx >= startloc)
	{
	  for(itemlist::size_type i = startloc; i < idx; ++i)
	    {
	      const wstring &title = items[i].title;
	      rval += wcswidth(title.c_str(), title.size());
	    }
	}
      else
	{
	  for(itemlist::size_type i = idx; i < startloc; ++i)
	    {
	      const wstring &title = items[i].title;
	      rval -= wcswidth(title.c_str(), title.size());
	    }
	}

      return rval;
    }

    void menubar::update_x_start()
    {
      if(!active)
	startloc = 0;
      else if(curloc < startloc)
	startloc = curloc;
      else
	{
	  int width = get_width();
	  if(width == 0)
	    return;


	  int start_x = get_menustart(startloc);

	  const wstring &curr_title = items[curloc].title;
	  int curr_x = get_menustart(curloc);
	  int curr_width = wcswidth(curr_title.c_str(),
				    curr_title.size());

	  if(width < curr_width)
	    while(curr_x >= start_x + width)
	      {
		const wstring &title = items[startloc].title;
		start_x += wcswidth(title.c_str(), title.size());
		++startloc;
	      }
	  else
	    while(curr_x + curr_width > start_x + width)
	      {
		const wstring &title = items[startloc].title;
		start_x += wcswidth(title.c_str(), title.size());
		++startloc;
	      }
	}
    }

    void menubar::append_item(const wstring &title,
			      const menu_ref &menu)
    {
      widget_ref tmpref(this);

      items.push_back(item(L' '+title+L' ', menu));

      menu->shown_sig.connect(sigc::bind(sigc::mem_fun(*this, &menubar::show_menu_bare), menu.weak_ref()));
      menu->hidden_sig.connect(sigc::bind(sigc::mem_fun(*this, &menubar::hide_menu_bare), menu.weak_ref()));
      menu->menus_goaway.connect(sigc::mem_fun(*this, &menubar::disappear));
      menu->set_owner(this);

      toplevel::update();
    }

    void menubar::set_subwidget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      if(subwidget.valid())
	{
	  subwidget->set_owner(NULL);
	  subwidget->unfocussed();
	}

      subwidget=w;

      if(subwidget.valid())
	{
	  subwidget->set_owner(this);
	  subwidget->focussed();
	}

      toplevel::queuelayout();
    }

    void menubar::show_all()
    {
      widget_ref tmpref(this);

      if(subwidget.valid())
	subwidget->show_all();
    }

    void menubar::add_widget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      eassert(!subwidget.valid());

      set_subwidget(w);
    }

    void menubar::rem_widget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      if(w == subwidget)
	set_subwidget(NULL);
      else
	{
	  eassert(w->get_owner().unsafe_get_ref() == this);

	  bool found = false;

	  // make a new strong reference
	  widget_ref w2 = w;

	  // hrm.
	  for(vector<item>::iterator i = items.begin();
	      i != items.end(); ++i)
	    {
	      if(i->child_menu == w2)
		{
		  found = true;
		  items.erase(i);
		  break;
		}
	    }

	  eassert(found);

	  active_menus.remove(w2);

	  w2->set_owner(NULL);
	}
    }

    int menubar::width_request()
    {
      widget_ref tmpref(this);

      int w=0;

      // Calculate the size of the bar itself.
      for(itemlist::size_type i=0; i<items.size(); i++)
	{
	  const wstring &title=items[i].title;

	  w+=wcswidth(title.c_str(), title.size());
	}

      // Expand the width as needed to account for active menus.
      for(activemenulist::iterator i=active_menus.begin();
	  i!=active_menus.end();
	  ++i)
	{
	  int menux=0;

	  // Calculate the starting X location of this menu.
	  for(itemlist::size_type j=0; j<items.size(); j++)
	    {
	      if(items[j].child_menu==*i)
		break;

	      const wstring &title=items[j].title;

	      menux+=wcswidth(title.c_str(), title.size());
	    }

	  // Now expand our width request.
	  w=max(w, menux+(*i)->width_request());
	}

      // Expand the width to account for the subwidget.
      if(subwidget.valid())
	w=max(w, subwidget->width_request());

      return w;
    }

    // TODO: What if the width is insufficient: should I scroll left/right
    // or wrap to the next line?
    int menubar::height_request(int w)
    {
      widget_ref tmpref(this);

      int h=always_visible?1:0;

      for(activemenulist::iterator i=active_menus.begin();
	  i!=active_menus.end();
	  ++i)
	h=max(h, 1+(*i)->height_request(w));

      if(subwidget.valid())
	{
	  int subwidget_h=subwidget->height_request(w);

	  if(always_visible)
	    subwidget_h+=1;

	  h=max(h, subwidget_h);
	}

      return h;
    }

    void menubar::layout_me()
    {
      widget_ref tmpref(this);

      update_x_start();

      // Find the starting X location of each active menu.
      for(activemenulist::iterator i=active_menus.begin();
	  i!=active_menus.end();
	  i++)
	{
	  int menuloc = -1;

	  for(itemlist::size_type j = 0; j < items.size(); j++)
	    {
	      if(items[j].child_menu == *i)
		{
		  menuloc = j;
		  break;
		}
	    }

	  int menux = get_menustart(menuloc);

	  int req_w=(*i)->width_request();

	  if(menux < 0)
	    menux = 0;
	  else if(getmaxx() < menux + req_w)
	    {
	      if(getmaxx() >= req_w)
		menux = getmaxx() - req_w;
	      else
		{
		  menux = 0;
		  req_w = getmaxx();
		}
	    }

	  int req_h = (*i)->height_request(req_w);

	  if(getmaxy() < 1 + req_h)
	    req_h = getmaxy()-1;

	  (*i)->alloc_size(menux,
			   1,
			   req_w,
			   req_h);
	}

      if(subwidget.valid())
	subwidget->alloc_size(0,
			      always_visible?1:0,
			      getmaxx(),
			      always_visible?getmaxy()-1:getmaxy());
    }

    void menubar::show_menu(const menu_ref &w)
    {
      widget_ref tmpref(this);

      if(active)
	{
	  widget_ref old_focus=get_focus();

	  for(activemenulist::iterator i=active_menus.begin();
	      i!=active_menus.end();
	      i++)
	    eassert(w != *i);

	  if(old_focus.valid())
	    old_focus->unfocussed();

	  active_menus.push_front(w);

	  w->focussed();

	  toplevel::queuelayout();
	  toplevel::update();
	}
    }

    void menubar::hide_menu(const menu_ref &w)
    {
      widget_ref tmpref(this);

      if(active)
	{
	  for(activemenulist::iterator i=active_menus.begin();
	      i!=active_menus.end();
	      i++)
	    {
	      if(*i==w)
		{
		  w->unfocussed();
		  active_menus.remove(w);

		  widget_ref new_focus=get_focus();
		  if(new_focus.valid())
		    new_focus->focussed();

		  toplevel::queuelayout();
		  toplevel::update();
		  return;
		}
	    }

	  abort();
	}
    }

    void menubar::hide_menu_bare(menu &w)
    {
      hide_menu(menu_ref(&w));
    }

    void menubar::show_menu_bare(menu &w)
    {
      show_menu(menu_ref(&w));
    }

    void menubar::appear()
    {
      widget_ref tmpref(this);

      if(!active)
	{
	  active=true;
	  if(subwidget.valid())
	    subwidget->unfocussed();

	  // Added to eliminate the weird "titles are selected but contents aren't"
	  // state that was normal with the previous settings:
	  if(items.size()>0)
	    items[curloc].child_menu->show();

	  update_x_start();
	  toplevel::update();
	}
    }

    void menubar::disappear()
    {
      widget_ref tmpref(this);

      if(active)
	{
	  while(!active_menus.empty())
	    active_menus.front()->hide();

	  active=false;
	  if(subwidget.valid())
	    subwidget->focussed();

	  curloc=0;

	  toplevel::update();
	}
    }

    bool menubar::handle_key(const config::key &k)
    {
      widget_ref tmpref(this);

      if(bindings->key_matches(k, "ToggleMenuActive"))
	{
	  if(active)
	    disappear();
	  else
	    appear();
	}
      else if(active)
	{
	  if(bindings->key_matches(k, "Cancel"))
	    {
	      disappear();

	      //if(!active_menus.empty())
	      //  active_menus.front()->hide();
	      //else
	      //  disappear();

	      toplevel::update();
	    }
	  else if(!active_menus.empty())
	    {
	      if(bindings->key_matches(k, "Right"))
		{
		  if(items.size()>0)
		    {
		      while(!active_menus.empty())
			active_menus.front()->hide();

		      active_menus.clear();

		      if(curloc<items.size()-1)
			curloc++;
		      else
			curloc=0;

		      items[curloc].child_menu->show();

		      update_x_start();
		      toplevel::update();
		    }
		}
	      else if(bindings->key_matches(k, "Left"))
		{
		  if(items.size()>0)
		    {
		      while(!active_menus.empty())
			active_menus.front()->hide();

		      active_menus.clear();

		      if(curloc>0)
			curloc--;
		      else
			curloc=items.size()-1;

		      items[curloc].child_menu->show();

		      update_x_start();
		      toplevel::update();
		    }
		}
	      else if(active_menus.front()->dispatch_key(k))
		return true;
	      else
		return widget::handle_key(k);
	    }
	  else if(bindings->key_matches(k, "Right"))
	    {
	      if(items.size()>0)
		{
		  if(curloc<items.size()-1)
		    curloc++;
		  else
		    curloc=0;

		  update_x_start();
		  toplevel::update();
		}
	    }
	  else if(bindings->key_matches(k, "Left"))
	    {
	      if(items.size()>0)
		{
		  if(curloc>0)
		    curloc--;
		  else
		    curloc=items.size()-1;

		  update_x_start();
		  toplevel::update();
		}
	    }
	  else if(bindings->key_matches(k, "Down") ||
		  bindings->key_matches(k, "Confirm"))
	    {
	      if(items.size()>0)
		items[curloc].child_menu->show();
	    }
	  else
	    return widget::handle_key(k);

	  return true;
	}
      else if(subwidget.valid() && subwidget->dispatch_key(k))
	return true;
      else
	return widget::handle_key(k);

      return true;
    }

    void menubar::paint(const style &st)
    {
      widget_ref tmpref(this);

      if(subwidget.valid())
	subwidget->display(st);

      if(active || always_visible)
	{
	  const style menubar_style=get_style("MenuBar");
	  const style highlightedmenubar_style=get_style("HighlightedMenuBar");

	  if(active)
	    for(activemenulist::reverse_iterator i=active_menus.rbegin();
		i!=active_menus.rend();
		i++)
	      (*i)->display(st);

	  int pos=0, maxx=getmaxx();

	  apply_style(menubar_style);
	  move(0, 0);
	  for(int i=0; i<maxx; i+=wcwidth(L' '))
	    add_wch(L' ');

	  move(0, 0);

	  itemlist::size_type i;
	  for(i = startloc; i < items.size() && pos < maxx; ++i)
	    {
	      if(active && i==curloc)
		apply_style(highlightedmenubar_style);
	      else
		apply_style(menubar_style);

	      wstring &title = items[i].title;
	      size_t titleloc = 0;

	      while(titleloc < title.size() && pos < maxx)
		{
		  wchar_t wch=title[titleloc];

		  add_wch(wch);
		  pos+=wcwidth(wch);
		  ++titleloc;
		}
	    }

	  apply_style(menubar_style);

	  if(startloc > 0)
	    mvadd_wch(0, 0, WACS_LARROW);
	  if(i < items.size() || pos > maxx)
	    mvadd_wch(0, maxx-1, WACS_RARROW);
	}
    }

    void menubar::dispatch_mouse(short id, int x, int y, int z, mmask_t bmask)
    {
      widget_ref tmpref(this);

      if(y==0 && (active || always_visible))
	{
	  if(bmask & (BUTTON1_CLICKED | BUTTON2_CLICKED |
		      BUTTON3_CLICKED | BUTTON4_CLICKED |
		      BUTTON1_RELEASED | BUTTON2_RELEASED |
		      BUTTON3_RELEASED | BUTTON4_RELEASED |
		      BUTTON1_PRESSED | BUTTON2_PRESSED |
		      BUTTON3_PRESSED | BUTTON4_PRESSED))
	    {
	      if(!active)
		appear();

	      int loc=0;
	      activemenulist::size_type i=0;

	      if(items.size()>0)
		{
		  loc += wcswidth(items[0].title.c_str(), items[0].title.size());

		  while(i<items.size()-1 && loc<=x)
		    {
		      loc += wcswidth(items[i+1].title.c_str(), items[i+1].title.size());

		      ++i;
		    }
		}

	      if(i<items.size())
		{
		  while(!active_menus.empty())
		    active_menus.front()->hide();

		  active_menus.clear();

		  curloc=i;

		  items[curloc].child_menu->show();

		  toplevel::update();
		}
	    }
	}
      else if(active)
	{
	  for(activemenulist::iterator i=active_menus.begin();
	      i!=active_menus.end();
	      i++)
	    if((*i)->enclose(y, x))
	      {
		(*i)->dispatch_mouse(id,
				     x-(*i)->get_startx(), y-(*i)->get_starty(), z,
				     bmask);
		return;
	      }
	}

      if(subwidget.valid())
	subwidget->dispatch_mouse(id,
				  x-subwidget->get_startx(),
				  y-subwidget->get_starty(), z, bmask);
    }

    void menubar::init_bindings()
    {
      bindings = new config::keybindings(&config::global_bindings);
    }

    void menubar::set_always_visible(bool _always_visible)
    {
      if(_always_visible!=always_visible)
	{
	  always_visible=_always_visible;
	  toplevel::update();
	  toplevel::queuelayout();
	}
    }
  }
}
