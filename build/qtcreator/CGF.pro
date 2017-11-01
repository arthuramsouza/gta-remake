QT       -= core
QT       -= gui

CONFIG   += console
CONFIG   += opengl
CONFIG   -= app_bundle
CONFIG   += warn_off
CONFIG   += staticlib

TEMPLATE = lib

#DESTDIR = ../../../bin

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wno-ignored-qualifiers

SOURCES += ../../lib/CGF/source/InputManager.cpp \
    ../../lib/CGF/source/tmxloader/QuadTreeNode.cpp \
    ../../lib/CGF/source/tmxloader/MapLoaderPublic.cpp \
    ../../lib/CGF/source/tmxloader/MapLoaderPrivate.cpp \
    ../../lib/CGF/source/pugixml/pugixml.cpp \
    ../../lib/CGF/source/TextureManager.cpp \
    ../../lib/CGF/source/Sprite.cpp \
    ../../lib/CGF/source/Game.cpp \
    ../../lib/CGF/source/Physics.cpp \
    ../../lib/CGF/source/EEDebugDraw3.cpp

HEADERS += ../../lib/CGF/include/TextureManager.h \
    ../../lib/CGF/include/InputManager.h \
    ../../lib/CGF/include/FrameClock.h \
    ../../lib/CGF/include/ClockHUD.h \
    ../../lib/CGF/include/tmxloader/zlib.h \
    ../../lib/CGF/include/tmxloader/zconf.h \
    ../../lib/CGF/include/tmxloader/QuadTreeNode.h \
    ../../lib/CGF/include/tmxloader/MapObject.h \
    ../../lib/CGF/include/tmxloader/MapLoader.h \
    ../../lib/CGF/include/tmxloader/Helpers.h \
    ../../lib/CGF/include/pugixml/pugixml.hpp \
    ../../lib/CGF/include/pugixml/pugiconfig.hpp \
    ../../lib/CGF/include/Sprite.h \
    ../../lib/CGF/include/Game.h \
    ../../lib/CGF/include/Debug.h \
    ../../lib/CGF/include/GameState.h \
    ../../lib/CGF/include/Anim.h \
    ../../lib/CGF/include/Physics.h \
    ../../lib/CGF/include/EEDebugDraw3.h

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
     LIBS += -stdlib=libc++
     QMAKE_POST_LINK += $${QMAKE_COPY} $${PWD}/../CGF-build-desktop/libCGF.a ../../lib/libCGF-OSX.a
#    QMAKE_POST_LINK += $(MKDIR) Frameworks &&
#    QMAKE_POST_LINK += $${QMAKE_COPY} -R $${PWD}/library/sfml/$${CONFIG_PLATFORM_PATH}/lib/* Frameworks
}

#win32 {
#    QMAKE_POST_LINK += $${QMAKE_COPY} $${PWD}/library/sfml/$${CONFIG_PLATFORM_PATH}/bin/* $(OBJECTS_DIR)
#}

unix:!macx {
    INCLUDEPATH += /usr/local/include/SFML ../../lib/Box2D-2.3.0/include
    LIBS += -lGLEW -L../../lib/Box2D-2.3.0/lib-Linux64
}

OTHER_FILES += \
    resources/win32/app.rc
