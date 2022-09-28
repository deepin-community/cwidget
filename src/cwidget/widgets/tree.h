// tree.h  (this is -*-c++-*-)
//
//  Copyright 1999-2001, 2004-2007 Daniel Burrows
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
//  A simple tree widget.

#ifndef TREE_H
#define TREE_H

#include "treeitem.h"
#include "widget.h"

#include <cwidget/generic/util/eassert.h>

namespace cwidget
{
  namespace config
  {
    class keybindings;
  }

  namespace widgets
  {
    // A predicate on treeitems:
    class tree_search_func
    {
    public:
      virtual bool operator()(const treeitem &item)=0;
      virtual ~tree_search_func() {}
    };

    class tree_search_string:public tree_search_func
    {
      std::wstring s;
    public:
      tree_search_string(const std::wstring &_s):s(_s) {}

      virtual bool operator()(const treeitem &item);
    };

    class tree : public widget
    {
      treeitem *root;
      treeiterator begin, end;

      treeiterator top;
      treeiterator selected;
      // The top item on the current page and the currently selected item.
      // NOTE: it's implicitly assumed in many places in the code that the 
      // currently selected item is visible (ie, on the screen).

      bool hierarchical;
      // If not true, display the tree as a series of "flat thingies".
      // Must be seen to be described :)

      // This structure is used to easily retrace our steps in flat-mode.
      // (it could probably be done without this, but this makes it MUCH simpler)
      // Note that we don't even bother with an STL list here; it's
      // just not worth it.
      struct flat_frame
      {
	treeiterator begin, end, top, selected;

	flat_frame *next;
	flat_frame(treeiterator _begin,
		   treeiterator _end,
		   treeiterator _top,
		   treeiterator _selected,
		   flat_frame *_next)
	  :begin(_begin), end(_end), top(_top), selected(_selected), next(_next) {}
      };
      flat_frame *prev_level;

      int line_of(treeiterator item);
      bool item_visible(treeiterator item);

      void do_shown();
    protected:
      void sync_bounds();
      // This is an awful hack; I've been thinking about an alternate design of
      // the tree code for a while, and this just confirms it.  Yuck! :)
      //  It'll be the first thing to be removed in the next version..
      //  -- well, it wasn't.

      virtual bool handle_key(const config::key &k);

    protected:
      tree();
      tree(treeitem *_root, bool showroot);

    public:
      static util::ref_ptr<tree>
      create()
      {
	util::ref_ptr<tree> rval(new tree);
	rval->decref();
	return rval;
      }

      static util::ref_ptr<tree>
      create(treeitem *root, bool showroot = false)
      {
	util::ref_ptr<tree> rval(new tree(root, showroot));
	rval->decref();
	return rval;
      }

      void set_root(treeitem *_root, bool showroot=false);

      /** \return the desired width of the widget. */
      int width_request();

      /** \param w the width of the widget.
       *
       *  \return the desired height of the widget for the given width.
       */
      int height_request(int w);

      bool get_cursorvisible();
      point get_cursorloc();
      virtual bool focus_me() {return true;}
      virtual void paint(const style &st);
      virtual void dispatch_mouse(short id, int x, int y, int z, mmask_t bstate);

      /** \brief Directly sets the selection to the given element.
       *
       *  \param to The element to select.
       *  \param force_to_top If true, the element will be placed at the
       *  top of the list if the list scrolls.
       *
       *  If the element's parents are not currently expanded, they will
       *  be immediately expanded.
       */
      void set_selection(treeiterator to, bool force_to_top = false);

      /** \brief Retrieve a reference to the currently selected entry in
       *  the tree.
       *
       *  This iterator might be invalidated by user interactions and
       *  should not be saved.
       */
      treeiterator get_selection() const
      {
	return selected;
      }

      /** \brief Retrieve an iterator referencing the start of the tree. */
      treeiterator get_begin()
      {
	return begin;
      }

      /** \brief Retrieve an iterator referencing the end of the tree. */
      treeiterator get_end()
      {
	return end;
      }

      virtual ~tree();

      void search_for(tree_search_func &matches);
      void search_for(const std::wstring &s)
      {
	tree_search_string matches(s);
	search_for(matches);
      }

      void search_back_for(tree_search_func &matches);
      void search_back_for(const std::wstring &s)
      {
	tree_search_string matches(s);
	search_back_for(matches);
      }

      void set_hierarchical(bool _hierarchical);
      bool get_hierarchical() {return hierarchical;}

      /** Send a 'highlighted' message to the currently selected item. */
      void highlight_current();

      /** Send an 'unhighlighted' message to the currently selected item. */
      void unhighlight_current();

      /** \brief Emitted when the selection moves to a new item.
       *
       *  The item will be NULL if nothing is selected.  The signal will
       *  also be emitted when the widget is shown/hidden and when the
       * 
       */
      sigc::signal1<void, treeitem *> selection_changed;

      // Execute the given command
      void line_up();
      void line_down();
      void page_up();
      void page_down();
      void jump_to_begin();
      void jump_to_end();
      void level_line_up();
      void level_line_down();

      static config::keybindings *bindings;
      static void init_bindings();
      // Sets up the bindings..
    };

    typedef util::ref_ptr<tree> tree_ref;
  }
}

#endif
