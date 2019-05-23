
ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = README README.CREDITS README.GRAMMAR README.INSTALL README.LICENSE VERSION VERSION.LIBRARY utils/test.sh utils/version2string utils/version_helper

SUBDIRS = doc include src

TESTS = utils/test.sh

dist-hook:
	@rm -f `find $(distdir) -type f -name ChangeLog`

dist-sign: dist
	@gpg --default-key "KL-EL Project (Code Signing Key)" --detach-sign $(distdir).tar.gz

include $(top_srcdir)/utils/common.am
