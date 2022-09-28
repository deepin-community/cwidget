// minibuf_win.cc
//
//  Copyright 2000-2005 Daniel Burrows
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.

#include "minibuf_win.h"
#include "label.h"
#include "multiplex.h"

#include <cwidget/toplevel.h>

#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>

using namespace std;

namespace cwidget
{
  namespace widgets
  {
    minibuf_win::minibuf_win()
      : passthrough(), main_widget(NULL)
    {
      do_layout.connect(sigc::mem_fun(*this, &minibuf_win::layout_me));

      status=multiplex::create();
      status_lbl=label::create("");
      status_lbl->set_bg_style(get_style("Status"));
      status->add_widget(status_lbl);
      header=label::create("");
      header->set_bg_style(get_style("Header"));

      status->set_owner(this);
      header->set_owner(this);
      status_lbl->show();
      status->show();
      header->show();
    }

    minibuf_win::~minibuf_win()
    {
    }

    void minibuf_win::destroy()
    {
      widget_ref tmpref(this);

      if(main_widget.valid())
	main_widget->destroy();
      eassert(!main_widget.valid());

      header->destroy();
      status->destroy();

      eassert(!header.valid());
      eassert(!status.valid());

      container::destroy();
    }

    void minibuf_win::set_main_widget(const widget_ref &w)
    {
      widget_ref tmpref(this);

      defocus();

      if(main_widget.valid())
	{
	  main_destroy_conn.disconnect();
	  main_widget->set_owner(NULL);
	}

      main_widget=w;

      if(main_widget.valid())
	{
	  main_widget->set_owner(this);
	  main_destroy_conn=main_widget->destroyed.connect(sigc::bind(sigc::mem_fun(*this, &minibuf_win::set_main_widget), (widget *) NULL));
	}
      refocus();

      toplevel::queuelayout();
      toplevel::update();
    }

    int minibuf_win::width_request()
    {
      widget_ref tmpref(this);

      int w=0;

      if(status.valid())
	w=max(w, status->width_request());

      if(header.valid())
	w=max(w, header->width_request());

      if(main_widget.valid())
	w=max(w, main_widget->width_request());

      return w;
    }

    int minibuf_win::height_request(int w)
    {
      widget_ref tmpref(this);

      int h=2;

      if(main_widget.valid())
	h=max(h, main_widget->height_request(w));
      return h;
    }

    void minibuf_win::layout_me()
    {
      widget_ref tmpref(this);

      if(header.valid())
	header->alloc_size(0, 0, getmaxx(), 1);

      if(getmaxy()>1)
	{
	  if(getmaxy()>2 && main_widget.valid())
	    main_widget->alloc_size(0, 1, getmaxx(), getmaxy()-2);

	  if(status.valid())
	    status->alloc_size(0, getmaxy()-1, getmaxx(), 1);
	}
    }

    void minibuf_win::paint(const style &st)
    {
      widget_ref tmpref(this);

      if(main_widget.valid() && main_widget->get_visible())
	main_widget->display(st);

      if(status.valid())
	status->display(st);
      if(header.valid())
	header->display(st);
    }

    void minibuf_win::set_header(string new_header)
    {
      widget_ref tmpref(this);

      header->set_text(new_header);
    }

    void minibuf_win::set_status(string new_status)
    {
      widget_ref tmpref(this);

      status_lbl->set_text(new_status);
    }

    void minibuf_win::add_widget(const widget_ref &widget)
    {
      widget_ref tmpref(this);

      defocus();
      status->add_widget(widget);
      refocus();
    }

    void minibuf_win::rem_widget(const widget_ref &widget)
    {
      widget_ref tmpref(this);

      eassert(widget.valid());

      if(widget == header)
	{
	  header->set_owner(NULL);
	  header = NULL;
	}
      else if(widget == status)
	{
	  status->set_owner(NULL);
	  status = NULL;
	}
      else if(widget == main_widget)
	{
	  main_widget->set_owner(NULL);
	  main_widget = NULL;
	}
      else
	{
	  defocus();
	  status->rem_widget(widget);
	  refocus();
	}
    }

    void minibuf_win::show_all()
    {
      widget_ref tmpref(this);

      if(main_widget.valid())
	main_widget->show_all();
      status->show();
      header->show();
    }

    widget_ref minibuf_win::get_focus()
    {
      widget_ref tmpref(this);

      if(status.valid() && status->focus_me())
	return status;
      else if(main_widget.valid() && main_widget->get_visible() && main_widget->focus_me())
	return main_widget;
      else
	return NULL;
    }

    void minibuf_win::display_error(string err)
    {
      add_widget(transientlabel::create(err, get_style("Error")));
    }
  }
}
