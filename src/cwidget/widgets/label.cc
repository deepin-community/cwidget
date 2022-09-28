// label.cc

#include "label.h"

#include <cwidget/config/colors.h>
#include <cwidget/toplevel.h>
#include <cwidget/fragment_cache.h>

#include <algorithm>

using namespace std;

namespace cwidget
{
  namespace widgets
  {
    label::label(fragment *f)
      :txt(new fragment_cache(f))
    {
    }

    label::label(const string &_txt, const style &st)
      :txt(new fragment_cache(text_fragment(_txt)))
    {
      set_bg_style(st);
    }

    label::label(const string &_txt)
      :txt(new fragment_cache(text_fragment(_txt)))
    {
    }

    label::label(const wstring &_txt, const style &st)
      :txt(new fragment_cache(text_fragment(_txt)))
    {
      set_bg_style(st);
    }

    label::label(const wstring &_txt)
      :txt(new fragment_cache(text_fragment(_txt)))
    {
    }

    label_ref label::create(const string &txt, const style &st)
    {
      label_ref rval(new label(txt, st));
      rval->decref();
      return rval;
    }

    label_ref label::create(const string &txt)
    {
      label_ref rval(new label(txt));
      rval->decref();
      return rval;
    }

    label_ref label::create(const wstring &txt, const style &st)
    {
      label_ref rval(new label(txt, st));
      rval->decref();
      return rval;
    }

    label_ref label::create(const wstring &txt)
    {
      label_ref rval(new label(txt));
      rval->decref();
      return rval;
    }

    label::~label()
    {
      delete txt;
    }

    bool label::get_cursorvisible()
    {
      return false;
    }

    point label::get_cursorloc()
    {
      return point(0,0);
    }

    void label::set_text(const string &_txt, const style &st)
    {
      set_text(text_fragment(_txt));
      set_bg_style(st);
    }

    void label::set_text(const string &_txt)
    {
      set_text(text_fragment(_txt));
    }

    void label::set_text(const wstring &_txt, const style &st)
    {
      set_text(text_fragment(_txt));
      set_bg_style(st);
    }

    void label::set_text(const wstring &_txt)
    {
      set_text(text_fragment(_txt));
    }

    void label::set_text(fragment *f)
    {
      delete txt;
      txt=new fragment_cache(f);
      // Our size might have changed, so re-layout the screen.
      toplevel::queuelayout();
    }

    void label::paint(const style &st)
    {
      fragment_contents lines=txt->layout(getmaxx(), getmaxx(), st);

      for(size_t i=0; i<lines.size() && i<(unsigned) getmaxy(); ++i)
	mvaddnstr(i, 0, lines[i], lines[i].size());
    }

    int label::width_request()
    {
      return txt->max_width(0, 0);
    }

    int label::height_request(int width)
    {
      return txt->layout(width, width, style()).size();
    }

    bool transientlabel::handle_char(chtype ch)
    {
      destroy();
      return true;
    }
  }
}
