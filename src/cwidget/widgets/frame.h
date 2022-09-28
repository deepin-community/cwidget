// frame.h        -*-c++-*-
//
// A container that draws a frame around the widget it contains.
// (needs a lot more work to gracefully handle layout issues :) )

#ifndef FRAME_H
#define FRAME_H

#include "bin.h"

namespace cwidget
{
  namespace widgets
  {
    class frame : public bin
    {
      void layout_me();

    protected:
      frame(const widget_ref &w);

    public:
      static util::ref_ptr<frame> create(const widget_ref &w)
      {
	util::ref_ptr<frame> rval(new frame(w));
	rval->decref();
	return rval;
      }

      /** \return the desired width of the frame.  A frame is 2 larger
       *   than its contents in every direction.
       */
      int width_request();

      /** Calculate the desired height of the frame.  A frame is 2 larger
       *  than its contents in every direction.
       *
       *  \param width the width of the frame
       *  \return the desired height
       */
      int height_request(int width);

      virtual void paint(const style &st);
    };

    typedef util::ref_ptr<frame> frame_ref;
  }
}

#endif
