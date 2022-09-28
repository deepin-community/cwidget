// editline.cc
//
//   Copyright (C) 2000, 2007 Daniel Burrows
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

#include "editline.h"

#include <cwidget/config/colors.h>
#include <cwidget/config/keybindings.h>
#include <cwidget/toplevel.h>
#include <cwidget/generic/util/transcode.h>

#include <sigc++/functors/mem_fun.h>

using namespace std;

namespace cwidget
{
  namespace widgets
  {
    config::keybindings *editline::bindings = NULL;

    editline::editline(const string &_prompt, const string &_text,
		       history_list *_history)
      : widget(), curloc(_text.size()),
	startloc(0), desired_size(-1), history(_history),
	history_loc(0), using_history(false), allow_wrap(false), clear_on_first_edit(false)
    {
      // Just spew a partial/null string if errors happen for now.
      util::transcode(_prompt.c_str(), prompt);
      util::transcode(_text.c_str(), text);

      set_bg_style(get_style("EditLine"));

      // This ensures that the cursor is set to the right location when the
      // widget is displayed or resized:
      do_layout.connect(sigc::mem_fun(*this, &editline::normalize_cursor));
    }

    editline::editline(const wstring &_prompt, const wstring &_text,
		       history_list *_history)
      : widget(), prompt(_prompt), text(_text), curloc(_text.size()),
        startloc(0), desired_size(-1), history(_history),
        history_loc(0), using_history(false), allow_wrap(false), clear_on_first_edit(false)
    {
      set_bg_style(get_style("EditLine"));

      // This ensures that the cursor is set to the right location when the
      // widget is displayed or resized:
      do_layout.connect(sigc::mem_fun(*this, &editline::normalize_cursor));
    }

    editline::editline(int maxlength, const string &_prompt,
		       const string &_text, history_list *_history)
      : widget(), curloc(0),
        startloc(0), desired_size(maxlength), history(_history), history_loc(0),
        using_history(false), allow_wrap(false), clear_on_first_edit(false)
    {
      // As above, ignore errors.
      util::transcode(_prompt, prompt);
      util::transcode(_text, text);

      set_bg_style(get_style("EditLine"));
      do_layout.connect(sigc::mem_fun(*this, &editline::normalize_cursor));
    }

    editline::editline(int maxlength, const wstring &_prompt,
		       const wstring &_text, history_list *_history)
      : widget(), prompt(_prompt), text(_text), curloc(0),
        startloc(0), desired_size(maxlength), history(_history), history_loc(0),
        using_history(false), allow_wrap(false), clear_on_first_edit(false)
    {
      set_bg_style(get_style("EditLine"));
      do_layout.connect(sigc::mem_fun(*this, &editline::normalize_cursor));
    }

    wchar_t editline::get_char(size_t loc)
    {
      widget_ref tmpref(this);

      if(loc>=prompt.size())
	return text[loc-prompt.size()];
      else
	return prompt[loc];
    }

    void editline::normalize_cursor()
    {
      widget_ref tmpref(this);

      if(get_width() <= 0 || get_height() <= 0)
	return;

      if(!allow_wrap)
	{
	  int w=get_width();

	  int promptwidth=wcswidth(prompt.c_str(), prompt.size());
	  int textwidth=wcswidth(text.c_str(), text.size());

	  int cursorx=0;
	  if(curloc+prompt.size()>startloc)
	    for(size_t i=startloc; i<curloc+prompt.size(); ++i)
	      cursorx+=wcwidth(get_char(i));
	  else
	    for(size_t i=curloc+prompt.size(); i<startloc; ++i)
	      cursorx-=wcwidth(get_char(i));

	  if(promptwidth+textwidth+1<w)
	    startloc=0;
	  else if(w>2)
	    {
	      // Need to move the screen start to this far behind the cursor
	      // loc.
	      int decamt=0;
	      bool needs_move=false;

	      if(cursorx>=w-2)
		{
		  decamt=w-2;
		  needs_move=true;
		}
	      else if(cursorx<2)
		{
		  decamt=2;
		  needs_move=true;
		}

	      if(needs_move) // equivalent to but more readable than decamt!=0
		{
		  // Do it by moving back this many chars
		  size_t chars=0;

		  while(decamt>0 && chars<curloc+prompt.size())
		    {
		      ++chars;
		      decamt-=wcwidth(get_char(prompt.size()+curloc-chars));
		    }

		  if(decamt<0 && chars>1)
		    --chars;

		  startloc=curloc+prompt.size()-chars;
		}
	    }
	  else
	    {
	      // if width=1, use a primitive approach (we're screwed anyway in
	      // this case)
	      if(cursorx>=w)
		startloc=prompt.size()+curloc-w+1;

	      if(cursorx<0)
		startloc=prompt.size()+curloc;
	    }

	  toplevel::updatecursor();
	}
      else
	{
	  const int width         = get_width();
	  const int startline     = get_line_of_character(startloc, width);
	  const int currline      = get_line_of_character(curloc,   width);
	  const int lastline      = get_line_of_character(get_num_chars(), width);
	  const int height        = get_height();

	  int newstartline = startline;

	  if(currline < startline)
	    newstartline = currline;
	  else if(currline - startline >= height)
	    newstartline = currline - (height - 1);

	  // If there are empty lines at the end and we could display all
	  // the text, scroll up.
	  if(newstartline > 0 && newstartline + height > lastline + 1)
	    {
	      if(lastline + 1 >= height)
		newstartline = lastline + 1 - height;
	      else
		newstartline = 0;
	    }

	  if(newstartline != startline)
	    {
	      startloc = get_character_of_line(newstartline, width);
	      toplevel::update();
	    }
	}
    }

    bool editline::get_cursorvisible()
    {
      return true;
    }

    point editline::get_cursorloc()
    {
      widget_ref tmpref(this);

      if(getmaxx()>0)
	{
	  int x=0;

	  const int width        = getmaxx();
	  const size_t whereami  = curloc + prompt.size();
	  const int curline      = get_line_of_character(whereami, width);
	  const int startline    = get_line_of_character(startloc, width);
	  const int curlinestart = get_character_of_line(curline,  width);
	  for(size_t loc = curlinestart; loc < whereami; ++loc)
	    x+=wcwidth(get_char(loc));

	  return point(x, curline - startline);
	}
      else
	return point(0,0);
    }

    bool editline::focus_me()
    {
      return true;
    }

    bool editline::handle_key(const config::key &k)
    {
      widget_ref tmpref(this);

      bool clear_on_this_edit = clear_on_first_edit;
      clear_on_first_edit = false;

      if(bindings->key_matches(k, "DelBack"))
	{
	  if(curloc>0)
	    {
	      text.erase(--curloc, 1);
	      normalize_cursor();
	      text_changed(wstring(text));
	      toplevel::queuelayout();
	    }
	  else
	    {
	      beep();
	      //	  refresh();
	    }
	  return true;
	}
      else if(bindings->key_matches(k, "DelForward"))
	{
	  if(curloc<text.size())
	    {
	      text.erase(curloc, 1);
	      normalize_cursor();
	      text_changed(wstring(text));
	      toplevel::queuelayout();
	    }
	  else
	    {
	      beep();
	      //	  refresh();
	    }
	  return true;
	}
      else if(bindings->key_matches(k, "Confirm"))
	{
	  // I create a new string here because otherwise modifications to
	  // "text" are seen by the widgets! (grr, sigc++)
	  entered(wstring(text));
	  return true;
	}
      else if(bindings->key_matches(k, "Left"))
	{
	  if(curloc>0)
	    {
	      curloc--;
	      normalize_cursor();
	      toplevel::update();
	    }
	  else
	    {
	      beep();
	      //	  refresh();
	    }
	  return true;
	}
      else if(bindings->key_matches(k, "Right"))
	{
	  if(curloc<text.size())
	    {
	      curloc++;
	      normalize_cursor();
	      toplevel::update();
	    }
	  else
	    {
	      beep();
	      //	  refresh();
	    }
	  return true;
	}
      else if(bindings->key_matches(k, "Begin"))
	{
	  curloc=0;
	  startloc=0;
	  normalize_cursor();
	  toplevel::update();
	  return true;
	}
      else if(bindings->key_matches(k, "End"))
	{
	  curloc=text.size();
	  normalize_cursor();
	  toplevel::update();
	  return true;
	}
      else if(bindings->key_matches(k, "DelEOL"))
	{
	  text.erase(curloc);
	  normalize_cursor();
	  text_changed(wstring(text));
	  toplevel::queuelayout();
	  return true;
	}
      else if(bindings->key_matches(k, "DelBOL"))
	{
	  text.erase(0, curloc);
	  curloc=0;
	  normalize_cursor();
	  text_changed(wstring(text));
	  toplevel::queuelayout();
	  return true;
	}
      else if(history && bindings->key_matches(k, "HistoryPrev"))
	{
	  if(history->size()==0)
	    return true;

	  if(!using_history)
	    {
	      using_history=true;
	      history_loc=history->size()-1;
	      pre_history_text=text;
	    }
	  else if(history_loc>0)
	    --history_loc;
	  else
	    // Break out
	    return true;

	  text=(*history)[history_loc];
	  curloc=text.size();
	  startloc=0;
	  normalize_cursor();
	  text_changed(wstring(text));
	  toplevel::queuelayout();

	  return true;
	}
      else if(history && bindings->key_matches(k, "HistoryNext"))
	{
	  if(history->size()==0 || !using_history)
	    return true;

	  if(history_loc>=history->size()-1)
	    {
	      using_history=false;
	      history_loc=0;
	      text=pre_history_text;
	      pre_history_text=L"";
	      curloc=text.size();
	      startloc=0;
	      normalize_cursor();
	      text_changed(wstring(text));
	      toplevel::queuelayout();

	      // FIXME: store the pre-history edit and restore that.
	      return true;
	    }

	  if(history_loc>=0)
	    {
	      ++history_loc;
	      text=(*history)[history_loc];
	      curloc=text.size();
	      startloc=0;
	      normalize_cursor();
	      text_changed(wstring(text));
	      toplevel::queuelayout();

	      return true;
	    }
	  else
	    return true;
	}
      else if(k.function_key)
	return widget::handle_key(k);
      else if(k.ch=='\t') // HACK
	return false;
      else
	{
	  if(clear_on_this_edit)
	    {
	      text.clear();
	      curloc = 0;
	      startloc = 0;
	    }

	  text.insert(curloc++, 1, k.ch);
	  normalize_cursor();
	  text_changed(wstring(text));
	  toplevel::queuelayout();
	  return true;
	}
    }

    int editline::get_line_of_character(size_t n, int width)
    {
      if(!allow_wrap)
	return 0;

      int rval = 0;
      int curr_line_width = 0;
      for(size_t i = 0; i < n && i < get_num_chars(); ++i)
	{
	  wchar_t ch = get_char(i);
	  int ch_width = wcwidth(ch);
	  if(curr_line_width + ch_width > width)
	    {
	      ++rval;
	      curr_line_width = ch_width;
	    }
	  else
	    {
	      curr_line_width += ch_width;
	      if(curr_line_width == width)
		{
		  ++rval;
		  curr_line_width = 0;
		}
	    }
	}

      return rval;
    }

    int editline::get_character_of_line(size_t n, int width)
    {
      if(!allow_wrap)
	return startloc;

      size_t curr_line = 0;
      int curr_line_width = 0;
      size_t i;
      for(i = 0; curr_line < n && i < get_num_chars(); ++i)
	{
	  wchar_t ch = get_char(i);
	  int ch_width = wcwidth(ch);
	  if(curr_line_width + ch_width > width)
	    {
	      ++curr_line;
	      curr_line_width = ch_width;
	    }
	  else
	    {
	      curr_line_width += ch_width;
	      if(curr_line_width == width)
		{
		  ++curr_line;
		  curr_line_width = 0;
		}
	    }
	}

      return (signed)i;
    }

    void editline::paint(const style &st)
    {
      if(getmaxy() == 0)
	return;

      widget_ref tmpref(this);

      int width=getmaxx();
      int y = 0;
      const int height = allow_wrap ? getmaxy() : 1;
      size_t linestart = startloc;

      wstring todisp = prompt + text;
      while(y < height && linestart < prompt.size() + text.size())
	{
	  int used = 0;
	  size_t chars = 0;

	  while(used < width && linestart + chars < prompt.size() + text.size())
	    {
	      wchar_t ch = get_char(linestart + chars);
	      used += wcwidth(ch);
	      ++chars;
	    }

	  if(used > width && chars > 1)
	    --chars;

	  mvaddstr(y, 0, wstring(todisp, linestart, chars));

	  ++y;
	  linestart += chars;
	}
    }

    void editline::dispatch_mouse(short id, int x, int y, int z, mmask_t bstate)
    {
      widget_ref tmpref(this);

      if(!allow_wrap)
	{
	  if(y > 0)
	    return;
	}

      const int width        = get_width();
      const int curlinestart = get_character_of_line(y, width);

      // mouseloc is used to compute the character at which the mouse
      // press occurred.
      size_t mouseloc = curlinestart;

      clear_on_first_edit = false;

      while(mouseloc < prompt.size() + text.size() && x > 0)
	{
	  int curwidth = wcwidth(get_char(mouseloc));

	  if(curwidth > x)
	    break;
	  else
	    {
	      ++mouseloc;
	      x -= curwidth;
	    }
	}

      if(mouseloc >= prompt.size())
	{
	  mouseloc -= prompt.size();

	  if(mouseloc <= text.size())
	    curloc = mouseloc;
	  else
	    curloc = text.size();
	}
      else
	return; // Do nothing if the user clicked on the prompt.

      toplevel::update();
    }

    void editline::add_to_history(std::wstring s,
				  history_list *lst)
    {
      eassert(lst);

      if(lst->empty() || lst->back()!=s)
	lst->push_back(s);
    }

    void editline::add_to_history(std::wstring s)
    {
      widget_ref tmpref(this);

      if(history)
	add_to_history(s, history);
    }

    void editline::reset_history()
    {
      widget_ref tmpref(this);

      pre_history_text=L"";
      using_history=false;
      history_loc=0;
    }

    void editline::set_text(wstring _text)
    {
      widget_ref tmpref(this);

      text=_text;
      if(curloc>text.size())
	curloc=text.size();
      text_changed(wstring(text));
      toplevel::update();
    }

    void editline::set_text(string _text)
    {
      widget_ref tmpref(this);

      wstring wtext;
      if(util::transcode(_text, wtext))
	set_text(wtext);
    }

    void editline::init_bindings()
    {
      bindings = new config::keybindings(&config::global_bindings);

      bindings->set("Left", config::key(KEY_LEFT, true));
      bindings->set("Right", config::key(KEY_RIGHT, true));
      // Override these for the case where left and right have multiple bindings
      // in the global keymap
    }

    int editline::width_request()
    {
      widget_ref tmpref(this);

      if(desired_size == -1)
	return wcswidth(prompt.c_str(), prompt.size())+wcswidth(text.c_str(), text.size());
      else
	return desired_size;
    }

    int editline::height_request(int width)
    {
      if(!allow_wrap)
	return 1;
      else
	{
	  const int last_line = get_line_of_character(prompt.size() + text.size(), width);

	  return last_line + 1;
	}
    }
  }
}
