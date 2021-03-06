TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


QMAKE_CXX = mpicxx
QMAKE_LINK = $$QMAKE_CXX
QMAKE_CC = mpicc

QMAKE_CFLAGS += $$system(mpicc --showme:compile)
QMAKE_LFLAGS += $$system(mpicxx --showme:link)
QMAKE_CXXFLAGS += $$system(mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK
QMAKE_CXXFLAGS_RELEASE += $$system(mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK

SOURCES += \
    date.c \
    event.c \
    tools.c \
    menu.c \
    odd.c \
    even.c \
    db.c

HEADERS += \
    date.h \
    event.h \
    segun.h \
    tools.h \
    menu.h \
    odd.h \
    even.h

LIBS += -lpthread -lrt
