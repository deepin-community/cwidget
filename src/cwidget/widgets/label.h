// label.h          -*-c++-*-

#ifndef LABEL_H
#define LABEL_H

#include "widget.h"

namespace cwidget
{
  class fragment;
  class fragment_cache;

  namespace widgets
  {
    /** label widgets display some (possibly formatted) text
     *  statically.  The text cannot be scrolled or selected in any way;
     *  if there isn't room for it, it just gets clipped.
     *
     *  Passing a "background" style into the constructor modifies the
     *  background style of the widget (as set_bg_style would); this
     *  differs from wrapping the text in a style_fragment in that it
     *  even affects parts of the widget which aren't covered by text.
     */
    class label : public widget
    {
      fragment_cache *txt;
    protected:
      label(fragment *f);
      label(const std::string &_txt, const style &st);
      label(const std::string &_txt);
      label(const std::wstring &_txt, const style &st);
      label(const std::wstring &_txt);

    public:
      static util::ref_ptr<label> create(fragment *f)
      {
	util::ref_ptr<label> rval(new label(f));
	rval->decref();
	return rval;
      }

      /** Create a label with the given text and background. */
      static util::ref_ptr<label> create(const std::string &txt, const style &st);

      /** Create a label with the given text. */
      static util::ref_ptr<label> create(const std::string &txt);

      /** Create a label with the given text and background. */
      static util::ref_ptr<label> create(const std::wstring &txt, const style &st);

      /** CReate a label with the given text. */
      static util::ref_ptr<label> create(const std::wstring &txt); 


      ~label();

      bool get_cursorvisible();
      point get_cursorloc();

      /** \return the maximum width of any line in the label. */
      int width_request();

      /** \return the number of lines in the label. */
      int height_request(int width);

      void paint(const style &st);
      void set_text(const std::string &_txt, const style &st);
      void set_text(const std::string &_txt);
      void set_text(const std::wstring &_txt, const style &st);
      void set_text(const std::wstring &_txt);
      void set_text(fragment *f);
    };

    class transientlabel:public label
    // Displays a transient message -- grabs the input focus and vanishes when a
    // key is pressed.
    {
    protected:
      virtual bool handle_char(chtype ch);

      transientlabel(const std::string &msg, const style &st)
	:label(msg, st)
      {
      }
    public:
      static
      util::ref_ptr<transientlabel> create(const std::string &msg,
					const style &st)
      {
	return new transientlabel(msg, st);
      }

      bool focus_me() {return true;}
    };

    typedef util::ref_ptr<label> label_ref;

    typedef util::ref_ptr<transientlabel> transientlabel_ref;
  }
}

#endif
