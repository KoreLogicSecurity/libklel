
EXTRA_DIST = BASIC.pod GUARDED_COMMANDS.pod INTRODUCTION.pod SEE_ALSO.pod VARIABLES.pod

man_MANS = kleltut.3

kleltut.pod:
	cat $(srcdir)/INTRODUCTION.pod $(srcdir)/BASIC.pod $(srcdir)/VARIABLES.pod $(srcdir)/GUARDED_COMMANDS.pod $(srcdir)/SEE_ALSO.pod > $@
	
kleltut.3: kleltut.pod
	pod2man --release='$(PACKAGE_VERSION)' --center='$(PACKAGE_NAME)' --section=1 kleltut.pod $@

CLEANFILES = kleltut.pod kleltut.3

include $(top_srcdir)/utils/common.am
