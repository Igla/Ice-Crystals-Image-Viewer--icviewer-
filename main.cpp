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

#include <QApplication>
#include <QImageReader>
#include <QStringBuilder>

#include "imageviewer.h"
#include "msettings.h"
#include "mfileiterator.h"

MSettings *APP_SETTINGS=0;
MFileIterator *APP_FILE_ITERATOR;

void initFileIterator(int argc, char *argv[]);
void setImageFilter(QStringList &filters);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Ice Crystals Dev Team");
    QCoreApplication::setOrganizationDomain("ice-crystals.home");
    QCoreApplication::setApplicationName("IC Viewer");
    MSettings settings;
    MFileIterator diterator;
    APP_SETTINGS = &settings;
    APP_FILE_ITERATOR = &diterator;
    initFileIterator(argc,argv);
    ImageViewer imageViewer;
#if defined(Q_OS_SYMBIAN)
    imageViewer.showMaximized();
#else
    imageViewer.show();
#endif
    return app.exec();
}


void initFileIterator(int argc, char *argv[])
{
    QStringList filters;
    setImageFilter(filters);
    APP_FILE_ITERATOR->init(filters,APP_SETTINGS->sorting());
//    APP_FILE_ITERATOR->setSorting(APP_SETTINGS->sorting());
//    APP_FILE_ITERATOR->setNameFilters(filters);

    if(argc>1) {
        APP_FILE_ITERATOR->setStartPoint(QString::fromUtf8(argv[argc-1]));
        QDir::setCurrent(APP_FILE_ITERATOR->currentPath());
    }
}

//Я знаю, что отнюдь не все форматы из этого списка будут работать.
//Например SVG. Ну, пока оставим как есть
void setImageFilter(QStringList &filters)
{
    QList<QByteArray> formats = QImageReader::supportedImageFormats();

    for(int i=0;i<formats.size();i++){
        filters << "*."+QString(formats[i].data()).toLower();
//        #ifdef __DEBUG
//            qDebug()<<filters.last();
//        #endif
    }

}
