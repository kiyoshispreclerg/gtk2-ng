#! /bin/sh

if test "x$gtk_all_c_sources" = x; then
	echo gtk_all_c_sources variable not defined
	exit 1
fi

srcdir=${srcdir-.}

grep 'IN_FILE' "$srcdir/gtk.symbols" | sed 's/^[^(]*(//;s/).*//;s/__GTK_MARSHAL_MARSHAL_C__/__gtk_marshal_MARSHAL_C__/' | grep __ | sort -u > expected-files
{
	for file in $gtk_all_c_sources; do
		test -f "$file" || file="$srcdir/$file"
		grep '^ *# *define __' "$file"
	done
} | sed 's/.*define //;s/ *$//;s/__GTK_MARSHAL_MARSHAL_C__/__gtk_marshal_MARSHAL_C__/' | sort > actual-files

diff expected-files actual-files && rm -f expected-files actual-files
