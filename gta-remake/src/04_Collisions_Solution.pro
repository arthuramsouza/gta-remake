QT       -= core
QT       -= gui

CONFIG   += console
CONFIG   += opengl
CONFIG   -= app_bundle
CONFIG   += warn_off

TEMPLATE = app

#DESTDIR = ../../../bin

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wno-ignored-qualifiers

SOURCES += \
    main.cpp \
    PlayState_Solution.cpp

HEADERS += \
    MenuState.h \
    PlayState_Solution.h

###############################################
# CONFIGURATION
###############################################

#CONFIG += copy_dir_files

macx {
    QMAKE_CXXFLAGS += -stdlib=libc++
    CONFIG_PLATFORM_PATH  = mac
    CONFIG_ICON_EXTENSION = icns

    ICON = $${PWD}/resources/$${CONFIG_PLATFORM_PATH}/app.$${CONFIG_ICON_EXTENSION}
}

win32 {
    CONFIG_PLATFORM_PATH  = win32
    CONFIG_ICON_EXTENSION = png

    RC_FILE = $${PWD}/resources/$${CONFIG_PLATFORM_PATH}/app.rc
}

unix:!macx {
    CONFIG_PLATFORM_PATH  = unix
    CONFIG_ICON_EXTENSION = png

    ICON = $${PWD}/resources/$${CONFIG_PLATFORM_PATH}/app.$${CONFIG_ICON_EXTENSION}
}

###############################################
# SFML LIBRARY
###############################################

INCLUDEPATH += ../../lib/CGF/include ../../lib/CGF/include/tinyxml ../../lib/CGF/include/tmxloader

#INCLUDEPATH += "$${PWD}/library/sfml/include"
LIBS += \#-L"$${PWD}/library/sfml/$${CONFIG_PLATFORM_PATH}/lib" \
    -lsfml-system \
    -lsfml-window \
    -lsfml-graphics \
    -lsfml-audio \
    -lsfml-network \
    -lBox2D \
    -lz

macx {
     LIBS += -stdlib=libc++ -L$${PWD}/../../lib -lCGF-OSX
#    QMAKE_POST_LINK += $(MKDIR) Frameworks &&
#    QMAKE_POST_LINK += $${QMAKE_COPY} -R $${PWD}/library/sfml/$${CONFIG_PLATFORM_PATH}/lib/* Frameworks
}

#win32 {
#    QMAKE_POST_LINK += $${QMAKE_COPY} $${PWD}/library/sfml/$${CONFIG_PLATFORM_PATH}/bin/* $(OBJECTS_DIR)
#}

unix:!macx {
    INCLUDEPATH += ../../lib/Box2D-2.3.0/include ../../lib/SFML-2.2/include
    CURLIB = $$LIBS
    LIBS = -L$${PWD}/../../lib/Box2D-2.3.0/lib-Linux64 -L$${PWD}/../../lib/SFML-2.2/lib-Linux64 -L$${PWD}/../../lib
    LIBS += $$CURLIB -lGLEW -lCGF-Linux -lz -lBox2D
}

OTHER_FILES += \
    resources/win32/app.rc
