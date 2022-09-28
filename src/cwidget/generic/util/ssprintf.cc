#include "ssprintf.h"

#include "eassert.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h> // For strerror_r
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_STRERROR_R
#error "strerror_r is required."
#endif

#ifndef HAVE_DECL_STRERROR_R
#ifdef STRERROR_R_CHAR_P
char *strerror_r(int errnum, char *buf, size_t buflen);
#else
int strerror_r(int errnum, char *buf, size_t buflen);
#endif
#endif

using namespace std;

namespace cwidget
{
  namespace util
  {
    string ssprintf(const char *format, ...)
    {
      va_list ap;

      va_start(ap, format);
      string rval = vssprintf(format, ap);
      va_end(ap);

      return rval;
    }

    const int initbufsize=512;

    string vssprintf(const char *format, va_list ap)
    {
      // We need to do this because you can't necessarily re-use a
      // va_list after stepping down it.
      va_list ap2;
      va_copy(ap2, ap);
      char buf[initbufsize];
      const int amt = vsnprintf(buf, initbufsize, format, ap);

      if(amt < initbufsize)
	{
	  va_end(ap2);
	  return buf;
	}
      else
	{
	  const int buf2size = amt + 1;
	  char *buf2 = new char[buf2size];

	  const int amt2 = vsnprintf(buf2, buf2size, format, ap2);

	  if (amt2 >= buf2size) {
	    delete[] buf2;
	    eassert(amt2 < buf2size);
	  }

	  string rval(buf2, amt2);

	  delete[] buf2;
	  va_end(ap2);

	  return rval;
	}
    }

    wstring swsprintf(const wchar_t *format, ...)
    {
      va_list ap;

      va_start(ap, format);
      wstring rval = vswsprintf(format, ap);
      va_end(ap);

      return rval;
    }

    wstring vswsprintf(const wchar_t *format, va_list ap)
    {
      // Unlike sprintf, swsprintf just returns -1 when there's an
      // overflow.  So we have to keep making bigger buffers until we
      // have one that's big enough.

      bool ok = false;

      wstring rval;
      int bufsize = initbufsize;

      while(!ok)
	{
	  va_list ap2;
	  va_copy(ap2, ap);

	  wchar_t *buf = new wchar_t[bufsize];

	  int amt = vswprintf(buf, bufsize, format, ap2);

	  if(amt >= 0 && amt < bufsize)
	    {
	      rval = buf;
	      ok = true;
	    }
	  else
	    bufsize *= 2;

	  delete[] buf;

	  va_end(ap2);
	}

      return rval;
    }

    // There are two variants of strerror_r to watch out for.  The GNU
    // version, used on e.g. Linux, returns a char* that may or may
    // not point to the buffer that was passed in as an argument, or
    // NULL if the function failed.  The XSI version returns 0 for
    // success and a non-zero value for failure.  The main difference
    // is that for the GNU version we return the return value of
    // strerror_r, while for XSI we return the temporary buffer.
    string sstrerror(int errnum)
    {
      size_t bufsize = 512;

      while(bufsize < 512 * 512)
	{
	  char *buf = new char[bufsize];

#ifdef STRERROR_R_CHAR_P
	  char *result = strerror_r(errnum, buf, bufsize);
	  bool failed = (result == NULL);
#else
	  int result = strerror_r(errnum, buf, bufsize);
	  bool failed = (result != 0);
#endif

	  if(failed)
	    {
	      delete[] buf;

	      if(errno == EINVAL)
		return ssprintf("Invalid error code %d", errnum);
	      else if(errno != ERANGE)
		return ssprintf("Unexpected error from strerror_r: %d", errnum);
	      else
		bufsize *= 2;
	    }
	  else
	    {
#ifdef STRERROR_R_CHAR_P
	      string rval(result);
#else
	      string rval(buf);
#endif
	      delete[] buf;
	      return rval;
	    }
	}

      return "";
    }
  }
}
