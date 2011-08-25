//============================================================================
// Name        : mfileiterator.cpp
// Author      : Igla
// Version     :
// Copyright   : 2011 Ice Crystals, Lopasoft corporation and this fucking world
// Description : Something to iterate files in directory
//============================================================================

#include <QFileInfo>
#include "mfileiterator.h"

#ifdef __DEBUG
#include <QDebug>
#endif

MFileIterator::MFileIterator():_pos(-2)
{
    // entryList only for data files
    setSorting(QDir::NoSort);
    _parent.setFilter(QDir::Files);
}

bool MFileIterator::setStartPoint(const QString &fileName)
{
    clear();
    QFileInfo finfo(fileName);
    if(!finfo.exists())
        return false;

    if(finfo.isDir()) {
        _parent.setPath(fileName);
        getList();
        _current = _list.isEmpty()?QString():getPath(_list.at(_pos = 0));

    }
    else {
        _parent.setPath(finfo.path());
        _current = fileName;
    }
    return true;
}

bool MFileIterator::haveNext()
{
    checkState();
    return _pos+1<_list.size();
}

const QString& MFileIterator::next()
{
    return _current = getPath(_list.at(++_pos));
}

bool MFileIterator::havePrevious()
{
    checkState();
    return _pos>=1;
}

const QString& MFileIterator::previous()
{
    return _current = getPath(_list.at(--_pos));
}

inline QString MFileIterator::getPath(const QString &fileName) const {
    return fileName.isNull()?QString():_parent.path() + QDir::separator() + fileName;
}

//inline void MFileIterator::setSorting(QDir::SortFlags order)
//{
//   if( _parent.sorting() != order ) {
//       _parent.setSorting(order);
//       updateOrder();
//   }
//}

//inline void MFileIterator::updateOrder()
//{
//    _list = _parent.entryList();
//    _pos = _list.indexOf(QFileInfo(current()).fileName());
//}

void MFileIterator::clear()
{
    _pos = -2;
    _list.clear();
    _current = QString();
}

const QString& MFileIterator::goFirst()
{
    checkState();
    if(_list.size()>0) {
        _pos = 0;
        return _current = getPath(_list.first());
    }
    return _current = QString();
}

const QString& MFileIterator::goLast()
{
    checkState();
    if(_list.size()>0) {
        _pos = _list.size()-1;
        return _current = getPath(_list.last());
    }
    return _current = QString();
}

const QString& MFileIterator::goUp()
{
    _parent.cdUp();
    setStartPoint(_parent.path());
    return current();
}

