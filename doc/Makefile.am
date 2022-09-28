MAINTAINERCLEANFILES=Makefile.in

DOXYGEN=@DOXYGEN@
IKIWIKI=@IKIWIKI@

# Distribute *.mdwn, *.png, and *.css from the ikiwiki directory.
# This trades off the danger of distributing too much (e.g., what
# would happen if we distributed the whole directory, tilde-files and
# all) against the pain of forcing everyone who adds a file to the
# documentation to register it (given that ikiwiki doesn't make it
# easy to audit for missing files).
EXTRA_DIST = $(srcdir)/ikiwiki.setup $(shell find $(srcdir)/ikiwiki -name \*.mdwn -or -name \*.png -or -name \*.css)

clean-local:
	rm -rf html latex ikiwiki-html
	rm -f doxygen*.tmp

doxygen:
if DOXYGEN
	mkdir -p html/documentation/api
	(cd $(top_srcdir) && $(DOXYGEN))
endif

ikiwiki:
if IKIWIKI
	(cd $(srcdir) && ikiwiki --setup ikiwiki.setup)
endif

doc: doxygen ikiwiki

.PHONY: doc doxygen ikiwiki
