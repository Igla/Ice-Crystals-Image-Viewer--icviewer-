//============================================================================
// Name        : mmovinglabel.cpp
// Author      : Igla
// Version     :
// Copyright   : 2011 Ice Crystals, Lopasoft corporation and this fucking world
// Description : Label, предполагающий, что с ним будут что-то делать через
//             : MMover интерфейс. Обрабатываем нажатия мыши и говорим, чтобы
//             : нас подвигали
//============================================================================

#ifndef MMOOVINGLABEL_H
#define MMOOVINGLABEL_H

#include <QLabel>
#include "mmover.h"

class QMouseEvent;
class QPoint;

class MMovingLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MMovingLabel(MMover *mover, QWidget *parent = 0);

    inline MMover* mover() const {return _mover;}

protected:
    virtual void mouseMoveEvent(QMouseEvent * ev);
    virtual void mousePressEvent(QMouseEvent * ev);
    virtual void mouseReleaseEvent(QMouseEvent * ev);
//    virtual void mouseDoubleClickEvent(QMouseEvent *ev);
signals:

public slots:
protected:
    inline bool isActive() const {return mover()->isMooving();}//_active;}
private:
//    bool _active;
    QPoint _lastPos;
    MMover* _mover;
};

#endif // MMOOVINGLABEL_H

#include "mmovinglabel.h"
#include <QMouseEvent>
#include <QPoint>

MMovingLabel::MMovingLabel(MMover *mover, QWidget *parent) :
    QLabel(parent), _mover(mover)
{
}


void MMovingLabel::mouseMoveEvent(QMouseEvent * ev)
{
    if(isActive()) {
        QPoint delta = _lastPos - ev->globalPos();
        _lastPos = ev->globalPos();
        mover()->move(delta);
    }
    QLabel::mouseMoveEvent(ev);
}

void MMovingLabel::mousePressEvent(QMouseEvent * ev)
{
    if(ev->button()==Qt::LeftButton) {
        _lastPos = ev->globalPos();
        mover()->startMooving();
    }
    QLabel::mousePressEvent(ev);
}

void MMovingLabel::mouseReleaseEvent(QMouseEvent * ev)
{
    if(ev->button()==Qt::LeftButton)
        mover()->endMooving();
    QLabel::mouseReleaseEvent(ev);
}
