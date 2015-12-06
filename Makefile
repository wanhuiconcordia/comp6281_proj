
MAKEFILE      = Makefile

####### Compiler, tools and options

CC            = mpicc
CXX           = mpicxx
DEFINES       = 
CFLAGS        = -pipe -pthread -O2 -march=x86-64 -mtune=generic -O2 -pipe -fstack-protector-strong -Wall -W -fPIC $(DEFINES)
CXXFLAGS      = -pipe -pthread -DMPICH_IGNORE_CXX_SEEK -O2 -march=x86-64 -mtune=generic -O2 -pipe -fstack-protector-strong -pthread -DMPICH_IGNORE_CXX_SEEK -std=c++0x -Wall -W -fPIC $(DEFINES)
INCPATH       += /usr/include

DEL_FILE      = rm -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p
COPY          = cp -f
COPY_FILE     = cp -f
COPY_DIR      = cp -f -R
INSTALL_FILE  = install -m 644 -p
INSTALL_PROGRAM = install -m 755 -p
INSTALL_DIR   = cp -f -R
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
TAR           = tar -cf
COMPRESS      = gzip -9f

DISTDIR = ./
LINK          = mpicxx

#LFLAGS        = -pthread -Wl,-rpath -Wl,/usr/lib/openmpi -Wl,--enable-new-dtags -L/usr/lib/openmpi -lmpi_cxx -lmpi -Wl,-O1 -Wl,-O1,--sort-common,--as-needed,-z,relro
LIBS          = $(SUBLIBS) -lpthread -lrt 
AR            = ar cqs
RANLIB        = 
SED           = sed
STRIP         = strip

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = date.c \
		event.c \
		tools.c \
		menu.c \
		odd.c \
		even.c \
		db.c 
OBJECTS       = date.o \
		event.o \
		tools.o \
		menu.o \
		odd.o \
		even.o \
		db.o
DIST          = date.h \
		event.h \
		segun.h \
		tools.h \
		menu.h \
		odd.h \
		even.h date.c \
		event.c \
		tools.c \
		menu.c \
		odd.c \
		even.c \
		db.c
DESTDIR       = #avoid trailing-slash linebreak
TARGET        = db


first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)


all: Makefile $(TARGET)

dist: distdir FORCE
	(cd `dirname $(DISTDIR)` && $(TAR) $(DISTNAME).tar $(DISTNAME) && $(COMPRESS) $(DISTNAME).tar) && $(MOVE) `dirname $(DISTDIR)`/$(DISTNAME).tar.gz . && $(DEL_FILE) -r $(DISTDIR)

distdir: FORCE
	@test -d $(DISTDIR) || mkdir -p $(DISTDIR)
	$(COPY_FILE) --parents $(DIST) $(DISTDIR)/


clean:
	-$(DEL_FILE) $(OBJECTS)

distclean: clean 
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


####### Compile

date.o: date.c date.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o date.o date.c

event.o: event.c event.h \
		date.h \
		tools.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o event.o event.c

tools.o: tools.c tools.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o tools.o tools.c

menu.o: menu.c menu.h \
		date.h \
		event.h \
		tools.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o menu.o menu.c

odd.o: odd.c odd.h \
		tools.h \
		event.h \
		date.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o odd.o odd.c

even.o: even.c even.h \
		tools.h \
		event.h \
		date.h \
		menu.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o even.o even.c

db.o: db.c segun.h \
		event.h \
		date.h \
		tools.h \
		odd.h \
		even.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o db.o db.c

####### Install

install:  FORCE

uninstall:  FORCE

FORCE:

