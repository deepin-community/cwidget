// button.h      -*-c++-*-
//
//  A button is just a widget which accepts keyboard focus and can be
// "pressed".  I'm going to make a stab at sharing code between
// normal buttons, radio buttons, and checkbuttons..this may not be
// worth it..

#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

#include <string>

namespace cwidget
{
  class fragment;
  class fragment_cache;

  namespace widgets
  {
    /** This class represents a push-button. */
    class button : public widget
    {
      fragment_cache *label;

      void accept_focus();
      void lose_focus();

    protected:
      bool handle_key(const config::key &k);
      fragment_cache *get_label() const {return label;}

      /** Instantiate a button.
       *
       *  \param _label the new label of this button; it will be placed
       *  inside a simple text_fragment.
       */
      button(const std::wstring &_label);
      button(fragment *_label);
      button(const std::string &_label);
    public:

      ~button();

      static util::ref_ptr<button>
      create(const std::wstring &label)
      {
	util::ref_ptr<button> rval(new button(label));
	// Remove the initial construction reference.
	rval->decref();
	return rval;
      }

      /** Instantiate a button.
       *
       *  \param _label the new label of this button; the button is
       *  responsible for deleting it.
       */
      static util::ref_ptr<button> create(fragment *label)
      {
	util::ref_ptr<button> rval(new button(label));
	rval->decref();
	return rval;
      }

      /** Instantiate a button.
       *
       *  \param _label the new label of this button; it will be placed
       *  inside a simple text_fragment.
       */
      static util::ref_ptr<button> create(const std::string &label)
      {
	util::ref_ptr<button> rval(new button(label));
	rval->decref();
	return rval;
      }

      void paint(const style &st);

      bool get_cursorvisible();
      point get_cursorloc();
      bool focus_me();

      int width_request();
      int height_request(int width);
      void dispatch_mouse(short id, int x, int y, int z, mmask_t bmask);

      void set_label(const fragment *_label);

      // Signals:

      // The button has been "pressed" (activated)
      sigc::signal0<void> pressed;
    };

    typedef util::ref_ptr<button> button_ref;
  }
}

#endif
