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


#ifndef HEADERCOMBOBOX_H
#define HEADERCOMBOBOX_H

#include <QComboBox>
#include <QStringList>

class HeaderComboBox : public QComboBox
{
    Q_OBJECT
    
public:
    explicit HeaderComboBox(QWidget* parent = 0);
    void SetCurrentElement(QString name);
    void SetParameterNames(const QStringList& names);
    
public slots:
    void AddParameter(QString name);
    void RemoveParameter(QString name);
    
protected:
    void keyPressEvent(QKeyEvent* ev);
    
private:
    QStringList parameter_names_;
};

#endif // HEADERCOMBOBOX_H