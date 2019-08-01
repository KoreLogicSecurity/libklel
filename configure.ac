########################################################################
#
# $Id: configure.ac,v 1.39 2019/05/01 21:07:04 klm Exp $
#
########################################################################

dnl ####################################################################
dnl #
dnl # Set project/package and library version information.
dnl #
dnl ####################################################################

m4_define(m4_libklel_current, m4_esyscmd([egrep '^CURRENT=[0-9]+$' VERSION.LIBRARY | awk -F= '{print $2}' | tr -d '\n']))
m4_define(m4_libklel_revision, m4_esyscmd([egrep '^REVISION=[0-9]+$' VERSION.LIBRARY | awk -F= '{print $2}' | tr -d '\n']))
m4_define(m4_libklel_age, m4_esyscmd([egrep '^AGE=[0-9]+$' VERSION.LIBRARY | awk -F= '{print $2}' | tr -d '\n']))
m4_define(m4_libklel_version, m4_esyscmd([echo -n "m4_libklel_current:m4_libklel_revision:m4_libklel_age"]))

m4_define(m4_libklel_release_number, m4_esyscmd([cat VERSION | tr -d '\n']))
m4_define(m4_libklel_release_string, m4_esyscmd([utils/version2string -t tar -v `cat VERSION` | tr -d '\n']))

m4_define(m4_libklel_major, m4_esyscmd([utils/version2string -t tar -v `cat VERSION` | awk -F. '{print $1}' | tr -d '\n']))
m4_define(m4_libklel_minor, m4_esyscmd([utils/version2string -t tar -v `cat VERSION` | awk -F. '{print $2}' | tr -d '\n']))
m4_define(m4_libklel_patch, m4_esyscmd([utils/version2string -t tar -v `cat VERSION` | awk -F. '{print $3}' | tr -d '\n']))

dnl ####################################################################
dnl #
dnl # Initialize.
dnl #
dnl ####################################################################

AC_PREREQ(2.68)
AC_INIT([libklel], [m4_libklel_release_string], [libklel-project@korelogic.com], [libklel])
AC_CONFIG_AUX_DIR(utils)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/libklel/parser.c])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([foreign])

AC_DEFINE(KLEL_LIBRARY_CURRENT, [m4_libklel_current], [Define to the current version number of this library.])
AC_DEFINE(KLEL_LIBRARY_REVISION, [m4_libklel_revision], [Define to the revision version number of this library.])
AC_DEFINE(KLEL_LIBRARY_AGE, [m4_libklel_age], [Define to the age version number of this library.])
AC_DEFINE(KLEL_LIBRARY_VERSION, ["m4_libklel_version"], [Define to the version number of this library.])
libklel_version="m4_libklel_version"
AC_SUBST(libklel_version)

AC_DEFINE(KLEL_RELEASE_NUMBER, [m4_libklel_release_number], [Define to the release number of this package.])

AC_DEFINE(KLEL_RELEASE_MAJOR, [m4_libklel_major], [Define to the major release number of this package.])
AC_DEFINE(KLEL_RELEASE_MINOR, [m4_libklel_minor], [Define to the minor release number of this package.])
AC_DEFINE(KLEL_RELEASE_PATCH, [m4_libklel_patch], [Define to the patch release number of this package.])

: ${CFLAGS="-O3"}

case "${host_cpu}" in
arm)
  case "${target_os}" in
  *linux*)
    LIBS="${LIBS} -lgcc"
    ;;
  esac
  ac_cv_func_malloc_0_nonnull="yes"
  ac_cv_func_realloc_0_nonnull="yes"
  ;;
esac

dnl ####################################################################
dnl #
dnl # Check for programs.
dnl #
dnl ####################################################################

AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AM_PROG_CC_STDC

dnl ####################################################################
dnl #
dnl # Check for certain operating systems that need special treatment.
dnl #
dnl ####################################################################

AC_EXEEXT

dnl ####################################################################
dnl #
dnl # Check for Libtool.
dnl #
dnl ####################################################################

AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

dnl ####################################################################
dnl #
dnl # Check for libraries.
dnl #
dnl ####################################################################

dnl ####################################################################
dnl #
dnl # Check for header files.
dnl #
dnl ####################################################################

AC_CHECK_HEADERS([limits.h stdint.h stdlib.h string.h sys/wait.h unistd.h])

dnl ####################################################################
dnl #
dnl # Check for typedefs, structures, and compiler characteristics.
dnl #
dnl ####################################################################

AC_TYPE_PID_T
AC_TYPE_SIZE_T

dnl ####################################################################
dnl #
dnl # Check for library functions.
dnl #
dnl ####################################################################

AC_FUNC_FORK
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memset strerror strtol])

dnl ####################################################################
dnl #
dnl # Check for PCRE.
dnl #
dnl ####################################################################

CheckPcreComponents()
{
  PCREDIR=${1}
  INCLUDE_LIST="pcre.h"
  for INCLUDE_FILE in ${INCLUDE_LIST} ; do
    if test ! -f "${PCREDIR}/include/${INCLUDE_FILE}" ; then
      return 1
    fi
  done
  my_pcredir=${PCREDIR}
  return 0
}

AC_MSG_CHECKING([for PCRE support])
AC_ARG_WITH(pcre,
  [  --with-pcre=DIR         Use PCRE with includes and libs from [DIR]/include and [DIR]/lib],
  [
    CheckPcreComponents "${withval}"
  ],
  [
    for my_basedir in "/usr" "/usr/local" "/usr/pkg" "/opt" "/opt/local" ; do
      for my_dir in "${my_basedir}" "${my_basedir}/pcre" ; do
        CheckPcreComponents "${my_dir}" && break 2
      done
    done
  ]
)
if test -z "${my_pcredir}" ; then
  AC_MSG_RESULT(missing or incomplete)
  echo
  echo "Unable to locate required PCRE components. Use --with-pcre=DIR to"
  echo "specify a known or different location. The files that must exist for"
  echo "this check to pass are:"
  echo
  echo "  DIR/include/pcre.h"
  echo
  exit 1
else
  AC_MSG_RESULT(${my_pcredir})
  CFLAGS="-I${my_pcredir}/include -DPCRE_STATIC ${CFLAGS}"
  LIBS="-L${my_pcredir}/lib -lpcre ${LIBS}"
  AC_CHECK_LIB([pcre], [pcre_compile], my_have_pcre=1, my_have_pcre=0)
  if test ${my_have_pcre} -eq 0 ; then
    AC_MSG_ERROR([Unable to locate libpcre.])
  fi
fi

dnl ####################################################################
dnl #
dnl # Check whether or not to enable debugging support.
dnl #
dnl ####################################################################

AC_MSG_CHECKING([whether to enable debugging support])
AC_ARG_ENABLE(debugging,
  [  --enable-debugging      Enable debugging support (disabled by default)],
  [
    case `echo ${enableval} | tr "A-Z" "a-z"` in
    yes)
      enable_debugging="1"
      ;;
    *)
      enable_debugging="0"
      ;;
    esac
  ],
  [ enable_debugging="0" ]
)
if test ${enable_debugging} -eq 1 ; then
  AC_MSG_RESULT(yes)
  CFLAGS="-g -DKLEL_DEBUG ${CFLAGS}"
else
  AC_MSG_RESULT(no)
fi

dnl ####################################################################
dnl #
dnl # Create files.
dnl #
dnl ####################################################################

AC_CONFIG_FILES(
  [
    Makefile
    doc/Makefile
    doc/klel-expr/Makefile
    doc/klelapi/Makefile
    doc/klellang/Makefile
    doc/klelstdlib/Makefile
    doc/kleltut/Makefile
    include/Makefile
    src/Makefile
    src/libklel/Makefile
    src/klel-expr/Makefile
  ]
)

AC_CONFIG_HEADERS([include/config.h])

AC_CONFIG_LINKS(
  [
    include/klel.h:include/klel.h
    include/klel-pstdint.h:include/pstdint.h
  ]
)

AC_OUTPUT

