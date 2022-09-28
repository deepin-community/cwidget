// statuschoice.cc
//
//  Copyright 2000, 2004-2005 Daniel Burrows
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
//  A status widget to let the user choose one of several choices.

#include "statuschoice.h"
#include "minibuf_win.h"

#include <cwidget/config/keybindings.h>

namespace cwidget
{
  namespace widgets
  {
    config::keybindings *statuschoice::bindings = NULL;

    using namespace std;

    bool statuschoice::handle_key(const config::key &k)
    {
      widget_ref tmpref(this);

      if(bindings->key_matches(k, "Confirm"))
	{
	  chosen(0);
	  destroy();
	  return true;
	}
      else if(bindings->key_matches(k, "Cancel"))
	{
	  destroy();
	  return true;
	}
      else if(k.function_key)
	{
	  beep();
	  return true;
	}
      else
	{
	  std::wstring::size_type where=choices.find(k.ch);
	  if(where==wstring::npos)
	    beep();
	  else
	    {
	      chosen(where);
	      destroy();
	    }
	  return true;
	}
    }

    void statuschoice::paint(const style &st)
    {
      wstring todisp=prompt+L" ["+choices[0]+L"]";
      for(unsigned int i=1; i<choices.size(); i++)
	todisp+=choices[i];
      mvaddstr(0, 0, todisp.c_str());
    }

    void statuschoice::init_bindings()
    {
      bindings = new config::keybindings(&config::global_bindings);
    }

    int statuschoice::width_request()
    {
      return wcswidth(prompt.c_str(), prompt.size())
	+wcswidth(choices.c_str(), choices.size()) + 5;
    }

    int statuschoice::height_request(int w)
    {
      return 1;
    }

    bool statuschoice::get_cursorvisible()
    {
      return true;
    }

    point statuschoice::get_cursorloc()
    {
      return point(width_request()-1, 0);
    }
  }
}
