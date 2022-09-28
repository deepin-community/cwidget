// size_box.h                       -*-c++-*-
//
// A container to ensure that its child has a particular minimum size
// (at least).
//
// Copyright 2004 Daniel Burrows
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

#ifndef SIZE_BOX_H
#define SIZE_BOX_H

#include "bin.h"

namespace cwidget
{
  namespace widgets
  {
    /** A size_box ensures that the requested size of its child is a
     *  given size or larger.
     */
    class size_box:public bin
    {
      size min_size;

      /** Internal: actually allocates the child's size. */
      void layout_me();
    protected:
      size_box(size s, const widget_ref &w);

    public:
      /** Create a size_box.
       *
       *  \param s the minimum size of this box
       *  \param w the widget initially contained in this box (\b NULL to
       *           create an initially empty box)
       */
      static util::ref_ptr<size_box> create(size s, const widget_ref &w=NULL)
      {
	util::ref_ptr<size_box> rval(new size_box(s, w));
	rval->decref();
	return rval;
      }

      /** \return the least upper bound of the minimum size passed to the
       *  constructor and the true size request of the child.
       */
      int width_request();

      /**  \param w the width for which a height should be calculated.
       *
       *   \return the least upper bound of the minimum size passed to the
       *   constructor and the true size request of the child.
       */
      int height_request(int w);
    };

    typedef util::ref_ptr<size_box> size_box_ref;
  }
}

#endif
