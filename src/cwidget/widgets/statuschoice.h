// statuschoice.h   -*-c++-*-
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
// A single-line widget that lets the user choose between one of
// several options with a keystroke.  The options should probably be
// typable characters (as opposed to, say, left-arrow)
//
// The options are given in a string, of which the first character
// should be the default value.  This is the value chosen if the user
// presses Return.  The string should be non-empty.

#ifndef STATUSCHOICE_H
#define STATUSCHOICE_H

#include <string>
#include <cwidget/generic/util/eassert.h>

#include "widget.h"

namespace cwidget
{
  namespace config
  {
    class keybindings;
  }

  namespace widgets
  {
    class statuschoice : public widget
    {
      std::wstring prompt;
      std::wstring choices;
      // A string containing all possible choices; the first one is considered
      // to be the "default".

    protected:
      bool handle_key(const config::key &k);

      statuschoice(const std::wstring &_prompt, const std::wstring &_choices)
	: widget(), prompt(_prompt), choices(_choices)
      {
	eassert(choices.size()>0);
      }

    public:
      static util::ref_ptr<statuschoice> create(const std::wstring &prompt,
						   const std::wstring &choices)
      {
	util::ref_ptr<statuschoice> rval(new statuschoice(prompt, choices));
	rval->decref();
	return rval;
      }

      int width_request();
      int height_request(int w);

      bool get_cursorvisible();
      point get_cursorloc();

      bool focus_me() {return true;}

      void paint(const style &st);

      sigc::signal1<void, int> chosen;
      // Called when one of the choices is selected (the arguments is the
      // position of the choice in the "choices" string)

      static config::keybindings *bindings;
      static void init_bindings();
    };

    typedef util::ref_ptr<statuschoice> statuschoice_ref;
  }
}

#endif
