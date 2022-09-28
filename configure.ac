dnl Process this file with autoconf to produce a configure script.
AC_INIT(cwidget, 0.5.18)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/cwidget/toplevel.cc])
AC_CONFIG_HEADERS([config.h cwidget-config.h])
AM_INIT_AUTOMAKE([dist-xz no-dist-gzip -Wno-portability])

dnl Use C++
AC_LANG([C++])

BUILD_LIBRARY=yes
AC_ARG_ENABLE(library, AS_HELP_STRING([--disable-library], [Disable building the library itself; only build documentation.]),
	      [if test x$enableval = xno; then BUILD_LIBRARY=no; fi],
	      )

AM_CONDITIONAL(BUILD_LIBRARY, [test x$BUILD_LIBRARY = xyes])


dnl AM_GNU_GETTEXT needs to be called unconditionally, or the
dnl configure script freaks out.  Otherwise this would be inside the
dnl stuff that gets disabled by --disable-library.
ALL_LINGUAS=""
AM_GNU_GETTEXT([external])


dnl Find the key tools.
AC_PROG_CXX
AC_PROG_LIBTOOL
dnl Try to find doxygen in the path.
AC_MSG_CHECKING([for the Doxygen documentation generator (http://www.doxygen.org)])
AC_PATH_PROG(DOXYGEN, doxygen)
AM_CONDITIONAL(DOXYGEN, [test x$DOXYGEN != x])
dnl Try to find doxygen in the path.
AC_MSG_CHECKING([for the ikiwiki Web site compiler (http://ikiwiki.info)])
AC_PATH_PROG(IKIWIKI, ikiwiki)
AM_CONDITIONAL(IKIWIKI, [test x$IKIWIKI != x])

RUN_TESTS=0

if test x$BUILD_LIBRARY = xyes
then


dnl Look for ncurses
AC_CHECK_LIB(ncursesw, initscr, ,
	[AC_MSG_ERROR([Can't find libncursesw -- please install libncursesw5-dev])])
dnl We need threading.
AC_CHECK_LIB(pthread, main,
	HAVE_LIBPTHREAD=1
	, [AC_MSG_ERROR([Can't find the POSIX thread libraries])])

if test x$HAVE_LIBPTHREAD = x1
then
  AC_CHECK_HEADER(pthread.h,
	CXXFLAGS="$CXXFLAGS -D_REENTRANT"
	LIBS="$LIBS -lpthread"
	[AC_DEFINE([HAVE_LIBPTHREAD], [] , [Define if pthread is available])]
	,
	[AC_MSG_ERROR([POSIX thread header not installed])])
fi


dnl We need strerror_r, but strerror_r has different signatures on GNU
dnl and in other places.
dnl
dnl We could try to hack around a missing strerror_r (e.g. by using
dnl strerror and locking around it) if someone wants to compile this
dnl on a horrid system without safe error string functions.
AC_FUNC_STRERROR_R
if test x$ac_cv_have_strerror_r = xno
then
  AC_MSG_ERROR([cwidget requires an implementation of strerror_r()])
fi

dnl Find sigc++
PKG_CHECK_MODULES(SIGC, sigc++-2.0)
CXXFLAGS="$CXXFLAGS $SIGC_CFLAGS"
LIBS="$LIBS $SIGC_LIBS"
AC_DEFINE_UNQUOTED(SIGC_VERSION, ["$(pkg-config --modversion sigc++-2.0)"], [The version of libsigc++ with which the program was compiled])

AC_CHECK_HEADER(locale.h, [AC_DEFINE([HAVE_LOCALE_H], [], [Define if locale.h is available])])

AC_CHECK_DECL(setlocale, [AC_DEFINE([HAVE_SETLOCALE], [], [Define if setlocale is available in locale.h])], , [#include <locale.h>])



dnl Figure out how to extend string traits
TRAITS_CLASS=""

AC_MSG_CHECKING([for the name of the character traits template])

for T in char_traits string_char_traits
do
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <string>],
				     [std::basic_string<unsigned char,
						        std::$T < unsigned char > > s;])],
		    TRAITS_CLASS=$T)
done

if test x$TRAITS_CLASS = xstring_char_traits
then
    AC_MSG_WARN([Your compiler uses string_char_traits for its character traits.  Some compilers (eg, g++ 2.95) which use this name for the character traits template are known to have hideously broken implementations of the standard string class, which cause aptitude to fail to compile.  If you have a compiler with this problem, please upgrade it to a version that has a more compliant version of the STL (g++ >=3.0 is known to work).  You can specify which compiler this script should use via the CXX environment variable.])
fi

if test x$TRAITS_CLASS = x
then
  AC_MSG_ERROR([can't find the name of the character traits template])
else
  AC_DEFINE_UNQUOTED(TRAITS_CLASS, $TRAITS_CLASS, [The name of the class used by the STL to define character traits])

  AC_MSG_RESULT([$TRAITS_CLASS])
fi



dnl Default to -Werror, but add an easy flag to disable it.
WERROR="-Werror -Wno-terminate"

AC_ARG_ENABLE(dynamic-backtrace,
	AS_HELP_STRING([--enable-dynamic-backtrace], [Modify the executable so that it can generate a backtrace for uncaught exceptions.  Will double the size of the stripped binary.]),
	[if test x$enableval = xyes
	then
	  AC_DEFINE([ENABLE_DYNAMIC_BACKTRACE], [], [Define to enable dynamic generation of backtraces if HAVE_EXECINFO_H is defined])
	  LDFLAGS="$LDFLAGS -Wl,--export-dynamic"
	fi]
	)
AC_ARG_ENABLE(werror,
	AS_HELP_STRING([--disable-werror], [do not compile with -Werror]),
	if test x$enableval = xno
	then
	  WERROR=""
	fi
	)
AC_SUBST(WERROR)

dnl Enable test cases if cppunit is present.
PKG_CHECK_MODULES(CPPUNIT, cppunit,
		   [HAVE_CPPUNIT=1
		    AC_SUBST(CPPUNIT_CFLAGS)
		    AC_SUBST(CPPUNIT_LIBS)],
		  [true])

# END LIBRARY CONFIGURATION
fi

AM_CONDITIONAL([HAVE_CPPUNIT], [test x$HAVE_CPPUNIT = x1])

AC_CONFIG_FILES([
	po/Makefile.in
	Doxyfile
	Makefile
	cwidget.pc
	doc/Makefile
	src/Makefile
	src/cwidget/Makefile
	src/cwidget/config/Makefile
	src/cwidget/widgets/Makefile
	src/cwidget/generic/Makefile
	src/cwidget/generic/threads/Makefile
	src/cwidget/generic/util/Makefile
	tests/Makefile
])

AC_OUTPUT
