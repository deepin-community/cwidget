// scrollbar.cc			-*-c++-*-
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


#include "scrollbar.h"

#include <cwidget/toplevel.h>

namespace cwidget
{
  namespace widgets
  {
    int scrollbar::get_slider()
    {
      widget_ref tmpref(this);

      int width = dir==HORIZONTAL?getmaxx():getmaxy();
      return max==0?-1:(width-1)*val/max;
    }

    void scrollbar::paint(const style &st)
    {
      widget_ref tmpref(this);

      if(dir==HORIZONTAL)
	{
	  int width=getmaxx();
	  int thumbloc=get_slider();

	  for(int x=0; x<width; x++)
	    if(x==thumbloc)
	      mvadd_wch(0, x, L'#');
	    else
	      mvadd_wch(0, x, WACS_CKBOARD);
	}
      else
	{
	  int height=getmaxy();
	  int thumbloc=get_slider();

	  for(int y=0; y<height; y++)
	    if(y==thumbloc)
	      mvadd_wch(y, 0, L'#');
	    else
	      mvadd_wch(y, 0, WACS_CKBOARD);
	}
    }

    int scrollbar::width_request()
    {
      return 1;
    }

    int scrollbar::height_request(int w)
    {
      return 1;
    }

    void scrollbar::set_slider(int newval, int newmax)
    {
      if(max!=newmax || val!=newval)
	{
	  max=newmax;
	  val=newval;

	  toplevel::update();
	}
    }

    bool scrollbar::get_cursorvisible()
    {
      return false;
    }

    point scrollbar::get_cursorloc()
    {
      return point(0, 0);
    }

    void scrollbar::dispatch_mouse(short id,
				   int x, int y, int z,
				   mmask_t bstate)
    {
      widget_ref tmpref(this);

      int slider_loc=get_slider();
      int mloc = dir==HORIZONTAL?x:y;

      if(slider_loc!=-1)
	{
	  if(mloc>slider_loc)
	    scrollbar_interaction(false);
	  else
	    scrollbar_interaction(true);
	}
    }
  }
}
