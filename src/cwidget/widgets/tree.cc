// tree.cc
//
//  Copyright 1999-2002, 2004-2007, 2009 Daniel Burrows
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
//  Implementation of stuff in tree.h

#include "tree.h"

#include "editline.h"

#include <cwidget/config/colors.h>
#include <cwidget/config/keybindings.h>
#include <cwidget/generic/util/i18n.h>
#include <cwidget/generic/util/transcode.h>
#include <cwidget/toplevel.h>

#include <sigc++/functors/ptr_fun.h>

using namespace std;

namespace cwidget
{
  namespace widgets
  {
    config::keybindings *tree::bindings=NULL;

    bool tree_search_string::operator()(const treeitem &item)
    {
      return item.matches(s);
    }

    tree::tree()
      : widget(),
	root(NULL),
	begin(new tree_root_iterator(NULL)),
	end(begin),
	top(begin),
	selected(top),
	hierarchical(true),
	prev_level(NULL)
    {
      focussed.connect(sigc::ptr_fun(toplevel::update));
      unfocussed.connect(sigc::ptr_fun(toplevel::update));
    }

    tree::tree(treeitem *_root, bool showroot)
      : widget(),
	root(NULL),
	begin(new tree_root_iterator(NULL)),
	end(begin),
	top(begin),
	selected(top),
	hierarchical(true),
	prev_level(NULL)
    {
      set_root(_root, showroot);

      focussed.connect(sigc::ptr_fun(toplevel::update));
      unfocussed.connect(sigc::ptr_fun(toplevel::update));
    }

    tree::~tree()
    {
      while(prev_level)
	{
	  flat_frame *next=prev_level->next;
	  delete prev_level;
	  prev_level=next;
	}

      delete root; root=NULL;
    }

    void tree::do_shown()
    {
      if(selected!=end)
	{
	  selected->highlighted_changed(true);
	  selection_changed(&*selected);
	}
      else
	selection_changed(NULL);
    }

    int tree::width_request()
    {
      return 1;
    }

    int tree::height_request(int w)
    {
      return 1;
    }

    void tree::set_root(treeitem *_root, bool showroot)
    {
      // Clear out the "history list"
      while(prev_level)
	{
	  flat_frame *next=prev_level->next;
	  delete prev_level;
	  prev_level=next;
	}

      if(selected!=end)
	{
	  selected->highlighted_changed(false);
	  selection_changed(&*selected);
	}
      else
	selection_changed(NULL);

      if(root)
	delete root;

      root=_root;

      if(root)
	{
	  if(showroot)
	    {
	      tree_root_iterator *realbegin=new tree_root_iterator(_root);
	      // NOTE: realbegin will be DELETED when it is assigned to begin,
	      // because the temporary that wraps it will be destroyed.
	      // This is Just Plain Evil (probably conversion from levelrefs
	      // to treeiterators shouldn't be allowed) but the workaround is
	      // here: reference its end() routine *before* we assign it.

	      end=realbegin->end();
	      begin=realbegin; // now realbegin is INVALID!!
	    }
	  else
	    {
	      begin=_root->begin();
	      end=_root->end();
	    }

	  top=begin;
	}
      else
	{
	  top=begin=end=new tree_root_iterator(NULL);
	}

      selected=top;
      while(selected!=end && !selected->get_selectable())
	selected++;
      if(selected!=end)
	{
	  selected->highlighted_changed(true);
	  selection_changed(&*selected);
	}
      else
	selection_changed(NULL);

      toplevel::update();
    }

    void tree::sync_bounds()
    // As I said: yuck!
    {
      begin=root->begin();
      if(top==end)
	top=begin;
      if(selected==end)
	selected=begin;
      end=root->end();
    }

    int tree::line_of(treeiterator item)
    // Returns the Y coordinate of the given item.  (so we have to count
    // from 1)
    {
      int j;
      treeiterator i=top;
      if(item==top)
	return 1;

      j=1;
      do {
	if(hierarchical)
	  ++i;
	else
	  i.move_forward_level();

	++j;

	if(i==item)
	  return j;
      } while(i!=end);

      i=top;
      j=1;
      do {
	if(hierarchical)
	  --i;
	else
	  i.move_backward_level();

	--j;

	if(i==item)
	  return j;
      } while(i!=begin);

      // Only happens if the iterator isn't in the visible range at all.
      abort();
    }

    bool tree::item_visible(treeiterator pkg)
    {
      int width,height;
      treeiterator i=top;

      getmaxyx(height,width);

      if(!hierarchical)
	--height;

      while(height>0 && i!=pkg && i!=end)
	{
	  --height;
	  ++i;
	}

      return height>0 && i!=end;
    }

    void tree::set_selection(treeiterator to, bool force_to_top)
    {
      // Expand all its parents so that it's possible to make it visible.
      treeiterator curr = to;
      while(!curr.is_root())
	{
	  curr = curr.get_up();
	  curr.expand();
	}

      // Expand the root as well if necessary.
      if(curr != to)
	curr.expand();

      if(item_visible(to))
	{
	  if(selected!=end)
	    selected->highlighted_changed(false);
	  selected=to;
	  if(selected!=end)
	    {
	      selected->highlighted_changed(true);
	      selection_changed(&*selected);
	    }
	  else
	    selection_changed(NULL);

	  toplevel::update();
	}
      else
	{
	  int height = getmaxy();
	  if(height == 0)
	    {
	      selected = top = to;
	      return;
	    }

	  // Give up and just directly determine the line of 'to'.
	  int l = line_of(to);

	  while(l < 1)
	    {
	      eassert(top != end);

	      if(hierarchical)
		--top;
	      else
		top.move_backward_level();

	      ++l;
	    }

	  while(l > (force_to_top ? 1 : height))
	    {
	      eassert(top != end);

	      if(hierarchical)
		++top;
	      else
		top.move_forward_level();

	      --l;
	    }

	  if(selected != to)
	    {
	      if(selected != end)
		selected->highlighted_changed(false);

	      if(to != end)
		{
		  to->highlighted_changed(true);
		  selection_changed(&*to);
		}
	      else
		selection_changed(NULL);
	    }

	  selected = to;

	  toplevel::update();
	}
    }

    bool tree::get_cursorvisible()
    {
      return (root != NULL && selected != end && selected->get_selectable());
    }

    point tree::get_cursorloc()
    {
      if(root == NULL)
	return point(0, 0);
      else if(selected==end || !selected->get_selectable())
	return point(0,0);
      else
	return point(0, hierarchical?line_of(selected)-1:line_of(selected));
    }

    void tree::line_down()
    {
      if(root == NULL)
	return;

      int width,height;
      getmaxyx(height,width);

      if(!hierarchical)
	--height;

      treeiterator orig = selected, prevtop = top;

      int newline = line_of(selected);
      int scrollcount = 0;
      bool moved = false;

      while(selected != end &&
	    scrollcount < 1 &&
	    (!moved || !selected->get_selectable()))
	{
	  if(hierarchical)
	    ++selected;
	  else
	    selected.move_forward_level();

	  ++newline;
	  moved = true;

	  // If we fell off the end of the screen and not off the end of
	  // the list, scroll the screen forward.
	  if(newline > height && selected != end)
	    {
	      if(hierarchical)
		++top;
	      else
		top.move_forward_level();

	      --newline;
	      ++scrollcount;
	    }
	}

      if(selected == end)
	{
	  if(hierarchical)
	    --selected;
	  else
	    selected.move_backward_level();

	  --newline;
	}

      if(orig != selected)
	{
	  if(orig != end)
	    orig->highlighted_changed(false);

	  if(selected != end)
	    {
	      selected->highlighted_changed(true);
	      selection_changed(&*selected);
	    }
	  else
	    selection_changed(NULL);
	}

      toplevel::update();
    }

    void tree::set_hierarchical(bool _hierarchical)
    {
      if(_hierarchical!=hierarchical)
	{
	  hierarchical=_hierarchical;

	  if(_hierarchical)
	    {
	      while(prev_level && prev_level->next)
		{
		  flat_frame *next=prev_level->next;
		  delete prev_level;
		  prev_level=next;
		}

	      if(prev_level)
		{
		  top=prev_level->top;
		  begin=prev_level->begin;
		  end=prev_level->end;
		  selected=prev_level->selected;

		  delete prev_level;
		  prev_level=NULL;
		}
	    }

	  toplevel::update();
	}
    }

    void tree::highlight_current()
    {
      if(root != NULL && selected != end)
	{
	  selected->highlighted_changed(true);
	  selection_changed(&*selected);
	}
      else
	selection_changed(NULL);
    }

    void tree::unhighlight_current()
    {
      if(root != NULL && selected != end)
	selected->highlighted_changed(false);
      selection_changed(NULL);
    }

    void tree::line_up()
    {
      if(root == NULL)
	return;

      int width,height;
      getmaxyx(height,width);

      if(!hierarchical)
	--height;

      treeiterator orig=selected;

      bool moved = false;
      int scrollcount = 0;

      // Guard against the selected entry being unexpectedly invalid (most
      // likely indicates that the tree is empty).
      if(selected == end)
	{
	  selected = top;
	  if(selected == end)
	    selected = begin;

	  if(selected == end)
	    return;
	}

      while(selected != begin &&
	    scrollcount < 1 &&
	    (!moved || !selected->get_selectable()))
	{
	  if(selected == top)
	    {
	      if(hierarchical)
		--top;
	      else
		top.move_backward_level();

	      ++scrollcount;
	    }

	  if(hierarchical)
	    --selected;
	  else
	    selected.move_backward_level();
	  moved = true;
	}

      // Handle the special case where the first element of the tree is
      // non-selectable.
      if(selected == begin && !selected->get_selectable())
	{
	  while(selected != end && !selected->get_selectable())
	    ++selected;

	  if(line_of(selected) >= height)
	    selected = begin;
	}

      if(orig != selected)
	{
	  if(orig != end)
	    orig->highlighted_changed(false);

	  if(selected != end)
	    {
	      selected->highlighted_changed(true);
	      selection_changed(&*selected);
	    }
	  else
	    selection_changed(NULL);
	}

      toplevel::update();
    }

    void tree::page_down()
    {
      if(root == NULL)
	return;

      int width,height;
      getmaxyx(height,width);

      if(!hierarchical)
	--height;

      int count=height;
      treeiterator newtop=top;
      while(count>0 && newtop!=end)
	{
	  if(hierarchical)
	    ++newtop;
	  else
	    newtop.move_forward_level();
	  count--;
	}

      if(count==0 && newtop!=end)
	{
	  int l=0;
	  (*selected).highlighted_changed(false);
	  selected=top=newtop;
	  while(l<height && selected!=end && !selected->get_selectable())
	    if(hierarchical)
	      ++selected;
	    else
	      selected.move_forward_level();
	  if(l==height || selected==end)
	    selected=top;
	  (*selected).highlighted_changed(true);
	  selection_changed(&*selected);
	  toplevel::update();
	}
    }

    void tree::page_up()
    {
      if(root == NULL)
	return;

      int width,height;
      getmaxyx(height,width);

      if(!hierarchical)
	--height;

      int count=height;
      treeiterator newtop=top;
      while(count>0 && newtop!=begin)
	{
	  if(hierarchical)
	    --newtop;
	  else
	    newtop.move_backward_level();
	  count--;
	}

      if(newtop!=top)
	{
	  int l=0;
	  if(selected!=end)
	    (*selected).highlighted_changed(false);
	  selected=top=newtop;
	  while(l<height && selected!=end && !selected->get_selectable())
	    if(hierarchical)
	      ++selected;
	    else
	      selected.move_forward_level();
	  if(l==height || selected==end)
	    selected=top;

	  if(selected!=end)
	    {
	      (*selected).highlighted_changed(true);
	      selection_changed(&*selected);
	    }
	  else
	    selection_changed(NULL);
	  toplevel::update();
	}
    }

    void tree::jump_to_begin()
    {
      if(root == NULL)
	return;

      int width,height;
      getmaxyx(height,width);

      if(!hierarchical)
	--height;

      int l=0;
      treeiterator prev=selected;

      if(selected!=end)
	selected->highlighted_changed(false);

      selected=begin;
      while(l<height && selected!=end && !selected->get_selectable())
	if(hierarchical)
	  ++selected;
	else
	  selected.move_forward_level();
      if(l==height || selected==end)
	selected=begin;

      if(selected!=end)
	{
	  selected->highlighted_changed(true);
	  selection_changed(&*selected);
	}
      else
	selection_changed(NULL);

      if(top!=begin)
	top=begin;

      toplevel::update();
    }

    void tree::jump_to_end()
    {
      if(root == NULL)
	return;

      int width,height;
      getmaxyx(height,width);

      if(!hierarchical)
	--height;

      int l=-1;
      treeiterator last=end,newtop=end,prev=selected;
      if(hierarchical)
	--last;
      else
	last.move_backward_level();
      while(newtop!=begin && newtop!=top && height>0)
	{
	  if(hierarchical)
	    --newtop;
	  else
	    newtop.move_backward_level();
	  --height;
	  l++;
	}

      if(selected!=end)
	selected->highlighted_changed(false);

      selected=last;
      while(l>=0 && selected!=end && !selected->get_selectable())
	{
	  if(hierarchical)
	    --selected;
	  else
	    selected.move_backward_level();
	  l--;
	}
      if(selected==end && l<0)
	selected=last;

      if(selected!=end)
	{
	  selected->highlighted_changed(true);
	  selection_changed(&*selected);
	}
      else
	selection_changed(NULL);

      if(newtop!=top)
	top=newtop;

      toplevel::update();
    }

    void tree::level_line_up()
    {
      if(root == NULL)
	return;

      treeiterator tmp=selected;
      tmp.move_backward_level();
      if(tmp!=end)
	set_selection(tmp);
    }

    void tree::level_line_down()
    {
      if(root == NULL)
	return;

      treeiterator tmp=selected;
      tmp.move_forward_level();
      if(tmp!=end)
	set_selection(tmp);
    }

    bool tree::handle_key(const config::key &k)
    {
      // umm...
      //width++;
      //height++;

      if(selected!=treeiterator(NULL))
	{
	  if(root != NULL && hierarchical && bindings->key_matches(k, "Parent"))
	    {
	      if(!selected.is_root())
		set_selection(selected.get_up());
	    }
	  else if(root != NULL && !hierarchical && prev_level && bindings->key_matches(k, "Left"))
	    {
	      selected->highlighted_changed(false);

	      top=prev_level->top;
	      begin=prev_level->begin;
	      end=prev_level->end;
	      selected=prev_level->selected;

	      flat_frame *next=prev_level->next;
	      delete prev_level;
	      prev_level=next;

	      selected->highlighted_changed(true);
	      selection_changed(&*selected);

	      toplevel::update();
	    }
	  else if(root != NULL && !hierarchical &&
		  selected!=end && selected->get_selectable() &&
		  selected->begin()!=selected->end() &&
		  (bindings->key_matches(k, "Right") ||
		   bindings->key_matches(k, "Confirm")))
	    {
	      selected->highlighted_changed(false);
	      prev_level=new flat_frame(begin, end, top, selected, prev_level);

	      begin=selected->begin();
	      end=selected->end();
	      top=begin;
	      selected=begin;

	      selected->highlighted_changed(true);
	      selection_changed(&*selected);

	      toplevel::update();
	    }
	  else if(bindings->key_matches(k, "Down"))
	    line_down();
	  else if(bindings->key_matches(k, "Up"))
	    line_up();
	  else if(bindings->key_matches(k, "NextPage"))
	    page_down();
	  else if(bindings->key_matches(k, "PrevPage"))
	    page_up();
	  else if(bindings->key_matches(k, "Begin"))
	    jump_to_begin();
	  else if(bindings->key_matches(k, "End"))
	    jump_to_end();
	  else if(bindings->key_matches(k, "LevelUp"))
	    level_line_up();
	  else if(bindings->key_matches(k, "LevelDown"))
	    level_line_down();
	  /*else if(bindings->key_matches(ch, "Search"))
	    {
	    statusedit *ed=new statusedit("Search for: ");
	    ed->entered.connect(sigc::mem_fun(this, &tree::search_for));
	    add_widget(ed);
	    toplevel::update();
	    }
	    else if(bindings->key_matches(ch, "ReSearch"))
	    search_for("");*/
	  else
	    {
	      if(root != NULL && selected!=end && selected->get_selectable() &&
		 selected->dispatch_key(k, this))
		toplevel::update();
	      else
		return widget::handle_key(k);
	    }
	  return true;
	}
      return false;
    }

    void tree::search_for(tree_search_func &matches)
    {
      if(root == NULL)
	return;

      treeiterator curr((selected==treeiterator(NULL))?begin:selected, hierarchical),
	start(curr);
      // Make an iterator that ignores all the rules >=)

      if(curr!=end)
	{
	  if(hierarchical)
	    ++curr;
	  else
	    curr.move_forward_level();

	  // Don't forget this case!
	  if(curr==end)
	    curr=begin;
	}

      while(curr!=start && !matches(*curr))
	{
	  if(hierarchical)
	    ++curr;
	  else
	    curr.move_forward_level();

	  if(curr==end)
	    curr=begin;
	}

      if(curr==start)
	beep();
      else
	{
	  set_selection(curr, true);
	  toplevel::update();
	}
    }

    void tree::search_back_for(tree_search_func &matches)
    {
      if(root == NULL)
	return;

      treeiterator curr((selected == treeiterator(NULL))
			? begin : selected, hierarchical),
	start(curr);

      // Skip the starting location, cycling to the end.
      if(curr != begin)
	{
	  if(hierarchical)
	    --curr;
	  else
	    curr.move_backward_level();
	}
      else
	{
	  if(hierarchical)
	    {
	      curr = end;
	      --curr;
	    }
	  else
	    {
	      treeiterator curr2 = curr;
	      curr2.move_forward_level();

	      while(curr2 != curr)
		{
		  curr = curr2;
		  curr2.move_forward_level();
		}
	    }
	}

      while(curr != start && !matches(*curr))
	{
	  // Code duplication alert
	  if(curr != begin)
	    {
	      if(hierarchical)
		--curr;
	      else
		curr.move_backward_level();
	    }
	  else
	    {
	      if(hierarchical)
		{
		  curr = end;
		  --curr;
		}
	      else
		{
		  treeiterator curr2 = curr;
		  curr2.move_forward_level();

		  while(curr2 != curr)
		    {
		      curr = curr2;
		      curr2.move_forward_level();
		    }
		}
	    }
	}

      if(curr == start)
	beep();
      else
	{
	  set_selection(curr);
	  toplevel::update();
	}
    }

    void tree::paint(const style &st)
    {
      if(root == NULL)
	return;

      int width,height;
      int selectedln=line_of(selected);

      getmaxyx(height,width);

      if(selectedln>height)
	{
	  while(selected!=top && selectedln>height)
	    {
	      if(hierarchical)
		++top;
	      else
		top.move_forward_level();
	      selectedln--;
	    }
	}
      else
	{
	  while(selected!=top && selectedln<0)
	    {
	      if(hierarchical)
		--top;
	      else
		top.move_backward_level();
	      selectedln++;
	    }
	}

      //if(selected!=end && selected->get_selectable())
      //selected->highlighted_changed(true);
      // Some classes need this to update display stuff properly.  For instance,
      // when a new pkg_tree is created, its 'update the status line' signal
      // won't be properly called without this.

      treeiterator i=top;
      int y=0;

      if(!hierarchical && y<height)
	{
	  wstring todisp;

	  // Uh...I'd rather use the iterators to do this..
	  flat_frame *curr=prev_level;
	  while(curr)
	    {
	      if(todisp.empty())
		todisp=curr->selected->label()+todisp;
	      else
		todisp=curr->selected->label()+(L"::"+todisp);
	      curr=curr->next;
	    }

	  if(todisp.empty())
	    todisp = util::transcode(_("TOP LEVEL"));

	  while(todisp.size()<(unsigned) width)
	    todisp+=L" ";

	  apply_style(st+get_style("Header"));
	  mvaddnstr(y, 0, todisp.c_str(), width);

	  ++y;
	}

      // FIXME: this is a hack around nasty edge cases.  All the tree code needs
      //       a rewrite.
      treeiterator prev=i;
      while(y<height && i!=end)
	{
	  treeitem *curr=&*i;

	  style curr_st;

	  if(get_isfocussed() && i==selected && i->get_selectable())
	    curr_st = st+curr->get_highlight_style();
	  else
	    curr_st = st+curr->get_normal_style();

	  apply_style(curr_st);
	  curr->paint(this, y, hierarchical, curr_st);

	  if(hierarchical)
	    ++i;
	  else
	    i.move_forward_level();
	  y++;

	  // FIXME: this is a hack.
	  if(i==prev) // If we hit the end, it will refuse to advance.
	    break;
	  prev=i;
	}
    }

    void tree::dispatch_mouse(short id, int x, int y, int z, mmask_t bstate)
    {
      // Only do something if this system's ncurses has both button 4
      // and button 5 (older ones didn't).
      //
      // TODO: this moves the selection; it should really move 'top'.
#if defined(BUTTON4_PRESSED) && defined(BUTTON5_PRESSED)
      const int mouse_wheel_scroll_lines =
	std::max(1, std::min(getmaxy() - 1, 3));

      if((bstate & BUTTON4_PRESSED) != 0)
	{
	  if((bstate & BUTTON5_PRESSED) == 0)
	    {
	      for(int i = 0; i < mouse_wheel_scroll_lines; ++i)
		line_up();
	    }

	  return;
	}
      else if((bstate & BUTTON5_PRESSED) != 0)
	{
	  for(int i = 0; i < mouse_wheel_scroll_lines; ++i)
	    line_down();

	  return;
	}
#endif


      if(root == NULL)
	return;

      if(!hierarchical)
	--y;

      treeiterator i=top;
      while(y>0 && i!=end)
	{
	  if(hierarchical)
	    ++i;
	  else
	    i.move_forward_level();

	  --y;
	}

      if(y==0 && i!=end)
	{
	  set_selection(i);

	  i->dispatch_mouse(id, x, bstate, this);
	}
    }

    void tree::init_bindings()
    {
      bindings = new config::keybindings(&config::global_bindings);
    }
  }
}
