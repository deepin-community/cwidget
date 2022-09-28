// center.h        -*-c++-*-
//
//  A simple container/layout widget which centers its child in itself.

#ifndef CENTER_H
#define CENTER_H

#include "bin.h"

namespace cwidget
{
  namespace widgets
  {
    class center:public bin
    {
      void layout_me();

    protected:
      center(const widget_ref &w = NULL);

    public:
      static util::ref_ptr<center> create(const widget_ref &w = NULL)
      {
	util::ref_ptr<center> rval(new center(w));
	rval->decref();
	return rval;
      }

      int width_request();
      int height_request(int width);
    };

    typedef util::ref_ptr<center> center_ref;
  }
}

#endif
