
EXTRA_DIST = GUARDED_COMMANDS.pod INTRODUCTION.pod NAMES.pod SEE_ALSO.pod TYPES.pod VALUE_EXPRESSIONS.pod

man_MANS = klellang.3

klellang.pod:
	cat $(srcdir)/INTRODUCTION.pod $(srcdir)/TYPES.pod $(srcdir)/NAMES.pod $(srcdir)/VALUE_EXPRESSIONS.pod $(srcdir)/GUARDED_COMMANDS.pod $(srcdir)/SEE_ALSO.pod > $@
	
klellang.3: klellang.pod
	pod2man --release='$(PACKAGE_VERSION)' --center='$(PACKAGE_NAME)' --section=1 klellang.pod $@

CLEANFILES = klellang.pod klellang.3

include $(top_srcdir)/utils/common.am
