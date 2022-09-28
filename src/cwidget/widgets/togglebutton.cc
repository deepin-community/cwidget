// togglebutton.cc

#include "togglebutton.h"

#include <cwidget/toplevel.h>
#include <cwidget/fragment.h>
#include <cwidget/fragment_cache.h>

#include <algorithm>

using namespace std;

namespace cwidget
{
  namespace widgets
  {
    togglebutton::togglebutton(char _bracketl, char _mark, char _bracketr,
			       fragment *_label, bool _checked)
      : button(_label), checked(_checked),
	bracketl(_bracketl), mark(_mark), bracketr(_bracketr)
    {
    }

    togglebutton::togglebutton(char _bracketl, char _mark, char _bracketr,
			       const std::string &_label, bool _checked)
 :button(_label), checked(_checked),
  bracketl(_bracketl), mark(_mark), bracketr(_bracketr)
    {
    }

    void togglebutton::paint_check(int row)
    {
      mvaddch(row, 0, bracketl);

      if(checked)
	addch(mark);
      else
	addch(' ');

      addch(bracketr);
    }

    point togglebutton::get_cursorloc()
    {
      return point(0, getmaxy()/2);
    }

    void togglebutton::paint(const style &st)
    {
      const size_t labelw=getmaxx()>=4?getmaxx()-4:0;
      const fragment_contents lines=get_label()->layout(labelw, labelw, st);
      const size_t checkheight=getmaxy()/2;

      const style button_style=get_isfocussed()?st+style_attrs_flip(A_REVERSE):st;

      for(size_t i=0; i<min<size_t>(lines.size(), getmaxy()); ++i)
	{
	  if(i==checkheight)
	    {
	      apply_style(button_style);

	      paint_check(i);

	      apply_style(st);
	    }

	  mvaddnstr(i, 4, lines[i], lines[i].size());
	}
    }

    void togglebutton::do_toggle()
    {
      checked=!checked;
      toggled();
      toplevel::update();
    }

    void togglebutton::silent_set_checked(bool _checked)
    {
      if(checked!=_checked)
	{
	  checked=_checked;
	  toplevel::update();
	}
    }
  }
}
