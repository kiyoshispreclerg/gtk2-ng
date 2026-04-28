include $(top_srcdir)/Makefile.decl

.PHONY: html pdf html-docbook html-no-docbook pdf-docbook pdf-no-docbook dist-hook-docbook dist-hook-no-docbook

EXTRA_DIST += \
	gtk-tut.sgml			\
	images/arrow.png		\
	images/aspectframe.png		\
	images/base.png			\
	images/buttonbox.png		\
	images/buttons.png		\
	images/calendar.png		\
	images/colorsel.png		\
	images/entry.png		\
	images/eventbox.png		\
	images/filesel.png		\
	images/fixed.png		\
	images/frame.png		\
	images/gtkdial.png		\
	images/helloworld.png		\
	images/helloworld2.png		\
	images/label.png		\
	images/menu.png			\
	images/notebook.png		\
	images/packbox1.png		\
	images/packbox2.png		\
	images/paned.png		\
	images/progressbar.png		\
	images/radiobuttons.png		\
	images/rangewidgets.png		\
	images/rulers.png		\
	images/scribble.png		\
	images/scrolledwin.png		\
	images/spinbutton.png		\
	images/statusbar.png		\
	images/table.png		\
	images/tictactoe.png

if HAVE_DOCBOOK
TUTORIAL_HTML_TARGET = html-docbook
TUTORIAL_PDF_TARGET = pdf-docbook
TUTORIAL_DIST_TARGET = dist-hook-docbook

all-local: html
else
TUTORIAL_HTML_TARGET = html-no-docbook
TUTORIAL_PDF_TARGET = pdf-no-docbook
TUTORIAL_DIST_TARGET = dist-hook-no-docbook
endif

html: $(TUTORIAL_HTML_TARGET)

html-docbook: gtk-tut.sgml
	if test -w $(srcdir); then \
	  (cd $(srcdir); \
	   db2html gtk-tut.sgml; \
	   test -d html && rm -r html; \
	   mv gtk-tut html; \
	   mkdir html/images; \
	   cp images/*.png html/images); \
	fi

html-no-docbook:
	echo "***"
	echo "*** Warning: Tutorial not built"
	echo "***"

pdf: $(TUTORIAL_PDF_TARGET)

pdf-docbook: gtk-tut.sgml
	if test -w $(srcdir); then \
	  (cd $(srcdir); db2pdf gtk-tut.sgml); \
	fi

pdf-no-docbook:
	echo "***"
	echo "*** Warning: Tutorial not built"
	echo "***"

dist-hook: $(TUTORIAL_DIST_TARGET)

dist-hook-docbook: html
	cp -Rp $(srcdir)/html $(distdir)

dist-hook-no-docbook:
	echo "***"
	echo "*** Warning: Tutorial not built"
	echo "*** DISTRIBUTION IS INCOMPLETE"
	echo "***"

-include $(top_srcdir)/git.mk
