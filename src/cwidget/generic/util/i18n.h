// globals.h                -*-c++-*-
//
//   Copyright (C) 2007, 2011 Daniel Burrows
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

#ifndef I18N_H
#define I18N_H

#include <config.h>

// i18n definitions

#define CWIDGET_DOMAIN "libcwidget3"


#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#ifndef HAVE_SETLOCALE
inline void setlocale(int, const char *)
{
}
#endif

#if ENABLE_NLS
# include <libintl.h>
# include <string.h>
# define _(Text) dgettext (CWIDGET_DOMAIN, Text)
# define W_(Text) transcode ( _(Text) )
# define N_(Text) Text

/** Strips everything up to and including the first pipe character
 *  from the translated string.  Translations without a pipe character are unchanged.
 */
#ifdef __GNUG__
__attribute__ ((format_arg(1)))
#endif
inline const char *P_(const char *Text)
{
  const char * const translation = dgettext(CWIDGET_DOMAIN, Text);
  const char * const stripto = strchr(translation, '|');

  if(stripto == NULL)
    return translation;
  else
    return stripto+1;
}

#else
# undef bindtextdomain
# define bindtextdomain(Domain, Directory) /* empty */
# undef textdomain
# define textdomain(Domain) /* empty */
# define _(Text) Text
# define N_(Text) Text
inline const char *P_(const char *Text)
{
  const char * const stripto = strchr(Text, '|');
  return stripto+1;
}
# define gettext(Text) Text
# define dgettext(Domain, Text) Text
#endif


#endif // GLOBALS_H
