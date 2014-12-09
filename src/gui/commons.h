// Copyright © 2014 Michael Jung
// 
// This file is part of Panga.
// 
// Panga is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Panga is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Panga.  If not, see <http://www.gnu.org/licenses/>.


#ifndef COMMONS_H
#define COMMONS_H

#include <QApplication>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QWidget>

const QString APPLICATION_NAME = "Panga";
const QString BINARY_FILE_EXTENSION = "panga";
const QString PORTABLE_FILE_EXTENSION = "panga_portable";

class ModelIndexLessThan
{
public:
    bool operator()(const QModelIndex& a, const QModelIndex& b) const
    {
        if (a.row() < b.row()) return true;
        if (a.row() > b.row()) return false;
        return a.column() < b.column();
    }
};

class StdStringToQString
{
public:
    QString operator()(const std::string& a) const
    {
        return QString::fromStdString(a);
    }
};

class SignalBlocker
{
public:
    SignalBlocker(QObject* object) :
        object_(object),
        prev_(object->blockSignals(true))
    {
    }
    
    ~SignalBlocker()
    {
        object_->blockSignals(prev_);
    }
    
private:
    QObject* object_;
    bool prev_;
};

class WindowWaitCursor
{
public:
    WindowWaitCursor(QWidget* widget) : widget_(widget)
    {
        widget_->setCursor(QCursor(Qt::WaitCursor));
    }
    
    ~WindowWaitCursor()
    {
        widget_->unsetCursor();
    }
private:
    QWidget* widget_;
};

class GlobalWaitCursor
{
public:
    GlobalWaitCursor()
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    }
    
    ~GlobalWaitCursor()
    {
        QApplication::restoreOverrideCursor();
    }
};

#endif // COMMONS_H