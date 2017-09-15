#-------------------------------------------------
#
# Project created by QtCreator 2017-09-02T10:22:27
#
#-------------------------------------------------

QT       += core gui\
		network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Ronin
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    r2handler.cpp \
    highlighters/disassemblyhighlighter.cpp \
    disassemblycore.cpp \
    files.cpp \
    resultsdialog.cpp \
    r2pipe/r2pipe-api.cxx \
    r2pipe/r2pipe.cxx \
    widgets/symbolstablewidget.cpp \
    widgets/fileinfotablewidget.cpp \
    widgets/relocationstable.cpp \
    widgets/importstablewidget.cpp \
    highlighters/pseudocodehighlighter.cpp

HEADERS  += mainwindow.h \
    r2handler.h \
    highlighters/disassemblyhighlighter.h \
    disassemblycore.h \
    files.h \
    resultsdialog.h \
    r2pipe/r2pipe.h \
    widgets/symbolstablewidget.h \
    widgets/fileinfotablewidget.h \
    widgets/relocationstable.h \
    widgets/importstablewidget.h \
    highlighters/pseudocodehighlighter.h

FORMS    += mainwindow.ui \
    resultsdialog.ui

INCLUDEPATH += /home/jbl/Desktop/radare2-1.6.0/libr/include
LIBS += -L/usr/local/radare2/lib -lr_core -lr_util

RESOURCES += \
    resources.qrc

DISTFILES +=
