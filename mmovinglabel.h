//============================================================================
// Name        : mmovinglabel.h
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
