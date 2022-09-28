// togglebutton.h			-*-c++-*-
//
//  I like having a "togglable" button which doesn't force a particular
// policy..it makes radio buttons much easier to do right.

#ifndef TOGGLEBUTTON_H
#define TOGGLEBUTTON_H

#include "button.h"

#include <sigc++/functors/mem_fun.h>

namespace cwidget
{
  namespace widgets
  {
    class togglebutton:public button
    {
      bool checked;
      char bracketl, mark, bracketr;

      void paint_check(int row);

    protected:
      void silent_set_checked(bool _checked);
      // to be used mainly to avoid emitting signals (eg, if you're trying to
      // coordinate a togglebutton with an underlying option)

      togglebutton(char _bracketl, char _mark, char _bracketr,
		   fragment *_label, bool _checked);

      togglebutton(char _bracketl, char _mark, char _bracketr,
		   const std::string &_label, bool _checked);

    public:
      static util::ref_ptr<togglebutton>
      create(char bracketl, char mark, char bracketr,
	     fragment *label, bool checked = false)
      {
	util::ref_ptr<togglebutton>
	  rval(new togglebutton(bracketl, mark, bracketr,
				label, checked));
	rval->decref();
	return rval;
      }

      static util::ref_ptr<togglebutton>
      create(char bracketl, char mark, char bracketr,
	     const std::string &label, bool checked = false)
      {
	util::ref_ptr<togglebutton>
	  rval(new togglebutton(bracketl, mark, bracketr,
				label, checked));
	rval->decref();
	return rval;
      }

      point get_cursorloc();

      void paint(const style &st);

      bool get_checked() {return checked;}
      void set_checked(bool _checked)
      {
	if(checked!=_checked)
	  do_toggle();
      }

      // named "do_toggle" to avoid typos wrt "toggled"
      void do_toggle();

      sigc::signal0<void> toggled;
    };

    class checkbutton:public togglebutton
    {
    protected:
      checkbutton(fragment *_label, bool _checked)
	:togglebutton('[', 'X', ']', _label, _checked)
      {
	pressed.connect(sigc::mem_fun(*this, &togglebutton::do_toggle));
      }

      checkbutton(const std::string &_label, bool _checked)
	:togglebutton('[', 'X', ']', _label, _checked)
      {
	pressed.connect(sigc::mem_fun(*this, &togglebutton::do_toggle));
      }

      checkbutton(char bracketr, char mark, char bracketl,
		  fragment *_label, bool _checked)
	:togglebutton(bracketr, mark, bracketl, _label, _checked)
      {
	pressed.connect(sigc::mem_fun(*this, &togglebutton::do_toggle));
      }

      checkbutton(char bracketr, char mark, char bracketl,
		  const std::string &_label, bool _checked)
	:togglebutton(bracketr, mark, bracketl, _label, _checked)
      {
	pressed.connect(sigc::mem_fun(*this, &togglebutton::do_toggle));
      }

    public:
      static util::ref_ptr<checkbutton>
      create(fragment *label, bool checked = false)
      {
	return new checkbutton(label, checked);
      }

      static util::ref_ptr<checkbutton>
      create(const std::string &label, bool checked = false)
      {
	return new checkbutton(label, checked);
      }

      static util::ref_ptr<checkbutton>
      create(char bracketr, char mark, char bracketl,
	     fragment *label, bool checked = false)
      {
	return new checkbutton(bracketr, mark, bracketl,
			       label, checked);
      }

      static util::ref_ptr<checkbutton>
      create(char bracketr, char mark, char bracketl,
	     const std::string &label, bool checked = false)
      {
	return new checkbutton(bracketr, mark, bracketl,
			       label, checked);
      }
    };

    class radiobutton:public togglebutton
    {
    protected:
      radiobutton(fragment *_label, bool _checked)
	:togglebutton('(', '*', ')', _label, _checked)
      {
      }

      radiobutton(const std::string &_label, bool _checked)
	:togglebutton('(', '*', ')', _label, _checked)
      {
      }

    public:
      static util::ref_ptr<radiobutton>
      create(fragment *label, bool checked = false)
      {
	return new radiobutton(label, checked);
      }

      static util::ref_ptr<radiobutton>
      create(const std::string &label, bool checked = false)
      {
	return new radiobutton(label, checked);
      }
    };

    typedef util::ref_ptr<togglebutton> togglebutton_ref;
    typedef util::ref_ptr<checkbutton> checkbutton_ref;
    typedef util::ref_ptr<radiobutton> radiobutton_ref;
  }
}

#endif
