AC_INIT([bbs], [2.4.0], [bug-automake@gnu.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC_C99

dnl The host is just used for informative output (in unix.c). We could easily remove it.
AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED([HOSTTYPE],["$host"],[canonical host])

AC_CONFIG_HEADERS([config.h])
AC_CHECK_HEADERS([termios.h iconv.h])
AC_CONFIG_FILES([
 Makefile
])
AC_OUTPUT
