/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QPrinter>
#include <QShortcut>

#ifndef __LOAD_IN_MAIN_THREAD
#include <QFutureWatcher> // не знаю, как без header-а объявить
#endif

#include "mmovinglabel.h"

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
class QCursor;
class QKeyEvent;
class QResizeEvent;
//template <typename T>
//class QFutureWatcher;
class QImage;
QT_END_NAMESPACE

class ImageViewer;
class CustomizeViewDialog;

typedef struct GrabState {
  void (ImageViewer::*move)(const QPoint &delta);
  bool (ImageViewer::*isMoving)() const;
  void (ImageViewer::*startMoving)();
  void (ImageViewer::*endMoving)();

  GrabState():move(0),isMoving(0),startMoving(0),endMoving(0) { }
} GrabState;

//! [0]
class ImageViewer : public QMainWindow, public MMover
{
    Q_OBJECT

public:
    ImageViewer();
    ~ImageViewer();

    //Mover
    void move(const QPoint &delta);
    bool isMooving() const;
    void startMooving();
    void endMooving();

protected:
    void changeEvent(QEvent *ev);
//    void keyPressEvent(QKeyEvent* event);
//    void keyReleaseEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent* event);
    void mouseDoubleClickEvent(QMouseEvent *ev);
    void showEvent(QShowEvent *ev);
private:
    void (ImageViewer::*showEventFunc)(QShowEvent *ev);

    void firstShowEvent(QShowEvent *ev);
    void otherShowEvent(QShowEvent *ev);

private slots:
    // Костыли на горячие клавиши. Какое-то разочарование на то как они в Qt реализованы в связке с QAction
    //Если просто соединять сигнал в сигнал, то у action не меняется состояние checked, если менюшка с ней скрыта
    void fitToWindowKostil();
    void fitToImageKostil();
    void changeModeKostil();
    //-------------------------


    void showContextMenu(const QPoint& point);

    void open();
    void print();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void fitToImage();
    void about();

    void changeMode();

#ifndef __LOAD_IN_MAIN_THREAD
    void imageLoaded();
#endif

    //sort slots
    void sortUnsort(); //названьечко
    void sortByName();
    void sortByTime();
    void sortBySize();
    void sortByType();
    void sortReverse();
    void sortCaseSensetive();

    //Go slots
    void goFirst();
    void goLast();
    void goNext();
    void goPrevious();
    void goUp();
    void goStart();

    void setGrabStateDefault();
    void setGrabStateNormalSize();
    void setGrabStateFitToWindow();
    void setGrabStateScale();

    void customizeView();

private:
    void open(const QString &fileName, bool showErrorMessage=false);
    void setEmptyLabel(const QString &label);
    void setInfoLabels(const QString &fileName=QString());

#ifndef __LOAD_IN_MAIN_THREAD
    void loadImage(const QString &fileName);
#endif
    void setGrabState();
    void loadSortSettings();
    void storeSortSettings();
    void storeGrabMode();

    void restoreGeometry();
    void restoreImageViewOptions();
    void setZoomLabels();

    void checkZoomInAct(double factor);
    void checkZoomOutAct(double factor);
    void setZoom();

    void setFullScreenMode();
    void setWindowMode();

    void moveScroll(QScrollBar *bar,int delta);

    void createActions();
    void createMenus();
//    void updateActions();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    void moveDefault(const QPoint &delta);
    bool isMoovingDefault() const;
    void startMoovingDefault();
    void endMoovingDefault();

    void startMoovingFitToWindow();
    void endMoovingFitToWindow();

    void startMoovingNormalSize();

    void startMoovingScale();
    void endMoovingScale();

    void storeScrollProportion();

    void acceptCustomizeDlg(const CustomizeViewDialog &dlg);

    void activateShortcuts(bool enabled);

    QRect getContentRect() const;

    bool scaleFactorLegal(double factor) const;
    bool scaleFactorLegalLowBorder(double factor) const;
    bool scaleFactorLegalHighBorder(double factor) const;

    void setViewStyle();

    MMovingLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor,_oldScaleFactor;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    GrabState grabState;

    QAction *openAct;
    QAction *printAct;
    QAction *exitAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    QAction *fitToImageAct;

    QActionGroup *fitGroup;

    QAction *customizeViewAct;

    QActionGroup *sortGroup;
    QAction *sortUnsortAct;
    QAction *sortNameAct;
    QAction *sortSizeAct;
    QAction *sortTimeAct;
    QAction *sortTypeAct;
    QAction *sortReversedAct;
    QAction *sortCaseSensetiveAct;

    QActionGroup *grabGroup;
    QAction *grabDefaultAct;
    QAction *grabFitToWindowAct;
    QAction *grabNormalSizeAct;
    QAction *grabScaleAct;

    QAction *goFirstAct;
    QAction *goLastAct;
    QAction *goNextAct;
    QAction *goPreviousAct;
    QAction *goUpAct;
    QAction *goStartAct;

    QAction *fullScreenAct;

    QMenu *sortMenu;
    QMenu *goMenu;
    QMenu *grabMenu;

    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    QCursor _lastCursor;
    QList<QShortcut*> shortCuts;

    int _lastFileFilterNum;

    int _frameStyle;
    int _storedWidth,_storedHeight;

    float scrollValH,scrollValV;

#ifndef __LOAD_IN_MAIN_THREAD
    QImage _image;
    QFutureWatcher<void> _loaderTask;
    bool _showErrorMessage;
#endif
};
//! [0]

#endif
