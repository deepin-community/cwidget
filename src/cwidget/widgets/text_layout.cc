// text_layout.cc
//
//   Copyright (C) 2004-2007 Daniel Burrows
//   Copyright (C) 2019 Manuel A. Fernandez Montecelo
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


#include "text_layout.h"

#include <cwidget/config/keybindings.h>
#include <cwidget/toplevel.h>
#include <cwidget/fragment.h>
#include <cwidget/fragment_contents.h>

#include <algorithm>

#include <sigc++/functors/mem_fun.h>

using namespace std;

namespace cwidget
{
  namespace widgets
  {
    config::keybindings *text_layout::bindings;

    text_layout::text_layout():start(0), f(newline_fragment()), stale(true), lastw(0)
    {
      do_layout.connect(sigc::mem_fun(*this, &text_layout::layout_me));
    }

    text_layout::text_layout(fragment *_f):start(0), f(_f), stale(true), lastw(0)
    {
      do_layout.connect(sigc::mem_fun(*this, &text_layout::layout_me));
    }

    void text_layout::init_bindings()
    {
      bindings = new config::keybindings(&config::global_bindings);
    }

    bool text_layout::handle_key(const config::key &k)
    {
      if(bindings->key_matches(k, "Up"))
	line_up();
      else if(bindings->key_matches(k, "Down"))
	line_down();
      else if(bindings->key_matches(k, "Begin"))
	move_to_top();
      else if(bindings->key_matches(k, "End"))
	move_to_bottom();
      else if(bindings->key_matches(k, "PrevPage"))
	page_up();
      else if(bindings->key_matches(k, "NextPage"))
	page_down();
      else
	return widget::handle_key(k);

      return true;
    }

    void text_layout::dispatch_mouse(short id, int x, int y, int z, mmask_t bstate)
    {
      // Only do something if this system's ncurses has both button 4
      // and button 5 (older ones didn't).
#if defined(BUTTON4_PRESSED) && defined(BUTTON5_PRESSED)
      const int mouse_wheel_scroll_lines =
	std::max(1, std::min(getmaxy() - 1, 3));

      if((bstate & BUTTON4_PRESSED) != 0)
	{
	  if((bstate & BUTTON5_PRESSED) == 0)
	    {
	      freshen_contents(lastst);
	      if(start > 0)
		set_start(std::max(0, static_cast<int>(start - mouse_wheel_scroll_lines)));
	    }
	}
      else if((bstate & BUTTON5_PRESSED) != 0)
	{
	  freshen_contents(lastst);
	  if(start + getmaxy() < contents.size())
	    set_start(std::min(contents.size() - getmaxy(),
			       start + mouse_wheel_scroll_lines));
	}
#endif
    }

    int text_layout::width_request()
    {
      if(f!=NULL)
	return f->max_width(0, 0);
      else
	return 0;
    }

    int text_layout::height_request(int w)
    {
      // Wasteful: calculate the contents and throw them away.
      if(f!=NULL)
	return f->layout(w, w, style()).size();
      else
	return 0;
    }

    text_layout::~text_layout()
    {
      delete f;
    }

    void text_layout::set_fragment(fragment *_f)
    {
      delete f;
      f=_f;

      stale=true;

      // Don't just do an update, because our ideal width might change,
      // which means other stuff also has to change around.
      toplevel::queuelayout();
    }

    void text_layout::append_fragment(fragment *_f)
    {
      f=sequence_fragment(f, _f, NULL);
      stale=true;

      toplevel::queuelayout();
    }

    void text_layout::set_start(unsigned int new_start)
    {
      if(new_start!=start)
	{
	  start=new_start;
	  do_signal();
	  toplevel::update();
	}
    }

    void text_layout::layout_me()
    {
      // If the width has changed, we need to recalculate the layout.
      if(getmaxx()!=lastw)
	stale=true;

      // We should always signal regardless of whether the width has
      // changed: it is possible that the scrollbar bounds are different
      // now. (e.g., if we just resized vertically)
      do_signal();
    }

    bool text_layout::get_cursorvisible()
    {
      return true;
    }

    /** The cursor is always located in the upper-left-hand corner. */
    point text_layout::get_cursorloc()
    {
      return point(0,0);
    }

    /** This widget can get focus if it can scroll: ie, if its contents take
     *  up more lines than it was allocated.
     */
    bool text_layout::focus_me()
    {
      freshen_contents(lastst);

      if(start>0 || contents.size()>(unsigned) getmaxy())
	return true;
      else
	return false;
    }

    /** Paint by refreshing the contents [if necessary], then drawing,
     *  starting from the current line.
     */
    void text_layout::paint(const style &st)
    {
      freshen_contents(st);

      if(start>=contents.size())
	{
	  if(contents.size()==0)
	    set_start(0);
	  else
	    set_start(contents.size()-1);
	}

      for(int i=0; i<getmaxy() && i+start<contents.size(); ++i)
	mvaddnstr(i, 0, contents[i+start], contents[i+start].size());
    }

    void text_layout::freshen_contents(const style &st)
    {
      if(stale || lastw != getmaxx() || lastst != st)
	{
	  contents=f->layout(getmaxx(), getmaxx(), st);
	  stale=false;
	  lastw=getmaxx();
	  lastst=st;

	  do_signal();
	}
    }

    void text_layout::line_down()
    {
      freshen_contents(lastst);

      if(start+getmaxy()<contents.size())
	set_start(start+1);
    }

    void text_layout::line_up()
    {
      freshen_contents(lastst);

      if(start>0)
	set_start(start-1);
    }


    void text_layout::move_to_top()
    {
      set_start(0);
    }

    void text_layout::move_to_bottom()
    {
      freshen_contents(lastst);

      set_start(max(start, contents.size()-getmaxy()));
    }

    void text_layout::page_up()
    {
      if(start<(unsigned) getmaxy())
	set_start(0);
      else
	set_start(start-getmaxy());
    }

    void text_layout::page_down()
    {
      freshen_contents(lastst);

      if(start+getmaxy()<contents.size())
	set_start(start+getmaxy());
    }

    // Assumes the contents are already fresh.
    void text_layout::do_signal()
    {
      if(((unsigned) getmaxy())>=contents.size() && start==0)
	location_changed(start, 0);
      else if(start+getmaxy()>=contents.size())
	location_changed(1, 1);
      else
	location_changed(start, contents.size()-getmaxy());
    }

    void text_layout::search_for(const wstring &s, bool search_forward)
    {
      freshen_contents(lastst);

      if(getmaxy() == 0)
	return;

      // Very simplistic routine.  Could be made quicker by incorporating
      // a more sophisticated search algorithm.
      size_t new_start = search_forward ? start + 1 : start - 1;

      // Look for the first character of the string.
      while(new_start > 0 && new_start < contents.size())
	{
	  fragment_line &line(contents[new_start]);

	  // Search this line and the following lines (if there's an
	  // overrun) for our string.
	  for(fragment_line::const_iterator i = line.begin();
	      i != line.end(); ++i)
	    {
	      if(i->ch == s[0])
		{
		  size_t tmp = new_start;
		  fragment_line::const_iterator j = i;
		  wstring::const_iterator loc = s.begin();

		  while(tmp < contents.size() && loc != s.end() &&
			j->ch == *loc)
		    {
		      ++loc;
		      ++j;

		      if(j == contents[tmp].end())
			{
			  ++tmp;
			  if(tmp < contents.size())
			    j = contents[tmp].begin();
			}
		    }

		  if(loc == s.end()) // success
		    {
		      set_start(new_start);
		      return;
		    }
		}
	    }

	  if(search_forward)
	    ++new_start;
	  else
	    --new_start;
	}
    }

    void text_layout::scroll(bool dir)
    {
      if(dir)
	page_up();
      else
	page_down();
    }
  }
}
