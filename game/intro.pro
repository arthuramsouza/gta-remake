QT       -= core
QT       -= gui

CONFIG   += console
CONFIG   += opengl
CONFIG   -= app_bundle

TEMPLATE = app

#DESTDIR = ../../../bin

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp 

###############################################
# CONFIGURATION
###############################################

#CONFIG += copy_dir_files

macx {
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

INCLUDEPATH += ../../include

#INCLUDEPATH += "$${PWD}/library/sfml/include"
LIBS += \#-L"$${PWD}/library/sfml/$${CONFIG_PLATFORM_PATH}/lib" \
    -lsfml-system \
    -lsfml-window \
    -lsfml-graphics \
    -lsfml-audio \
    -lsfml-network \
    -lz

#macx {
#    QMAKE_POST_LINK += $(MKDIR) Frameworks &&
#    QMAKE_POST_LINK += $${QMAKE_COPY} -R $${PWD}/library/sfml/$${CONFIG_PLATFORM_PATH}/lib/* Frameworks
#}

#win32 {
#    QMAKE_POST_LINK += $${QMAKE_COPY} $${PWD}/library/sfml/$${CONFIG_PLATFORM_PATH}/bin/* $(OBJECTS_DIR)
#}

unix:!macx {
    INCLUDEPATH += /usr/local/include/SFML
    LIBS += -lGLEW
}

OTHER_FILES += \
    resources/win32/app.rc
