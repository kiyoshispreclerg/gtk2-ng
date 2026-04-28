#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir
PROJECT=Gtk+
TEST_TYPE=-d
FILE=gdk

DIE=0

if libtoolize --version < /dev/null > /dev/null 2>&1 ; then : ; else
	echo
	echo "You must have libtool installed to compile $PROJECT."
	echo "Install the appropriate package for your distribution,"
	echo "or get the source tarball at http://ftp.gnu.org/gnu/libtool/"
	DIE=1
fi

(gtkdocize --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have gtk-doc installed to compile $PROJECT."
	echo "Install the appropriate package for your distribution,"
	echo "or get the source tarball at http://ftp.gnome.org/pub/GNOME/sources/gtk-doc/"
	DIE=1
}

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to compile $PROJECT."
	echo "Install the appropriate package for your distribution,"
	echo "or get the source tarball at http://ftp.gnu.org/gnu/autoconf/"
	DIE=1
}

if test -z "$AUTOMAKE"; then
	for am_version in 1.18 1.17 1.16 1.15 1.14 1.13 1.12 1.11 1.10 1.7; do
		if automake-$am_version --version < /dev/null > /dev/null 2>&1 ; then
			AUTOMAKE=automake-$am_version
			ACLOCAL=aclocal-$am_version
			break
		fi
	done
fi

if test -z "$AUTOMAKE"; then
	if automake --version < /dev/null > /dev/null 2>&1 ; then
		AUTOMAKE=automake
		ACLOCAL=aclocal
	else
		echo
		echo "You must have automake installed to compile $PROJECT."
		echo "Install the appropriate package for your distribution,"
		echo "or get the source tarball at http://ftp.gnu.org/gnu/automake/"
		DIE=1
	fi
fi

if $AUTOMAKE --version < /dev/null > /dev/null 2>&1 ; then : ; else
	echo
	echo "You must have a working automake installed to compile $PROJECT."
	echo "Install the appropriate package for your distribution,"
	echo "or get the source tarball at http://ftp.gnu.org/gnu/automake/"
	DIE=1
fi

if test -z "$ACLOCAL"; then
	case "$AUTOMAKE" in
		automake-*)
			am_aclocal=`echo "$AUTOMAKE" | sed 's/^automake/aclocal/'`
			if $am_aclocal --version < /dev/null > /dev/null 2>&1 ; then
				ACLOCAL=$am_aclocal
			else
				ACLOCAL=aclocal
			fi
			;;
		*)
			ACLOCAL=aclocal
			;;
	esac
fi

if $ACLOCAL --version < /dev/null > /dev/null 2>&1 ; then : ; else
	echo
	echo "You must have aclocal installed to compile $PROJECT."
	echo "Install the appropriate package for your distribution,"
	echo "or get the source tarball at http://ftp.gnu.org/gnu/automake/"
	DIE=1
fi

if test "$DIE" -eq 1; then
	exit 1
fi

test $TEST_TYPE $FILE || {
	echo "You must run this script in the top-level $PROJECT directory"
	exit 1
}

# NOCONFIGURE is used by gnome-common; support both
if ! test -z "$AUTOGEN_SUBDIR_MODE"; then
    NOCONFIGURE=1
fi

if test -z "$NOCONFIGURE"; then
        if test -z "$*"; then
                echo "I am going to run ./configure with no arguments - if you wish "
                echo "to pass any to it, please specify them on the $0 command line."
        fi
fi

if test -z "$ACLOCAL_FLAGS"; then

	acdir=`$ACLOCAL --print-ac-dir`
        m4list="glib-2.0.m4 glib-gettext.m4"

	for file in $m4list
	do
		if [ ! -f "$acdir/$file" ]; then
			echo "WARNING: aclocal's directory is $acdir, but..."
			echo "         no file $acdir/$file"
			echo "         You may see fatal macro warnings below."
			echo "         If these files are installed in /some/dir, set the ACLOCAL_FLAGS "
			echo "         environment variable to \"-I /some/dir\", or install"
			echo "         $acdir/$file."
			echo ""
		fi
	done
fi

rm -rf autom4te.cache

# Automake runs in foreign mode; README and INSTALL are not required
# merely to bootstrap the build tree.

$ACLOCAL -I m4 $ACLOCAL_FLAGS || exit $?

libtoolize --copy --force || exit $?
gtkdocize --copy || exit $?

autoheader || exit $?

$AUTOMAKE --add-missing || exit $?
autoconf || exit $?
cd $ORIGDIR || exit $?

if test -z "$NOCONFIGURE"; then
        $srcdir/configure --enable-maintainer-mode $AUTOGEN_CONFIGURE_ARGS "$@" || exit $?

        echo 
        echo "Now type 'make' to compile $PROJECT."
fi
