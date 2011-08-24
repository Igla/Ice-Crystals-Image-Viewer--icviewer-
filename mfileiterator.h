//============================================================================
// Name        : mfileiterator.h
// Author      : Igla
// Version     :
// Copyright   : 2011 Ice Crystals, Lopasoft corporation and this fucking world
// Description : Something to iterate files in directory
//============================================================================

#ifndef MSTRINGIITERATOR_H
#define MSTRINGIITERATOR_H

#include <QString>
#include <QDir>

//#define M_ORDER_ASCENDING 1
//#define M_ORDER_DESCENDING 0
//enum MFileOrder {MFileOrderNone=2, MFileOrderName, MFileOrderSize, MFileOrderTime};

class MFileIterator
{
public:
    MFileIterator();

    // fileName - if file is a directory then start from the begining of entry list.
    // return false if file not found
    bool setStartPoint(const QString &fileName);

    inline QString currentPath() const { return _parent.path(); }
    inline const QString& current() const { return _current; }

    bool haveNext();
    const QString& next();

    bool havePrevious();
    const QString& previous();

    //    virtual int count() const = 0;
    //    virtual int currentNum() const = 0;
    //    virtual void setCurrent(int num) = 0;


    inline void setSorting(QDir::SortFlags order) {
        if( _parent.sorting() != order ) {
            _parent.setSorting(order);
            updateOrder();
        }
    }

    inline QDir::SortFlags sorting() const { return _parent.sorting(); }


    inline void setNameFilters(const QStringList & nameFilters) {
        _parent.setNameFilters(nameFilters);
        updateOrder();
    }

    inline QStringList nameFilters() const {
        return _parent.nameFilters();
    }

    // Используется, чтобы установить всё, что необходимо и не пересоздавать лист
    // каждый раз
    inline void init(const QStringList & nameFilters, QDir::SortFlags order) {
        _parent.setSorting(order);
        _parent.setNameFilters(nameFilters);
        updateOrder();
    }


private:
    QString getPath(const QString &fileName) const;
    void clear();

    inline void getList() {
        _list = _parent.entryList();//nameFilters(),_parent.filter(),sorting());
    }

    inline void updateOrder() {
        getList();
        _pos = _list.indexOf(QFileInfo(current()).fileName());
    }

private:
    QDir _parent;
    int _pos;
    QString _current;
    QStringList _list;
};

#endif // MSTRINGIITERATOR_H
