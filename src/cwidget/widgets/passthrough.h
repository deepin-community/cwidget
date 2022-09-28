// passthrough.h                -*-c++-*-
//
//  A widget that by default passes focus and cursor handling through to
// a "currently focussed" widget.

#ifndef PASSTHROUGH_H
#define PASSTHROUGH_H

#include "container.h"

namespace cwidget
{
  namespace widgets
  {
    class passthrough:public container
    {
      void gained_focus();
      void lost_focus();

    protected:
      virtual bool handle_key(const config::key &k);

      // These call focussed() and unfocussed() on the result of get_focus().
      // (convenience methods)
      //
      // Provided to make it easier to manage focus simply.
      void defocus();
      void refocus();

    protected:
      passthrough();

    public:
      // Returns the currently focussed widget, if any.
      virtual widget_ref get_focus()=0;

      widget_ref get_active_widget();

      virtual void dispatch_mouse(short id, int x, int y, int z, mmask_t bstate);

      virtual bool focus_me();
      virtual bool get_cursorvisible();
      virtual point get_cursorloc();
    };

    typedef util::ref_ptr<passthrough> passthrough_ref;
  }
}

#endif
