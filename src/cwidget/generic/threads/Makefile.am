MAINTAINERCLEANFILES = Makefile.in

localedir = $(datadir)/locale
AM_CPPFLAGS = -Wall @WERROR@ -I$(top_builddir) -I$(top_srcdir)/src
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

genericthreadsincludedir=$(pkgincludedir)/generic/threads

noinst_LTLIBRARIES = libgeneric-threads.la

genericthreadsinclude_HEADERS = \
	event_queue.h	\
	threads.h

libgeneric_threads_la_SOURCES = \
	threads.cc
