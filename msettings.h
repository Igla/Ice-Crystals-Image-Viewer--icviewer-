//============================================================================
// Name        : msettings.h
// Author      : Igla
// Version     :
// Copyright   : 2011 Ice Crystals, Lopasoft corporation and this fucking world
// Description : Тут у нас настроечки.
//============================================================================

#ifndef DBSETTINGS_H
#define DBSETTINGS_H

#include "mfileiterator.h"
class QSettings;

#define SCALE_LIMIT_MIN 3

class MSettings
{
public:
    enum GrabMode {
        GrabDefault,
        GrabNormalSize,
        GrabScale,
        GrabFitToWindow
    };

    //useGlobalSettings - принудительно загрузить глобальные настройкил
    MSettings(bool useGlobalSettings=false);

    //Путь к настройкам пользователя
    static const QString& userSettingsPath();
    // сохранить настройки
    void storeUserSettings() const;

    //стандартный шаг масштабирования изображения
    inline float scaleDelta() const { return _scaleDelta; }
    inline void setScaleDelta(float delta) {
        if(_scaleDelta!=delta) {
            _scaleDelta = delta;
            setChanged();
        }
    }

    //шаг масштабирования в режиме увеличения по нажатию мыши
    inline float grabScale() const { return _grabScale; }
    inline void setGrabScale(float scale) {
        if(scale>scaleLimit())
            scale = scaleLimit();
        if(_grabScale!=scale) {
            _grabScale = scale;
            setChanged();
        }
    }

    inline GrabMode grabMode() const { return _grabMode; }
    inline void setGrabMode(GrabMode mode) {
        if(GrabMode()!=mode) {
            _grabMode = mode;
            setChanged();
        }
    }

    inline static GrabMode getGrabMode(int mode) {
        GrabMode ret;
        switch(mode) {
            case GrabNormalSize:
                ret = GrabNormalSize;
            break;
            case GrabScale:
                ret = GrabScale;
            break;
            case GrabFitToWindow:
                ret = GrabFitToWindow;
            break;
            default:
                ret = GrabDefault;
        }
        return ret;
    }

    //максимальное увеличение. Так, костыль в связи с нежеланием копаться
    //в способах выделение памяти при масштабировании QLabel
    inline float scaleLimit() const { return _scaleLimit; }
    inline void setScaleLimit(float limit) {
        if(_scaleLimit!=limit && limit>=SCALE_LIMIT_MIN) {
            _scaleLimit= limit;
            setChanged();
        }
    }

    // Memory limit for one bitmap image in megabytes // real pixmap occupies much more space
    // Реально весьма условная величина. Значение по умолчанию подобрал так, чтобы совместно с
    // scaleLimit на моих 2Гб оперативной памяти не приходилось бы использовать swap иначе смерть.
    inline int memoryLimit() const { return _memoryLimit; }
    inline void setMemoryLimit(int limit) {
        if(_memoryLimit!=limit) {
            _memoryLimit = limit;
            setChanged();
        }
    }

    // Скороость с которой таскается картинка относительно скорости движения мыши.
    // В конфиге хранится в виде целого числа. Просто надо поделить на 100.
    inline float mouseGrabSpeed() const { return _mouseGrabSpeed; }
    inline void setMouseGrabSpeed(float speed) {
        if(_mouseGrabSpeed!=speed) {
            _mouseGrabSpeed = speed;
            setChanged();
        }
    }

    //флаги сортировки
    inline QDir::SortFlags sorting() const {return _sorting;}
    inline void setSorting(QDir::SortFlags flags) {
        if(sorting()!=flags) {
            _sorting = flags;
            setChanged();
        }
    }

    // Если набор QDir::SortFlags и/или способы его использования изменятся в следующих выпусках
    // Qt(4.7.3), то это надо будет отразить в сей функции
    static QDir::SortFlags getSortFlags(int value);

    // Для сохранения состояния главного окна
    inline Qt::WindowStates windowState() const {return _windowState;}
    inline void setWindowState(Qt::WindowStates state) {
        if(windowState()!=state) {
            _windowState = state;
            setChanged();
        }
    }

    // Если набор Qt::WindowStates и/или способы его использования изменятся в следующих выпусках
    // Qt(4.7.3), то это надо будет отразить в сей функции
    static Qt::WindowStates getWindowState(int value);

    // Ширина главного окна
    inline int windowWidth() const { return _windowWidth; }
    inline void setWindowWidth(int width) {
        if(windowWidth()!=width && widthAvailable(width)) {
            _windowWidth = width;
            setChanged();
        }
    }

    // Высота главного окна
    inline int windowHeight() const { return _windowHeight; }
    inline void setWindowHeight(int height) {
        if(windowHeight()!=height && heightAvailable(height)) {
            _windowHeight = height;
            setChanged();
        }
    }

    // Нужно ли подгонять изображение к размеру окна
    inline bool fitToScreen() const { return _fitToScreen==1; }
    inline void setFitToScreen(bool flag) {
        if(fitToScreen()!=flag ) {
            _fitToScreen = flag?1:0;
            setChanged();
        }
    }

    // Нужно ли подгонять изображение к размеру окна
    inline bool fitToImage() const { return _fitToScreen==2; }
    inline void setFitToImage(bool flag) {
        if(fitToImage()!=flag ) {
            _fitToScreen = flag?2:0;
            setChanged();
        }
    }


    // Изменились ли настройки в процессе работы программы
    inline bool isChanged() const { return _changeFlag; }

    // Функции проверяют возможность использования указанных размеров для рабочего стола на котором, теоретически,
    // запущено данное прилоэение.
    // Если при использовании нескольких экранов возникнут ошибки,
    // для определения экрана надо использовать виджет главного окна, который передавать параметром
    // или получать из QApplication
    static bool heightAvailable(int height);
    static bool widthAvailable(int width);

    static QString loadStyleSheet();

private:
    inline void setChanged(bool changed=true) { _changeFlag = changed; }
    void loadSettings(const QSettings &settings);

    QDir::SortFlags _sorting;
    Qt::WindowStates _windowState;
    int _memoryLimit;
    float _scaleLimit, _scaleDelta,_mouseGrabSpeed, _grabScale;
    int _windowWidth, _windowHeight;

    int _fitToScreen;
    GrabMode _grabMode;

    bool _changeFlag;
};

#endif // DBSETTINGS_H

