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

#include <QtGui>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QScrollArea>
#include <QResizeEvent>
#include <QActionGroup>
#include <QSize>

#ifdef __DEBUG
#include <QDebug>
#endif


#include "QCursor"
#include "imageviewer.h"
#include "msettings.h"
#include "mfileiterator.h"
#include "customizeviewdialog.h"

extern MSettings *APP_SETTINGS;
extern MFileIterator *APP_FILE_ITERATOR;

#define ZOOM_IN 1
#define ZOOM_OUT -1
#define ZOOM_DEFAULT 0

#define ZOOM_IN_FACTOR (1.0+APP_SETTINGS->scaleDelta())
#define ZOOM_OUT_FACTOR (1.0/(1.0f+APP_SETTINGS->scaleDelta()))

#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

#define NOT_NORMAL_STATE (Qt::WindowFullScreen|Qt::WindowMaximized|Qt::WindowMinimized)

int zoom_fl = ZOOM_DEFAULT;
float zoom_back;

ImageViewer::ImageViewer():showEventFunc(&ImageViewer::firstShowEvent),
    _storedWidth(0),_storedHeight(0),
    scaleFactor(1.0),scrollValH(0.0f),scrollValV(0.0f)
{
    QColor color(0,0,0,0);

    imageLabel = new MMovingLabel(this);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    imageLabel->setAlignment(Qt::AlignCenter);

    QFont labelFont(imageLabel->font(), imageLabel);
    int fsize = labelFont.pixelSize();
    if(fsize==-1) {
        fsize = labelFont.pointSize();
        labelFont.setPointSize(fsize<<1);
    }
    else {
        labelFont.setPixelSize(fsize<<1);
    }
    imageLabel->setFont(labelFont);

    QPalette p(imageLabel->palette());
    p.setBrush(QPalette::Background,QBrush(Qt::NoBrush));
    imageLabel->setPalette(p);

    scrollArea = new QScrollArea; 
    scrollArea->setAlignment(Qt::AlignCenter);


    scrollArea->setBackgroundRole(QPalette::Dark);

    createActions();
    createMenus();

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showContextMenu(const QPoint&)));

#ifndef __LOAD_IN_MAIN_THREAD
    connect(&_loaderTask,SIGNAL(finished()),this,SLOT(imageLoaded()));
#endif

    _lastCursor = cursor();

    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);

    loadSortSettings();
    setGrabState();
    restoreGeometry();

    open(APP_FILE_ITERATOR->current());

//    restoreImageViewOptions();
}

ImageViewer::~ImageViewer()
{
    storeSortSettings();
    storeGrabMode();

    APP_SETTINGS->setFitToScreen(fitToWindowAct->isChecked());
    APP_SETTINGS->setFitToImage(fitToImageAct->isChecked());

    APP_SETTINGS->setWindowState(windowState());

    if((windowState()&NOT_NORMAL_STATE)==0) {
        _storedWidth = width();
        _storedHeight = height();
    }

    if(_storedWidth>0 && _storedHeight>0) {
        APP_SETTINGS->setWindowWidth(_storedWidth);
        APP_SETTINGS->setWindowHeight(_storedHeight);
    }
    if(APP_SETTINGS->isChanged())
        APP_SETTINGS->storeUserSettings();
}

void ImageViewer::restoreGeometry() {
   Qt::WindowStates states = APP_SETTINGS->windowState(); 
    resize(APP_SETTINGS->windowWidth(), APP_SETTINGS->windowHeight());

    if((states&Qt::WindowFullScreen)==Qt::WindowFullScreen) {
//        setWindowState(states^Qt::WindowFullScreen);
        fullScreenAct->activate(QAction::Trigger);
        //setFullScreenMode();
    }
    else
        setWindowMode();

    setWindowState(states);

    restoreImageViewOptions();
}

void ImageViewer::restoreImageViewOptions()
{
    if(APP_SETTINGS->fitToScreen())
        fitToWindowAct->setChecked(true);
    else
    if(APP_SETTINGS->fitToImage()) {
        fitToImageAct->setChecked(true);//activate(QAction::Trigger);//
    }
}


static const QString STR_IMAGE_IS_ABSENT(QString::fromUtf8("Картинки нема !!!"));

void ImageViewer::setEmptyLabel(const QString &label) {
    imageLabel->setPixmap(QPixmap());
    imageLabel->setText(label);
    imageLabel->adjustSize();
}

void ImageViewer::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open File"), APP_FILE_ITERATOR->currentPath());
    if(!fileName.isNull()) {
        APP_FILE_ITERATOR->setStartPoint(fileName);
        open(APP_FILE_ITERATOR->current(),true);
    }
}

void ImageViewer::storeScrollProportion()
{
    if(scrollArea->horizontalScrollBar()->isVisible()) {
        scrollValH = scrollArea->horizontalScrollBar()->maximum()-scrollArea->horizontalScrollBar()->minimum();
        if(scrollValH>0)
            scrollValH = scrollArea->horizontalScrollBar()->value()/scrollValH;
    }
    if(scrollArea->verticalScrollBar()->isVisible()) {
        scrollValV = scrollArea->verticalScrollBar()->maximum()-scrollArea->verticalScrollBar()->minimum();
        if(scrollValV>0)
            scrollValV = scrollArea->verticalScrollBar()->value()/scrollValV;
    }
}

void ImageViewer::open(const QString &fileName, bool showErrorMessage)
{
    if (!fileName.isEmpty()) {

#ifndef __LOAD_IN_MAIN_THREAD
        if(_loaderTask.isRunning()) {
            _loaderTask.cancel();
            _loaderTask.waitForFinished();
        }

        storeScrollProportion();

        setWindowTitle(QFileInfo(fileName).fileName() + " - " + QApplication::applicationName());
        setEmptyLabel(QString::fromUtf8("Грузим картинку !!!"));

        _showErrorMessage = showErrorMessage;
        QFuture<void> future = QtConcurrent::run(this,&ImageViewer::loadImage,fileName);
        _loaderTask.setFuture(future);
#else
//        setWindowTitle(QFileInfo(fileName).fileName() + " - " + QApplication::applicationName());
        QPixmap _image(fileName);
//        QImage _image(fileName);
        if (_image.isNull()) {
            setInfoLabels();
            if(showErrorMessage)
                QMessageBox::information(this, tr("Image Viewer"),
                                     tr("Cannot load %1.").arg(APP_FILE_ITERATOR->current()));

            setEmptyLabel(STR_IMAGE_IS_ABSENT);
            return;
        }
        storeScrollProportion();
        imageLabel->setPixmap(_image);
        printAct->setEnabled(true);
        setZoom();

#endif
    }
    else {
        setWindowTitle(QApplication::applicationName());
        setEmptyLabel(STR_IMAGE_IS_ABSENT);
        printAct->setEnabled(false);
    }
}


#ifndef __LOAD_IN_MAIN_THREAD
void ImageViewer::loadImage(const QString &fileName)
{
    _image = QImage(fileName);

}

void ImageViewer::imageLoaded()
{
    if (_loaderTask.isCanceled() || _image.isNull()) {
        setInfoLabels();
        if(_showErrorMessage)
            QMessageBox::information(this, tr("Image Viewer"),
                                 (_loaderTask.isCanceled()?tr("Loading \"%1\" canceled."):tr("Cannot load %1.")).arg(APP_FILE_ITERATOR->current()));
        if(_loaderTask.isCanceled())
            setEmptyLabel(QString::fromUtf8("Загрузка отменена"));
        else
            setEmptyLabel(STR_IMAGE_IS_ABSENT);
        return;
    }
    imageLabel->setPixmap(QPixmap::fromImage(_image));
    _image = QImage();
    printAct->setEnabled(true);
    setZoom();
}
#endif


void ImageViewer::setZoom()
{
//    if(!isVisible())
//        return;
    //Вот странно, не думал, что imageLabel->pixmap() будет null возвращать.
    //Даже в начале пытался QPixmap() ему присвоить, бесполезно,
    //прийдётся оба условия проверять
    if(!imageLabel->pixmap() || imageLabel->pixmap()->isNull()) {
        setInfoLabels();
    }
    else
    {
        if(fitToWindowAct->isChecked()) {
            const QPixmap *pm = imageLabel->pixmap();
            QRect rect(getContentRect());

            if(pm->width()<=rect.width() && pm->height()<=rect.height()) {
                imageLabel->adjustSize();
            }
            else {
                double scaleFactort = min(double(rect.width())/pm->width(),double(rect.height())/pm->height());
                imageLabel->resize(scaleFactort * pm->size());
            }
        }
        else {

            checkZoomInAct(ZOOM_IN_FACTOR);
            checkZoomOutAct(ZOOM_OUT_FACTOR);

            if(!zoomInAct->isEnabled()) {
                checkZoomInAct(1.0);
                if(!zoomInAct->isEnabled()) {
                    scaleFactor = 1.0;
                    checkZoomOutAct(ZOOM_OUT_FACTOR);
                }
                checkZoomInAct(ZOOM_IN_FACTOR);
            }


            if(scaleFactor == 1.0) {
                imageLabel->adjustSize();
            }
            else {
                imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());
            }

            if(/*!isFullScreen()*/fitToImageAct->isChecked() && fitToImageAct->isEnabled()) {
//                update();
                QSize windowSize(size());
                QRect rect(getContentRect());
                windowSize.rwidth()+=imageLabel->width()-rect.width();
                windowSize.rheight()+=imageLabel->height()-rect.height();
                resize(windowSize);
//                update();
            }
            //Вот мы поставили новую картинку, но значения скролбаров до выхода из функции не изменятся,
            //поэтому надо либо обновлять, либо отслеживать когда он сформируется далее, что проблематично
            scrollArea->update();
            scrollArea->horizontalScrollBar()->setValue((scrollArea->horizontalScrollBar()->maximum() -scrollArea->horizontalScrollBar()->minimum())*scrollValH);
            scrollArea->verticalScrollBar()->setValue((scrollArea->verticalScrollBar()->maximum() -scrollArea->verticalScrollBar()->minimum())*scrollValV);
        }

        setInfoLabels(APP_FILE_ITERATOR->current());
    }
}

void ImageViewer::setInfoLabels(const QString &fileName) {
    QString label;
    if(!fileName.isNull()) {
        label = QFileInfo(fileName).fileName()+QString(" - Scale %1(%2x%3)").arg(scaleFactor,0,'g',2).arg(imageLabel->width()).arg(imageLabel->height());
        setWindowTitle(label + " - " + QApplication::applicationName());
    }
    imageLabel->setToolTip(label);
}

void ImageViewer::print()
{
    Q_ASSERT(imageLabel->pixmap());
#ifndef QT_NO_PRINTER
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = imageLabel->pixmap()->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(imageLabel->pixmap()->rect());
        painter.drawPixmap(0, 0, *imageLabel->pixmap());
    }
#endif
}

void ImageViewer::zoomIn()
{
    scaleImage(ZOOM_IN_FACTOR);
}

void ImageViewer::zoomOut()
{
    scaleImage(ZOOM_OUT_FACTOR);
}

void ImageViewer::normalSize()
{

    scaleFactor = 1.0;
    fitToWindowAct->setChecked(false);
    //imageLabel->adjustSize();
    setZoom();
}

//Одна заставляет выполнить вторую zoom, если они обе отмечены
void ImageViewer::fitToWindow()
{
    if(fitToWindowAct->isChecked())// && fitToImageAct->isChecked())
        fitToImageAct->setChecked(false);//activate(QAction::Trigger);//
    setZoom();
}

void ImageViewer::fitToImage()
{
    if(fitToImageAct->isChecked()) {
//        if(fitToWindowAct->isChecked())
          fitToWindowAct->setChecked(false);//activate(QAction::Trigger);//
        setZoom();
    }
}

void ImageViewer::about()
{
    QMessageBox::about(this, tr("About")+" "+QApplication::applicationName(),
            tr("<p align=\"center\">The <b>Ice Crystals Image Viewer</b> is a simple image viewer<br/>based on "
               "standard qt example.</p>"));
}

void ImageViewer::setZoomLabels() {
    int scalePercents = int(APP_SETTINGS->scaleDelta()*100);
    zoomInAct->setText(tr("Zoom &In (%1%)").arg(scalePercents));
    zoomOutAct->setText(tr("Zoom &Out (%1%)").arg(scalePercents));

    scalePercents = int(APP_SETTINGS->grabScale()*100);
    grabScaleAct->setText(tr("Scale(%1%)").arg(scalePercents));
}

void ImageViewer::createActions()
{
    QShortcut *shortcut;

    openAct = new QAction(tr("&Open..."), this);
    openAct->setIcon(QIcon::fromTheme(QLatin1String("document-open-data")));//QIcon(":/images/16x16/document-open-data.png"));
    openAct->setShortcut(QKeySequence(QKeySequence::Open));//tr("Ctrl+O"));
//    openAct->setShortcutContext(Qt::ApplicationShortcut);

    shortcut = new QShortcut(openAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this, SLOT(open()));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    printAct = new QAction(tr("&Print..."), this);
    printAct->setIcon(QIcon::fromTheme(QLatin1String("document-print")));//QIcon(":/images/16x16/document-print.png"));
    printAct->setShortcut(QKeySequence::Print);//tr("Ctrl+P"));
//    printAct->setShortcutContext(Qt::ApplicationShortcut);
//    printAct->setEnabled(false);
    shortcut = new QShortcut(printAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this, SLOT(print()));
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setIcon(QIcon::fromTheme(QLatin1String("application-exit")));//QIcon(":/images/16x16/application-exit.png"));
    exitAct->setShortcut(QKeySequence::Quit);//tr("Ctrl+Q"));
//    exitAct->setShortcutContext(Qt::ApplicationShortcut);
    shortcut = new QShortcut(exitAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this, SLOT(close()));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));


    fullScreenAct = new QAction(tr("F&ull Screen Mode"),this);
    fullScreenAct->setIcon(QIcon::fromTheme("view-fullscreen"));//QIcon(":/images/16x16/view-fullscreen.png"));
    fullScreenAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Return));//,Qt::CTRL+Qt::Key_Enter));//tr("Ctrl+Return"));
    fullScreenAct->setCheckable(true);
//    fullScreenAct->setShortcutContext(Qt::ApplicationShortcut);
    //Хотел ещё Esc добавить, но хрен та там. Пришлось ещё один shortcut добавлять
    shortcut = new QShortcut(fullScreenAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this,SLOT(changeModeKostil()));//fullScreenAct, SIGNAL(triggered()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_Escape),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this,SLOT(changeModeKostil()));
    connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(changeMode()));

    zoomInAct = new QAction(this);
    zoomInAct->setIcon(QIcon::fromTheme("zoom-in"));//QIcon(":/images/16x16/zoom-in.png"));
    zoomInAct->setShortcut(QKeySequence::ZoomIn);//tr("Ctrl++"));
//    zoomInAct->setShortcutContext(Qt::ApplicationShortcut);
//    zoomInAct->setEnabled(false);
    shortcut = new QShortcut(zoomInAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this, SLOT(zoomIn()));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(this);
    zoomOutAct->setIcon(QIcon::fromTheme("zoom-out"));//QIcon(":/images/16x16/zoom-out.png"));
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);//tr("Ctrl+-"));
//    zoomOutAct->setShortcutContext(Qt::ApplicationShortcut);
//    zoomOutAct->setEnabled(false);
    shortcut = new QShortcut(zoomOutAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this, SLOT(zoomOut()));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setIcon(QIcon::fromTheme("zoom-original"));//QIcon(":/images/16x16/zoom-original.png"));
    normalSizeAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_S/*,Qt::CTRL+Qt::Key_1*/));//tr("Ctrl+S"));
//    normalSizeAct->setShortcutContext(Qt::ApplicationShortcut);
    shortcut = new QShortcut(normalSizeAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this, SLOT(normalSize()));
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

    fitToWindowAct = new QAction(tr("&Fit to Window"), this);
    fitToWindowAct->setIcon(QIcon::fromTheme("zoom-fit-best"));//QIcon(":/images/16x16/zoom-fit-best.png"));
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_F/*,Qt::CTRL+Qt::Key_2*/));//tr("Ctrl+F"));
//    fitToWindowAct->setShortcutContext(Qt::ApplicationShortcut);
    shortcut = new QShortcut(fitToWindowAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this,SLOT(fitToWindowKostil()));
    connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(fitToWindow()));

    fitToImageAct = new QAction(tr("&Fit to Image"), this);
    fitToImageAct->setIcon(QIcon::fromTheme("image-x-generic"));//QIcon(":/images/16x16/zoom-fit-best.png"));
    fitToImageAct->setCheckable(true);
    fitToImageAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_I));

    shortcut = new QShortcut(fitToImageAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this,SLOT(fitToImageKostil()));
    connect(fitToImageAct, SIGNAL(triggered()), this, SLOT(fitToImage()));

//    fitGroup = new QActionGroup(this);
//    fitGroup->addAction(fitToWindowAct);
//    fitGroup->addAction(fitToImageAct);

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    // Sort actions
    sortUnsortAct = new QAction(tr("NoSort"),this);
    sortUnsortAct->setCheckable(true);
    connect(sortUnsortAct, SIGNAL(triggered()), this, SLOT(sortUnsort()));

    sortNameAct = new QAction(tr("Name"),this);
    sortNameAct->setCheckable(true);
    connect(sortNameAct, SIGNAL(triggered()), this, SLOT(sortByName()));

    sortTimeAct = new QAction(tr("Time"),this);
    sortTimeAct->setCheckable(true);
    connect(sortTimeAct, SIGNAL(triggered()), this, SLOT(sortByTime()));

    sortSizeAct = new QAction(tr("Size"),this);
    sortSizeAct->setCheckable(true);
    connect(sortSizeAct, SIGNAL(triggered()), this, SLOT(sortBySize()));

    sortTypeAct = new QAction(tr("Type"),this);
    sortTypeAct->setCheckable(true);
    connect(sortTypeAct, SIGNAL(triggered()), this, SLOT(sortByType()));

    sortGroup = new QActionGroup(this);
    sortGroup->addAction(sortNameAct);
    sortGroup->addAction(sortTimeAct);
    sortGroup->addAction(sortTypeAct);
    sortGroup->addAction(sortSizeAct);
    sortGroup->addAction(sortUnsortAct);


    sortReversedAct = new QAction(tr("Reverse"),this);
    sortReversedAct->setCheckable(true);
    connect(sortReversedAct, SIGNAL(triggered()), this, SLOT(sortReverse()));

    sortCaseSensetiveAct = new QAction(tr("Case Sensetive"),this);
    sortCaseSensetiveAct->setCheckable(true);
    connect(sortCaseSensetiveAct, SIGNAL(triggered()), this, SLOT(sortCaseSensetive()));



    grabDefaultAct = new QAction(tr("Default"),this);
    grabDefaultAct->setCheckable(true);
    connect(grabDefaultAct, SIGNAL(triggered()), this, SLOT(setGrabStateDefault()));
    grabFitToWindowAct = new QAction(tr("Fit To Window"),this);
    grabFitToWindowAct->setCheckable(true);
    connect(grabFitToWindowAct, SIGNAL(triggered()), this, SLOT(setGrabStateFitToWindow()));
    grabNormalSizeAct = new QAction(tr("Normal Size"),this);
    grabNormalSizeAct->setCheckable(true);
    connect(grabNormalSizeAct, SIGNAL(triggered()), this, SLOT(setGrabStateNormalSize()));
    grabScaleAct = new QAction(this);
    grabScaleAct->setCheckable(true);
    connect(grabScaleAct, SIGNAL(triggered()), this, SLOT(setGrabStateScale()));

    grabGroup = new QActionGroup(this);
    grabGroup->addAction(grabDefaultAct);
    grabGroup->addAction(grabNormalSizeAct);
    grabGroup->addAction(grabFitToWindowAct);
    grabGroup->addAction(grabScaleAct);

    customizeViewAct = new QAction(tr("Customize"),this);
    customizeViewAct->setIcon(QIcon::fromTheme("configure"));
    connect(customizeViewAct, SIGNAL(triggered()), this, SLOT(customizeView()));


    goNextAct = new QAction(tr("&Next"), this);
    goNextAct->setIcon(QIcon::fromTheme("go-next"));//QIcon(":/images/16x16/go-next.png"));
    goNextAct->setShortcut(tr("Space"));
//    goNextAct->setShortcutContext(Qt::ApplicationShortcut);
    shortcut = new QShortcut(goNextAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this, SLOT(goNext()));
    connect(goNextAct, SIGNAL(triggered()), this, SLOT(goNext()));

    goPreviousAct = new QAction(tr("&Previous"), this);
    goPreviousAct->setIcon(QIcon::fromTheme("go-previous"));//QIcon(":/images/16x16/go-previous.png"));
    goPreviousAct->setShortcut(tr("Backspace"));
//    goPreviousAct->setShortcutContext(Qt::ApplicationShortcut);
    shortcut = new QShortcut(goPreviousAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this, SLOT(goPrevious()));
    connect(goPreviousAct, SIGNAL(triggered()), this, SLOT(goPrevious()));

    goFirstAct = new QAction(tr("&First"), this);
    goFirstAct->setIcon(QIcon::fromTheme("go-first"));
    goFirstAct->setShortcut(tr("Home"));
//    goFirstAct->setShortcutContext(Qt::ApplicationShortcut);
    shortcut = new QShortcut(goFirstAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this, SLOT(goFirst()));
    connect(goFirstAct, SIGNAL(triggered()), this, SLOT(goFirst()));

    goLastAct = new QAction(tr("&Last"), this);
    goLastAct->setIcon(QIcon::fromTheme("go-last"));
    goLastAct->setShortcut(tr("End"));
//    goLastAct->setShortcutContext(Qt::ApplicationShortcut);
    shortcut = new QShortcut(goLastAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this, SLOT(goLast()));
    connect(goLastAct, SIGNAL(triggered()), this, SLOT(goLast()));

    goUpAct = new QAction(tr("&Up"), this);
    goUpAct->setIcon(QIcon::fromTheme("go-up"));//QIcon(":/images/16x16/go-up.png"));
    goUpAct->setShortcut(tr("Alt+Up"));
//    goUpAct->setShortcutContext(Qt::ApplicationShortcut);
    shortcut = new QShortcut(goUpAct->shortcut(),this);
    shortCuts.append(shortcut);

    connect(shortcut, SIGNAL(activated()), this, SLOT(goUp()));
    connect(goUpAct, SIGNAL(triggered()), this, SLOT(goUp()));

    goStartAct = new QAction(tr("&Start Page"), this);
    goStartAct->setIcon(QIcon::fromTheme("go-home"));
//    goStartAct->setShortcutContext(Qt::ApplicationShortcut);
    connect(goStartAct, SIGNAL(triggered()), this, SLOT(goStart()));

    setZoomLabels();
}

void ImageViewer::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    sortMenu = new QMenu(tr("Sort"),this);
    sortMenu->addAction(sortNameAct);
    sortMenu->addAction(sortSizeAct);
    sortMenu->addAction(sortTimeAct);
    sortMenu->addAction(sortTypeAct);
    sortMenu->addAction(sortUnsortAct);
    sortMenu->addSeparator();
    sortMenu->addAction(sortReversedAct);
    sortMenu->addAction(sortCaseSensetiveAct);

    grabMenu = new QMenu(tr("Grab Mode"),this);
    grabMenu->addAction(grabDefaultAct);
    grabMenu->addAction(grabNormalSizeAct);
    grabMenu->addAction(grabFitToWindowAct);
    grabMenu->addAction(grabScaleAct);


    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(customizeViewAct);
    viewMenu->addAction(fullScreenAct);
    viewMenu->addSeparator();
    viewMenu->addMenu(grabMenu);
    viewMenu->addMenu(sortMenu);
    viewMenu->addSeparator();
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(normalSizeAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fitToWindowAct);
    viewMenu->addAction(fitToImageAct);
    \
    goMenu = new QMenu(tr("&Go"),this);
    goMenu->addAction(goFirstAct);
    goMenu->addAction(goNextAct);
    goMenu->addAction(goPreviousAct);
    goMenu->addAction(goLastAct);
    goMenu->addSeparator();
    goMenu->addAction(goUpAct);
    goMenu->addSeparator();
    goMenu->addAction(goStartAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(goMenu);
    menuBar()->addMenu(helpMenu);
}


void ImageViewer::scaleImage(double factor)
{
    if(imageLabel->pixmap() && !imageLabel->pixmap()->isNull() && scaleFactorLegal(factor)) {
//        Q_ASSERT(pm);

        scaleFactor *= factor;

        imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());
        adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
        adjustScrollBar(scrollArea->verticalScrollBar(), factor);

        if(factor>1.0)
            checkZoomInAct(factor);
        else
            checkZoomOutAct(factor);

        setInfoLabels(APP_FILE_ITERATOR->current());
        fitToWindowAct->setChecked(false);
        setZoom();
    }
}

inline bool ImageViewer::scaleFactorLegal(double factor) const
{
    if(factor>1.0)
        return scaleFactorLegalHighBorder(scaleFactor*factor);
    return scaleFactorLegalLowBorder(scaleFactor*factor);
}

bool ImageViewer::scaleFactorLegalLowBorder(double factor) const
{
    const QPixmap *pm = imageLabel->pixmap();
    return pm->width()*factor > 3 && pm->height()*factor > 3;
}

bool ImageViewer::scaleFactorLegalHighBorder(double factor) const
{
    const QPixmap *pm = imageLabel->pixmap();
    //8388608: 8 - bits in byte, memoryLimit() in megabytes
    return factor<=APP_SETTINGS->scaleLimit() && factor*(pm->width() * pm->height()*pm->depth())/8388608 <= APP_SETTINGS->memoryLimit();
}

inline void ImageViewer::checkZoomInAct(double factor)
{
    zoomInAct->setEnabled(scaleFactorLegalHighBorder(factor*scaleFactor));
}

inline void ImageViewer::checkZoomOutAct(double factor)
{
    zoomOutAct->setEnabled(scaleFactorLegalLowBorder(factor*scaleFactor));
}

//! [24]

//! [25]
void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
//! [25] //! [26]
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}
//! [26]

void ImageViewer::moveScroll(QScrollBar *bar,int delta)
{
    if(bar->isVisible() && delta!=0) {

        int range = bar->maximum() - bar->minimum();
        int size;
        if(bar->orientation() == Qt::Horizontal)
            size = scrollArea->widget()->width();
        else
            size = scrollArea->widget()->height();

        size = APP_SETTINGS->mouseGrabSpeed()*range*delta/(size/*bar->singleStep()*/)+0.5;
        if(size==0) {
            size = delta<0?-1:1;
        }
        bar->setValue(bar->value()+size/*bar->singleStep()*/);
    }
}

void ImageViewer::move(const QPoint &delta)
{
    if(grabState.move) {
        (this->*grabState.move)(delta);
    }
}

bool ImageViewer::isMooving() const
{
    if(grabState.isMoving)
        return (this->*grabState.isMoving)();
    return false;
}

void ImageViewer::startMooving()
{
    if(!isMooving() && grabState.startMoving) {
        (this->*grabState.startMoving)();
    }
}

void ImageViewer::endMooving()
{
    if(isMooving() && grabState.endMoving) {
        (this->*grabState.endMoving)();
    }
}

//void ImageViewer::keyPressEvent(QKeyEvent * event)
//{
//    if(event->modifiers()==Qt::NoModifier) {
//        switch(event->key()){
//            case Qt::Key_Space:
//              if(isFullScreen()) {
////            case Qt::Key_Right:
////                if(APP_FILE_ITERATOR->haveNext())
//                    goNextAct->activate(QAction::Trigger);
////                    open(APP_FILE_ITERATOR->next());
////                event->accept();
//              }
//            break;
//            case Qt::Key_Backspace:
////            case Qt::Key_Left:
////                if(APP_FILE_ITERATOR->havePrevious())
//                    goPreviousAct->activate(QAction::Trigger);
////                    open(APP_FILE_ITERATOR->previous());
////                event->accept();
//            break;
//#ifndef __LOAD_IN_MAIN_THREAD
//            case Qt::Key_Escape:
//                _loaderTask.cancel();
////                event->accept();
//            break;
//#endif
//        }
////        if(event->isAccepted())
////            return;
//    }
//    else {
//        if((event->modifiers() == Qt::ControlModifier)) { // == Qt::ControlModifier) {
//            if( (isFullScreen() && event->key() == Qt::Key_Return)) {
////                case Qt::Key_F:
////                changeMode();
////                event->accept();
//                fullScreenAct->activate(QAction::Trigger);
//           }
//           else {
//                if(isFullScreen()) {
//                    switch(event->key()) {
//                        case Qt::Key_Q:
//                            exitAct->activate(QAction::Trigger);
////                            event->accept();
//                            break;
//                        case Qt::Key_S:
//                            normalSizeAct->activate(QAction::Trigger);
////                            event->accept();
//                            break;
//                        case Qt::Key_O:
//                            openAct->activate(QAction::Trigger);
////                            event->accept();
//                            break;
//                        case Qt::Key_F:
//                            fitToWindowAct->activate(QAction::Trigger);
////                            event->accept();
//                            break;
//                        case Qt::Key_Plus:
//                            zoomInAct->activate(QAction::Trigger);
////                            event->accept();
//                            break;
//                        case Qt::Key_Minus:
//                            zoomOutAct->activate(QAction::Trigger);
////                            event->accept();
//                            break;
//                    }
//                }
//           }
//        }
//    }
//    event->accept();
////    QMainWindow::keyPressEvent(event);
//}

void ImageViewer::changeMode()
{
    if(isFullScreen()) {
        setWindowMode();
    }
    else {
        setFullScreenMode();
    }
}

void ImageViewer::setFullScreenMode()
{
    this->setWindowState(this->windowState() | Qt::WindowFullScreen);
    this->menuBar()->setVisible(false);
    activateShortcuts(true);
    scrollArea->setBackgroundRole(QPalette::Shadow);
    _frameStyle = scrollArea->frameStyle();
    scrollArea->setFrameStyle(QFrame::NoFrame);
}

void ImageViewer::setWindowMode()
{
    this->setWindowState(this->windowState() ^ Qt::WindowFullScreen);
    activateShortcuts(false);
    this->menuBar()->setVisible(true);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setFrameStyle(_frameStyle);
}


void ImageViewer::changeEvent(QEvent *ev)
{
    if(ev->type()==QEvent::WindowStateChange){
        QWindowStateChangeEvent *event = static_cast<QWindowStateChangeEvent*>(ev);

        if((windowState() & NOT_NORMAL_STATE)==0) {
            //Ну, пущай размер окна изменяется только в обычном оконном режиме не MAXIMIZED и не FULL_SCREEN
            fitToImageAct->setEnabled(true);
            if(fitToImageAct->isChecked())
                setZoom();
        }
        else {
            fitToImageAct->setEnabled(false);
            if((event->oldState() & NOT_NORMAL_STATE)==0 && (windowState() & NOT_NORMAL_STATE)!=0) {
                QSize s(size());
                _storedWidth = s.width();
                _storedHeight = s.height();
            }
        }

    }
    QMainWindow::changeEvent(ev);
}

void ImageViewer::resizeEvent(QResizeEvent* event)
{
    if(fitToWindowAct->isChecked())
        setZoom();
    QMainWindow::resizeEvent(event);
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent *ev)
{
    changeMode();
    ev->accept();
}

void ImageViewer::showEvent(QShowEvent *ev)
{
    (this->*showEventFunc)(ev);
}

void ImageViewer::firstShowEvent(QShowEvent *ev)
{
    showEventFunc = &ImageViewer::otherShowEvent;
    QMainWindow::showEvent(ev);
    setZoom();
}

void ImageViewer::otherShowEvent(QShowEvent *ev)
{
    QMainWindow::showEvent(ev);
}


void ImageViewer::loadSortSettings()
{
    QDir::SortFlags flags = APP_SETTINGS->sorting();

    //Порядок важен
    if((flags&QDir::Unsorted)==QDir::Unsorted)
        sortUnsortAct->setChecked(true);
    else
    if((flags&QDir::Name)==QDir::Name)
        sortNameAct->setChecked(true);
    else
    if((flags&QDir::Size)==QDir::Size)
        sortSizeAct->setChecked(true);
    else
    if((flags&QDir::Type)==QDir::Type)
        sortTypeAct->setChecked(true);
    else
    if((flags&QDir::Time)==QDir::Time)
        sortTimeAct->setChecked(true);    

    if((flags&QDir::Reversed)==QDir::Reversed)
        sortReversedAct->setChecked(true);

    if((flags&QDir::IgnoreCase)!=QDir::IgnoreCase)
        sortCaseSensetiveAct->setChecked(true);
}

void ImageViewer::storeSortSettings()
{
    APP_SETTINGS->setSorting(APP_FILE_ITERATOR->sorting());
}

void ImageViewer::storeGrabMode()
{
    MSettings::GrabMode mode;
    if(grabNormalSizeAct->isChecked())
        mode = MSettings::GrabNormalSize;
    else
    if(grabScaleAct->isChecked())
        mode = MSettings::GrabScale;
    else
    if(grabFitToWindowAct->isChecked())
        mode = MSettings::GrabFitToWindow;
    else
        mode = MSettings::GrabDefault;
    APP_SETTINGS->setGrabMode(mode);
}


void ImageViewer::sortUnsort() {
    APP_FILE_ITERATOR->setSorting((APP_FILE_ITERATOR->sorting() & (~(QDir::Type|QDir::Size|QDir::Name|QDir::Time|QDir::Unsorted))) | QDir::Unsorted);
}

void ImageViewer::sortByName()
{
    APP_FILE_ITERATOR->setSorting((APP_FILE_ITERATOR->sorting() & (~(QDir::Type|QDir::Size|QDir::Name|QDir::Time|QDir::Unsorted))) | QDir::Name);
}

void ImageViewer::sortByTime()
{
    APP_FILE_ITERATOR->setSorting((APP_FILE_ITERATOR->sorting() & (~(QDir::Type|QDir::Size|QDir::Name|QDir::Time|QDir::Unsorted))) | QDir::Time);
}

void ImageViewer::sortBySize()
{
    APP_FILE_ITERATOR->setSorting((APP_FILE_ITERATOR->sorting() & (~(QDir::Type|QDir::Size|QDir::Name|QDir::Time|QDir::Unsorted))) | QDir::Size);
}

void ImageViewer::sortByType()
{
    APP_FILE_ITERATOR->setSorting((APP_FILE_ITERATOR->sorting() & (~(QDir::Type|QDir::Size|QDir::Name|QDir::Time|QDir::Unsorted))) | QDir::Type);
}

void ImageViewer::sortReverse()
{
    APP_FILE_ITERATOR->setSorting(APP_FILE_ITERATOR->sorting()^QDir::Reversed);
}

void ImageViewer::sortCaseSensetive()
{
    APP_FILE_ITERATOR->setSorting(APP_FILE_ITERATOR->sorting()^QDir::IgnoreCase);
}

void ImageViewer::setGrabState()
{
    switch(APP_SETTINGS->grabMode()) {
        case MSettings::GrabNormalSize:
            grabNormalSizeAct->activate(QAction::Trigger);
        break;
        case MSettings::GrabFitToWindow:
            grabFitToWindowAct->activate(QAction::Trigger);
        break;
        case MSettings::GrabScale:
            grabScaleAct->activate(QAction::Trigger);
        break;
        default:
            grabDefaultAct->activate(QAction::Trigger);
    }

//    grabDefaultAct->setChecked(true);

}

void ImageViewer::setGrabStateDefault()
{
    endMooving();
    grabState.move = &ImageViewer::moveDefault;
    grabState.isMoving = &ImageViewer::isMoovingDefault;
    grabState.startMoving = &ImageViewer::startMoovingDefault;
    grabState.endMoving = &ImageViewer::endMoovingDefault;
}

void ImageViewer::setGrabStateNormalSize()
{
    endMooving();
    grabState.move = &ImageViewer::moveDefault;
    grabState.isMoving = &ImageViewer::isMoovingDefault;
    grabState.startMoving = &ImageViewer::startMoovingNormalSize;
    grabState.endMoving = &ImageViewer::endMoovingScale;
}

void ImageViewer::setGrabStateFitToWindow()
{
    endMooving();
    grabState.move = 0;
    grabState.isMoving = &ImageViewer::isMoovingDefault;
    grabState.startMoving = &ImageViewer::startMoovingFitToWindow;
    grabState.endMoving = &ImageViewer::endMoovingFitToWindow;
}

void ImageViewer::setGrabStateScale()
{
    endMooving();
    grabState.move = &ImageViewer::moveDefault;
    grabState.isMoving = &ImageViewer::isMoovingDefault;
    grabState.startMoving = &ImageViewer::startMoovingScale;
    grabState.endMoving = &ImageViewer::endMoovingScale;
}

void ImageViewer::moveDefault(const QPoint &delta)
{
    moveScroll(scrollArea->horizontalScrollBar(),delta.x());
    moveScroll(scrollArea->verticalScrollBar(),delta.y());
}

bool ImageViewer::isMoovingDefault() const
{
    return _lastCursor.handle()!=cursor().handle();
}

void ImageViewer::startMoovingDefault()
{
    _lastCursor = cursor();
    setCursor(QCursor(Qt::ClosedHandCursor));
}

void ImageViewer::endMoovingDefault()
{
    setCursor(_lastCursor);
}

void ImageViewer::startMoovingFitToWindow()
{
    storeScrollProportion();
    startMoovingDefault();
//    fitToWindowAct->setChecked(true);
    fitToWindowAct->activate(QAction::Trigger);
}

void ImageViewer::endMoovingFitToWindow()
{
    endMoovingDefault();
//    fitToWindowAct->setChecked(false);
    fitToWindowAct->activate(QAction::Trigger);
}

void ImageViewer::startMoovingNormalSize()
{
    _oldScaleFactor = scaleFactor;
    scaleFactor = 1.0;
    startMoovingDefault();
    setZoom();
}

void ImageViewer::startMoovingScale()
{
    startMoovingDefault();
    _oldScaleFactor = scaleFactor;

    if(scaleFactorLegalLowBorder(APP_SETTINGS->grabScale()) && scaleFactorLegalHighBorder(APP_SETTINGS->grabScale())) {
        scaleFactor = APP_SETTINGS->grabScale();
        setZoom();
    }

}

void ImageViewer::endMoovingScale()
{
    storeScrollProportion();
    scaleFactor = _oldScaleFactor;
    endMoovingDefault();
    setZoom();
}

void ImageViewer::customizeView()
{
    CustomizeViewDialog dlg;
    dlg.init(*APP_SETTINGS);
    if(dlg.exec()==QDialog::Accepted) {
        acceptCustomizeDlg(dlg);
    }
}

void ImageViewer::acceptCustomizeDlg(const CustomizeViewDialog &dlg)
{
    APP_SETTINGS->setScaleLimit(dlg.maxScale());
    APP_SETTINGS->setScaleDelta(dlg.scaleStep());
    APP_SETTINGS->setGrabScale(dlg.grabScale());

    setZoomLabels();
}

void ImageViewer::goFirst()
{
    open(APP_FILE_ITERATOR->goFirst());
}

void ImageViewer::goLast()
{
    open(APP_FILE_ITERATOR->goLast());
}

void ImageViewer::goNext()
{
    if(APP_FILE_ITERATOR->haveNext())
        open(APP_FILE_ITERATOR->next());
}

void ImageViewer::goPrevious()
{
    if(APP_FILE_ITERATOR->havePrevious())
        open(APP_FILE_ITERATOR->previous());
}

void ImageViewer::goUp()
{
    open(APP_FILE_ITERATOR->goUp());
}

void ImageViewer::goStart()
{
    APP_FILE_ITERATOR->setStartPoint(QDir::currentPath());
    open(APP_FILE_ITERATOR->current());
}

void ImageViewer::showContextMenu(const QPoint& point)
{
    QPoint globalPos = mapToGlobal(point);

    QMenu myMenu;
    myMenu.addAction(fullScreenAct);
    myMenu.addSeparator();
    myMenu.addMenu(goMenu);
    myMenu.addSeparator();
    myMenu.addMenu(sortMenu);
    myMenu.addMenu(grabMenu);
    myMenu.addSeparator();
    myMenu.addAction(zoomInAct);
    myMenu.addAction(zoomOutAct);
    myMenu.addAction(normalSizeAct);
    myMenu.addSeparator();
    myMenu.addAction(fitToWindowAct);
    if(fitToImageAct->isEnabled())
        myMenu.addAction(fitToImageAct);

    myMenu.exec(globalPos);
}

// Костыльки
void ImageViewer::activateShortcuts(bool enabled)
{
    foreach(QShortcut* shortcut,shortCuts)
        shortcut->setEnabled(enabled);
}

void ImageViewer::fitToWindowKostil()
{
    fitToWindowAct->activate(QAction::Trigger);
}

void ImageViewer::fitToImageKostil()
{
    fitToImageAct->activate(QAction::Trigger);
}

void ImageViewer::changeModeKostil()
{
    fullScreenAct->activate(QAction::Trigger);
}
//---------
QRect ImageViewer::getContentRect() const
{
    QRect rect(scrollArea->contentsRect());
    if(!isFullScreen()) {
        if(scrollArea->horizontalScrollBar()->isVisible())
            rect.setHeight(rect.height()+(scrollArea->horizontalScrollBar()->height()*3/4));
        if(scrollArea->verticalScrollBar()->isVisible())
            rect.setWidth(rect.width()+(scrollArea->verticalScrollBar()->width()*3/4));
    }
    return rect;
}
