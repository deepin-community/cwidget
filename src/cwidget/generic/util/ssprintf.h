// ssprintf.h
//
//   Copyright (C) 2005, 2007, 2009 Daniel Burrows
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

#ifndef SSPRINTF_H
#define SSPRINTF_H

#include <string>
#include <stdarg.h>

namespace cwidget
{
  namespace util
  {

    // Printf for std::string.
#ifdef __GNUG__
    __attribute__ ((format (printf, 1, 2)))
#endif
    std::string ssprintf(const char *format, ...);
    std::string vssprintf(const char *format, va_list ap);

    std::wstring swsprintf(const wchar_t *format, ...);

    std::wstring vswsprintf(const wchar_t *format, va_list ap);

    /** Like strerror_r, but handles all memory allocation and returns a
     *  C++ string.
     */
    std::string sstrerror(int errnum);
  }
}

#endif // SSPRINTF_H
