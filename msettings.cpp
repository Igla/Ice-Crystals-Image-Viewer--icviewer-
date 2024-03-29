//============================================================================
// Name        : msettings.cpp
// Author      : Igla
// Version     :
// Copyright   : 2011 Ice Crystals, Lopasoft corporation and this fucking world
// Description : Тут у нас настроечки.
//============================================================================

#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDesktopWidget>
#include <QApplication>

#ifdef __DEBUG
#include <QDebug>
#endif

#include "msettings.h"

static const QString CONF_NAME("iciv.cnf");
static const QString STYLE_NAME("style.main");
static const QString GLOBAL_SETTINGS_PATH("/etc/ice-crystals/");
static const QString USER_SETTINGS_PATH(QDir::homePath()+ "/.config/ice-crystals/ICImageViewer/");
#define GLOBAL_SETTINGS_CONF (GLOBAL_SETTINGS_PATH+CONF_NAME)
static const QString USER_SETTINGS_CONF(USER_SETTINGS_PATH+CONF_NAME);

#define GLOBAL_SETTINGS_STYLE (GLOBAL_SETTINGS_PATH+STYLE_NAME)
#define USER_SETTINGS_STYLE (USER_SETTINGS_PATH+STYLE_NAME)

// I think there is no need for style sheets bigger then 1Mb
#define MAX_STYLE_SIZE 1048576

#define DEFAULT_SCALE_DELTA 25
static const QString ITEM_SCALE_DELTA("scale_delta");

#define DEFAULT_SCALE_LIMIT 1500
static const QString ITEM_SCALE_LIMIT("scale_limit");

static const QString ITEM_MEMORY_LIMIT("memory_limit");
#define DEFAULT_MEMORY_LIMIT 100

static const QString ITEM_MOUSE_GRAB_SPEED("mouse_speed");
#define DEFAULT_MOUSE_SPEED 2.5f


static const QString ITEM_SORT_ORDER("sort_order");
#define DEFAULT_SORT_ORDER int(QDir::Name | QDir::LocaleAware)

static const QString ITEM_WINDOW_WIDTH("window_width");
#define DEFAULT_WINDOW_WIDTH 500

static const QString ITEM_WINDOW_HEIGHT("window_height");
#define DEFAULT_WINDOW_HEIGHT 400

static const QString ITEM_WINDOW_STATE("window_state");
#define DEFAULT_WINDOW_STATE Qt::WindowActive

static const QString ITEM_FIT_TO_SCREEN("fit_to_screen");
#define DEFAULT_FIT_TO_SCREEN 0

static const QString ITEM_GRAB_SCALE("grab_scale");
#define DEFAULT_GRAB_SCALE 200

static const QString ITEM_GRAB_MODE("grab_mode");
#define DEFAULT_GRAB_MODE MSettings::GrabDefault

MSettings::MSettings(bool useGlobalSettings):_changeFlag(false)
{
    QFileInfo f(USER_SETTINGS_CONF);
    #if defined(Q_OS_UNIX)
    if(!useGlobalSettings &&/*f.exists() &&*/ f.isReadable()) {
        QSettings s(USER_SETTINGS_CONF,QSettings::IniFormat);
        loadSettings(s);
    }
    else {
        QSettings s(GLOBAL_SETTINGS_CONF, QSettings::IniFormat);
        loadSettings(s);
    }
    #else
        QSettings s(USER_SETTINGS_CONF,QSettings::IniFormat);
        loadSettings(s);
    #endif

}

void MSettings::loadSettings(const QSettings &settings)
{
     _scaleDelta = (float)settings.value(ITEM_SCALE_DELTA,DEFAULT_SCALE_DELTA).toInt()/100;
     _scaleLimit = (float)settings.value(ITEM_SCALE_LIMIT,DEFAULT_SCALE_LIMIT).toInt()/100;
     if(_scaleLimit<SCALE_LIMIT_MIN)
         _scaleLimit = SCALE_LIMIT_MIN;
     _grabScale = (float)settings.value(ITEM_GRAB_SCALE,DEFAULT_GRAB_SCALE).toInt()/100;
     if(_grabScale>_scaleLimit)
        _grabScale = _scaleLimit;
     _grabMode = getGrabMode(settings.value(ITEM_GRAB_MODE,DEFAULT_GRAB_MODE).toInt());

     _memoryLimit = settings.value(ITEM_MEMORY_LIMIT,DEFAULT_MEMORY_LIMIT).toInt();
     _mouseGrabSpeed = settings.value(ITEM_MOUSE_GRAB_SPEED,DEFAULT_MOUSE_SPEED).toFloat();
     _sorting = getSortFlags(settings.value(ITEM_SORT_ORDER,DEFAULT_SORT_ORDER).toInt());

     _windowState = getWindowState(settings.value(ITEM_WINDOW_STATE,DEFAULT_WINDOW_STATE).toInt());

     _windowWidth = settings.value(ITEM_WINDOW_WIDTH ,DEFAULT_WINDOW_WIDTH).toInt();
     if(!widthAvailable(_windowWidth))
         _windowWidth = DEFAULT_WINDOW_WIDTH;
     _windowHeight = settings.value(ITEM_WINDOW_HEIGHT,DEFAULT_WINDOW_HEIGHT).toInt();
     if(!heightAvailable(_windowHeight))
         _windowHeight = DEFAULT_WINDOW_HEIGHT;

     _fitToScreen = settings.value(ITEM_FIT_TO_SCREEN,DEFAULT_FIT_TO_SCREEN).toInt();
}

void MSettings::storeUserSettings() const
{
    QFileInfo f(USER_SETTINGS_CONF);
    if(!f.exists()){
        QDir d(f.dir());
        if(!d.exists())
            d.mkpath(".");
    }

    QSettings settings(USER_SETTINGS_CONF,QSettings::IniFormat);
    settings.setValue(ITEM_SCALE_DELTA,(int)(scaleDelta()*100));
    settings.setValue(ITEM_SCALE_LIMIT,(int)(scaleLimit()*100));
    settings.setValue(ITEM_GRAB_SCALE,(int)(grabScale()*100));
    settings.setValue(ITEM_GRAB_MODE,(int)(grabMode()));
    settings.setValue(ITEM_MEMORY_LIMIT,memoryLimit());
    settings.setValue(ITEM_MOUSE_GRAB_SPEED,mouseGrabSpeed());
    settings.setValue(ITEM_SORT_ORDER,int(sorting()));
    settings.setValue(ITEM_WINDOW_STATE,int(windowState()));

    settings.setValue(ITEM_WINDOW_WIDTH,windowWidth());
    settings.setValue(ITEM_WINDOW_HEIGHT,windowHeight());

    settings.setValue(ITEM_FIT_TO_SCREEN,_fitToScreen);
}

inline const QString& MSettings::userSettingsPath()
{
    return USER_SETTINGS_CONF;
}

QDir::SortFlags MSettings::getSortFlags(int value)
{
    QDir::SortFlags ret;
    if(value == QDir::NoSort) {
//        ret = QDir::NoSort;
        ret = QDir::Unsorted;
    }
    else {
        //Порядок важен
        if((value&QDir::Unsorted)==QDir::Unsorted)
            ret = QDir::Unsorted;
        else
        if((value&QDir::Type)==QDir::Type)
            ret = QDir::Type;
        else
        if((value&(QDir::Name|QDir::Time|QDir::Size))==QDir::Name)
            ret = QDir::Name;
        else
        if((value&(QDir::Name|QDir::Time|QDir::Size))==QDir::Size)
            ret = QDir::Size;
        else
        if((value&(QDir::Name|QDir::Time|QDir::Size))==QDir::Time)
            ret = QDir::Time;

        if((value&QDir::Reversed)==QDir::Reversed)
            ret |= QDir::Reversed;

        if((value&QDir::IgnoreCase)!=QDir::IgnoreCase)
            ret |= QDir::IgnoreCase;

    }
    ret |= QDir::LocaleAware;

    return ret;
}

Qt::WindowStates MSettings::getWindowState(int value)
{
    Qt::WindowStates ret;

    if((value & Qt::WindowMaximized) == Qt::WindowMaximized)
        ret = Qt::WindowMaximized;
    else
    if((value & Qt::WindowMinimized) == Qt::WindowMinimized)
        ret = Qt::WindowMinimized;
    else
    if((value & Qt::WindowFullScreen) == Qt::WindowFullScreen)
        ret = Qt::WindowFullScreen;
    else
        ret = Qt::WindowNoState;

    if((value & Qt::WindowActive) == Qt::WindowActive)
        ret |= Qt::WindowActive;

    return ret;
}

bool MSettings::heightAvailable(int height)
{
    return height>0 && height <= QApplication::desktop()->availableGeometry().height();
}

bool MSettings::widthAvailable(int width)
{
    return width>0 && width <= QApplication::desktop()->availableGeometry().width();
}

QString MSettings::loadStyleSheet()
{
    QFile file(USER_SETTINGS_STYLE);
    if(!file.exists() || file.size()>MAX_STYLE_SIZE) {
        #ifdef Q_OS_UNIX
        file.setFileName(GLOBAL_SETTINGS_STYLE);
        if(!file.exists() || file.size()>MAX_STYLE_SIZE)
        #endif
            return QString();
    }
    file.open(QIODevice::ReadOnly);
    return QString::fromUtf8(file.readAll().constData());
}
