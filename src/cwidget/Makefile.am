MAINTAINERCLEANFILES=Makefile.in

SUBDIRS=config generic widgets

EXTRA_DIST=README.layout

localedir = $(datadir)/locale
AM_CPPFLAGS = -Wall @WERROR@ -I$(top_builddir) -I$(top_srcdir)/src
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
LDADD = @LIBINTL@

lib_LTLIBRARIES=libcwidget.la
noinst_PROGRAMS=testcwidget

cwidgetincludedir = $(pkgincludedir)

cwidgetinclude_HEADERS = \
	columnify.h	\
	curses++.h	\
	dialogs.h	\
	fragment.h	\
	fragment_cache.h\
	fragment_contents.h \
	style.h		\
	toplevel.h

libcwidget_la_SOURCES=	\
	columnify.cc	\
	curses++.cc	\
	dialogs.cc	\
	fragment.cc	\
	fragment_cache.cc\
	style.cc	\
	toplevel.cc

libcwidget_la_LDFLAGS = -version-info 4:0:0
libcwidget_la_LIBADD = config/libconf.la \
		       generic/threads/libgeneric-threads.la \
		       generic/util/libgeneric-util.la \
		       widgets/libwidgets.la

testcwidget_SOURCES=	\
	testcwidget.cc

testcwidget_LDADD=libcwidget.la
