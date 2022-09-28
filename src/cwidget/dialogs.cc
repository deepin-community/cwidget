// util.cc
//
//   Copyright (C) 2000-2007 Daniel Burrows
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of
//   the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; see the file COPYING.  If not, write to
//   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//   Boston, MA 02111-1307, USA.

#include "dialogs.h"

#include <cwidget/generic/util/transcode.h>

#include <cwidget/widgets/button.h>
#include <cwidget/widgets/center.h>
#include <cwidget/widgets/editline.h>
#include <cwidget/widgets/frame.h>
#include <cwidget/widgets/label.h>
#include <cwidget/widgets/pager.h>
#include <cwidget/widgets/scrollbar.h>
#include <cwidget/widgets/table.h>
#include <cwidget/widgets/text_layout.h>

#include <cwidget/config/colors.h>
#include <cwidget/config/keybindings.h>

#include <sigc++/adaptors/bind.h>
#include <sigc++/adaptors/hide.h>
#include <sigc++/functors/mem_fun.h>

#include <cwidget/generic/util/i18n.h>

namespace cwidget
{
  using namespace util;
  using namespace widgets;

  namespace dialogs
  {
    static void do_slot0_dialog(widget &ownerBare,
				slot0arg okslot)
    {
      widget_ref owner(&ownerBare);

      owner->destroy();
      if(okslot)
	(*okslot)();
    }

    widget_ref ok(const widget_ref &w,
		  slot0arg okslot,
		  const std::wstring &label,
		  const style &st)
    {
      center_ref center = center::create();

      table_ref table = table::create();

      button_ref okbutton = button::create(label);

      okbutton->pressed.connect(sigc::bind(sigc::ptr_fun(&do_slot0_dialog),
					   center.weak_ref(), okslot));

      table->add_widget(w, 0, 0, 1, 1, true, true);
      table->add_widget(center::create(okbutton), 1, 0, 1, 1, false, false);
      table->connect_key("Confirm", &config::global_bindings, okbutton->pressed.make_slot());

      frame_ref frame = frame::create(table);

      center->add_widget(frame);
      frame->set_bg_style(st);
      return center;
    }

    widget_ref ok(fragment *msg, slot0arg okslot,
		  const std::wstring &label,
		  const style &st, bool scrollbar)
    {
      widget_ref w;

      if(scrollbar)
	{
	  table_ref t = table::create();
	  w=t;

	  text_layout_ref l = text_layout::create(msg);
	  scrollbar_ref s = scrollbar::create(scrollbar::VERTICAL);

	  t->add_widget(l, 0, 0, 1, 1, true, true);
	  t->add_widget_opts(s, 0, 1, 1, 1,
			     table::ALIGN_RIGHT,
			     table::ALIGN_CENTER | table::FILL);

	  l->location_changed.connect(sigc::mem_fun(*s.unsafe_get_ref(), &scrollbar::set_slider));
	  s->scrollbar_interaction.connect(sigc::mem_fun(*l.unsafe_get_ref(), &text_layout::scroll));
	}
      else
	w=text_layout::create(msg);

      return ok(w, okslot, label, st);
    }

    widget_ref ok(fragment *msg, slot0arg okslot, const style &st, bool scrollbar)
    {
      return ok(msg, okslot, transcode(_("Ok")), st, scrollbar);
    }

    widget_ref ok(fragment *msg, slot0arg okslot, bool scrollbar)
    {
      return ok(msg, okslot, style_attrs_flip(A_REVERSE), scrollbar);
    }

    widget_ref ok(const std::wstring &msg, slot0arg okslot,
		  const style &st)
    {
      widget_ref l=label::create (msg);

      return ok(l, okslot, transcode(_("Ok")), st);
    }

    widget_ref ok(const std::wstring &msg, slot0arg okslot)
    {
      return ok(msg, okslot, style_attrs_flip(A_REVERSE));
    }


    widget_ref yesno(const widget_ref &widget,
		     slot0arg yesslot,
		     const std::wstring &yeslabel,
		     slot0arg noslot,
		     const std::wstring &nolabel,
		     const style &st,
		     bool deflt)
    {
      center_ref center = center::create();

      table_ref table = table::create();

      button_ref yesbutton = button::create(yeslabel);
      button_ref nobutton = button::create(nolabel);

      yesbutton->pressed.connect(sigc::bind(sigc::ptr_fun(&do_slot0_dialog),
					    center.weak_ref(), yesslot));
      nobutton->pressed.connect(sigc::bind(sigc::ptr_fun(&do_slot0_dialog),
					   center.weak_ref(), noslot));

      table->connect_key("Yes", &config::global_bindings, yesbutton->pressed.make_slot());
      table->connect_key("No", &config::global_bindings, nobutton->pressed.make_slot());
      table->connect_key("Cancel", &config::global_bindings, nobutton->pressed.make_slot());

      table->add_widget(widget, 0, 0, 1, 2, true, true);
      table->add_widget_opts(yesbutton, 1, 0, 1, 1, table::SHRINK|table::ALIGN_CENTER, 0);
      table->add_widget_opts(nobutton, 1, 1, 1, 1, table::SHRINK|table::ALIGN_CENTER, 0);

      widget->show();
      yesbutton->show();
      nobutton->show();

      if(deflt)
	table->focus_widget(yesbutton);
      else
	table->focus_widget(nobutton);

      frame_ref frame = frame::create(table);
      frame->set_bg_style(st);

      center->add_widget(frame);

      return center;
    }

    widget_ref yesno(const std::wstring &msg,
		     slot0arg yesslot,
		     const std::wstring &yeslabel,
		     slot0arg noslot,
		     const std::wstring &nolabel,
		     const style &st,
		     bool deflt)
    {
      widget_ref txt=label::create(msg);

      return yesno(txt, yesslot, yeslabel, noslot, nolabel, st, deflt);
    }

    widget_ref yesno(const std::wstring &msg,
		     slot0arg yesslot,
		     slot0arg noslot,
		     const style &st,
		     bool deflt)
    {
      return yesno(msg, yesslot, transcode(_("Yes")),
		   noslot, transcode(_("No")), st, deflt);
    }

    widget_ref yesno(const std::wstring &msg,
		     slot0arg yesslot,
		     slot0arg noslot,
		     bool deflt)
    {
      return yesno(msg,
		   yesslot,
		   noslot,
		   style_attrs_flip(A_REVERSE),
		   deflt);
    }


    widget_ref yesno(fragment *msg,
		     slot0arg yesslot,
		     slot0arg noslot,
		     bool scrollbar,
		     bool deflt)
    {
      return yesno(msg,
		   yesslot,
		   noslot,
		   style_attrs_flip(A_REVERSE),
		   scrollbar,
		   deflt);
    }

    widget_ref yesno(fragment *msg,
		     slot0arg yesslot,
		     slot0arg noslot,
		     const style &st,
		     bool scrollbar,
		     bool deflt)
    {
      return yesno(msg, yesslot, transcode(_("Yes")),
		   noslot, transcode(_("No")), st,
		   scrollbar, deflt);
    }

    widget_ref yesno(fragment *msg,
		     slot0arg yesslot,
		     const std::wstring &yeslabel,
		     slot0arg noslot,
		     const std::wstring &nolabel,
		     const style &st,
		     bool scrollbar,
		     bool deflt)
    {
      widget_ref w;

      if(scrollbar)
	{
	  table_ref t = table::create();
	  w=t;

	  text_layout_ref l = text_layout::create(msg);
	  scrollbar_ref s = scrollbar::create(scrollbar::VERTICAL);

	  t->add_widget(l, 0, 0, 1, 1, true, true);
	  t->add_widget_opts(s, 0, 1, 1, 1,
			     table::ALIGN_RIGHT,
			     table::ALIGN_CENTER | table::FILL);

	  l->location_changed.connect(sigc::mem_fun(*s.unsafe_get_ref(), &scrollbar::set_slider));
	  s->scrollbar_interaction.connect(sigc::mem_fun(*l.unsafe_get_ref(), &text_layout::scroll));
	}
      else
	w=text_layout::create(msg);

      return yesno(w, yesslot, yeslabel, noslot, nolabel, st, deflt);
    }

    widget_ref fileview(const std::string &fn,
			slot0arg okslot,
			slotarg<sigc::slot1<void, pager &> > search_slot,
			slotarg<sigc::slot1<void, pager &> > repeat_search_slot,
			slotarg<sigc::slot1<void, pager &> > repeat_search_back_slot,
			const style &st,
			const char *encoding)
    {
      file_pager_ref p = file_pager::create(fn, encoding);
      scrollbar_ref scrollbar = scrollbar::create(scrollbar::VERTICAL, 0, 0);
      table_ref t = table::create();

      t->add_widget_opts(p, 0, 0, 1, 1,
			 table::EXPAND | table::FILL | table::SHRINK | table::ALIGN_CENTER,
			 table::FILL | table::SHRINK | table::ALIGN_CENTER);
      t->add_widget_opts(scrollbar, 0, 1, 1, 1,
			 table::ALIGN_CENTER,
			 table::EXPAND | table::FILL);

      //t->set_bg_style(style_attrs_off(A_REVERSE));

      p->line_changed.connect(sigc::mem_fun(*scrollbar.unsafe_get_ref(), &scrollbar::set_slider));
      p->do_line_signal();
      scrollbar->scrollbar_interaction.connect(sigc::mem_fun(*p.unsafe_get_ref(), &pager::scroll_page));

      if(search_slot)
	p->connect_key("Search", &config::global_bindings, sigc::bind(*search_slot, p.weak_ref()));

      if(repeat_search_slot)
	p->connect_key("ReSearch", &config::global_bindings, sigc::bind(*repeat_search_slot, p.weak_ref()));

      if(repeat_search_back_slot)
	p->connect_key("RepeatSearchBack", &config::global_bindings, sigc::bind(*repeat_search_back_slot, p.weak_ref()));

      return ok(t, okslot, transcode(_("Ok")), st);
    }

    widget_ref fileview(const std::string &fn,
			slot0arg okslot,
			slotarg<sigc::slot1<void, pager &> > search_slot,
			slotarg<sigc::slot1<void, pager &> > repeat_search_slot,
			slotarg<sigc::slot1<void, pager &> > repeat_search_back_slot,
			const char *encoding)
    {
      return fileview(fn, okslot, search_slot,
		      repeat_search_slot, repeat_search_back_slot,
		      style_attrs_flip(A_REVERSE),
		      encoding);
    }

    static void do_string(editline &eBare,
			  widget &dialogBare,
			  slotarg<sigc::slot1<void, std::wstring> > thestrslot)
    {
      editline_ref e(&eBare);
      widget_ref   dialog(&dialogBare);

      dialog->destroy();

      e->add_to_history(e->get_text());
      if(thestrslot)
	(*thestrslot)(e->get_text());
    }

    widget_ref string(const widget_ref &msg,
		      std::wstring deflt,
		      slotarg<sigc::slot1<void, std::wstring> > slot,
		      slotarg<sigc::slot0<void> > cancel_slot,
		      slotarg<sigc::slot1<void, std::wstring> > changed_slot,
		      editline::history_list *history,
		      const style &st)
    {
      table_ref t = table::create();
      editline_ref e = editline::create(rootwin.getmaxx()-6, L"", deflt, history);
      button_ref bok = button::create(_("Ok"));
      button_ref bcancel = button::create(_("Cancel"));
      frame_ref f = frame::create(t);
      center_ref c = center::create(f);

      e->set_allow_wrap(true);
      e->set_clear_on_first_edit(true);

      f->set_bg_style(st);

      t->add_widget(msg, 0, 0, 1, 2);
      t->add_widget(e, 1, 0, 1, 2);
      t->add_widget_opts(bok, 2, 0, 1, 1,
			 table::ALIGN_CENTER|table::SHRINK,
			 table::ALIGN_CENTER);
      t->add_widget_opts(bcancel, 2, 1, 1, 1,
			 table::ALIGN_CENTER|table::SHRINK,
			 table::ALIGN_CENTER);

      e->entered.connect(sigc::hide(bok->pressed.make_slot()));
      if(changed_slot)
	e->text_changed.connect(*changed_slot);

      t->connect_key_post("Cancel", &config::global_bindings,
			  bcancel->pressed.make_slot());

      bok->pressed.connect(sigc::bind(sigc::ptr_fun(do_string),
				      e.weak_ref(), c.weak_ref(), slot));

      bcancel->pressed.connect(sigc::bind(sigc::ptr_fun(&do_slot0_dialog),
					  c.weak_ref(), cancel_slot));

      return c;
    }

    widget_ref string(fragment *msg,
		      const std::wstring &deflt,
		      slotarg<sigc::slot1<void, std::wstring> > slot,
		      slotarg<sigc::slot0<void> > cancel_slot,
		      slotarg<sigc::slot1<void, std::wstring> > changed_slot,
		      editline::history_list *history,
		      const style &st)
    {
      return dialogs::string(label::create(msg),
			     deflt,
			     slot,
			     cancel_slot,
			     changed_slot,
			     history,
			     st);
    }

    widget_ref string(const std::wstring &msg,
		      const std::wstring &deflt,
		      slotarg<sigc::slot1<void, std::wstring> > slot,
		      slotarg<sigc::slot0<void> > cancel_slot,
		      slotarg<sigc::slot1<void, std::wstring> > changed_slot,
		      editline::history_list *history,
		      const style &st)
    {
      return dialogs::string(label::create(msg),
			     deflt,
			     slot,
			     cancel_slot,
			     changed_slot,
			     history,
			     st);
    }

    widget_ref string(const std::wstring &msg,
		      const std::wstring &deflt,
		      slotarg<sigc::slot1<void, std::wstring> > slot,
		      slotarg<sigc::slot0<void> > cancel_slot,
		      slotarg<sigc::slot1<void, std::wstring> > changed_slot,
		      editline::history_list *history)
    {
      return dialogs::string(msg,
			     deflt,
			     slot,
			     cancel_slot,
			     changed_slot,
			     history,
			     style_attrs_flip(A_REVERSE));
    }
  }
}
