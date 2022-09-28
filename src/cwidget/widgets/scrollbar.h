// scrollbar.h                         -*-c++-*-
//
//   Copyright (C) 2004-2006 Daniel Burrows
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


#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "widget.h"

namespace cwidget
{
  namespace widgets
  {
    class scrollbar : public widget
    {
    public:
      enum direction {HORIZONTAL, VERTICAL};

    private:
      direction dir;

      int max, val;
      // The current slider maximum and value (FIXME: use floats?)

      /** Get the current X or Y location of the slider in the widget. 
       *
       *  \return the slider location, or -1 if it is not visible.
       */
      int get_slider();
    protected:
      scrollbar(direction _dir, int _val, int _max)
	:dir(_dir), max(_max), val(_val) {}

      scrollbar(direction _dir)
	:dir(_dir), max(0), val(0) {}
    public:
      static
      util::ref_ptr<scrollbar> create(direction dir, int val, int max)
      {
	util::ref_ptr<scrollbar> rval(new scrollbar(dir, val, max));
	rval->decref();
	return rval;
      }

      static
      util::ref_ptr<scrollbar> create(direction dir)
      {
	util::ref_ptr<scrollbar> rval(new scrollbar(dir));
	rval->decref();
	return rval;
      }

      void paint(const style &st);

      int width_request();
      int height_request(int w);

      bool get_cursorvisible();
      point get_cursorloc();
      void dispatch_mouse(short id, int x, int y, int z, mmask_t bstate);

      void set_slider(int newval, int newmax);

      /** This signal is emitted if the user "pages up" or "pages down"
       *  via the scrollbar.  Its argument is \b true for a "page up" and
       *  \b false for a "page down".
       */
      sigc::signal1<void, bool> scrollbar_interaction;
    };

    typedef util::ref_ptr<scrollbar> scrollbar_ref;
  }
}

#endif
