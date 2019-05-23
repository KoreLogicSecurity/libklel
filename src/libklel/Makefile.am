
lib_LTLIBRARIES = libklel.la

EXTRA_DIST = all-includes.h api.h app-includes.h interpreter.h library.h parser.h sys-includes.h typecheck.h inner-string.h

libklel_la_SOURCES = api.c interpreter.c inner-string.c library.c parser.c typecheck.c

libklel_la_LDFLAGS = -version-info @libklel_version@

include $(top_srcdir)/utils/common.am
