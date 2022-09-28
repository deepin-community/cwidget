// dialogs.h      -*-c++-*-
//
//   Copyright (C) 2000, 2007-2008 Daniel Burrows
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

/** \file dialogs.h
 *
 *  \brief Utility functions to construct prefabricated widgets.
 *
 *  For instance, basic message boxes and yes/no queries can be
 *  constructed here.
 */

#ifndef DIALOGS_H
#define DIALOGS_H

#include <cwidget/widgets/editline.h> // for editline::history_list

#include <cwidget/generic/util/slotarg.h>

#include <string>

namespace cwidget
{
  class fragment;
  class style;

  namespace widgets
  {
    class pager;
    class widget;
  }

  /** \brief Routines to construct simple dialog boxes. */
  namespace dialogs
  {
    // Canned dialog-boxes:

    /** Create a dialog box with a single button.
     *
     *  \param widget the widget to place above the button.
     *
     *  \param okslot the slot to be triggered when the button is pressed.
     *
     *  \param label the label of the button
     *
     *  \param attr the attributes to use for the background of the dialog
     *  box, defaults to reverse-video of DefaultWidgetBackground.
     */
    widgets::widget_ref ok(const widgets::widget_ref &widget,
			   util::slot0arg okslot, const std::wstring &label,
			   const style &st);

    widgets::widget_ref ok(fragment *msg, util::slot0arg okslot = nullptr, bool scrollbar=false);
    widgets::widget_ref ok(fragment *msg, util::slot0arg okslot, const style &st, bool scrollbar=false);
    widgets::widget_ref ok(fragment *msg, util::slot0arg okslot, const std::wstring &label,
			   const style &st, bool scrollbar=false);

    widgets::widget_ref ok(const std::wstring &msg, util::slot0arg okslot = nullptr);
    widgets::widget_ref ok(const std::wstring &msg, util::slot0arg okslot,
			   const style &st);
    widgets::widget_ref ok(const std::wstring &msg, util::slot0arg okslot, const std::wstring &label,
			   const style &st);

    /** Create a dialog box with two buttons, labelled "yes" and "no".
     *
     *  \param widget the widget to place above the buttons
     *
     *  \param yesslot the callback to be triggered when "yes" is selected
     *
     *  \param yeslabel the label of the "yes" button
     *
     *  \param noslot the callback to be triggered when "no" is selected
     *
     *  \param yeslabel the label of the "no" button
     *
     *  \param attr the attribute to use as the background of widgets
     *  created by this routine
     *
     *  \param deflt if \b true, the "yes" button will be selected by default;
     *  otherwise, the "no" button will be selected by default.
     */
    widgets::widget_ref yesno(const widgets::widget_ref &widget,
			      util::slot0arg yesslot,
			      const std::wstring &yeslabel,
			      util::slot0arg noslot,
			      const std::wstring &nolabel,
			      const style &st,
			      bool deflt=true);

    widgets::widget_ref yesno(fragment *msg,
			      util::slot0arg yesslot,
			      util::slot0arg noslot,
			      bool scrollbar=false,
			      bool deflt=true);
    widgets::widget_ref yesno(fragment *msg,
			      util::slot0arg yesslot,
			      util::slot0arg noslot,
			      const style &st,
			      bool scrollbar=false,
			      bool deflt=true);
    widgets::widget_ref yesno(fragment *msg,
			      util::slot0arg yesslot,
			      const std::wstring &yeslabel,
			      util::slot0arg noslot,
			      const std::wstring &nolabel,
			      const style &st,
			      bool scrollbar=false,
			      bool deflt=true);

    widgets::widget_ref yesno(const std::wstring &msg,
			      util::slot0arg yesslot,
			      util::slot0arg noslot,
			      bool deflt=true);
    widgets::widget_ref yesno(const std::wstring &msg,
			      util::slot0arg yesslot,
			      util::slot0arg noslot,
			      const style &st,
			      bool deflt=true);
    widgets::widget_ref yesno(const std::wstring &msg,
			      util::slot0arg yesslot,
			      const std::wstring &yeslabel,
			      util::slot0arg noslot,
			      const std::wstring &nolabel,
			      const style &st,
			      bool deflt=true);

    widgets::widget_ref fileview(const std::string &fn,
				 util::slot0arg okslot = nullptr,
				 util::slotarg<sigc::slot1<void, widgets::pager &> > search_slot = nullptr,
				 util::slotarg<sigc::slot1<void, widgets::pager &> > repeat_search_slot = nullptr,
				 util::slotarg<sigc::slot1<void, widgets::pager &> > repeat_search_back_slot = nullptr,
				 const char *encoding = nullptr);
    widgets::widget_ref fileview(const std::string &fn,
				 util::slot0arg okslot,
				 util::slotarg<sigc::slot1<void, widgets::pager &> > search_slot,
				 util::slotarg<sigc::slot1<void, widgets::pager &> > repeat_search_slot,
				 util::slotarg<sigc::slot1<void, widgets::pager &> > repeat_search_back_slot,
				 const style &st,
				 const char *encoding = nullptr);

    widgets::widget_ref string(fragment *msg,
			       const std::wstring &deflt,
			       util::slotarg<sigc::slot1<void, std::wstring> > okslot,
			       util::slotarg<sigc::slot0<void> > cancel_slot,
			       util::slotarg<sigc::slot1<void, std::wstring> > changed_slot,
			       widgets::editline::history_list *history,
			       const style &st);

    widgets::widget_ref string(const std::wstring &msg,
			       const std::wstring &deflt,
			       util::slotarg<sigc::slot1<void, std::wstring> > okslot,
			       util::slotarg<sigc::slot0<void> > cancel_slot,
			       util::slotarg<sigc::slot1<void, std::wstring> > changed_slot,
			       widgets::editline::history_list *history,
			       const style &st);

    widgets::widget_ref string(const std::wstring &msg,
			       const std::wstring &deflt,
			       util::slotarg<sigc::slot1<void, std::wstring> > slot,
			       util::slotarg<sigc::slot0<void> > cancel_slot,
			       util::slotarg<sigc::slot1<void, std::wstring> > changed_slot,
			       widgets::editline::history_list *history);
  }
}

#endif
