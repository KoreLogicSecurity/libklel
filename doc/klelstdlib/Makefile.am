
EXTRA_DIST = FUNCTIONS.pod INTRODUCTION.pod VARIABLES.pod SEE_ALSO.pod

man_MANS = klelstdlib.3

klelstdlib.pod:
	cat $(srcdir)/INTRODUCTION.pod $(srcdir)/FUNCTIONS.pod $(srcdir)/VARIABLES.pod $(srcdir)/SEE_ALSO.pod > $@
	
klelstdlib.3: klelstdlib.pod
	pod2man --release='$(PACKAGE_VERSION)' --center='$(PACKAGE_NAME)' --section=1 klelstdlib.pod $@

CLEANFILES = klelstdlib.pod klelstdlib.3

include $(top_srcdir)/utils/common.am
