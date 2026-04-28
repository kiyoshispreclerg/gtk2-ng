include $(top_srcdir)/Makefile.decl

EXTRA_DIST += 			\
	gtk-faq.sgml

if HAVE_DOCBOOK
FAQ_HTML_TARGET = html-docbook
FAQ_PDF_TARGET = pdf-docbook
FAQ_DIST_TARGET = dist-hook-docbook

all-local: html
else
FAQ_HTML_TARGET = html-no-docbook
FAQ_PDF_TARGET = pdf-no-docbook
FAQ_DIST_TARGET = dist-hook-no-docbook
endif

html: $(FAQ_HTML_TARGET)

html-docbook:
	if test -w $(srcdir); then \
	  (cd $(srcdir); \
	   db2html gtk-faq.sgml; \
	   test -d html && rm -r html; \
	   mv gtk-faq html); \
	fi

html-no-docbook:
	echo "***"
	echo "*** Warning: FAQ not built"
	echo "***"

pdf: $(FAQ_PDF_TARGET)

pdf-docbook:
	if test -w $(srcdir); then \
	  (cd $(srcdir); db2pdf gtk-faq.sgml); \
	fi

pdf-no-docbook:
	echo "***"
	echo "*** Warning: FAQ not built"
	echo "***"

dist-hook: $(FAQ_DIST_TARGET)

dist-hook-docbook: html
	cp -Rp $(srcdir)/html $(distdir)

dist-hook-no-docbook:
	echo "***"
	echo "*** Warning: FAQ not built"
	echo "*** DISTRIBUTION IS INCOMPLETE"
	echo "***"

.PHONY: html pdf html-docbook html-no-docbook pdf-docbook pdf-no-docbook dist-hook-docbook dist-hook-no-docbook

-include $(top_srcdir)/git.mk
