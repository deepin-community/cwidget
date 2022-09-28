MAINTAINERCLEANFILES = Makefile.in

if HAVE_CPPUNIT

AM_CPPFLAGS = -Wall @WERROR@ -I$(top_builddir) -I$(top_srcdir)/src @CPPUNIT_CFLAGS@
LDADD = $(top_builddir)/src/cwidget/libcwidget.la @CPPUNIT_LIBS@

check_PROGRAMS = test

TESTS = test

test_SOURCES = \
	main.cc \
	test_eassert.cc \
	test_ssprintf.cc \
	test_threads.cc

endif # HAVE_CPPUNIT
