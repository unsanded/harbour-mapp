# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = mapp

CONFIG += sailfishapp


CONFIG+=C++11


debug{
    CONFIG+=declarative_debug
    CONFIG+=qml_debug
    DEFINES+=_DEBUG
}

QT += gui positioning

SOURCES += \
    src/slippyprovider.cpp \
    src/tile.cpp \
    src/slippycache.cpp \
    src/providers/osmprovider.cpp \
    src/mapp.cpp \
    src/slippyview.cpp \
    src/providers/googlemapsprovider.cpp \
    src/drawable.cpp \
    src/stores/slippytilestore.cpp \
    src/stores/folderstore.cpp \
    src/tilemanager.cpp \
    src/slippycoordinates.cpp

OTHER_FILES += \
    qml/cover/CoverPage.qml \
    qml/pages/FirstPage.qml \
    qml/pages/SecondPage.qml \
    rpm/mapp.changes.in \
    rpm/mapp.spec \
    rpm/mapp.yaml \
    translations/*.ts \
    mapp.desktop \
    qml/mapp.qml \
    qml/pages/Menu.qml \
    qml/pages/ThirdPage.qml \
    qml/pages/FourthPage.qml \
    mapp.png

# to disable building translations every time, comment out the
# following CONFIG line
#CONFIG += sailfishapp_i18n
TRANSLATIONS += translations/map-de.ts

HEADERS += \
    src/slippyprovider.h \
    src/tile.h \
    src/slippycache.h \
    src/providers/osmprovider.h \
    src/slippyview.h \
    src/providers/googlemapsprovider.h \
    src/drawable.h \
    src/stores/slippytilestore.h \
    src/stores/folderstore.h \
    src/tilemanager.h \
    src/slippycoordinates.h

