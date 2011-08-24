//============================================================================
// Name        : mmover.h
// Author      : Igla
// Version     :
// Copyright   : 2011 Ice Crystals, Lopasoft corporation and this fucking world
// Description : Интерфейс ничего серьёзного из себя не представляет, просто
//             : реализовано так, хотя можно было и иначе. Набор callback функций.
//============================================================================

#ifndef MMOVER_H
#define MMOVER_H

#include <QPoint>

class MMover
{
public:
    virtual void move(const QPoint &delta) = 0;
    virtual bool isMooving() const = 0;
    virtual void startMooving() = 0;
    virtual void endMooving() = 0;
//    virtual void changeMode() = 0;
};

#endif // MMOVER_H

