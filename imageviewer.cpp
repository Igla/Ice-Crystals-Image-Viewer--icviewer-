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

#ifdef __DEBUG
#include <QDebug>
#endif


#include "QCursor"
#include "imageviewer.h"
#include "msettings.h"
#include "mfileiterator.h"

extern MSettings *APP_SETTINGS;
extern MFileIterator *APP_FILE_ITERATOR;

#define ZOOM_IN 1
#define ZOOM_OUT -1
#define ZOOM_DEFAULT 0

#define ZOOM_IN_FACTOR (1.0+APP_SETTINGS->scaleDelta())
#define ZOOM_OUT_FACTOR (1.0/(1.0f+APP_SETTINGS->scaleDelta()))

#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

int zoom_fl = ZOOM_DEFAULT;
float zoom_back;

ImageViewer::ImageViewer():scaleFactor(1.0),scrollValH(0.0f),scrollValV(0.0f)
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

#ifndef __LOAD_IN_MAIN_THREAD
    connect(&_loaderTask,SIGNAL(finished()),this,SLOT(imageLoaded()));
#endif


    open(APP_FILE_ITERATOR->current());

    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);

    _lastCursor = cursor();

    restoreGeometry();
}

ImageViewer::~ImageViewer()
{
    APP_SETTINGS->setFitToScreen(fitToWindowAct->isChecked());

    APP_SETTINGS->setWindowState(windowState());
    APP_SETTINGS->setWindowWidth(width());
    APP_SETTINGS->setWindowHeight(height());
    if(APP_SETTINGS->isChanged())
        APP_SETTINGS->storeUserSettings();
}

void ImageViewer::restoreGeometry() {
    resize(APP_SETTINGS->windowWidth(), APP_SETTINGS->windowHeight());
    setWindowState(APP_SETTINGS->windowState());
    if(isFullScreen())
        setFullScreenMode();
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

void ImageViewer::open(const QString &fileName, bool showErrorMessage)
{
    if (!fileName.isEmpty()) {

#ifndef __LOAD_IN_MAIN_THREAD
        if(_loaderTask.isRunning()) {
            _loaderTask.cancel();
            _loaderTask.waitForFinished();
        }

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

//#ifdef __DEBUG
//            qDebug() << "1: " <<scrollArea->horizontalScrollBar()->maximum() << " : " << scrollArea->verticalScrollBar()->maximum();
//#endif

        setWindowTitle(QFileInfo(fileName).fileName() + " - " + QApplication::applicationName());
        setEmptyLabel(QString::fromUtf8("Грузим картинку !!!"));

        _showErrorMessage = showErrorMessage;
        QFuture<void> future = QtConcurrent::run(this,&ImageViewer::loadImage,fileName);
        _loaderTask.setFuture(future);
#else
        QPixmap _image(fileName);
//        QImage _image(fileName);
        if (_image.isNull()) {
            if(showErrorMessage)
                QMessageBox::information(this, tr("Image Viewer"),
                                     tr("Cannot load %1.").arg(APP_FILE_ITERATOR->current()));

            setEmptyLabel(STR_IMAGE_IS_ABSENT);
            setInfoLabels();
            return;
        }
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
            QRect rect(scrollArea->contentsRect());
            if(scrollArea->horizontalScrollBar()->isVisible())
                rect.setHeight(rect.height()+(scrollArea->horizontalScrollBar()->height()>>1));
            if(scrollArea->verticalScrollBar()->isVisible())
                rect.setWidth(rect.width()+(scrollArea->verticalScrollBar()->width()>>1));

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
//    #ifdef __DEBUG
//                qDebug() << "2: " <<scrollArea->horizontalScrollBar()->maximum() << " : " << scrollArea->verticalScrollBar()->maximum();
//    #endif
            }
            //Вот мы поставили новую картинку, но значения скролбаров до выхода из функции не изменятся,
            //поэтому надо либо обновлять, либо отслеживать когда он сформируется далее, что проблематично
            scrollArea->update();
//#ifdef __DEBUG
//            qDebug() << "3: " <<scrollArea->horizontalScrollBar()->maximum() << " : " << scrollArea->verticalScrollBar()->maximum();
//#endif
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
    fitToWindowAct->setChecked(false);

    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void ImageViewer::fitToWindow()
{
    setZoom();
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
}

void ImageViewer::createActions()
{

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcut(tr("Ctrl+P"));
    printAct->setEnabled(false);
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    zoomInAct = new QAction(this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    setZoomLabels();

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

    fitToWindowAct = new QAction(tr("&Fit to Window"), this);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));
    connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(fitToWindow()));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void ImageViewer::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(normalSizeAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fitToWindowAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(helpMenu);
}


void ImageViewer::scaleImage(double factor)
{
    if(imageLabel->pixmap() && !imageLabel->pixmap()->isNull()) {
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
    }
}

void ImageViewer::checkZoomInAct(double factor)
{
    const QPixmap *pm = imageLabel->pixmap();
    //8388608: 8 - bits in byte, memoryLimit() in megabytes
    factor *= scaleFactor;
    zoomInAct->setEnabled(factor<=APP_SETTINGS->scaleLimit() && factor*(pm->width() * pm->height()*pm->depth())/8388608 <= APP_SETTINGS->memoryLimit());
}

void ImageViewer::checkZoomOutAct(double factor)
{
    const QPixmap *pm = imageLabel->pixmap();
    factor *= scaleFactor;
    zoomOutAct->setEnabled( pm->width()*factor > 3 && pm->height()*factor > 3);
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
    moveScroll(scrollArea->horizontalScrollBar(),delta.x());
    moveScroll(scrollArea->verticalScrollBar(),delta.y());
//    scrollArea->update();
}

bool ImageViewer::isMooving() const
{
    return _lastCursor.handle()!=cursor().handle();
}

void ImageViewer::startMooving()
{
    if(!isMooving()) {
        _lastCursor = cursor();
        setCursor(QCursor(Qt::ClosedHandCursor));
    }
}

void ImageViewer::endMooving()
{
    if(isMooving()) {
        setCursor(_lastCursor);
    }
}

void ImageViewer::keyPressEvent(QKeyEvent * event)
{
    if(event->modifiers()==Qt::NoModifier) {
        switch(event->key()){
            case Qt::Key_Space:
//            case Qt::Key_Right:
                if(APP_FILE_ITERATOR->haveNext())
                    open(APP_FILE_ITERATOR->next());
                event->accept();
            break;
            case Qt::Key_Backspace:
//            case Qt::Key_Left:
                if(APP_FILE_ITERATOR->havePrevious())
                    open(APP_FILE_ITERATOR->previous());
                event->accept();
            break;
#ifndef __LOAD_IN_MAIN_THREAD
            case Qt::Key_Escape:
                _loaderTask.cancel();
                event->accept();
            break;
#endif
        }
        if(event->isAccepted())
            return;
    }
    else {
        if((event->modifiers() == Qt::ControlModifier)) { // == Qt::ControlModifier) {
            if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
//                case Qt::Key_F:
                changeMode();
                event->accept();
           }
           else {
                if(isFullScreen()) {
                    switch(event->key()) {
                        case Qt::Key_Q:
                            exitAct->activate(QAction::Trigger);
                            event->accept();
                            break;
                        case Qt::Key_S:
                            normalSizeAct->activate(QAction::Trigger);
                            event->accept();
                            break;
                        case Qt::Key_O:
                            openAct->activate(QAction::Trigger);
                            event->accept();
                            break;
                        case Qt::Key_F:
                            fitToWindowAct->activate(QAction::Trigger);
                            event->accept();
                            break;
                        case Qt::Key_Plus:
                            zoomInAct->activate(QAction::Trigger);
                            event->accept();
                            break;
                        case Qt::Key_Minus:
                            zoomOutAct->activate(QAction::Trigger);
                            event->accept();
                            break;
                    }
                }
           }
        }
    }
    QMainWindow::keyPressEvent(event);
}

void ImageViewer::changeMode()
{
    if(isFullScreen()) {
        this->setWindowState(this->windowState() ^ Qt::WindowFullScreen);
        setWindowMode();
    }
    else {
        this->setWindowState(this->windowState() | Qt::WindowFullScreen);
        setFullScreenMode();
    }
}

void ImageViewer::setFullScreenMode()
{
    this->menuBar()->setVisible(false);
    scrollArea->setBackgroundRole(QPalette::Shadow);
    _frameStyle = scrollArea->frameStyle();
    scrollArea->setFrameStyle(QFrame::NoFrame);
}

void ImageViewer::setWindowMode()
{
    this->menuBar()->setVisible(true);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setFrameStyle(_frameStyle);
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
