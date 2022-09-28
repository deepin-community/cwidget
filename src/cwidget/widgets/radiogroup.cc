// radiogroup.cc

#include "radiogroup.h"

#include "togglebutton.h"

#include <stdlib.h>

#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>

namespace cwidget
{
  namespace widgets
  {
    radiogroup::radiogroup()
      :selected(items.max_size())
    {
    }

    radiogroup::~radiogroup()
    {
    }

    bool radiogroup::selection_valid()
    {
      return selected != items.max_size();
    }

    int radiogroup::get_selected()
    {
      return items[selected].id;
    }

    void radiogroup::button_pressed(itemlist::size_type index)
    {
      eassert(index<items.size());

      if(selected!=items.max_size())
	items[selected].b->set_checked(false);
      selected=index;

      if(index!=items.max_size())
	{
	  items[index].b->set_checked(true);
	  item_selected(items[index].id);
	}
    }

    void radiogroup::add_button(const togglebutton_ref &b, int id)
    {
      eassert(id>=0);

      for(itemlist::iterator i=items.begin(); i!=items.end(); i++)
	eassert(i->b!=b);

      items.push_back(item(b, id,
			   b->destroyed.connect(sigc::bind(sigc::mem_fun(*this, &radiogroup::rem_button_bare), b.weak_ref())),
			   b->pressed.connect(sigc::bind(sigc::mem_fun(*this, &radiogroup::button_pressed), items.size()))));

      if(selected==items.max_size() || b->get_checked())
	button_pressed(items.size()-1);
    }

    void radiogroup::rem_button(const togglebutton_ref &b)
    {
      for(itemlist::size_type i=0; i<items.size(); i++)
	if(items[i].b==b)
	  {
	    items[i].destroyed_conn.disconnect();
	    items[i].pressed_conn.disconnect();
	    if(selected==i)
	      {
		if(i>0)
		  button_pressed(i-1);
		else if(i+1<items.size())
		  button_pressed(i+1);
		else
		  eassert(items.size() == 1);
	      }

	    if(i==items.size()-1)
	      items.pop_back();
	    else
	      {
		items[i]=items[items.size()-1];
		if(selected==items.size()-1)
		  selected=i;
		items.pop_back();
		items[i].pressed_conn.disconnect();
		items[i].pressed_conn=items[i].b->pressed.connect(sigc::bind(sigc::mem_fun(*this, &radiogroup::button_pressed), i));
	      }

	    return;
	  }
    }

    void radiogroup::rem_button_bare(togglebutton &b)
    {
      rem_button(togglebutton_ref(&b));
    }

    void radiogroup::select(int id)
    {
      for(itemlist::size_type i=0; i<items.size(); i++)
	if(items[i].id==id)
	  {
	    button_pressed(i);

	    return;
	  }

      // Er, no buttons match the given ID.
      abort();
    }

    void radiogroup::destroy()
    {
      delete this;
    }
  }
}
