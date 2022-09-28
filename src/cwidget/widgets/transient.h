// transient.h  -*-c++-*-
//
//   Copyright 2005 Daniel Burrows

#ifndef TRANSIENT_H
#define TRANSIENT_H

#include "bin.h"

namespace cwidget
{
  namespace widgets
  {
    /** This class is a visually transparent wrapper around another
     *  widget.  It captures all keystrokes (preventing the subwidget from
     *  recieving them), and destroys itself upon receiving one.
     */
    class transient : public bin
    {
    private:
      /** Handle layout: the subwidget is assigned the entire area of this
       *  widget.
       */
      void layout_me();

    protected:
      transient(const widget_ref &w);
    public:
      /** Create a new transient.
       *
       *  \param w the widget to place inside the transient wrapper.
       */
      static util::ref_ptr<transient>
      create(const widget_ref &w = NULL)
      {
	util::ref_ptr<transient> rval(new transient(w));
	rval->decref();
	return rval;
      }

      /** \return the desired width of the subwidget. */
      int width_request();

      /** Calculate the desired height of the subwidget.
       *
       *  \param width the width of this widget
       *  \return the desired height
       */
      int height_request(int width);

      /** \return \b true: transients can always be focussed. */
      bool focus_me();

      /** Destroy the transient.
       *
       *  \return \b true.
       */
      bool handle_char(chtype ch);
    };

    typedef util::ref_ptr<transient> transient_ref;
  }
}

#endif // TRANSIENT_H
