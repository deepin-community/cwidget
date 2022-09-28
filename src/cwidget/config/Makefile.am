MAINTAINERCLEANFILES=Makefile.in

localedir = $(datadir)/locale
AM_CPPFLAGS = -Wall @WERROR@ -I$(top_builddir) -I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
LDADD = @LIBINTL@

noinst_LTLIBRARIES=libconf.la

configincludedir=$(pkgincludedir)/config

configinclude_HEADERS= \
	colors.h	\
	column_definition.h\
	keybindings.h

libconf_la_SOURCES=\
	colors.cc	\
	column_definition.cc\
	keybindings.cc
