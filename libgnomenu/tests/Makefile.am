include $(top_srcdir)/Makefile.gtest

if ENABLE_TESTS
TEST_PROGS +=  $(noinst_PROGRAMS)
noinst_PROGRAMS =$(VALASOURCES:.vala=)
endif

#VALASOURCES = test-xlib.vala test-label.vala test-grab.vala test-parser.vala  test-menubar.vala test-window.vala test-leak.vala test-globalmenu.vala test-application.vala

VALASOURCES = test-adapter.vala test-globalmenuitem.vala test-menubar.vala test-globalmenu.vala

STAMP = $(VALASOURCES:.vala=.c)
SUFFIXES = .vala .c

.vala.c:
	$(VALAC) $(VALA_DEFAULT_PKGS) -C --vapidir=$(top_srcdir)/tests --vapidir=$(top_srcdir)/libgnomenu --pkg testman --pkg gtk+-2.0 --pkg gnomenu $^ 
	if [ $@ == test-leak.c ]; then bash patch.sh; fi;
	touch $@

AM_CPPFLAGS = -I$(top_srcdir)/libgnomenu -I$(top_srcdir)/tests $(GTK_CFLAGS) $(GLIB_CFLAGS) $(WNCK_CFLAGS) -include $(CONFIG_HEADER) 
BUILT_SOURCES = $(STAMP)
LDADD = $(GTK_LIBS) $(GLIB_LIBS) $(WNCK_LIBS) ../libgnomenu.la ../../tests/libtestman.la
EXTRA_DIST += $(STAMP) $(VALASOURCES) evo-nolabel.xml evo.xml
