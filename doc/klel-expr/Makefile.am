
EXTRA_DIST = DESCRIPTION.pod EXAMPLES.pod NAME.pod RETURN_VALUES.pod SEE_ALSO.pod SYNOPSIS.pod

man_MANS = klel-expr.1

klel-expr.pod:
	cat $(srcdir)/NAME.pod $(srcdir)/SYNOPSIS.pod $(srcdir)/DESCRIPTION.pod $(srcdir)/RETURN_VALUES.pod $(srcdir)/EXAMPLES.pod $(srcdir)/SEE_ALSO.pod > $@
	
klel-expr.1: klel-expr.pod
	pod2man --release='$(PACKAGE_VERSION)' --center='$(PACKAGE_NAME)' --section=1 klel-expr.pod $@

CLEANFILES = klel-expr.pod klel-expr.1

include $(top_srcdir)/utils/common.am
