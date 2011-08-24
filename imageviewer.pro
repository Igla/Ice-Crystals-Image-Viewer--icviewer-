HEADERS       = imageviewer.h \
    msettings.h \
    mmover.h \
    mmovinglabel.h \
    mfileiterator.h
SOURCES       = imageviewer.cpp \
                main.cpp \
    msettings.cpp \
    mmovinglabel.cpp \
    mfileiterator.cpp

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/widgets/imageviewer
#sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS imageviewer.pro
#sources.path = $$[QT_INSTALL_EXAMPLES]/widgets/imageviewer
#INSTALLS += target sources

TARGET  =   icviewer

symbian: include($$PWD/../../symbianpkgrules.pri)

#Symbian has built-in component named imageviewer so we use different target
symbian: TARGET = imageviewerexample

wince*: {
   DEPLOYMENT_PLUGIN += qjpeg qmng qgif
}
maemo5: include($$PWD/../../maemo5pkgrules.pri)

symbian: warning(This example might not fully work on Symbian platform)
maemo5: warning(This example might not fully work on Maemo platform)
simulator: warning(This example might not fully work on Simulator platform)

#DEFINES += __LOAD_IN_MAIN_THREAD

debug {
    DEFINES += __DEBUG
}
