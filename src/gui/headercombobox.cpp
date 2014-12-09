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


#include <QKeyEvent>

#include "headercomboboxlistview.h"

#include "headercombobox.h"

HeaderComboBox::HeaderComboBox(QWidget* parent) : QComboBox(parent)
{
    setView(new HeaderComboBoxListView(this, this));
    AddParameter("");
}

void HeaderComboBox::SetCurrentElement(QString name)
{
    int index = parameter_names_.indexOf(name);
    if (index == -1)
        index = parameter_names_.indexOf("");
    setCurrentIndex(index);
}

void HeaderComboBox::SetParameterNames(const QStringList& names)
{
    clear();
    parameter_names_.clear();
    AddParameter("");
    for (const QString& name : names)
        AddParameter(name);
}

void HeaderComboBox::AddParameter(QString name)
{
    if (!parameter_names_.contains(name))
        parameter_names_.push_back(name);
    qSort(parameter_names_.begin(), parameter_names_.end());
    clear();
    addItems(parameter_names_);
}

void HeaderComboBox::RemoveParameter(QString name)
{
    if (name == "")
        return;
    parameter_names_.removeAll(name);
    clear();
    addItems(parameter_names_);
}

void HeaderComboBox::keyPressEvent(QKeyEvent* ev)
{
    if (ev->key() == Qt::Key_Escape)
        hide();
    QComboBox::keyPressEvent(ev);
}
