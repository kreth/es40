#! /bin/sh

set -x
aclocal -I m4
# we are currently not using libtool
#libtoolize --force --copy
autoheader
automake --add-missing --copy
autoconf
