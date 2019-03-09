#-------------------------------------------------
#
# Project created by QtCreator 2012-04-06T22:13:38
#
#-------------------------------------------------

QT += widgets \
    svg

TARGET = Biorytm
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++17 \
    -Wall \
    -Wextra \
    -Wpedantic \
    -Wconversion

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT \
        QT_NO_INFO_OUTPUT \
        QT_NO_WARNING_OUTPUT
}

win32:CONFIG(release, debug|release) {
    #QMAKE_LFLAGS += -static-libstdc++ -static-libgcc
    #LIBS += -lstdc++

    # application icon and information
    RC_FILE = Biorytm.rc

    # manifest
    WINSDK_DIR = C:\\Program Files (x86)\\Microsoft SDKs\\Windows\\v7.1a
    WIN_PWD = $$replace(PWD, /, \\)
    WIN_OUT_PWD = $$replace(OUT_PWD, /, \\)
    QMAKE_POST_LINK += "$$WINSDK_DIR\\bin\\x64\\mt.exe -manifest $$quote($$WIN_PWD\\${TARGET}.manifest) -outputresource:$$quote($$WIN_OUT_PWD\\${DESTDIR_TARGET};1)"
}

SOURCES += main.cpp \
    mainwindow.cpp \
    biorhythmgraphwidget.cpp \
    optionsdialog.cpp \
    aboutdialog.cpp \
    usersmanagerdialog.cpp \
    settings.cpp \
    userswidget.cpp \
    userframe.cpp \
    usereditwidget.cpp \
    mylabel.cpp \
    licensedialog.cpp \
    examplesavatarsdialog.cpp \
    helpdialog.cpp

HEADERS += mainwindow.hpp \
    aboutdialog.hpp \
    biorhythmgraphwidget.hpp \
    examplesavatarsdialog.hpp \
    helpdialog.hpp \
    licensedialog.hpp \
    mylabel.hpp \
    optionsdialog.hpp \
    settings.hpp \
    usereditwidget.hpp \
    userframe.hpp \
    usersmanagerdialog.hpp \
    userswidget.hpp \
    constants.hpp

FORMS += mainwindow.ui \
    usersmanagerdialog.ui \
    optionsdialog.ui \
    aboutdialog.ui \
    usereditwidget.ui \
    licensedialog.ui \
    examplesavatarsdialog.ui \
    helpdialog.ui

RESOURCES += images.qrc \
    avatars.qrc \
    other.qrc

OTHER_FILES +=

DISTFILES +=
