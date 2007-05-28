# Makefile.am 

lib_LTLIBRARIES = libflt.la

# (BTW - if you use include_HEADERS instead of pkginclude_HEADERS, you 
# will end up with your headers installed in /usr/local/include rather than 
# /usr/local/include/whatever)

# headers to be installed when 'make install' is run:
#pkginclude_HEADERS = 
include_HEADERS = \
	flt.h \
	fltMath.h \
	fltw.h 

libflt_la_SOURCES =		\
	buffer.c\
	fileio.c\
	records.c\
	stack.c\
	writer.c\
	math.c\
	util.c


libflt_la_LDFLAGS = 		\
	-release $(LT_RELEASE)	\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

noinst_PROGRAMS = flt fltw

flt_LDADD = libflt.la -lm
fltw_LDADD = libflt.la -lm

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: distcheck

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpm -ta $(PACKAGE)-$(VERSION).tar.gz

# installation info for the pkg-config files
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	fltlib.pc
$(pkgconfig_DATA): $(top_builddir)/config.status

