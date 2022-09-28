// layout_item.h                                       -*-c++-*-
//
//   Copyright (C) 2004-2005, 2007 Daniel Burrows
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
// A bridge from the layout code to the tree code.

#ifndef LAYOUT_ITEM_H
#define LAYOUT_ITEM_H

#include "text_layout.h"
#include "treeitem.h"

namespace cwidget
{
  class fragment;

  namespace widgets
  {
    class layout_item : public treeitem
    {
    protected:
      class layout_line;

    private:
      typedef std::vector<layout_line *> child_list;

      child_list children;
      fragment *f;
      fragment_contents lines;

      int lastw;
      int lastbasex;

    protected:
      class layout_line:public treeitem
      {
	int n;
	layout_item &parent;
      public:
	layout_line(int _n, layout_item &_parent);

	void paint(tree *win, int y, bool hierarchical,
		   const style &st);

	const wchar_t *tag();
	const wchar_t *label();
      };

      // Assumes that children.size()>0
      class levelref:public tree_levelref
      {
	size_t item_num;
	const child_list &lines;

      public:
	levelref(const levelref &x);
	levelref(size_t n, const child_list &_lines);

	treeitem *get_item();
	virtual void advance_next();
	virtual void return_prev();
	bool is_begin();
	bool is_end();
	levelref *clone() const;
      };

    public:
      layout_item(fragment *f);

      const wchar_t *tag();
      const wchar_t *label();

      /** Paints the nth line of this item at the given location in 'win'. */
      void paint_line(int n,
		      tree *win, int y, bool hierarchical,
		      const style &st);
      void paint(tree *win, int y, bool hierarchical,
		 const style &st);

      int get_normal_attr();

      levelref *begin();
      levelref *end();
      bool has_visible_children();

      const fragment_line &get_line(tree *win, size_t n, int basex,
				    const style &st);

      ~layout_item();
    };
  }
}

#endif
