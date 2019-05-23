
EXTRA_DIST = DESCRIPTION.pod NAME.pod RETURN_VALUE.pod SEE_ALSO.pod SYNOPSIS.pod TYPES.pod

man_MANS = klelapi.3

klelapi.pod:
	cat $(srcdir)/NAME.pod $(srcdir)/SYNOPSIS.pod $(srcdir)/DESCRIPTION.pod $(srcdir)/TYPES.pod $(srcdir)/RETURN_VALUE.pod $(srcdir)/SEE_ALSO.pod > $@
	
klelapi.3: klelapi.pod
	pod2man --release='$(PACKAGE_VERSION)' --center='$(PACKAGE_NAME)' --section=1 klelapi.pod $@

CLEANFILES = klelapi.pod klelapi.3

include $(top_srcdir)/utils/common.am
