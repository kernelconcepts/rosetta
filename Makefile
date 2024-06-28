# Makefile - makefile for rosetta
# (c) 2003 Ole Reinhardt <ole.reinhardt@kernelconcepts.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 
 
CROSSCOMPILE = no
PACKAGE      = rosetta
GTK_PACKAGE  = rosettalearn
CVSBUILD     = no

BUILD = build

VERSION = 0.01

DESTDIR =
PREFIX  = /usr/local
OPTIONS = -DNEW_NORMALIZE_CODE -DNEW_MATCH_CODE

ifeq ($(CROSSCOMPILE),yes)

CC      = arm-linux-gcc
STRIP   = arm-linux-strip

PKG_CONFIG_PATH=/skiff/local/arm-linux/lib/pkgconfig/

LDFLAGS     += -L/skiff/local/arm-linux/lib
LDFLAGS     += -L/skiff/local/arm-linux/lib/X11
LDFLAGS     += -lX11 -lXtst -lXext -lm 

else

CC      = gcc
STRIP   = strip

LDFLAGS     += -L/usr/X11R6/lib -lX11 -lXtst -lm 

endif

CONTROL = control

CFLAGS += -Os -Wall
CFLAGS += -MD $(OPTIONS) -g  
CFLAGS += -DPACKAGE=\"$(PACKAGE)\" -DGTK_PACKAGE=\"$(GTK_PACKAGE)\" -DPREFIX=\"$(PREFIX)\" 
CFLAGS += -DPACKAGE_LOCALE_DIR=\"$(PREFIX)/share/locale\" 
CFLAGS += -DPACKAGE_DATA_DIR=\"$(PREFIX)/share/\"
CFLAGS += -I$(GPE_DIR)/libgpewidget
CFLAGS += -DENABLE_NLS -D_GNU_SOURCE
CFLAGS += $(GPECFLAGS)
#CFLAGS += -DDEBUG

.SUFFIXES: .d .c

MEMBERS      = precalc database analize normalize matching helper 
MEMBERS     += dictionary libvirtkeys configfile main

GTK_MEMBERS  = precalc database analize normalize matching helper 
GTK_MEMBERS += dictionary libvirtkeys configfile callbacks gui_learn learn

OBJS     = $(patsubst %,%.o,$(MEMBERS)) 
GTK_OBJS = $(patsubst %,%.o,$(GTK_MEMBERS)) 

DEPS = $(patsubst %,%.d,$(MEMBERS)) $(patsubst %,%.d,$(GTK_MEMBERS))

all: $(PACKAGE) $(GTK_PACKAGE)

$(PACKAGE): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	
$(GTK_PACKAGE): $(GTK_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(GPELIBS) -lXrender -lXinerama

install-program: all 
	install -D $(PACKAGE) $(DESTDIR)$(PREFIX)/bin/$(PACKAGE)
	install -D $(GTK_PACKAGE) $(DESTDIR)$(PREFIX)/bin/$(GTK_PACKAGE)
	$(STRIP) $(DESTDIR)$(PREFIX)/bin/$(PACKAGE)
	$(STRIP) $(DESTDIR)$(PREFIX)/bin/$(GTK_PACKAGE)
	mkdir   -p $(DESTDIR)$(PREFIX)/share/pixmaps
	mkdir   -p $(DESTDIR)$(PREFIX)/share/$(PACKAGE)
	mkdir   -p $(DESTDIR)/etc
	install -m 644 $(PACKAGE).png  $(DESTDIR)$(PREFIX)/share/pixmaps/$(PACKAGE).png
	install -m 644 penwrite24.png  $(DESTDIR)$(PREFIX)/share/pixmaps/penwrite24.png
	install -m 644 $(PACKAGE).pc   $(DESTDIR)$(PREFIX)/share/$(PACKAGE)
	install -m 666 $(PACKAGE).db   $(DESTDIR)$(PREFIX)/share/$(PACKAGE)
	install -m 644 $(PACKAGE).conf $(DESTDIR)/etc
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications/inputmethods
	install -m 644 $(PACKAGE).desktop $(DESTDIR)$(PREFIX)/share/applications/inputmethods/
	install -m 644 $(GTK_PACKAGE).desktop $(DESTDIR)$(PREFIX)/share/applications/

clean:
	rm -f $(PACKAGE) $(OBJS) $(GTK_PACKAGE) $(GTK_OBJS) $(DEPS)

include $(BUILD)/Makefile.dpkg_ipkg

-include $(DEPS)
