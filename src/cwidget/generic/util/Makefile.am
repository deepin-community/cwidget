MAINTAINERCLEANFILES = Makefile.in

localedir = $(datadir)/locale
AM_CPPFLAGS = -Wall @WERROR@ -I$(top_builddir) -I$(top_srcdir)/src
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

noinst_LTLIBRARIES = libgeneric-util.la

genericutilincludedir = $(pkgincludedir)/generic/util

genericutilinclude_HEADERS = \
	bool_accumulate.h \
	eassert.h	\
	exception.h	\
	ref_ptr.h	\
	slotarg.h	\
	ssprintf.h	\
	transcode.h

# Note that i18n.h is not installed: installing it would export
# information about the configuration of the package that shouldn't
# affect user code. (e.g., --enable-nls)
libgeneric_util_la_SOURCES = \
	eassert.cc	\
	exception.cc	\
	i18n.h		\
	ssprintf.cc	\
	transcode.cc
